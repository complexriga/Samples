#include <windows.h>
#include <vector>
#include <memory>
#include <mmDeviceapi.h>
#include <audiopolicy.h>
#include <avrt.h> // Add "avrt.lib" in Project Properties > Linker > Input > Additional Dependencies.
#include <thread>
#include <mutex>
#include <condition_variable>

///////////////////////////////////////////////////////////
// DO NOT MOVE CONST CLAUSES, they are neccesary to compile the assigning constructors correctly.
///////////////////////////////////////////////////////////

#ifndef IMAGE_TOOLS
///////////////////////////////////////////////////////////

#define IMAGE_TOOLS

namespace tools {
    namespace compression {
        namespace huffman {
            struct BitStream {
                std::vector <unsigned char> stream;
                unsigned long inx = 0;
                unsigned char mask = 0;
                unsigned char buf = 0;
            };

            struct Node {
                unsigned short symbol = 0;
                size_t one = 0;
                size_t zero = 0;
            };

            struct CodeLengthEntry {
                unsigned short symbol;
                unsigned short len = 0;
            };

            class Huffman {
                private:
                    Huffman();

                public:
                    static std::vector <Node> createCodes(std::vector <CodeLengthEntry>& aspTable);
                    static void extractLengthTable(std::vector <Node>& aHCLENTree, BitStream& spStream, std::vector <CodeLengthEntry>& slLengthTable);
            };
        }

        class ZLIB {
            protected:
                ZLIB();

            public:
                static std::shared_ptr <std::vector <unsigned char>> inflate(std::vector <unsigned char>& apData);
        };
    }

    namespace image {
        const unsigned char DEFAULT_ALPHA_VALUE = 255;

        //////////////////////////////////////////////////////////////////////////////////////
        // MISCELANEOUS OBJECTS

        struct BMP_HEADER {
            BITMAPFILEHEADER cab;
            BITMAPINFOHEADER inf;
        };

        int reverse4BytesOrder(int data);
        short reverse2BytesOrder(short data);

        //////////////////////////////////////////////////////////////////////////////////////
        // PIXEL FORMATS

        struct HSV;

        struct RGBA {
            unsigned char b; // Blue
            unsigned char g; // Green
            unsigned char r; // Red
            unsigned char a; // Alpha

            RGBA();
            RGBA(unsigned char cpBlue, unsigned char cpGreen, unsigned char cpRed, unsigned char cpAlpha);
            RGBA(const RGBA& spPixel);
            RGBA(const HSV& spPixel);
        };

        bool operator==(const RGBA& lhs, const RGBA& rhs);
        bool operator!=(const RGBA& lhs, const RGBA& rhs);

        struct HSV {
            float h; // Hue
            float s; // Saturation
            float v; // Value

            HSV();
            HSV(float fpHue, float fpSaturation, float fpValue);
            HSV(const HSV& spPixel);
            HSV(const RGBA& spPixel);

            static HSV fromRGB(RGBA spPixel);
            static RGBA toRGB(HSV spPixel);
        };

        //////////////////////////////////////////////////////////////////////////////////////
        // IMAGE FORMATS

        class ImageTools;

        class BaseImage {
            protected:
                bool bgIsPlaceholder;
                unsigned long lgHeight;
                unsigned long lgWidth;
                unsigned long lgRowSize; // Total size on bytes of each line.
                std::vector <unsigned char> atgData;

            public:
                virtual ~BaseImage() {};

                unsigned long getHeight() const;
                unsigned long getWidth() const;
                bool isPlaceholder() const;

                virtual RGBA getPixel(unsigned long lpX, unsigned long lpY) const = 0;
                virtual void setPixel(unsigned long lpX, unsigned long lpY, const RGBA& tpPixel) = 0;
        };

        template <typename TPIXEL>
        class ExtendedImage : public BaseImage {
            friend class ImageTools;

            protected:
                void init(unsigned long lpWidth, unsigned long lpHeight, bool blIsPlaceholder);

            public:
                ExtendedImage();
                ExtendedImage(unsigned long lpWidth, unsigned long lpHeight, const std::vector <unsigned char>& abpData);
                ExtendedImage(unsigned long lpWidth, unsigned long lpHeight, bool blIsPlaceholder = false);
                ExtendedImage(const ExtendedImage <TPIXEL>& opImage);
                ExtendedImage(const BaseImage& opImage);
                ExtendedImage(const std::shared_ptr <const ExtendedImage <TPIXEL>> opImage);
                ExtendedImage(const std::shared_ptr <const BaseImage> opImage);

                RGBA getPixel(unsigned long lpX, unsigned long lpY) const;
                void setPixel(unsigned long lpX, unsigned long lpY, const RGBA& tpPixel);
                const TPIXEL& getRealPixel(unsigned long lpX, unsigned long lpY) const;
                void setRealPixel(unsigned long lpX, unsigned long lpY, const TPIXEL& tpPoint);

