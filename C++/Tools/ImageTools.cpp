#include "Tools.h"
#include <fstream>
#include <sstream>
#include <string>
#include <limits>

using namespace std;
using namespace tools::image;

#undef max

#define MIN(x, y, z) ((y) <= (z)? ((x) <= (y)? (x): (y)): ((x) <= (z)? (x): (z)))
#define MAX(x, y, z) ((y) >= (z)? ((x) >= (y)? (x): (y)): ((x) >= (z)? (x): (z)))

struct HSV_HEADER {
    unsigned short type;
    unsigned long height;
    unsigned long width;
};

struct RGB_COORD {
    float r, g, b;
};

//////////////////////////////////////////////////////////////////////////////////////
// MISCELANEOUS FUNCTIONS

int tools::image::reverse4BytesOrder(int data) {
    unsigned char ch1, ch2, ch3, ch4;
    ch1 = data & 255;
    ch2 = (data >> 8) & 255;
    ch3 = (data >> 16) & 255;
    ch4 = (data >> 24) & 255;

    return ((int) ch1 << 24) + ((int) ch2 << 16) + ((int) ch3 << 8) + ch4;
}

short tools::image::reverse2BytesOrder(short data) {
    unsigned char ch1, ch2;
    ch1 = data & 255;
    ch2 = (data >> 8) & 255;

    return ((short) ch2 << 8) + ch1;
}

template <typename T>
T round(T tpValue, unsigned ipPlaces) {
    double dlTmp = pow(10, ipPlaces);
    return (float) (round(tpValue * dlTmp) / dlTmp);
}

//////////////////////////////////////////////////////////////////////////////////////
// PIXEL FORMATS

RGBA::RGBA() {
    r = 0;
    g = 0;
    b = 0;
    a = DEFAULT_ALPHA_VALUE;
}

RGBA::RGBA(unsigned char cpBlue, unsigned char cpGreen, unsigned char cpRed, unsigned char cpAlpha) {
    r = cpRed;
    g = cpGreen;
    b = cpBlue;
    a = cpAlpha;
}

RGBA::RGBA(const RGBA& spPixel) {
    r = spPixel.r;
    g = spPixel.g;
    b = spPixel.b;
    a = spPixel.a;
}

RGBA::RGBA(const HSV& spPixel) {
    RGBA slTmp = HSV::toRGB(spPixel);
    b = slTmp.b;
    g = slTmp.g;
    r = slTmp.r;
    a = slTmp.a;
}

bool tools::image::operator==(const RGBA& lhs, const RGBA& rhs) {
    return lhs.r == rhs.r && lhs.g == rhs.g && lhs.b == rhs.b && lhs.a == rhs.a;
}

bool tools::image::operator!=(const RGBA& lhs, const RGBA& rhs) {
    return lhs.r != rhs.r || lhs.g != rhs.g || lhs.b != rhs.b || lhs.a != rhs.a;
}

HSV::HSV() {
    h = 0;
    s = 0;
    v = 0;
}

HSV::HSV(float fpHue, float fpSaturation, float fpValue) {
    h = fpHue;
    s = fpSaturation;
    v = fpValue;
}

HSV::HSV(const HSV& spPixel) {
    (*this) = spPixel;
}

HSV::HSV(const RGBA& spPixel) {
    (*this) = fromRGB(spPixel);
}

HSV HSV::fromRGB(RGBA spPixel) {
    float flMax, flMin;
    RGB_COORD rlOrg;
    HSV rlResult;

    rlOrg.r = round<float>(((float) spPixel.r / 255), 2); //Red
    rlOrg.g = round<float>((float) spPixel.g / 255, 2); //Green
    rlOrg.b = round<float>((float) spPixel.b / 255, 2); //Blue

    flMax = MAX(rlOrg.r, rlOrg.g, rlOrg.b);
    flMin = MIN(rlOrg.r, rlOrg.g, rlOrg.b);

    rlResult.v = flMax;
    if(rlResult.v != 0.0) {
        rlResult.s = (flMax - flMin) / rlResult.v;
        if(rlResult.s != 0.0) {
            if(flMax == rlOrg.r)
                rlResult.h = (float) (60.0 * (0.0 + ((rlOrg.g - rlOrg.b) / (flMax - flMin))));
            else if(flMax == rlOrg.g)
                rlResult.h = (float) (60.0 * (2.0 + ((rlOrg.b - rlOrg.r) / (flMax - flMin))));
            else
                rlResult.h = (float) (60.0 * (4.0 + ((rlOrg.r - rlOrg.g) / (flMax - flMin))));

            if(rlResult.h < 0.0)
                rlResult.h += 360.0;
        } else
            rlResult.h = 0.0;
    } else {
        rlResult.h = 0.0;
        rlResult.s = 0.0;
    }

    return rlResult;
}

