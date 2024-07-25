#include"pch.h"
#include "FrequencyDomainManager.h"
#include "ImageManager.h"

#define NEXT_POWER_OF_2(x) ((x) & ((x) - 1)) ? (1 << (32 - __builtin_clz((x) - 1))) : (x)
#define DBL_MIN 2.2250738585072014e-308
#define DBL_MAX 1.7976931348623158e+308

void FdSystem::initFd(Fd& fd, const Image& im) noexcept {

    fd.image = const_cast<Image*>(&im);
    fd.imgWidth = im.width;
    fd.imgHeight = im.height;
    fd.width = NEXT_POWER_OF_2(fd.imgWidth);
    fd.height = NEXT_POWER_OF_2(fd.imgHeight);
    fd.img = new Complex[fd.height * fd.width];
    fd.original = new Complex[fd.height * fd.width];

    fft2d(fd, true);
    shifting(fd);
}

void FdSystem::destroyFd(Fd& fd) noexcept {
    delete[] fd.img;
    delete[] fd.original;
}


void FdSystem::fft(Complex* x, int size,bool invert) noexcept {
    if (size <= 1) return;

    Complex* even = new Complex[size / 2];
    Complex* odd = new Complex[size / 2];

    for (int i = 0; i < size / 2; i++) {
        even[i] = x[2 * i];
        odd[i] = x[2 * i + 1];
    }

    fft(even, size / 2);
    fft(odd, size / 2);

    double angle = 2 * M_PI / size * (invert ? -1 : 1);
    Complex w(1);
    Complex wn(cos(angle), sin(angle));

    for (int i = 0; i < size / 2; i++) {
        x[i] = even[i] + w * odd[i];
        x[i + size / 2] = even[i] - w * odd[i];
        if (invert)
        {
            x[i] /= 2;
            x[i + size / 2] /= 2;
        }
        w *= wn;
    }

    delete[] even;
    delete[] odd;
}

void FdSystem::fft2d(Fd& fd,bool inverse) noexcept {
    for (int y = 0; y < fd.height; y++) {
        fft(&fd.img[y * fd.width], fd.width,inverse);
    }

    Complex* column = new Complex[fd.height];
    for (int x = 0; x < fd.width; x++) {
        for (int y = 0; y < fd.height; y++) {
            column[y] = fd.img[y * fd.width + x];
        }
        fft(column, fd.height,inverse);
        for (int y = 0; y < fd.height; y++) {
            fd.img[y * fd.width + x] = column[y];
        }
    }
    delete[] column;
}

void FdSystem::transformToFrequencyDomain(Fd& fd) noexcept {
    for (int y = 0; y < fd.height; y++) {
        for (int x = 0; x < fd.width; x++) {
            int color = ImageSystem::getRGB(*fd.image, x, y);
            int gray = color & 0xff;
            fd.img[y * fd.width + x] = Complex(gray, 0);
        }
    }
    
    fft2d(fd);
    
    for (int i = 0; i < (fd.height * fd.width); i++) {
        fd.original[i] = fd.img[i];
    }
}

bool FdSystem::writeSpectrumLogScale(const Fd& fd, std::string_view fileName) noexcept {
    std::cout << "Starting to write spectrum log scale..." << std::endl;

    Image img;
    ImageSystem::initImage(img);
    img.width = fd.imgWidth;
    img.height = fd.imgHeight;
    img.bitDepth = 24;  // We're creating a 24-bit color image

    std::cout << "Image initialized with dimensions: " << img.width << "x" << img.height << std::endl;

    // Copy the existing header
    std::memcpy(img.header, fd.image->header, BMP_HEADER_SIZE);

    // Update the width and height in the header
    *reinterpret_cast<int*>(&img.header[18]) = img.width;
    *reinterpret_cast<int*>(&img.header[22]) = img.height;

    // Update the bit depth in the header
    *reinterpret_cast<short*>(&img.header[28]) = img.bitDepth;

    // Calculate and update the file size in the header
    int fileSize = BMP_HEADER_SIZE + (img.width * img.height * 3);
    *reinterpret_cast<int*>(&img.header[2]) = fileSize;

    std::cout << "Header updated." << std::endl;

    try {
        img.buf = new uint8_t[img.width * img.height * 3];
        std::cout << "Image buffer allocated." << std::endl;
    } catch (const std::bad_alloc& e) {
        std::cerr << "Memory allocation failed: " << e.what() << std::endl;
        ImageSystem::destroyImage(img);
        return false;
    }

    double maxMagnitude = 0;
    for (int y = 0; y < fd.imgHeight; y++) {
        for (int x = 0; x < fd.imgWidth; x++) {
            Complex c = fd.img[y * fd.width + x];
            double magnitude = std::abs(c);
            if (magnitude > maxMagnitude) maxMagnitude = magnitude;
        }
    }

    std::cout << "Max magnitude calculated: " << maxMagnitude << std::endl;

    for (int y = 0; y < fd.imgHeight; y++) {
        for (int x = 0; x < fd.imgWidth; x++) {
            Complex c = fd.img[y * fd.width + x];
            double magnitude = std::abs(c);
            int intensity = static_cast<int>(255 * std::log(1 + magnitude) / std::log(1 + maxMagnitude));

            int index = (y * img.width + x) * 3;
            img.buf[index] = intensity;
            img.buf[index + 1] = intensity;
            img.buf[index + 2] = intensity;
        }
    }

    std::cout << "Spectrum data processed." << std::endl;

    bool result = ImageSystem::writeImage(img, fileName);
    std::cout << "Image writing " << (result ? "successful" : "failed") << std::endl;

    ImageSystem::destroyImage(img);
    std::cout << "Temporary image destroyed." << std::endl;

    return result;
}

