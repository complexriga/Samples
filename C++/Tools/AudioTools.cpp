#include <fstream>
#include <sstream>
#include <typeinfo>
#include <chrono>
#include "Tools.h"

#pragma comment(lib, "winmm.lib")

const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
const IID IID_IAudioClient = __uuidof(IAudioClient);
const IID IID_IAudioRenderClient = __uuidof(IAudioRenderClient);

#define REFTIMES_PER_SEC  10000000.0 // 10,000,000
#define REFTIMES_PER_MILLISEC  10000 // 10,000
#define SAFE_RELEASE(punk)  if ((punk) != NULL) { (punk)->Release(); (punk) = NULL; }

using namespace std;
using namespace std::chrono;

unsigned short swapBytesShort(unsigned short in) {
    return ((in << 8) | (in >> 8));
}

unsigned long swapBytesLong(unsigned long in) {
    unsigned short *p;
    p = (unsigned short*) &in;

    return ((((unsigned long) swapBytesShort(p[0])) << 16) | (unsigned long) swapBytesShort(p[1]));
}

namespace tools {

    namespace audio {

        struct WAV_HEADER {
            char RIFF0;
            char RIFF1;
            char RIFF2;
            char RIFF3;
            unsigned int size;
            char WAVE0;
            char WAVE1;
            char WAVE2;
            char WAVE3;
        }  rlWavHeader;

        struct WAV_CHUNK {
            char TAG0;
            char TAG1;
            char TAG2;
            char TAG3;
            unsigned int size;
        } rlWavChunk;

        struct WAVE_FORMAT {
            WORD        wFormatTag;         /* format type */
            WORD        nChannels;          /* number of channels (i.e. mono, stereo...) */
            DWORD       nSamplesPerSec;     /* sample rate */
            DWORD       nAvgBytesPerSec;    /* for buffer estimation */
            WORD        nBlockAlign;        /* block size of data */
            WORD        wBitsPerSample;     /* number of bits per sample of mono data */
        };

        const char TAG_RIFF[4] = {'R', 'I', 'F', 'F'};
        const char TAG_WAVE[4] = {'W', 'A', 'V', 'E'};
        const char TAG_FRMT[4] = {'f', 'm', 't', ' '};
        const char TAG_DATA[4] = {'d', 'a', 't', 'a'};

        const std::shared_ptr <const std::vector <unsigned char>> Audio::getData() const {
            return agData;
        }

        void SynchronousPlayer::playAll() {
            play();
            while(next());
            stop();
        }

        bool SynchronousPlayer::isPaused() const {
            return bgIsPause;
        }

        namespace wav {

            WAVAudio::WAVAudio() {
                rgFormat = make_shared <WAVEFORMATEX> ();
                agData = make_shared <vector <BYTE>> ();
            }

            WAVAudio::WAVAudio(const shared_ptr <const WAVEFORMATEX> rpFormat, const shared_ptr <const vector <BYTE>> apData) {
                if(!rpFormat)
                    throw string("Audio::Audio: Format cannot be null.");
                if(!apData)
                    throw string("Audio::Audio: Data cannot be null.");

                rgFormat = make_shared <WAVEFORMATEX> ();
                *rgFormat = *rpFormat;
                agData = make_shared <vector <BYTE>> (*apData);
            }

            const std::shared_ptr <const WAVEFORMATEX>  WAVAudio::getFormat() const {
                return rgFormat;
            }

            WAVSynchronousPlayer::WAVSynchronousPlayer(const shared_ptr <const WAVAudio> opAudio, bool bpExclusiveMode, bool bpCallbackEvent) {
                ogAudio = opAudio;
                bgIsPause = true;
                ilBufferInx = 0;
                bgExclusiveMode = bpExclusiveMode;
                bgEventCallback = bpCallbackEvent;

                CoInitialize(NULL);
                if(CoCreateInstance(CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, IID_IMMDeviceEnumerator, (void**) &pEnumerator) != S_OK)
                    throw string("SynchronousPlayer::SynchronousPlayer: Cannot get an Enumerator.");
                if(pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice) != S_OK)
                    throw string("SynchronousPlayer::SynchronousPlayer: Cannot get Default Device.");
                if(pDevice->Activate(IID_IAudioClient, CLSCTX_ALL, NULL, (void**) &pAudioClient) != S_OK)
                    throw string("SynchronousPlayer::SynchronousPlayer: Cannot get Client.");
                if(pAudioClient->GetMixFormat(&plDeviceWaveFormat) != S_OK)
                    throw string("SynchronousPlayer::SynchronousPlayer: Cannot get Client Supporte Wave Format.");

