#ifndef __FREQUENCY_DOMAIN_MANAGER__
#define __FREQUENCY_DOMAIN_MANAGER__


struct Image;
using Complex = std::complex<double>;

struct Fd {
    Complex* img;
    Complex* original;
    Image* image;
    int width;
    int height;
    int imgWidth;
    int imgHeight;
};

struct FdSystem {

    static void initFd(Fd& fd, const Image& im) noexcept;
    static void destroyFd(Fd& fd) noexcept;
    static void fft2d(Fd& fd,bool inverse=false) noexcept;
    static void transformToFrequencyDomain(Fd& fd) noexcept;
    static bool writeSpectrumLogScale(Fd& fd, std::string_view fileName) noexcept;
    static bool writePhase(Fd& fd, std::string_view fileName) noexcept;
    static void ILPF(Fd& fd,double radius) noexcept;
    static void getInverse(Fd& fd) noexcept;  
    
private:
         
    static void fft(Complex* x, int size,bool inverse=false) noexcept;
    static void shifting(Fd& fd) noexcept;
    static bool writeBufferToBMP(Fd& fd, std::string_view fileName, const unsigned char* buf, size_t bufferSize) noexcept;


};

#endif // __FREQUENCY_DOMAIN_MANAGER__