RGBA HSV::toRGB(HSV spPixel) {
    double dlH, dlF, dlP, dlQ, dlT;
    RGB_COORD rlTmp;
    RGBA rlResult;

    dlH = floor(spPixel.h / 60);
    dlF = (spPixel.h / 60) - dlH;
    dlP = spPixel.v * (1 - spPixel.s);
    dlQ = spPixel.v * (1 - spPixel.s * dlF);
    dlT = spPixel.v * (1 - spPixel.s * (1 - dlF));

    if(dlH == 0.0 || dlH == 6.0) {
        rlTmp.r = spPixel.v;
        rlTmp.g = (float) dlT;
        rlTmp.b = (float) dlP;
    } else if(dlH == 1.0) {
        rlTmp.r = (float) dlQ;
        rlTmp.g = spPixel.v;
        rlTmp.b = (float) dlP;
    } else if(dlH == 2.0) {
        rlTmp.r = (float) dlP;
        rlTmp.g = spPixel.v;
        rlTmp.b = (float) dlT;
    } else if(dlH == 3.0) {
        rlTmp.r = (float) dlP;
        rlTmp.g = (float) dlQ;
        rlTmp.b = spPixel.v;
    } else if(dlH == 4.0) {
        rlTmp.r = (float) dlT;
        rlTmp.g = (float) dlP;
        rlTmp.b = spPixel.v;
    } else {
        rlTmp.r = spPixel.v;
        rlTmp.g = (float) dlP;
        rlTmp.b = (float) dlQ;
    }

    rlResult.r = unsigned char(rlTmp.r * 255);
    rlResult.g = unsigned char(rlTmp.g * 255);
    rlResult.b = unsigned char(rlTmp.b * 255);
    rlResult.a = DEFAULT_ALPHA_VALUE;

    return rlResult;
}

//////////////////////////////////////////////////////////////////////////////////////
// IMAGE FORMATS

unsigned long BaseImage::getHeight() const {
    return lgHeight;
};

unsigned long BaseImage::getWidth() const {
    return lgWidth;
};

bool BaseImage::isPlaceholder() const {
    return bgIsPlaceholder;
}

//////////////////////////////////////////////////////////////////////////////////////
// CONVERSION TOOLS