bool FdSystem::writePhase(const Fd& fd, std::string_view fileName) noexcept {
    std::cout << "Starting to write phase..." << std::endl;

    Image img;
    ImageSystem::initImage(img);
    img.width = fd.imgWidth;
    img.height = fd.imgHeight;
    img.bitDepth = 24;  // We're creating a 24-bit color image

    std::cout << "Image initialized with dimensions: " << img.width << "x" << img.height << std::endl;

    // Copy the existing header
    std::memcpy(img.header, fd.image->header, BMP_HEADER_SIZE);

    // Update the width and height in the header
    *reinterpret_cast<int*>(&img.header[18]) = img.width;
    *reinterpret_cast<int*>(&img.header[22]) = img.height;

    // Update the bit depth in the header
    *reinterpret_cast<short*>(&img.header[28]) = img.bitDepth;

    // Calculate and update the file size in the header
    int fileSize = BMP_HEADER_SIZE + (img.width * img.height * 3);
    *reinterpret_cast<int*>(&img.header[2]) = fileSize;

    std::cout << "Header updated." << std::endl;

    try {
        img.buf = new uint8_t[img.width * img.height * 3];
        std::cout << "Image buffer allocated." << std::endl;
    } catch (const std::bad_alloc& e) {
        std::cerr << "Memory allocation failed: " << e.what() << std::endl;
        ImageSystem::destroyImage(img);
        return false;
    }

    for (int y = 0; y < fd.imgHeight; y++) {
        for (int x = 0; x < fd.imgWidth; x++) {
            Complex c = fd.img[y * fd.width + x];
            double phase = std::arg(c);
            int intensity = static_cast<int>((phase + M_PI) / (2 * M_PI) * 255);

            int index = (y * img.width + x) * 3;
            img.buf[index] = intensity;
            img.buf[index + 1] = intensity;
            img.buf[index + 2] = intensity;
        }
    }

    std::cout << "Phase data processed." << std::endl;

    bool result = ImageSystem::writeImage(img, fileName);
    std::cout << "Image writing " << (result ? "successful" : "failed") << std::endl;

    ImageSystem::destroyImage(img);
    std::cout << "Temporary image destroyed." << std::endl;

    return result;
}





void FdSystem::shifting(Fd& fd) noexcept {
        int halfWidth = fd.width / 2;
        int halfHeight = fd.height / 2;
        for (int y = 0; y < halfHeight; y++) {
            for (int x = 0; x < fd.width; x++) {
                Complex temp = Complex(fd.img[y * fd.width + x].real(), fd.img[y * fd.width + x].imag());
                fd.img[y * fd.width + x] = Complex(fd.img[(y + halfHeight) * fd.width + x].real(), fd.img[(y + halfHeight) * fd.width + x].imag());
                fd.img[(y + halfHeight) * fd.width + x] = Complex(temp.real(), temp.imag());
            }
        }
        for (int y = 0; y < fd.height; y++) {
            for (int x = 0; x < halfWidth; x++) {
                Complex temp = Complex(fd.img[y * fd.width + x].real(), fd.img[y * fd.width + x].imag());
                fd.img[y * fd.width + x] = Complex(fd.img[y * fd.width + x + halfWidth].real(), fd.img[y * fd.width + x + halfWidth].imag());
                fd.img[y * fd.width + x + halfWidth] = Complex(temp.real(), temp.imag());
            }
        }
}