                if(bgExclusiveMode) {
                    if(pAudioClient->IsFormatSupported(AUDCLNT_SHAREMODE_EXCLUSIVE, ogAudio->getFormat().get(), NULL) != S_OK)
                        throw string("SynchronousPlayer::SynchronousPlayer: Data Wave Format is not supported by the Device.");
                    if(pAudioClient->GetDevicePeriod(NULL, &hnsPeriod) != S_OK)
                        throw string("SynchronousPlayer::SynchronousPlayer: Cannot get the Device's Period.");
                    if(bgEventCallback) {
                        HRESULT hr;
                        if(hr = pAudioClient->Initialize(AUDCLNT_SHAREMODE_EXCLUSIVE, AUDCLNT_STREAMFLAGS_EVENTCALLBACK | AUDCLNT_STREAMFLAGS_RATEADJUST | AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM, hnsPeriod, hnsPeriod, ogAudio->getFormat().get(), NULL) != S_OK) {
                            if(hr == AUDCLNT_E_BUFFER_SIZE_NOT_ALIGNED) {
                                if(pAudioClient->GetBufferSize(&nFramesInBuffer) != S_OK)
                                    throw string("SynchronousPlayer::SynchronousPlayer: Cannot get Buffer Size.");
                                hnsPeriod = (REFERENCE_TIME) ((REFTIMES_PER_SEC * nFramesInBuffer / ogAudio->getFormat()->nSamplesPerSec) + 0.5);
                                if(pAudioClient->Release() != S_OK)
                                    throw string("SynchronousPlayer::SynchronousPlayer: Cannot free Client.");
                                if(pDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (void**) &pAudioClient) != S_OK)
                                    throw string("SynchronousPlayer::SynchronousPlayer: Cannot reactivate Client.");
                                if((pAudioClient->Initialize(AUDCLNT_SHAREMODE_EXCLUSIVE, AUDCLNT_STREAMFLAGS_EVENTCALLBACK | AUDCLNT_STREAMFLAGS_RATEADJUST | AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM, hnsPeriod, hnsPeriod, ogAudio->getFormat().get(), NULL)) != S_OK)
                                    throw string("SynchronousPlayer::SynchronousPlayer: Cannot reinitialize Client.");
                            } else { // shouldn't happen.
                                if(pAudioClient->Release() != S_OK)
                                    throw string("SynchronousPlayer::SynchronousPlayer: Cannot free Client.");
                                if(pDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (void**) &pAudioClient) != S_OK)
                                    throw string("SynchronousPlayer::SynchronousPlayer: Cannot reactivate Client.");
                                if((pAudioClient->Initialize(AUDCLNT_SHAREMODE_EXCLUSIVE, AUDCLNT_STREAMFLAGS_RATEADJUST | AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM, hnsPeriod, 0, ogAudio->getFormat().get(), NULL)) != S_OK)
                                    throw string("SynchronousPlayer::SynchronousPlayer: Cannot reinitialize Client.");
                                bgEventCallback = false;
                            }
                        }
                    } else // not eventcallback.
                        if(pAudioClient->Initialize(AUDCLNT_SHAREMODE_EXCLUSIVE, AUDCLNT_STREAMFLAGS_RATEADJUST | AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM, hnsPeriod, 0, ogAudio->getFormat().get(), NULL) != S_OK)
                            throw string("SynchronousPlayer::SynchronousPlayer: Cannot initialize Client.");
                } else { // if shared mode
                    HRESULT hr;
                    if(hr = pAudioClient->IsFormatSupported(AUDCLNT_SHAREMODE_SHARED, ogAudio->getFormat().get(), &plDeviceWaveFormat) != S_OK) {
                        if(hr != S_FALSE) { // S_FALSE would mean &internalwaveformat was good, so using it.
                            throw string("SynchronousPlayer::SynchronousPlayer: The Data Wave Format is not supported by the Device.");
                        }
                    }
                    if(pAudioClient->GetDevicePeriod(&hnsPeriod, NULL) != S_OK)
                        throw string("SynchronousPlayer::SynchronousPlayer: Cannot get Device's Period.");
                    if(bgEventCallback) {
                        if(pAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_EVENTCALLBACK | AUDCLNT_STREAMFLAGS_RATEADJUST | AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM, 0, 0, ogAudio->getFormat().get(), NULL) != S_OK)
                            throw string("SynchronousPlayer::SynchronousPlayer: Cannot initialize Client.");
                    } else {
                        if(pAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_RATEADJUST | AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM, hnsPeriod, 0, ogAudio->getFormat().get(), NULL) != S_OK)
                            throw string("SynchronousPlayer::SynchronousPlayer: Cannot initialize Client.");
                    }
                }

                if(pAudioClient->GetService(IID_IAudioRenderClient, (void**) &pAudioRenderClient) != S_OK)
                    throw string("SynchronousPlayer::SynchronousPlayer: Cannot get the Service.");
            }

            const shared_ptr <const WAVAudio> WAVSynchronousPlayer::getAudio() const {
                return ogAudio;
            }

            void WAVSynchronousPlayer::play() {
                if(pAudioClient->GetBufferSize(&nFramesInBuffer) != S_OK)
                    throw string("SynchronousPlayer::play: Cannot get the Buffer Size.");
                nFramesInFile = (DWORD) ogAudio->getData()->size() / ogAudio->getFormat()->nBlockAlign;
                if(bgEventCallback) {
                    hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
                    if(pAudioClient->SetEventHandle(hEvent) != S_OK)
                        throw string("SynchronousPlayer::play: Cannot set Event Handle.");
                }
                if(pAudioRenderClient->GetBuffer(nFramesInBuffer, &pData) != S_OK)
                    throw string("SynchronousPlayer::play: Cannot get the Buffer.");
                memset(pData, 0, nFramesInBuffer);
                if(pAudioRenderClient->ReleaseBuffer(nFramesInBuffer, 0) != S_OK)
                    throw string("SynchronousPlayer::play: Cannot free the Buffer.");
                if(bgExclusiveMode)
                    if((hTask = AvSetMmThreadCharacteristics(TEXT("Pro Audio"), &taskIndex)) == NULL)
                        throw string("SynchronousPlayer::play: Cannot set AvSetMmThreadCharacteristics.");
                if(pAudioClient->Start() != S_OK)
                    throw string("SynchronousPlayer::play: Cannot start Audio.");
                nFramesPlayed = 0;
                ilBufferInx = 0;
                bgIsPause = false;
            }

            bool WAVSynchronousPlayer::next() {
                if(bgIsPause)
                    return true;

                UINT32 nFramesPlaying = 0, nFramesAvailable = 0, nBytesAvailable = 0;
                if(bgEventCallback) {
                    if(bgExclusiveMode) {
                        if(nFramesPlayed >= nFramesInFile)
                            return false;
                        if(WAIT_OBJECT_0 != WaitForSingleObject(hEvent, 2000))
                            return false;
                        if(pAudioRenderClient->GetBuffer(nFramesInBuffer, &pData) != S_OK)
                            throw string("SynchronousPlayer::next: Cannot get the Buffer.");
                        nBytesAvailable = nFramesInBuffer * ogAudio->getFormat()->nBlockAlign;
                        memcpy(pData, &ogAudio->getData()->at(ilBufferInx), nBytesAvailable);
                        ilBufferInx += nBytesAvailable;
                        if(pAudioRenderClient->ReleaseBuffer(nFramesInBuffer, 0) != S_OK)
                            throw string("SynchronousPlayer::next: Cannot free the Buffer.");
                        nFramesPlayed += nFramesInBuffer;
                    } else { // shared mode.
                        if(nFramesPlayed >= nFramesInFile)
                            return false;
                        if(WAIT_OBJECT_0 != WaitForSingleObject(hEvent, 2000))
                            return false;
                        if(pAudioClient->GetCurrentPadding(&nFramesPlaying) != S_OK)
                            throw string("SynchronousPlayer::next: Cannot get the Current Padding.");
                        nFramesAvailable = nFramesInBuffer - nFramesPlaying;
                        if(nFramesAvailable > (nFramesInFile - nFramesPlayed))
                            nFramesAvailable = nFramesInFile - nFramesPlayed;
                        if(pAudioRenderClient->GetBuffer(nFramesAvailable, &pData) != S_OK)
                            throw string("SynchronousPlayer::next: Cannot get the Buffer.");
                        nBytesAvailable = nFramesAvailable * ogAudio->getFormat()->nBlockAlign;
                        memcpy(pData, &ogAudio->getData()->at(ilBufferInx), nBytesAvailable);
                        ilBufferInx += nBytesAvailable;
                        if(pAudioRenderClient->ReleaseBuffer(nFramesAvailable, 0) != S_OK)
                            throw string("SynchronousPlayer::next: Cannot free the Buffer.");
                        nFramesPlayed += nFramesAvailable;
                    }
                } else { // not using event callback.
                    if(nFramesPlayed >= nFramesInFile) {
                        Sleep((DWORD) (hnsPeriod / REFTIMES_PER_MILLISEC));
                        return false;
                    }
                    if(!bgExclusiveMode)
                        Sleep((DWORD) (hnsPeriod / REFTIMES_PER_MILLISEC / 2));
                    if(pAudioClient->GetCurrentPadding(&nFramesPlaying) != S_OK)
                        throw string("SynchronousPlayer::next: Cannot get the Current Padding.");
                    nFramesAvailable = nFramesInBuffer - nFramesPlaying;
                    if(nFramesAvailable > (nFramesInFile - nFramesPlayed))
                        nFramesAvailable = nFramesInFile - nFramesPlayed;
                    if(pAudioRenderClient->GetBuffer(nFramesAvailable, &pData) != S_OK)
                        throw string("SynchronousPlayer::next: Cannot get the Buffer.");
                    nBytesAvailable = nFramesAvailable * ogAudio->getFormat()->nBlockAlign;
                    memcpy(pData, &ogAudio->getData()->at(ilBufferInx), nBytesAvailable);
                    ilBufferInx += nBytesAvailable;
                    if(pAudioRenderClient->ReleaseBuffer(nFramesAvailable, 0) != S_OK)
                        throw string("SynchronousPlayer::next: Cannot free the Buffer.");
                    nFramesPlayed += nFramesAvailable;
                }
                return true;
            }

            void WAVSynchronousPlayer::stop() {
                pAudioClient->Stop();
                pAudioClient->Reset();
                bgIsPause = true;
            }

            void WAVSynchronousPlayer::pause() {
                bgIsPause = true;
            }

            void WAVSynchronousPlayer::resume() {
                bgIsPause = false;
            }

            WAVSynchronousPlayer::~WAVSynchronousPlayer() {
                if(hEvent)
                    CloseHandle(hEvent);
                if(hTask)
                    AvRevertMmThreadCharacteristics(hTask);
                if(pData)
                    VirtualFree(pData, 0, MEM_RELEASE);
                SAFE_RELEASE(pEnumerator)
                SAFE_RELEASE(pDevice)
                SAFE_RELEASE(pAudioClient)
                SAFE_RELEASE(pAudioRenderClient)
            }

        }

        namespace midi {

            MIDIAudio::MIDIAudio() {
                rgHeader = make_shared <StreamHeader> ();
                agTracks = make_shared <vector <size_t>> ();
                agData = make_shared <vector <unsigned char>> ();
            }

            MIDIAudio::MIDIAudio(const shared_ptr <const StreamHeader> rpHeader, const shared_ptr <const vector <size_t>> apTracks, const shared_ptr <const vector <unsigned char>> apData) {
                if(!rpHeader)
                    throw string("MIDIAudio::MIDIAudio: Header cannot be null.");
                if(!apTracks)
                    throw string("MIDIAudio::MIDIAudio: Track Information cannot be null.");
                if(!apData)
                    throw string("MIDIAudio::MIDIAudio: Data cannot be null.");
                if(rpHeader->tracks != apTracks->size())
                    throw string("MIDIAudio::MIDIAudio: The Size of the Tracks Vector must be equal as the Value of tracks Field in the Header.");
                if(apData->size() == 0 || apData->size() <= apTracks->size() * sizeof(unsigned int))
                    throw string("MIDIAudio::MIDIAudio: Data cannot be 0 or less than the Size of one Message per Track.");

                rgHeader = make_shared <StreamHeader> (*rpHeader);
                agTracks = make_shared <vector <size_t>> (*apTracks);
                agData = make_shared <vector <unsigned char>> (*apData);
            }

            const shared_ptr <const MIDIAudio::StreamHeader> MIDIAudio::getFormat() const {
                return rgHeader;
            }

            const shared_ptr <const vector <size_t>> MIDIAudio::getTracks() const {
                return agTracks;
            }

            MIDISynchronousPlayer::Event MIDISynchronousPlayer::getNextEvent(TrackControl& rpTrack) {
                unsigned char c;
                unsigned long ilDuration = 0;
                size_t ilNewDataInx = rpTrack.buf_index;
                Event rlEvent;

                do { // Obtains the time that the event lasts.
                    c = ogAudio->getData()->at(ilNewDataInx++);
                    ilDuration = (ilDuration << 7) + (c & 0x7f);
                } while(c & 0x80);

                rlEvent.absolute_time = rpTrack.absolute_time + ilDuration;
                rlEvent.event = ogAudio->getData()->at(ilNewDataInx);
                rlEvent.buf_index = ilNewDataInx;

                return rlEvent;
            }

            unsigned int MIDISynchronousPlayer::getBuffer(vector <TrackControl>& apTracks, vector <unsigned int>& apBuffer) {
                static unsigned int ilBufferTime = 0;
                unsigned int ilStreamLength = 0;
                MIDIEVENT rlMIDIEvent;

                while(true) {
                    unsigned int ilCurrentTime = (unsigned int) -1;
                    unsigned int ilTrackInx = -1;
                    unsigned char clData;
                    struct Event rlEvent;

                    if(((ilStreamLength + 3) * sizeof(unsigned int)) >= igBufferSize)
                        break;

                    // Finds out the next event to process on the temporal line.
                    for(unsigned int i = 0; i < apTracks.size(); i++) {
                        rlEvent = getNextEvent(apTracks[i]);
                        // Checks that the event is not a meta event that marks the end of the track, and that It is the event closest in time to the last processed event.
                        if(!(rlEvent.event == 0xff && ogAudio->getData()->at(rlEvent.buf_index + 1) == 0x2f) && rlEvent.absolute_time < ilCurrentTime) {
                            ilCurrentTime = rlEvent.absolute_time;
                            ilTrackInx = i;
                        }
                    }

                    if(ilTrackInx == -1) // If idx == -1 then all tracks have been read until the end marker of the track.
                        break;

                    rlMIDIEvent.dwStreamID = 0; // Always sends the commands through the 0 flow.

                    rlEvent = getNextEvent(apTracks[ilTrackInx]);

                    apTracks[ilTrackInx].absolute_time = rlEvent.absolute_time;
                    rlMIDIEvent.dwDeltaTime = apTracks[ilTrackInx].absolute_time - ilBufferTime;
                    ilBufferTime = apTracks[ilTrackInx].absolute_time;

                    if(!(rlEvent.event & 0x80)) { // Processes the parameters of the command that is being processed in the flow.
                        unsigned char last = apTracks[ilTrackInx].last_event;
                        clData = ogAudio->getData()->at(rlEvent.buf_index++); // Gets the first byte of the data.
                        rlMIDIEvent.dwEvent = ((unsigned long) MEVT_SHORTMSG << 24) | ((unsigned long) last) | ((unsigned long) clData << 8);
                        if(!((last & 0xf0) == 0xc0 || (last & 0xf0) == 0xd0)) {
                            clData = ogAudio->getData()->at(rlEvent.buf_index++); // Gets the second byte of the data.
                            rlMIDIEvent.dwEvent |= ((unsigned long) clData << 16);
                        }

                        *(MIDIEVENT*) &apBuffer[ilStreamLength] = rlMIDIEvent;

                        ilStreamLength += 3;
                        apTracks[ilTrackInx].buf_index = rlEvent.buf_index;
                    } else if(rlEvent.event == 0xff) { // Processes the meta events.
                        rlEvent.buf_index++; // Jumps to the first byte.
                        switch(ogAudio->getData()->at(rlEvent.buf_index++)) { // Gets the byte of the code of the meta event.
                            case 0x51: // Only time events matter.
                                {
                                    unsigned int len = ogAudio->getData()->at(rlEvent.buf_index++); // Gets the byte of the size, which always should be 3.
                                    unsigned char a = ogAudio->getData()->at(rlEvent.buf_index++);
                                    unsigned char b = ogAudio->getData()->at(rlEvent.buf_index++);
                                    unsigned char c = ogAudio->getData()->at(rlEvent.buf_index++);

                                    rlMIDIEvent.dwEvent = ((unsigned long) MEVT_TEMPO << 24) | ((unsigned long) a << 16) | ((unsigned long) b << 8) | ((unsigned long) c << 0);

                                    *(MIDIEVENT*) &apBuffer[ilStreamLength] = rlMIDIEvent;

                                    ilStreamLength += 3;
                                }
                                break;
                            default: // skip all other meta events.
                                rlEvent.buf_index += ogAudio->getData()->at(rlEvent.buf_index++); // Gets the byte of the size of the meta event and adds It to the current index of the event.
                        }
                        apTracks[ilTrackInx].buf_index = rlEvent.buf_index;
                    } else if((rlEvent.event & 0xf0) != 0xf0) { // Processes the normal commands, which are the first in the series.
                        apTracks[ilTrackInx].last_event = rlEvent.event;
                        rlEvent.buf_index++; // Jumps the byte of the event.
                        clData = ogAudio->getData()->at(rlEvent.buf_index++); // Gets the first byte of data.
                        rlMIDIEvent.dwEvent = ((unsigned long) MEVT_SHORTMSG << 24) | ((unsigned long) rlEvent.event << 0) | ((unsigned long) clData << 8);
                        if(!((rlEvent.event & 0xf0) == 0xc0 || (rlEvent.event & 0xf0) == 0xd0)) {
                            clData = ogAudio->getData()->at(rlEvent.buf_index++); // Gets the second byte of data.
                            rlMIDIEvent.dwEvent |= ((unsigned long) clData << 16);
                        }

                        *(MIDIEVENT*) &apBuffer[ilStreamLength] = rlMIDIEvent;

                        ilStreamLength += 3;
                        apTracks[ilTrackInx].buf_index = rlEvent.buf_index;
                    }

                }

                return ilStreamLength * sizeof(unsigned int);
            }

            MIDISynchronousPlayer::MIDISynchronousPlayer(const std::shared_ptr <const MIDIAudio> opAudio, unsigned int ipBufferSize) {
                if(!opAudio)
                    throw string("MIDISynchronousPlayer::MIDISynchronousPlayer: Audio cannot be null.");
                ogAudio = opAudio;
                bgIsPause = true;

                if(ipBufferSize < MIN_BUFFER_SIZE)
                    igBufferSize = MIN_BUFFER_SIZE; // Only allows the processing of one event at the time.
                else if(ipBufferSize > MAX_BUFFER_SIZE)
                    igBufferSize = MAX_BUFFER_SIZE; // The maximum possible size for the buffer is 512 * 12.
                else
                    igBufferSize = ipBufferSize;

                agTracksInx = vector <TrackControl> (ogAudio->getTracks()->size());
                ogStream = vector <unsigned int> (igBufferSize, 0);

                if((hgLock = CreateEvent(0, FALSE, FALSE, 0)) == NULL)
                    throw string("MIDISynchronousPlayer::MIDISynchronousPlayer: Cannot create Lock.");

                hgDevice = 0;
                if(midiStreamOpen(&hgMIDIStream, &hgDevice, 1, (DWORD_PTR) hgLock, 0, CALLBACK_EVENT) != MMSYSERR_NOERROR)
                    throw string("MIDISynchronousPlayer::MIDISynchronousPlayer: Cannot open the Stream.");

                hgMIDIProperty.cbStruct = sizeof(MIDIPROPTIMEDIV);
                hgMIDIProperty.dwTimeDiv = ogAudio->getFormat()->ticks;
                if(midiStreamProperty(hgMIDIStream, (LPBYTE) &hgMIDIProperty, MIDIPROP_SET | MIDIPROP_TIMEDIV) != MMSYSERR_NOERROR)
                    throw string("MIDISynchronousPlayer::MIDISynchronousPlayer: Cannot set the Stream Properties.");

                hgMIDIHeader.lpData = (char*) &ogStream[0];
                hgMIDIHeader.dwBufferLength = hgMIDIHeader.dwBytesRecorded = igBufferSize;
                hgMIDIHeader.dwFlags = 0;

                if(midiOutPrepareHeader((HMIDIOUT) hgMIDIStream, &hgMIDIHeader, sizeof(MIDIHDR)) != MMSYSERR_NOERROR)
                    throw string("MIDISynchronousPlayer::MIDISynchronousPlayer: Cannot prepare the Stream Header.");

            }

            MIDISynchronousPlayer::~MIDISynchronousPlayer() {
                if(hgMIDIStream != NULL) {
                    midiOutReset((HMIDIOUT) hgMIDIStream);
                    midiOutUnprepareHeader((HMIDIOUT) hgMIDIStream, &hgMIDIHeader, sizeof(MIDIHDR));
                    midiStreamClose(hgMIDIStream);
                }
                if(hgLock != NULL)
                    CloseHandle(hgLock);
            }

            const shared_ptr <const MIDIAudio> MIDISynchronousPlayer::getAudio() const {
                return ogAudio;
            }

            void MIDISynchronousPlayer::play() {
                for(size_t i = 0; i < agTracksInx.size(); i++) {
                    agTracksInx[i].absolute_time = 0;
                    agTracksInx[i].last_event = 0;
                    agTracksInx[i].buf_index = ogAudio->getTracks()->at(i);
                }

                if(midiStreamRestart(hgMIDIStream) != MMSYSERR_NOERROR)
                    throw string("MIDISynchronousPlayer::play: Cannot start Stream.");
                bgIsPause = false;
            }

            bool MIDISynchronousPlayer::next() {
                if(bgIsPause)
                    return true;

                unsigned int ilStreamLength = 0;
                if((ilStreamLength = getBuffer(agTracksInx, ogStream)) == 0)
                    return false;

                hgMIDIHeader.dwBytesRecorded = ilStreamLength;
                //hlMIDIHeader.dwFlags = 0;

                MMRESULT ilResult = midiStreamOut(hgMIDIStream, &hgMIDIHeader, sizeof(MIDIHDR));
                if(ilResult != MMSYSERR_NOERROR && ilResult != MIDIERR_STILLPLAYING) {
                    if(ilResult == MMSYSERR_NOMEM)
                        throw string("MIDISynchronousPlayer::next: MMSYSERR_NOMEM.");
                    else if(ilResult == MIDIERR_STILLPLAYING)
                        throw string("MIDISynchronousPlayer::next: MIDIERR_STILLPLAYING.");
                    else if(ilResult == MIDIERR_UNPREPARED)
                        throw string("MIDISynchronousPlayer::next: MIDIERR_UNPREPARED.");
                    else if(ilResult == MMSYSERR_INVALHANDLE)
                        throw string("MIDISynchronousPlayer::next: MMSYSERR_INVALHANDLE.");
                    else if(ilResult == MMSYSERR_INVALPARAM)
                        throw string("MIDISynchronousPlayer::next: MMSYSERR_INVALPARAM.");
                    else {
                        char tmp[10];
                        _itoa_s(ilResult, tmp, 10);
                        throw string("MIDISynchronousPlayer::next: Other Error : ") + tmp;
                    }
                }

                do {
                    WaitForSingleObject(hgLock, INFINITE);
                } while(!(hgMIDIHeader.dwFlags & MHDR_DONE));

                return true;
            }

            void MIDISynchronousPlayer::stop() {
                midiOutReset((HMIDIOUT) hgMIDIStream);
                bgIsPause = true;
            }

            void MIDISynchronousPlayer::pause() {
                if(midiStreamPause(hgMIDIStream) != MMSYSERR_NOERROR)
                    throw string("MIDISynchronousPlayer::pause: Cannot pause Stream.");
                bgIsPause = true;
            }

            void MIDISynchronousPlayer::resume() {
                if(midiStreamRestart(hgMIDIStream) != MMSYSERR_NOERROR)
                    throw string("MIDISynchronousPlayer::resume: Cannot restart Stream.");
                bgIsPause = false;
            }

        }

        void AsynchronousPlayer::executeLoop() {
            if(!ogPlayMutex.try_lock())
                return;

            unique_lock <mutex> olPauseLock(ogPauseMutex);

            try {
                bool blInd = true;
                while(blInd) {
                    ogInternalPlayer->play();
                    while(ogInternalPlayer->next())
                        if(igCommand == STOP) {
                            bgLoop = false;
                            break;
                        } else if(igCommand == PAUSE) {
                            ogInternalPlayer->pause();
                            ogPauseCtl.wait(olPauseLock);
                            ogInternalPlayer->resume();
                        }
                    ogInternalPlayer->stop();
                    if(!bgLoop || ++igActualLoop == igLoopLimit)
                        blInd = false;
                    else
                        this_thread::sleep_for(milliseconds(igLoopDelay));
                }
            } catch(...) {}

            ogPlayMutex.unlock();
        }

        AsynchronousPlayer::AsynchronousPlayer(const shared_ptr <const Audio> opAudio) {
            ogInternalPlayer = AudioTools::buildSynchronousPlayer(opAudio);
            bgLoop = false;
            igCommand = RESUME;
            igLoopLimit = 0;
            igActualLoop = 0;
            igLoopDelay = 0;
        }

        AsynchronousPlayer::~AsynchronousPlayer() {
            stop();
            wait();
        }

        bool AsynchronousPlayer::play(bool bpLoop, unsigned int ipLoopLimit, unsigned long ipLoopDelay) {
            if(!ogPlayMutex.try_lock())
                return false;
            ogPlayMutex.unlock();

            bgLoop = bpLoop;
            igLoopLimit = ipLoopLimit;
            igLoopDelay = ipLoopDelay;
            igActualLoop = 0;
            igCommand = RESUME;

            if(ogThread.joinable())
                ogThread.join();
            ogThread = thread(&AsynchronousPlayer::executeLoop, this);

            return true;
        }

        void AsynchronousPlayer::pause() {
            igCommand = PAUSE;
        }

        void AsynchronousPlayer::resume() {
            ogPauseCtl.notify_all();
            igCommand = RESUME;
        }

        void AsynchronousPlayer::stop() {
            igCommand = STOP;
        }

        bool AsynchronousPlayer::isPlaying() {
            if(ogPlayMutex.try_lock()) {
                ogPlayMutex.unlock();
                return false;
            }
            return true;
        }

        bool AsynchronousPlayer::isLooping() const {
            return bgLoop;
        }

        unsigned int AsynchronousPlayer::getLoopLimit() const {
            return igLoopLimit;
        }

        unsigned int AsynchronousPlayer::getActualLoop() const {
            return igActualLoop;
        }

        unsigned long AsynchronousPlayer::getLoopDelay() const {
            return igLoopDelay;
        }

        void AsynchronousPlayer::wait() {
            if(ogThread.joinable())
                ogThread.join();
        }

        shared_ptr <wav::WAVAudio> AudioTools::readWAVFromStream(shared_ptr <istream> opStream) {
            if(!opStream)
                throw string("AudioTools::readWAVFromStream: Stream cannot be null.");

            opStream->seekg(0, opStream->end);
            if(!opStream->good()) {
                if(opStream->bad()) throw string("STREAM BAD.");
                if(opStream->fail()) throw string("STREAM FAIL.");
                if(opStream->eof()) throw string("STREAM EOF.");
                throw string("Audio::Audio: Unexpected Error reading File.");
            }

            DWORD ilSize;
            shared_ptr <vector <BYTE>> alData;
            WAVEFORMATEXTENSIBLE rlFormat;
            shared_ptr <WAVEFORMATEX> rlWaveFormat;
            WAVE_FORMAT rlAltFormat;
            bool blFmtCtl = false;
            bool blDtaCtl = false;

            streamoff fileSize = opStream->tellg();
            opStream->seekg(0, opStream->beg);

            opStream->read((char *) &rlWavHeader, sizeof(WAV_HEADER));
            if((DWORD) rlWavHeader.RIFF0 != (DWORD) TAG_RIFF[0] || (DWORD) rlWavHeader.WAVE0 != (DWORD) TAG_WAVE[0])
                throw string("AudioTools::readWAVFromStream: Format Error in Header.");

            while(!opStream->eof()) {
                opStream->read((char *) &rlWavChunk, sizeof(WAV_CHUNK));
                if((DWORD) rlWavChunk.TAG0 == (DWORD) TAG_FRMT[0]) {
                    if(rlWavChunk.size != sizeof(WAVEFORMATEXTENSIBLE)) {
                        opStream->read((char *) &rlAltFormat, sizeof(WAVE_FORMAT));
                        if(rlAltFormat.wFormatTag == WAVE_FORMAT_EXTENSIBLE)
                            throw string("AudioTools::readWAVFromStream: Format Error in Details");
                        rlFormat.Format.nAvgBytesPerSec = rlAltFormat.nAvgBytesPerSec;
                        rlFormat.Format.nBlockAlign = rlAltFormat.nBlockAlign;
                        rlFormat.Format.nChannels = rlAltFormat.nChannels;
                        rlFormat.Format.nSamplesPerSec = rlAltFormat.nSamplesPerSec;
                        rlFormat.Format.wBitsPerSample = rlAltFormat.wBitsPerSample;
                        rlFormat.Format.wFormatTag = rlAltFormat.wFormatTag;
                        rlFormat.Format.cbSize = 0;
                    } else {
                        opStream->read((char *) &rlFormat, sizeof(WAVEFORMATEXTENSIBLE));
                        if(rlFormat.Format.wFormatTag == WAVE_FORMAT_EXTENSIBLE)
                            throw string("AudioTools::readWAVFromStream: Format Error in Data.");
                        rlFormat.Format.cbSize = sizeof(WAVEFORMATEXTENSIBLE);
                        rlFormat.dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT;
                        rlFormat.SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
                    }
                    rlWaveFormat = make_shared <WAVEFORMATEX> (rlFormat.Format);
                    blFmtCtl = true;
                    continue;
                } else if((DWORD) rlWavChunk.TAG0 == (DWORD) TAG_DATA[0]) {
                    ilSize = rlWavChunk.size;
                    alData = make_shared <vector <BYTE>> (ilSize);
                    opStream->read((char *) &alData->at(0), ilSize);
                    blDtaCtl = true;
                    break;
                }
                opStream->ignore(rlWavChunk.size);
            }
            if(!blFmtCtl || !blDtaCtl)
                throw string("AudioTools::readWAVFromStream: Format Error in Data End");

            return make_shared <wav::WAVAudio> (rlWaveFormat, alData);
        }

        shared_ptr <wav::WAVAudio> AudioTools::readWAVFromFile(const string& spPath, bool bpLoadIntoMemory) {
            if(spPath.find(".wav\0") == string::npos)
                throw string("AudioTools::readWAVFromFile: File is not a Wav: ") + spPath.data();

            shared_ptr <wav::WAVAudio> olAudio;
            if(bpLoadIntoMemory) {
                ifstream olFile(spPath.data(), ios::binary | ios::ate);
                if(!olFile.is_open())
                    throw string("AudioTools::readWAVFromFile: File cannot be opened: ") + spPath.data();

                streamsize ilSize = olFile.tellg();
                olFile.seekg(0);

                vector <char> alBuffer(ilSize);
                olFile.read(alBuffer.data(), ilSize);
                if(!olFile.good()) {
                    if(olFile.bad()) throw string("AudioTools::readWAVFromFile: STREAM BAD.");
                    if(olFile.fail()) throw string("AudioTools::readWAVFromFile: STREAM FAIL.");
                    if(olFile.eof()) throw string("AudioTools::readWAVFromFile: STREAM EOF.");
                    throw string("AudioTools::readWAVFromFile: Unexpected Error.");
                }
                olFile.close();

                olAudio = readWAVFromStream(make_shared <istringstream> (string(string(alBuffer.data(), alBuffer.size()))));
            } else {
                shared_ptr <ifstream> olFile = make_shared <ifstream> (spPath.data(), ios::binary);
                if(!olFile->is_open())
                    throw string("AudioTools::readWAVFromFile: File cannot be opened: ") + spPath.data();

                olAudio = readWAVFromStream(olFile);

                olFile->close();
            }

            return olAudio;
        }

        shared_ptr <midi::MIDIAudio> AudioTools::readMIDIFromStream(shared_ptr <istream> opStream) {
            if(!opStream)
                throw string("AudioTools::readMIDIFromStream: Stream cannot be null.");

            shared_ptr <midi::MIDIAudio::StreamHeader> rlHeader = make_shared <tools::audio::midi::MIDIAudio::StreamHeader> ();

            opStream->read((char*) &rlHeader->id, sizeof(unsigned int));
            opStream->read((char*) &rlHeader->size, sizeof(unsigned int));
            opStream->read((char*) &rlHeader->format, sizeof(unsigned short));
            opStream->read((char*) &rlHeader->tracks, sizeof(unsigned short));
            opStream->read((char*) &rlHeader->ticks, sizeof(unsigned short));

            rlHeader->size = swapBytesLong(rlHeader->size);
            rlHeader->format = swapBytesShort(rlHeader->format);
            rlHeader->tracks = swapBytesShort(rlHeader->tracks);
            rlHeader->ticks = swapBytesShort(rlHeader->ticks);

            shared_ptr <vector <size_t>> alTrackInx = make_shared <vector <size_t>> (rlHeader->tracks, 0);
            shared_ptr <vector <unsigned char>> alData = make_shared <vector <unsigned char>> ();
            unsigned int ilLength = 0;
            for(size_t i = 0; i < alTrackInx->size(); i++) {
                alTrackInx->at(i) = i != 0 ? alTrackInx->at(i - 1) + ilLength: 0;
                opStream->ignore(sizeof(unsigned int));
                opStream->read((char*) &ilLength, sizeof(unsigned int));
                ilLength = swapBytesLong(ilLength);
                alData->insert(alData->end(), ilLength, 0);
                opStream->read((char*) &alData->at(alTrackInx->at(i)), ilLength);
            }

            return make_shared <midi::MIDIAudio> (rlHeader, alTrackInx, alData);
        }

        shared_ptr <midi::MIDIAudio> AudioTools::readMIDIFromFile(const string& spPath, bool bpLoadIntoMemory) {
            if(spPath.find(".mid\0") == string::npos)
                throw string("AudioTools::readMIDIFromFile: File is not a MIDI: ") + spPath.data();

            shared_ptr <midi::MIDIAudio> olAudio;
            if(bpLoadIntoMemory) {
                ifstream olFile(spPath.data(), ios::binary | ios::ate);
                if(!olFile.is_open())
                    throw string("AudioTools::readMIDIFromFile: File cannot be opened: ") + spPath.data();

                streamsize ilSize = olFile.tellg();
                olFile.seekg(0);

                vector <char> alBuffer(ilSize);
                olFile.read(alBuffer.data(), ilSize);
                if(!olFile.good()) {
                    if(olFile.bad()) throw string("AudioTools::readMIDIFromFile: STREAM BAD.");
                    if(olFile.fail()) throw string("AudioTools::readMIDIFromFile: STREAM FAIL.");
                    if(olFile.eof()) throw string("AudioTools::readMIDIFromFile: STREAM EOF.");
                    throw string("AudioTools::readMIDIFromFile: Unexpected Error.");
                }
                olFile.close();

                olAudio = readMIDIFromStream(make_shared <istringstream>(string(string(alBuffer.data(), alBuffer.size()))));
            } else {
                shared_ptr <ifstream> olFile = make_shared <ifstream> (spPath.data(), ios::binary);
                if(!olFile->is_open())
                    throw string("AudioTools::readMIDIFromFile: File cannot be opened: ") + spPath.data();

                olAudio = readMIDIFromStream(olFile);

                olFile->close();
            }

            return olAudio;
        }

        shared_ptr <SynchronousPlayer> AudioTools::buildSynchronousPlayer(const shared_ptr <const Audio> opAudio) {
            if(!opAudio)
                throw string("AudioTools::buildSynchronousPlayer: Audio cannot be null.");

            shared_ptr <SynchronousPlayer> opPlayer;
            if(typeid(*opAudio) == typeid(wav::WAVAudio))
                opPlayer = make_shared <wav::WAVSynchronousPlayer> (reinterpret_cast <const shared_ptr <const wav::WAVAudio>&> (opAudio));
            else if(typeid(*opAudio) == typeid(midi::MIDIAudio))
                opPlayer = make_shared <midi::MIDISynchronousPlayer> (reinterpret_cast <const shared_ptr <const midi::MIDIAudio>&> (opAudio));
            else
                throw string("AudioTools::buildSynchronousPlayer: Audio Type not recognized.");

            return opPlayer;
        }

    }

}