                const std::vector <unsigned char>& getRawData() const;
                size_t getRawLineSize() const;
                unsigned short getRawElementSize() const;

                std::shared_ptr <ExtendedImage <TPIXEL>> getImage(unsigned long lpX, unsigned long lpY, unsigned long lpWidth, unsigned long lpHeight) const;
        };

    #include "ExtendedImage.h"

    #define RGBImage ExtendedImage <tools::image::RGBX>
    #define HSVImage ExtendedImage <tools::image::HSV>
    #define RGBAImage ExtendedImage <tools::image::RGBA>

        //////////////////////////////////////////////////////////////////////////////////////
        // MISCELANEOUS TOOLS

        class ImageTools {
            protected:
                ImageTools() = default;

            public:
                //////////////////////////////////////////////////////////
                // FUNCTIONS TO IMPORT AND EXPORT OF IMAGES
                //////////////////////////////////////////////////////////
                static std::shared_ptr <RGBAImage> readBMPFromStream(std::shared_ptr <std::istream> opStream, bool bpPlaceholder = false);
                static std::shared_ptr <RGBAImage> readBMPFromFile(const std::string& spPath, bool bpPlaceholder = false, bool bpLoadIntoMemory = false);
                static void writeToBMPFile(const std::string& spPath, std::shared_ptr <const RGBAImage> opImage);
                static std::shared_ptr <HSVImage> readHSVFromStream(std::shared_ptr <std::istream> opStream, bool bpPlaceholder = false);
                static std::shared_ptr <HSVImage> readHSVFromFile(const std::string& spPath, bool bpPlaceholder = false, bool bpLoadIntoMemory = false);
                static void writeToHSVFile(const std::string& spPath, std::shared_ptr <const HSVImage> opImage);
                static std::shared_ptr <RGBAImage> readPNGFromStream(std::shared_ptr <std::istream> opStream, bool bpPlaceholder = false);
                static std::shared_ptr <RGBAImage> readPNGFromFile(const std::string& spPath, bool bpPlaceholder = false, bool bpLoadIntoMemory = false);
        };
    }

    namespace audio {

        class Audio {
            protected:
                std::shared_ptr <std::vector <unsigned char>> agData;

            public:
                Audio() = default;

                virtual const std::shared_ptr <const std::vector <unsigned char>> getData() const;
        };

        class SynchronousPlayer {
            protected:
                bool bgIsPause;

            public:
                SynchronousPlayer() = default;

                virtual void play() = 0; // Starts play back.
                virtual bool next() = 0; // Gets the section to play back and executes It.
                virtual void stop() = 0; // Stops completely the play back
                virtual void playAll(); // Plays back all the audio.
                virtual void pause() = 0; // Pauses play back.
                virtual void resume() = 0; // Unpauses play back.

                bool isPaused() const;

                SynchronousPlayer& operator=(const SynchronousPlayer& opAudio) = delete;
                SynchronousPlayer& operator=(const SynchronousPlayer&& opAudio) = delete;
        };

        namespace wav {

            class WAVAudio : public Audio {
                protected:
                    std::shared_ptr <WAVEFORMATEX> rgFormat;

                public:
                    WAVAudio();
                    WAVAudio(const std::shared_ptr <const WAVEFORMATEX> rpFormat, const std::shared_ptr <const std::vector <BYTE>> apData);

                    const std::shared_ptr <const WAVEFORMATEX> getFormat() const;
            };

            class WAVSynchronousPlayer : public SynchronousPlayer {
                protected:
                    std::shared_ptr <const WAVAudio> ogAudio;

                    IMMDeviceEnumerator* pEnumerator = NULL;
                    IMMDevice* pDevice = NULL;
                    IAudioClient* pAudioClient = NULL;
                    IAudioRenderClient* pAudioRenderClient = NULL;

                    WAVEFORMATEX* plDeviceWaveFormat = NULL;

                    DWORD ilBufferInx;

                    BOOL bgExclusiveMode, bgEventCallback;

                    DWORD taskIndex = 0;
                    UINT32 nFramesInFile, nFramesInBuffer, nFramesPlayed;
                    BYTE* pData = NULL;
                    HANDLE hEvent = NULL;
                    HANDLE hTask = NULL;
                    REFERENCE_TIME hnsPeriod = 0;

                public:
                    WAVSynchronousPlayer(const std::shared_ptr <const WAVAudio> opAudio, bool bpExclusiveMode = false, bool bpCallbackEvent = false);
                    WAVSynchronousPlayer(const WAVSynchronousPlayer& opPlayer) = delete;
                    WAVSynchronousPlayer(WAVSynchronousPlayer&& opPlayer) = default;
                    ~WAVSynchronousPlayer();

                    const std::shared_ptr <const WAVAudio> getAudio() const;

                    void play();
                    bool next();
                    void stop();
                    void pause();
                    void resume();
            };

        }

        namespace midi {

