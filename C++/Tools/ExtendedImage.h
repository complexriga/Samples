//////////////////////////////////////////////////////////////////////////////////////

template <typename TPIXEL>
ExtendedImage <TPIXEL>::ExtendedImage() {
    atgData = std::vector <unsigned char>(0, 0);
    lgHeight = 0;
    lgWidth = 0;
    lgRowSize = 0;
    bgIsPlaceholder = false;
}

template <typename TPIXEL>
ExtendedImage <TPIXEL>::ExtendedImage(unsigned long lpWidth, unsigned long lpHeight, const std::vector <unsigned char> & abpData) {
    init(lpWidth, lpHeight, false);
    if(lpHeight * lgRowSize != abpData.size())
        throw std::string("Array Size does not correspond with Image Size.");

    atgData = std::vector <unsigned char> (abpData);
};

template <typename TPIXEL>
ExtendedImage <TPIXEL>::ExtendedImage(unsigned long lpWidth, unsigned long lpHeight, bool blIsPlaceholder) {
    init(lpWidth, lpHeight, blIsPlaceholder);
    atgData = std::vector <unsigned char> (lgRowSize * lpHeight);
};

//////////////////////////////////////////////////////////////////////////////////////
// Builds the image from other image.

template <typename TPIXEL>
ExtendedImage <TPIXEL>::ExtendedImage(const ExtendedImage<TPIXEL>& opImage) {
    init(opImage.getWidth(), opImage.getHeight());
    atgData = std::vector <unsigned char>(opImage.getRawData());
}

template <typename TPIXEL>
ExtendedImage <TPIXEL>::ExtendedImage(const BaseImage& opImage) {
    init(opImage.getWidth(), opImage.getHeight());
    atgData = std::vector <unsigned char>(lgRowSize * opImage.getHeight() * sizeof(TPIXEL), DEFAULT_ALPHA_VALUE);

    for(unsigned long y = 0; y < opImage.getHeight(); y++)
        for(unsigned long x = 0; x < opImage.getWidth(); x++)
            setPixel(x, y, opImage.getPixel(x, y));
}

//////////////////////////////////////////////////////////////////////////////////////

template <typename TPIXEL>
ExtendedImage <TPIXEL>::ExtendedImage(std::shared_ptr <const ExtendedImage<TPIXEL>> opImage) {
    init(opImage->getWidth(), opImage->getHeight(), opImage->isPlaceholder());
    atgData = std::vector <unsigned char>(opImage->getRawData());
}

template <typename TPIXEL>
ExtendedImage <TPIXEL>::ExtendedImage(std::shared_ptr <const BaseImage> opImage) {
    init(opImage->getWidth(), opImage->getHeight());
    atgData = std::vector <unsigned char>(lgRowSize * opImage->getHeight() * sizeof(TPIXEL), DEFAULT_ALPHA_VALUE);

    for(unsigned long y = 0; y < opImage->getHeight(); y++)
        for(unsigned long x = 0; x < opImage->getWidth(); x++)
            setPixel(x, y, opImage->getPixel(x, y));
}

//////////////////////////////////////////////////////////////////////////////////////

template <typename TPIXEL>
void ExtendedImage <TPIXEL>::init(unsigned long lpWidth, unsigned long lpHeight, bool blIsPlaceholder) {
    lgHeight = lpHeight;
    lgWidth = lpWidth;
    bgIsPlaceholder = blIsPlaceholder;
    lgRowSize = bgIsPlaceholder ? 0: (((lgWidth * sizeof(TPIXEL) * 8) + 31) / 32) * 4; // The sequence of bytes of each line is adjusted always to multiples of 4.
}

template <typename TPIXEL>
const std::vector <unsigned char>& ExtendedImage <TPIXEL>::getRawData() const {
    return atgData;
}

template <typename TPIXEL>
size_t ExtendedImage <TPIXEL>::getRawLineSize() const {
    return lgRowSize;
}

template <typename TPIXEL>
unsigned short ExtendedImage <TPIXEL>::getRawElementSize() const {
    return sizeof(TPIXEL);
}

template <typename TPIXEL>
RGBA ExtendedImage <TPIXEL>::getPixel(unsigned long lpX, unsigned long lpY) const {
    return getRealPixel(lpX, lpY);  // If RGBA& is chained directly to TPIXEL& do not realize the conversion.
}

template <typename TPIXEL>
void ExtendedImage <TPIXEL>::setPixel(unsigned long lpX, unsigned long lpY, const RGBA& tpPixel) {
    setRealPixel(lpX, lpY, tpPixel);
}

template <typename TPIXEL>
const TPIXEL& ExtendedImage <TPIXEL>::getRealPixel(unsigned long lpX, unsigned long lpY) const {
    return *((TPIXEL*) &atgData[(lpX * sizeof(TPIXEL)) + (lgRowSize * (getHeight() - lpY - 1))]);
}

template <typename TPIXEL>
void ExtendedImage <TPIXEL>::setRealPixel(unsigned long lpX, unsigned long lpY, const TPIXEL& rpPoint) {
    memcpy((void*) &atgData[(lpX * sizeof(TPIXEL)) + (lgRowSize * (getHeight() - lpY - 1))], (void*) &rpPoint, sizeof(TPIXEL));
};

template <typename TPIXEL>
std::shared_ptr <ExtendedImage <TPIXEL>> ExtendedImage <TPIXEL>::getImage(unsigned long lpX, unsigned long lpY, unsigned long lpWidth, unsigned long lpHeight) const {
    if(lpX + lpWidth - 1 >= getWidth() || lpY + lpHeight - 1 >= getHeight())
        throw std::string("ExtendedImage:getImage: Dimensions defined for the New Image are not within the Area of the Actual Image.");

    std::shared_ptr <ExtendedImage <TPIXEL>> plImage = std::make_shared <ExtendedImage <TPIXEL>> (lpWidth, lpHeight);

    for(unsigned long y0 = lpY, y1 = 0; y1 < plImage->getHeight(); y0++, y1++)
        memcpy((void*) &plImage->atgData[plImage->getRawLineSize() * y1], (void*) &atgData[(lpX * sizeof(TPIXEL)) + (lgRowSize * y0)], plImage->getRawLineSize());

    return plImage;
}

//////////////////////////////////////////////////////////////////////////////////////