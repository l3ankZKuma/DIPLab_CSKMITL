#include"pch.h"
#include "FrequencyDomainManager.h"
#include "ImageManager.h"

#define NEXT_POWER_OF_2(x) (1 << (32 - __builtin_clz((x) - 1)))

void FdSystem::initFd(Fd& fd, const Image& im) noexcept {
    fd.image = const_cast<Image*>(&im);
    fd.imgWidth = im.width;
    fd.imgHeight = im.height;
    fd.width = NEXT_POWER_OF_2(fd.imgWidth);
    fd.height = NEXT_POWER_OF_2(fd.imgHeight);
    fd.img = new Complex[fd.height * fd.width];
    fd.original = new Complex[fd.height * fd.width];
}

void FdSystem::destroyFd(Fd& fd) noexcept {
    delete[] fd.img;
    delete[] fd.original;
}

void FdSystem::fft(Complex* x, int size) noexcept {
    if (size <= 1) return;

    Complex* even = new Complex[size / 2];
    Complex* odd = new Complex[size / 2];

    for (int i = 0; i < size / 2; i++) {
        even[i] = x[2 * i];
        odd[i] = x[2 * i + 1];
    }

    fft(even, size / 2);
    fft(odd, size / 2);

    double angle = 2 * M_PI / size;
    Complex w(1);
    Complex wn(cos(angle), sin(angle));

    for (int i = 0; i < size / 2; i++) {
        x[i] = even[i] + w * odd[i];
        x[i + size / 2] = even[i] - w * odd[i];
        w *= wn;
    }

    delete[] even;
    delete[] odd;
}

void FdSystem::fft2d(Fd& fd) noexcept {
    for (int y = 0; y < fd.height; y++) {
        fft(&fd.img[y * fd.width], fd.width);
    }

    Complex* column = new Complex[fd.height];
    for (int x = 0; x < fd.width; x++) {
        for (int y = 0; y < fd.height; y++) {
            column[y] = fd.img[y * fd.width + x];
        }
        fft(column, fd.height);
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

bool FdSystem::writePhase(const Fd& fd, std::string_view fileName) noexcept {
    Image phaseImg;
    ImageSystem::initImage(phaseImg);
    phaseImg.width = fd.width;
    phaseImg.height = fd.height;
    phaseImg.bitDepth = 24; // We'll use 24-bit color depth
    phaseImg.buf = new uint8_t[phaseImg.height * phaseImg.width * 3];

    // Find min and max phase values
    double max = -M_PI, min = M_PI;
    for (int y = 0; y < fd.height; ++y) {
        for (int x = 0; x < fd.width; ++x) {
            double phase = std::arg(fd.img[y * fd.width + x]);
            if (phase > max) max = phase;
            if (phase < min) min = phase;
        }
    }

    // Normalize and write phase values
    for (int y = 0; y < fd.height; ++y) {
        for (int x = 0; x < fd.width; ++x) {
            double phase = std::arg(fd.img[y * fd.width + x]);
            // Normalize phase to [0, 255]
            uint8_t phaseColor = static_cast<uint8_t>((phase - min) * 255.0 / (max - min));
            
            int bufIndex = (y * fd.width + x) * 3;
            phaseImg.buf[bufIndex] = phaseColor;
            phaseImg.buf[bufIndex + 1] = phaseColor;
            phaseImg.buf[bufIndex + 2] = phaseColor;
        }
    }

    // Copy header from original image
    std::memcpy(phaseImg.header, fd.image->header, BMP_HEADER_SIZE);

    // Update header for new image dimensions and bit depth
    *reinterpret_cast<int*>(&phaseImg.header[18]) = phaseImg.width;
    *reinterpret_cast<int*>(&phaseImg.header[22]) = phaseImg.height;
    *reinterpret_cast<int*>(&phaseImg.header[28]) = phaseImg.bitDepth;

    int paddedRowSize = ((phaseImg.width * 3 + 3) / 4) * 4;
    int fileSize = BMP_HEADER_SIZE + (paddedRowSize * phaseImg.height);
    *reinterpret_cast<int*>(&phaseImg.header[2]) = fileSize;

    bool result = ImageSystem::writeImage(phaseImg, fileName);

    ImageSystem::destroyImage(phaseImg);

    return result;
}

bool FdSystem::writeSpectrumLogScale(const Fd& fd, std::string_view fileName) noexcept {
    Image spectrumImg;
    ImageSystem::initImage(spectrumImg);
    spectrumImg.width = fd.width;
    spectrumImg.height = fd.height;
    spectrumImg.bitDepth = 24; // Assuming 24-bit color depth
    spectrumImg.buf = new uint8_t[spectrumImg.height * spectrumImg.width * 3];

    // Find the maximum magnitude for normalization
    double maxMagnitude = 0.0;
    for (int i = 0; i < fd.width * fd.height; ++i) {
        double magnitude = std::abs(fd.img[i]);
        if (magnitude > maxMagnitude) maxMagnitude = magnitude;
    }

    for (int y = 0; y < fd.height; ++y) {
        for (int x = 0; x < fd.width; ++x) {
            int index = y * fd.width + x;
            double magnitude = std::abs(fd.img[index]);
            
            // Apply log scale and normalize to [0, 255]
            double logMagnitude = std::log(1 + magnitude);
            uint8_t intensity = static_cast<uint8_t>(255 * logMagnitude / std::log(1 + maxMagnitude));
            
            int bufIndex = (y * fd.width + x) * 3;
            spectrumImg.buf[bufIndex] = intensity;
            spectrumImg.buf[bufIndex + 1] = intensity;
            spectrumImg.buf[bufIndex + 2] = intensity;
        }
    }

    bool result = ImageSystem::writeImage(spectrumImg, fileName);
    ImageSystem::destroyImage(spectrumImg);
    return result;
}