// Builds an image from an BMP file.
shared_ptr <RGBAImage> ImageTools::readBMPFromStream(shared_ptr <istream> opStream, bool bpPlaceholder) {
    if(!opStream)
        throw string("ImageTools::readBMPFromMemory: The Stream cannot be null.");

    BMP_HEADER rlData;
    unsigned long llSize = 0; // Size of the original image.
    unsigned long llColorDepth = 0; // Depth of color space.
    unsigned long llPad; // Number of bytes to add for making the size of each line of alOrigMap a multiple of 4.
    unsigned long llNetSize; // Size in bytes of each line of alOrigMap before adding ilPad.
    unsigned long llSrcRowSize = 0; // Size in bytes of each line of alDestMap, equals to ilNetSize + llPad.
    unsigned long llDstRowSize = 0; // Size in butes of each line of alOrigMap.

    vector <RGBQUAD> rlColor; // Color table.
    vector <unsigned char> alOrigMap; // Original image.
    vector <unsigned char> alDestMap; // Destiny image.

    // Reads the first part of the header.
    opStream->read((char *) &rlData.cab, sizeof(BITMAPFILEHEADER));
    if(!opStream->good()){
        if(opStream->bad()) throw string("STREAM BAD.");
        if(opStream->fail()) throw string("STREAM FAIL.");
        if(opStream->eof()) throw string("STREAM EOF.");
        throw string("Unexpected Error.");
    }

    // Reads the second part of the header.
    opStream->read((char *) &rlData.inf, sizeof(BITMAPINFOHEADER));
    if(!opStream->good()){
        if(opStream->bad()) throw string("STREAM BAD.");
        if(opStream->fail()) throw string("STREAM FAIL.");
        if(opStream->eof()) throw string("STREAM EOF.");
        throw string("Unexpected Error.");
    }

    if(bpPlaceholder)
        return make_shared <RGBAImage>(rlData.inf.biWidth, rlData.inf.biHeight, true);

    // Jumps extra data that may exist after the header and the color table for formats different to BI_BITFIELDS.
    if(rlData.inf.biCompression != BI_BITFIELDS)
        opStream->ignore(rlData.inf.biSize - sizeof(BITMAPINFOHEADER));

    switch(rlData.inf.biBitCount){
        case 1: llColorDepth = 2;
                break;

        case 4: llColorDepth = 16;
                break;

        case 8: llColorDepth = 256;
                break;

        case 24: break;

        case 32: if(rlData.inf.biCompression != BI_BITFIELDS)
                    throw string("Format not supported.");
                 llColorDepth = 4;
                 break;

        default: throw string("Color Depth Error.");
    }

    // Gets the color table for color depths lesser than 24 bits.
    if(rlData.inf.biBitCount != 24) {
        llColorDepth = (rlData.inf.biClrUsed == 0 ? llColorDepth : rlData.inf.biClrUsed);
        rlColor = vector <RGBQUAD> (llColorDepth);

        opStream->read((char *) &rlColor[0], llColorDepth * sizeof(RGBQUAD));
        if(!opStream->good()){
            if(opStream->bad()) throw string("STREAM BAD.");
            if(opStream->fail()) throw string("STREAM FAIL.");
            if(opStream->eof()) throw string("STREAM EOF.");
            throw string("Unexpected Error.");
        }

        if(rlData.inf.biCompression == BI_BITFIELDS) {
            char slBuffer[5];
            opStream->read((char *) slBuffer, 4);
            slBuffer[4] = '\0';
            if(!strcmp(slBuffer, "BGRs") || !strcmp(slBuffer, " niW"))
                opStream->ignore(sizeof(CIEXYZTRIPLE) + (sizeof(long) * 3));
            else
                opStream->seekg(-4, ios_base::cur);
        }
    }

    llNetSize = long(ceil((float) (rlData.inf.biBitCount * rlData.inf.biWidth) / 8.0));
    llPad = ((llNetSize % 4) != 0)? 4 - llNetSize % 4: 0;
    llSrcRowSize = llNetSize + llPad;

    if(rlData.inf.biCompression != BI_RGB || rlData.inf.biCompression != BI_BITFIELDS)
        llSize = rlData.inf.biSizeImage;
    else
        llSize = llSrcRowSize * rlData.inf.biHeight;

    alOrigMap = vector <unsigned char> (llSize, 0);

    // Reads the bitmap.
    opStream->read((char *) &alOrigMap[0], llSize);
    if(!opStream->good()){
        if(opStream->bad()) throw string("STREAM BAD.");
        if(opStream->fail()) throw string("STREAM FAIL.");
        if(opStream->eof()) throw string("STREAM EOF.");
        throw string("Unexpected Error.");
    }

    // Since the result of the function always is going to be an image with color depth fo 24 bits then alDestMap is created with these characteristics.
    llDstRowSize = (((32 * rlData.inf.biWidth) + 31) / 32) * 4;
    alDestMap = vector <unsigned char> (llDstRowSize * rlData.inf.biHeight, 0);

    if(rlData.inf.biBitCount == 1) { // Conversion of an image of 1 bit depth color into an image of 24 bits depth color.
        long ilByte = -1;
        unsigned char ilMask;

        for(long ii = 0; ii < rlData.inf.biHeight; ii++){ // Goes through the image from the first line of the array that corresponds to the lower part of the image.
            for(long i = 0; i < rlData.inf.biWidth; i++){ // Goes through each pixen on a line.
                if((i % 8) == 0){
                    ilMask = 128;
                    ilByte++;
                }

                long ilTmp = (i * 4) + (llDstRowSize * ii);
                if((ilMask & alOrigMap[ilByte]) != 0){
                    alDestMap[ilTmp] = rlColor[1].rgbBlue;
                    alDestMap[ilTmp + 1] = rlColor[1].rgbGreen;
                    alDestMap[ilTmp + 2] = rlColor[1].rgbRed;
                    alDestMap[ilTmp + 3] = 1;
                }else{
                    alDestMap[ilTmp] = rlColor[0].rgbBlue;
                    alDestMap[ilTmp + 1] = rlColor[0].rgbGreen;
                    alDestMap[ilTmp + 2] = rlColor[0].rgbRed;
                    alDestMap[ilTmp + 3] = DEFAULT_ALPHA_VALUE;
                }

                ilMask >>= 1;
            }
            ilByte += llPad;
        }
    } else if(rlData.inf.biBitCount == 4) { // Conversion of an image of 4 bit depth color into an image of 24 bits depth color.
        if(rlData.inf.biCompression != BI_RLE4 && rlData.inf.biCompression != BI_RGB)
            throw string("Wrong Compression Type for Color Depth 4.");

        long ilByte = rlData.inf.biCompression == BI_RGB ? -1: 0;
        long ilInx;
        bool blSwitch = false;
        bool blAlternate = true;
        bool blIsEscape = false;
        bool blNotAligned = false;
        unsigned char clPixelCnt = 0;

        for(long ii = 0; ii < rlData.inf.biHeight; ii++) { // Goes through the image from the first line of the array that corresponds to the lower part of the image.
            for(long i = 0; i < rlData.inf.biWidth; i++) { // Goes through each pixen on a line.
                if(rlData.inf.biCompression == BI_RGB) {
                    if(i % 2 == 0)
                        ilInx = alOrigMap[++ilByte] >> 4;
                    else
                        ilInx = 15 & alOrigMap[ilByte];
                } else {
                    if(!blSwitch) {
                        if(alOrigMap[ilByte] == 0) {
                            ilByte++;
                            if(alOrigMap[ilByte] == 0 || alOrigMap[ilByte] == 1)
                                throw string("Found End of the Line or of the Image in Compressed Data before reach the End of the Line or of the Image.");
                            else if(alOrigMap[ilByte] == 2)
                                throw string("Not Supported RLE Escape for Color Depth 4.");
                            blSwitch = true;
                            blIsEscape = true;
                            clPixelCnt = alOrigMap[ilByte];
                            blNotAligned = clPixelCnt % 4 == 1 || clPixelCnt % 4 == 2;
                        } else {
                            blSwitch = true;
                            blIsEscape = false;
                            clPixelCnt = alOrigMap[ilByte];
                            blNotAligned = false;
                            ilByte++;
                        }
                    }
                    if(blAlternate) {
                        if(blSwitch && blIsEscape)
                            ilByte++;
                        ilInx = alOrigMap[ilByte] >> 4;
                    } else
                        ilInx = 15 & alOrigMap[ilByte];
                    blAlternate = !blAlternate;
                }

                long ilTmp = (i * 4) + (llDstRowSize * ii);
                alDestMap[ilTmp] = rlColor[ilInx].rgbBlue;
                alDestMap[ilTmp + 1] = rlColor[ilInx].rgbGreen;
                alDestMap[ilTmp + 2] = rlColor[ilInx].rgbRed;
                alDestMap[ilTmp + 3] = DEFAULT_ALPHA_VALUE;

                if(blSwitch && --clPixelCnt == 0) {
                    blSwitch = false;
                    blAlternate = true;
                    ilByte += blNotAligned ? 2 : 1;
                }
            }
            ilByte += rlData.inf.biCompression == BI_RGB ? llPad: 2;
        }
    } else if(rlData.inf.biBitCount == 8) { // Conversion of an image of 8 bit depth color into an image of 24 bits depth color.
        if(rlData.inf.biCompression != BI_RLE8 && rlData.inf.biCompression != BI_RGB)
            throw string("Wrong Compression Type for Color Depth 8.");

        long ilByte = rlData.inf.biCompression == BI_RGB ? -1 : 0;
        bool blSwitch = false;
        bool blIsEscape = false;
        bool blNotAligned = false;
        unsigned char clPixelCnt = 0;

        for(long ii = 0; ii < rlData.inf.biHeight; ii++) { // Goes through the image from the first line of the array that corresponds to the lower part of the image.
            for(long i = 0; i < rlData.inf.biWidth; i++) { // Goes through each pixen on a line.
                if(rlData.inf.biCompression == BI_RGB)
                    ilByte++;
                else {
                    if(!blSwitch) {
                        if(alOrigMap[ilByte] == 0) {
                            ilByte++;
                            if(alOrigMap[ilByte] == 0 || alOrigMap[ilByte] == 1)
                                throw string("Found End of the Line or of the Image in Compressed Data before reach the End of the Line or of the Image.");
                            else if(alOrigMap[ilByte] == 2)
                                throw string("Not Supported RLE Escape for Color Depth 8.");
                            blSwitch = true;
                            blIsEscape = true;
                            clPixelCnt = alOrigMap[ilByte];
                            blNotAligned = clPixelCnt % 2 == 1;
                        } else {
                            blSwitch = true;
                            blIsEscape = false;
                            clPixelCnt = alOrigMap[ilByte];
                            blNotAligned = false;
                            ilByte++;
                        }
                    }
                    if(blSwitch && blIsEscape)
                        ilByte++;
                }

                long ilTmp = (i * 4) + (llDstRowSize * ii);
                alDestMap[ilTmp] = rlColor[alOrigMap[ilByte]].rgbBlue;
                alDestMap[ilTmp + 1] = rlColor[alOrigMap[ilByte]].rgbGreen;
                alDestMap[ilTmp + 2] = rlColor[alOrigMap[ilByte]].rgbRed;
                alDestMap[ilTmp + 3] = DEFAULT_ALPHA_VALUE;

                if(blSwitch && --clPixelCnt == 0) {
                    blSwitch = false;
                    ilByte += blNotAligned ? 2: 1;
                }
            }
            ilByte += rlData.inf.biCompression == BI_RGB ? llPad : 2;
        }
    } else if(rlData.inf.biBitCount == 24) { // Adjust the image of 24 bit color depth.
        for(long ii = 0, ilByte = 0; ii < rlData.inf.biHeight; ii++, ilByte += llPad) // Goes through the image from the first line of the array that corresponds to the lower part of the image.
            for(long i = 0; i < rlData.inf.biWidth; i++, ilByte += 3) { // Goes through each pixen on a line.
                long ilTmp = (i * 4) + (llDstRowSize * ii);
                alDestMap[ilTmp] = alOrigMap[ilByte];
                alDestMap[ilTmp + 1] = alOrigMap[ilByte + 1];
                alDestMap[ilTmp + 2] = alOrigMap[ilByte + 2];
                alDestMap[ilTmp + 3] = DEFAULT_ALPHA_VALUE;
            }
    } else if(rlData.inf.biBitCount == 32) { // Adjust the image of 32 bit color depth.
        alDestMap = alOrigMap;
    }

    return make_shared <RGBAImage> (rlData.inf.biWidth, rlData.inf.biHeight, alDestMap);
}

