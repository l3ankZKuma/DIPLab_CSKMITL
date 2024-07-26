#ifndef __IMAGE_MANAGER__
#define __IMAGE_MANAGER__

#define BYTE 8
#define BMP_COLOR_TABLE_SIZE 1024
#define BMP_HEADER_SIZE 54

struct Fd;


struct Image {
    uint32_t width;
    uint32_t height;
    uint32_t bitDepth;
    uint8_t* header;
    uint8_t* colorTable;
    uint8_t* buf;
    uint8_t* original;
};


struct ImageSystem {
    static void initImage(Image& img) noexcept;
    static void destroyImage(Image& img) noexcept;
    static Fd& getFrequencyDomain(const Image& img) noexcept;


    [[nodicard]]static bool readImage(Image& img,std::string_view fileName) noexcept;

    static bool writeImage(const Image& img,std::string_view fileName) noexcept;

    static void convertToRed(Image& img) noexcept;
    static void convertToGreen(Image& img) noexcept;
    static void convertToBlue(Image& img) noexcept;
    static void convertToGrayscale(Image& img) noexcept;
    static void restoreToOriginal(Image& img) noexcept;

    [[nodiscard]]static int getRGB(const Image& img,int x,int y) noexcept;
    static void setRGB(Image& img, int x, int y, int color) noexcept;

    template<int brightness>
    static void adjustBrightness(Image& img) noexcept;

    static void invert(Image& img) noexcept;
    static int* getGrayscaleHistogram(const Image& img) noexcept;


    static void writeHistogramToCSV(std::string_view src,std::string_view dest) noexcept;

    static float getContrast(const Image& img) noexcept;

    template<int contrast>
    static void adjustContrast(Image& img) noexcept;

    template<auto gamma>
    static void adjustGamma(Image &image) noexcept;

    template<int rTemp, int gTemp, int bTemp>
    static void setTemperature(Image& img) noexcept;

    template<int Size>
    static void averagingFilter(Image& img) noexcept;

    template<int size>
    static void medianFilter(Image& img) noexcept;

    template<int k, int size>
    static void unsharpMasking(Image& img) noexcept;


    template<auto percent>
    static void adddPepperSaltNoise(Image& img) noexcept;

};



#endif // __IMAGE_MANAGER__
