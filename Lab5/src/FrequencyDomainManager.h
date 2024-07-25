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
    static void fft2d(Fd& fd) noexcept;
    static void transformToFrequencyDomain(Fd& fd) noexcept;
    static bool writeSpectrumLogScale(const Fd& fd, std::string_view fileName) noexcept;
    static bool writePhase(const Fd& fd, std::string_view fileName) noexcept;
    
private:
    static void fft(Complex* x, int size) noexcept;

};

#endif // __FREQUENCY_DOMAIN_MANAGER__