struct BMP_HEADER_EXT {
    BITMAPFILEHEADER cab;
    BITMAPV4HEADER inf;
};

BMP_HEADER_EXT initBMPHeader(unsigned long ipHeight, unsigned long ipWidth, size_t lpSize) {
    BMP_HEADER_EXT rlHeader;
    
    rlHeader.cab.bfSize = (unsigned long) lpSize + (DWORD) sizeof(BITMAPFILEHEADER) + (DWORD) sizeof(BITMAPV4HEADER);
    rlHeader.cab.bfOffBits = (DWORD) sizeof(BITMAPFILEHEADER) + (DWORD) sizeof(BITMAPV4HEADER);
    rlHeader.cab.bfType = 0x4D42; //BM
    rlHeader.cab.bfReserved1 = 0;
    rlHeader.cab.bfReserved2 = 0;

    rlHeader.inf.bV4Size = sizeof(BITMAPV4HEADER);
    rlHeader.inf.bV4Width = ipWidth;
    rlHeader.inf.bV4Height = ipHeight;
    rlHeader.inf.bV4Planes = 1;
    rlHeader.inf.bV4BitCount = 32;
    rlHeader.inf.bV4V4Compression = BI_BITFIELDS;
    rlHeader.inf.bV4SizeImage = (unsigned long) lpSize;
    rlHeader.inf.bV4XPelsPerMeter = 0;
    rlHeader.inf.bV4YPelsPerMeter = 0;
    rlHeader.inf.bV4ClrUsed = 0;
    rlHeader.inf.bV4ClrImportant = 0;
    rlHeader.inf.bV4RedMask = 0;
    ((unsigned char*) &rlHeader.inf.bV4RedMask)[2] = 255; // {0, 0, 255, 0}
    rlHeader.inf.bV4GreenMask = 0;
    ((unsigned char*) &rlHeader.inf.bV4GreenMask)[1] = 255; // {0, 255, 0, 0}
    rlHeader.inf.bV4BlueMask = 0;
    ((unsigned char*) &rlHeader.inf.bV4BlueMask)[0] = 255; // {255, 0, 0, 0}
    rlHeader.inf.bV4AlphaMask = 0;
    ((unsigned char*) &rlHeader.inf.bV4AlphaMask)[3] = 255; // {0, 0, 0, 255}
    // {'R', 'G', 'B', 's'}
    ((char*) &rlHeader.inf.bV4CSType)[3] = 'R';
    ((char*) &rlHeader.inf.bV4CSType)[2] = 'G';
    ((char*) &rlHeader.inf.bV4CSType)[1] = 'B';
    ((char*) &rlHeader.inf.bV4CSType)[0] = 's';
    rlHeader.inf.bV4Endpoints = {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
    rlHeader.inf.bV4GammaRed = 0;
    rlHeader.inf.bV4GammaGreen = 0;
    rlHeader.inf.bV4GammaBlue = 0;

    return rlHeader;
}

shared_ptr <RGBAImage> ImageTools::readBMPFromFile(const string& spPath, bool bpPlaceholder, bool bpLoadIntoMemory) {
    if(spPath.find(".bmp\0") == string::npos)
        throw string("File is not a Bitmap: ") + spPath.data();

    shared_ptr <RGBAImage> olImage;
    if(bpLoadIntoMemory) {
        ifstream olFile(spPath.data(), ios::binary | ios::ate);
        if(!olFile.is_open())
            throw string("File cannot be opened: ") + spPath.data();

        streamsize ilSize = olFile.tellg();
        olFile.seekg(0);

        vector <char> alBuffer(ilSize);
        olFile.read(alBuffer.data(), ilSize);
        if(!olFile.good()) {
            if(olFile.bad()) throw string("STREAM BAD.");
            if(olFile.fail()) throw string("STREAM FAIL.");
            if(olFile.eof()) throw string("STREAM EOF.");
            throw string("Unexpected Error.");
        }
        olFile.close();

        olImage = readBMPFromStream(make_shared <istringstream> (string(alBuffer.data(), alBuffer.size())), bpPlaceholder);
    } else {
        shared_ptr <ifstream> olFile = make_shared <ifstream> (spPath.data(), ios::binary);
        if(!olFile->is_open())
            throw string("File cannot be opened: ") + spPath.data();

        olImage = readBMPFromStream(olFile, bpPlaceholder);

        olFile->close();
    }

    return olImage;
}

void ImageTools::writeToBMPFile(const string& spPath, shared_ptr <const RGBAImage> opImage) {
    if(opImage->atgData.size() == 0)
        throw string("Empty Image.");

    ofstream slFile;
    
    slFile.open(spPath.data(), ios::app | ios::binary);
    
    if(!slFile.is_open())
        throw string("File cannot be opened.");
    
    BMP_HEADER_EXT rlHeader = initBMPHeader(opImage->getHeight(), opImage->getWidth(), opImage->getRawData().size());
    slFile.write((char*) &rlHeader.cab, sizeof(BITMAPFILEHEADER));
    slFile.write((char*) &rlHeader.inf, sizeof(BITMAPV4HEADER));
    slFile.write((char*) &opImage->atgData[0], opImage->atgData.size());
    
    slFile.close();
}

shared_ptr <HSVImage> ImageTools::readHSVFromStream(std::shared_ptr <std::istream> opStream, bool bpPlaceholder) {
    if(!opStream)
        throw string("ImageTools::readHSVFromStream: The Stream cannot be null.");

    // Reads the first part of the header.
    HSV_HEADER rlHeader;
    opStream->read((char *) &rlHeader, sizeof(HSV_HEADER));
    if(!opStream->good()) {
        if(opStream->bad()) throw string("STREAM BAD.");
        if(opStream->fail()) throw string("STREAM FAIL.");
        if(opStream->eof()) throw string("STREAM EOF.");
        throw string("Unexpected Error.");
    }

    if(bpPlaceholder)
        return make_shared <HSVImage>(rlHeader.width, rlHeader.height, true);

    vector <unsigned char> atlData(((((sizeof(HSV) * rlHeader.width) + 31) / 32) * 4) * rlHeader.height);

    opStream->read((char *) &atlData[0], atlData.size() * sizeof(unsigned char));
    if(!opStream->good()) {
        if(opStream->bad()) throw string("STREAM BAD.");
        if(opStream->fail()) throw string("STREAM FAIL.");
        if(opStream->eof()) throw string("STREAM EOF.");
        throw string("Unexpected Error.");
    }

    return make_shared <HSVImage> (rlHeader.width, rlHeader.height, atlData);
};

shared_ptr <HSVImage> ImageTools::readHSVFromFile(const string& spPath, bool bpPlaceholder, bool bpLoadIntoMemory) {
    shared_ptr <HSVImage> olImage;
    if(bpLoadIntoMemory) {
        ifstream olFile(spPath.data(), ios::binary | ios::ate);
        if(!olFile.is_open())
            throw string("File cannot be opened: ") + spPath.data();

        streamsize ilSize = olFile.tellg();
        olFile.seekg(0);

        vector <char> alBuffer(ilSize);
        olFile.read(alBuffer.data(), ilSize);
        if(!olFile.good()) {
            if(olFile.bad()) throw string("STREAM BAD.");
            if(olFile.fail()) throw string("STREAM FAIL.");
            if(olFile.eof()) throw string("STREAM EOF.");
            throw string("Unexpected Error.");
        }
        olFile.close();

        olImage = readHSVFromStream(make_shared <istringstream> (string(alBuffer.data(), alBuffer.size())), bpPlaceholder);
    } else {
        shared_ptr <ifstream> olFile = make_shared <ifstream>(spPath.data(), ios::binary);
        if(!olFile->is_open())
            throw string("File cannot be opened: ") + spPath.data();

        olImage = readHSVFromStream(olFile, bpPlaceholder);

        olFile->close();
    }

    return olImage;
}

void ImageTools::writeToHSVFile(const string& spPath, shared_ptr <const HSVImage> opImage) {
    if(opImage->atgData.size() == 0)
        throw string("Imagen Vacia");

    ofstream slFile;

    slFile.open(spPath.data(), ios::app | ios::binary);

    if(!slFile.is_open())
        throw string("Error abriendo archivo para escritura.");

    HSV_HEADER rlHeader;
    rlHeader.type = 0x4853;
    rlHeader.height = opImage->getHeight();
    rlHeader.width = opImage->getWidth();

    slFile.write((char*) &rlHeader, sizeof(HSV_HEADER));
    slFile.write((char*) &opImage->getRawData()[0], opImage->getRawData().size());

    slFile.close();
}

const unsigned long IHDR = 0x52444849;
const unsigned long PLTE = 0x45544c50;
const unsigned long IDAT = 0x54414449;
const unsigned long IEND = 0x444e4549;

const unsigned short DEFAULT_PIXEL_SIZE = 4;

shared_ptr <RGBAImage> ImageTools::readPNGFromStream(shared_ptr <istream> opStream, bool bpPlaceholder) {
    if(!opStream)
        throw string("ImageTools::readPNGFromStream: The Stream cannot be null.");

    opStream->seekg(0, opStream->end);
    if(!opStream->good()) {
        if(opStream->bad()) throw string("STREAM BAD.");
        if(opStream->fail()) throw string("STREAM FAIL.");
        if(opStream->eof()) throw string("STREAM EOF.");
        throw string("Unexpected Error.");
    }
    
    streamoff ilFileSize = opStream->tellg();
    if(ilFileSize < 45) // Minimum size of a PNG file when It is empty.
        throw string("Empty File.");
    opStream->seekg(0, opStream->beg);

    struct {
        unsigned long qword0;
        unsigned long qword1;
    } slSignature;

    opStream->read((char *) &slSignature, sizeof(slSignature));

    if(slSignature.qword0 != 0x474e5089 || slSignature.qword1 != 0x0a1a0a0d)
        throw string("Invalid Signature.");

    struct {
        unsigned long size;
        unsigned long marker;
    } slChunk;

    struct {
        unsigned long width;
        unsigned long height;
    } slImageSize;

    struct {
        unsigned char bitDepth;
        unsigned char colourType;
        unsigned char compressionMethod;
        unsigned char filterMethod;
        unsigned char interlaceMethod;
    } slParameters;

    struct RGB {
        unsigned char b; // Blue
        unsigned char g; // Green
        unsigned char r; // Red
    };

    bool ilDatInd = false;
    unsigned short ilHdrCnt = 0;
    unsigned short ilPltCnt = 0;
    unsigned short ilDatCnt = 0;
    unsigned short ilPixelSize = 0;
    unsigned long long ilInx = 0;
    vector <unsigned char> alExtractData; // The information extracted from the IDAT sections are added in order of apperance; ALWAYS contains the compression header, the headers of the ZLIB data blocks and the ADLER32 validation code.
    vector <RGB> alPalette; // Information about the color palette, It is only used when the type of color is 3, the red and blue are already inverted.
    
    // EXTRACTION STAGE (According to ISO/IEC 15948:2003 specification)

    // Reads the sections that make up the file:
    //      IHDR: Contains the description of the image (1 Instance).
    //      PLTE: Contains the color palette if the color type is 3 (1 Instance between IHDR and the first IDAT).
    //      IDAT: Contains the compressed data stream (1/* Consecutive instances, after IHDR section).
    //      IEND: Marks the end of the file (1 Instance, last section).

    while(!opStream->eof()) {
        opStream->read((char *) &slChunk, sizeof(slChunk));
        slChunk.size = reverse4BytesOrder(slChunk.size);

        if(slChunk.marker == IHDR) {
            if(ilHdrCnt != 0)
                throw string("Multiple IHDR Chuncks found.");
            ilHdrCnt++;

            opStream->read((char *) &slImageSize, sizeof(slImageSize));
            slImageSize.width = reverse4BytesOrder(slImageSize.width);
            slImageSize.height = reverse4BytesOrder(slImageSize.height);
            if(bpPlaceholder)
                return make_shared <RGBAImage> (slImageSize.width, slImageSize.height, true);

            opStream->read((char *) &slParameters, sizeof(slParameters));

            if(slParameters.bitDepth == 8)
                ilPixelSize = 1;
            else
                throw string("Not supported Bit Depth.");

            if(slParameters.colourType == 0)
                ilPixelSize *= 1;
            else if(slParameters.colourType == 2)
                ilPixelSize *= 3;
            else if(slParameters.colourType == 3)
                ilPixelSize *= 1;
            else if(slParameters.colourType == 4)
                ilPixelSize *= 2;
            else if(slParameters.colourType == 6)
                ilPixelSize *= 4;
            else
                throw string("Not supported Colour Type.");

            if(slParameters.compressionMethod != 0)
                throw string("Not supported Compression Method.");

            if(slParameters.filterMethod != 0)
                throw string("Not supported Filter Method.");

            if(slParameters.interlaceMethod != 0)
                throw string("Not supported Interlace Method.");
        } else if(slChunk.marker == PLTE) {
            if(ilHdrCnt == 0)
                throw string("No IHDR.");
            if(ilPltCnt != 0)
                throw string("Multiple PLTE Chunks found.");
            if(slChunk.size % 3 != 0 || slChunk.size / 3 > 256)
                throw string("Error in Palette Size.");
            ilPltCnt++;

            alPalette.resize(slChunk.size);
            opStream->read((char *) & alPalette[0], alPalette.size());
        } else if(slChunk.marker == IDAT) {
            if(ilHdrCnt == 0)
                throw string("No IHDR.");
            if(slParameters.colourType == 3 && ilPltCnt == 0)
                throw string("No PLTE.");
            if(!ilDatInd && ilDatCnt != 0)
                throw string("No Consecutive IDAT Chuncks found.");
            ilDatInd = true;
            ilDatCnt++;
            
            alExtractData.resize(alExtractData.size() + slChunk.size);
            opStream->read((char *) & alExtractData[ilInx], slChunk.size);
            ilInx += slChunk.size;
        } else if(slChunk.marker == IEND) {
            if(ilHdrCnt == 0)
                throw string("No IHDR.");
            if(ilFileSize != 4 + opStream->tellg())
                throw string("Extra Data after IEND.");
            break;
        } else {
            if(ilHdrCnt == 0)
                throw string("No IHDR.");
            ilDatInd = false;
            opStream->seekg(slChunk.size, opStream->cur);
        }

        unsigned long ilCRC;
        opStream->read((char *) & ilCRC, sizeof(ilCRC));
    }
    

    // FILTERING STAGE (According to ISO/IEC 15948:2003 specification)
    shared_ptr <vector <unsigned char>> olExtractor = tools::compression::ZLIB::inflate(alExtractData); // Initializes the decompression (According to RFC1950).
    unsigned int ilBInx = 0;
    short ilFilter = 0;
    unsigned long ilSrcLineSize = slImageSize.width * ilPixelSize;
    unsigned long ilDstLineSize = slImageSize.width * 4;
    unique_ptr <vector <unsigned char>> plLine0(new vector <unsigned char>(slImageSize.width * ilPixelSize, 0));
    unique_ptr <vector <unsigned char>> plLine1(new vector <unsigned char>(slImageSize.width * ilPixelSize, 0));
    // Contains the final data of the image, without the filter types at the beginning of the lines.
    vector <unsigned char> alData(slImageSize.height * slImageSize.width * DEFAULT_PIXEL_SIZE);
    for(unsigned long k = 0; k < slImageSize.height; k++) {
        for(unsigned long i = 0, j = -1, p = 1; i < (slImageSize.width * ilPixelSize) + 1; i++, j += i != 0 ? 1 : 0) {
            if(i != 0) {
                if(ilFilter == 0)
                    plLine1->at(j) = olExtractor->at(ilBInx++);
                else if(ilFilter == 1)
                    plLine1->at(j) = olExtractor->at(ilBInx++) + (i <= ilPixelSize ? 0 : plLine1->at(j - ilPixelSize));
                else if(ilFilter == 2)
                    plLine1->at(j) = olExtractor->at(ilBInx++) + plLine0->at(j);
                else if(ilFilter == 3)
                    plLine1->at(j) = olExtractor->at(ilBInx++) + (((i <= ilPixelSize ? 0 : plLine1->at(j - ilPixelSize)) + plLine0->at(j)) / 2); // The floor function was deleted because the Integer Arithmetic always takes the integer part.
                else if(ilFilter == 4) {
                    unsigned char x = olExtractor->at(ilBInx++);
                    unsigned char a = (i <= ilPixelSize ? 0 : plLine1->at(j - ilPixelSize));
                    unsigned char b = plLine0->at(j);
                    unsigned char c = (i <= ilPixelSize ? 0 : plLine0->at(j - ilPixelSize));
                    int p = a + b - c;
                    int pa = abs(p - a);
                    int pb = abs(p - b);
                    int pc = abs(p - c);

                    if(pa <= pb && pa <= pc)
                        plLine1->at(j) = x + a;
                    else if(pb <= pc)
                        plLine1->at(j) = x + b;
                    else
                        plLine1->at(j) = x + c;
                } else {
                    char alTmp[50];
                    sprintf_s(alTmp, sizeof(alTmp), "ImageTools::readFromPNGFile: Not recognize %d Filter.", ilFilter);
                    throw string(alTmp);
                }

                // Converts the final format of the image to the format required by the RGBAImage object:
                //  1- The positions of the red and blue channels are switched.
                //  2- The positions of the lines in the image are inverted in order to work with windows os.
                //  3- The channel alpha is added if the original image does not have It.
                if(p == ilPixelSize) {
                    unsigned long x = (j / ilPixelSize) * 4;
                    unsigned long y = (slImageSize.height - k - 1) * ilDstLineSize;
                    if(ilPixelSize == 1) {
                        if(slParameters.colourType != 3) {
                            alData[y + x++] = plLine1->at(j);
                            alData[y + x++] = plLine1->at(j);
                            alData[y + x++] = plLine1->at(j);
                        } else {
                            if(plLine1->at(j) >= alPalette.size())
                                throw string("ImageTools::readFromPNGFile: Bad Palette Index.");

                            // When the color palette is read the reds are in the greens positions and viceversa, that's whay r and b are switched.
                            alData[y + x++] = alPalette[plLine1->at(j)].r;
                            alData[y + x++] = alPalette[plLine1->at(j)].g;
                            alData[y + x++] = alPalette[plLine1->at(j)].b;
                        }
                        alData[y + x++] = DEFAULT_ALPHA_VALUE;
                    } else if(ilPixelSize == 2) {
                        alData[y + x++] = plLine1->at(j - 1);
                        alData[y + x++] = plLine1->at(j - 1);
                        alData[y + x++] = plLine1->at(j - 1);
                        alData[y + x++] = plLine1->at(j);
                    } else if(ilPixelSize == 3) {
                        alData[y + x++] = plLine1->at(j);
                        alData[y + x++] = plLine1->at(j - 1);
                        alData[y + x++] = plLine1->at(j - 2);
                        alData[y + x++] = DEFAULT_ALPHA_VALUE;
                    } else if(ilPixelSize == 4) {
                        alData[y + x++] = plLine1->at(j - 1);
                        alData[y + x++] = plLine1->at(j - 2);
                        alData[y + x++] = plLine1->at(j - 3);
                        alData[y + x++] = plLine1->at(j);
                    }
                    p = 1;
                } else
                    p++;
            } else
                ilFilter = olExtractor->at(ilBInx++);

            if(olExtractor->size() < ilBInx)
                throw string("ImageTools::readFromPNGFile: Bad Decompression.");
        }
        plLine0.swap(plLine1);
    }

    return make_shared <RGBAImage> (slImageSize.width, slImageSize.height, alData);
}

shared_ptr <RGBAImage> ImageTools::readPNGFromFile(const string& spPath, bool bpPlaceholder, bool bpLoadIntoMemory) {
    if(spPath.find(".png\0") == string::npos)
        throw string("File is not a PNG: ") + spPath.data();

    shared_ptr <RGBAImage> olImage;
    if(bpLoadIntoMemory) {
        ifstream olFile(spPath.data(), ios::binary | ios::ate);
        if(!olFile.is_open())
            throw string("File cannot be opened: ") + spPath.data();

        streamsize ilSize = olFile.tellg();
        olFile.seekg(0);

        vector <char> alBuffer(ilSize);
        olFile.read(alBuffer.data(), ilSize);
        if(!olFile.good()) {
            if(olFile.bad()) throw string("STREAM BAD.");
            if(olFile.fail()) throw string("STREAM FAIL.");
            if(olFile.eof()) throw string("STREAM EOF.");
            throw string("Unexpected Error.");
        }
        olFile.close();

        olImage = readPNGFromStream(make_shared <istringstream> (string(alBuffer.data(), alBuffer.size())), bpPlaceholder);
    } else {
        shared_ptr <ifstream> olFile = make_shared <ifstream>(spPath.data(), ios::binary);
        if(!olFile->is_open())
            throw string("File cannot be opened: ") + spPath.data();

        olImage = readPNGFromStream(olFile, bpPlaceholder);

        olFile->close();
    }

    return olImage;
}

//////////////////////////////////////////////////////////////////////////////////////