            class MIDIAudio : public Audio {
                public:
                    struct StreamHeader {
                        unsigned int	id;		// identifier "MThd".
                        unsigned int	size;	// always 6 in big-endian format.
                        unsigned short	format;	// big-endian format.
                        unsigned short  tracks;	// number of tracks, big-endian.
                        unsigned short	ticks;	// number of ticks per quarter note, big-endian.
                    };

                protected:
                    std::shared_ptr <StreamHeader> rgHeader;
                    std::shared_ptr <std::vector <size_t>> agTracks;

                public:
                    MIDIAudio();
                    MIDIAudio(const std::shared_ptr <const StreamHeader> rpHeader, const std::shared_ptr <const std::vector <size_t>> apTracks, const std::shared_ptr <const std::vector <unsigned char>> apData);

                    const std::shared_ptr <const StreamHeader> getFormat() const;
                    const std::shared_ptr <const std::vector <size_t>> getTracks() const;

            };

            class MIDISynchronousPlayer : public SynchronousPlayer {
                protected:
                    struct TrackControl {
                        unsigned char last_event;
                        unsigned int absolute_time;
                        size_t buf_index;
                    };

                    struct Event {
                        unsigned int absolute_time;
                        unsigned char event;
                        size_t buf_index;
                    };

                    const unsigned int MIN_BUFFER_SIZE = sizeof(unsigned int) * 4;
                    const unsigned int MAX_BUFFER_SIZE = 512 * 12;

                    unsigned int igBufferSize;
                    unsigned int hgDevice;
                    HANDLE hgLock;
                    HMIDISTRM hgMIDIStream;
                    MIDIPROPTIMEDIV hgMIDIProperty;
                    MIDIHDR hgMIDIHeader;
                    std::shared_ptr <const MIDIAudio> ogAudio;
                    std::vector <TrackControl> agTracksInx;
                    std::vector <unsigned int> ogStream;

                    Event getNextEvent(TrackControl& rpTrack);
                    unsigned int getBuffer(std::vector <TrackControl>& apTracks, std::vector <unsigned int>& apBuffer);

                public:
                    MIDISynchronousPlayer(const std::shared_ptr <const MIDIAudio> opAudio, unsigned int ipBufferSize = 0);
                    MIDISynchronousPlayer(const MIDISynchronousPlayer& opPlayer) = delete;
                    MIDISynchronousPlayer(MIDISynchronousPlayer&& opPlayer) = default;
                    ~MIDISynchronousPlayer();

                    const std::shared_ptr <const MIDIAudio> getAudio() const;

                    void play();
                    bool next();
                    void stop();
                    void pause();
                    void resume();
            };

        }

        class AsynchronousPlayer {
            protected:
                static const unsigned char RESUME = 0;
                static const unsigned char PAUSE = 1;
                static const unsigned char STOP = 2;

                bool bgLoop;
                unsigned char igCommand;
                unsigned int igLoopLimit;
                unsigned int igActualLoop;
                unsigned long igLoopDelay;

                std::mutex ogPlayMutex;
                std::mutex ogPauseMutex;
                std::condition_variable ogPauseCtl;
                std::thread ogThread;
                std::shared_ptr <SynchronousPlayer> ogInternalPlayer;

                void executeLoop();

            public:
                AsynchronousPlayer(const std::shared_ptr <const Audio> opAudio);
                AsynchronousPlayer(const AsynchronousPlayer& opPlayer) = delete;
                AsynchronousPlayer(AsynchronousPlayer&& opPlayer) = default;
                ~AsynchronousPlayer();

                bool play(bool bpLoop = false, unsigned int ipLoopLimit = 0, unsigned long ipLoopDelay = 0);
                void pause();
                void resume();
                void stop();

                bool isPlaying();
                bool isLooping() const;
                unsigned int getLoopLimit() const;
                unsigned int getActualLoop() const;
                unsigned long getLoopDelay() const;

                void wait();

                AsynchronousPlayer& operator=(const AsynchronousPlayer& opAudio) = delete;
                AsynchronousPlayer& operator=(const AsynchronousPlayer&& opAudio) = delete;
        };

        class AudioTools {
            protected:
                AudioTools() = default;

            public:
                static std::shared_ptr <tools::audio::wav::WAVAudio> readWAVFromStream(std::shared_ptr <std::istream> opStream);
                static std::shared_ptr <tools::audio::wav::WAVAudio> readWAVFromFile(const std::string& spPath, bool bpLoadIntoMemory = false);
                static std::shared_ptr <tools::audio::midi::MIDIAudio> readMIDIFromStream(std::shared_ptr <std::istream> opStream);
                static std::shared_ptr <tools::audio::midi::MIDIAudio> readMIDIFromFile(const std::string& spPath, bool bpLoadIntoMemory = false);
                static std::shared_ptr <SynchronousPlayer> buildSynchronousPlayer(const std::shared_ptr <const Audio> opAudio);
        };

    }

}

///////////////////////////////////////////////////////////
#endif