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

    unsigned char *buf = new unsigned char[fd.imgHeight * fd.imgWidth * (fd.image->bitDepth / BYTE)];

    double maxMagnitude = DBL_MIN;
    for (int y = 0; y < fd.imgHeight; y++) {
        for (int x = 0; x < fd.imgWidth; x++) {
            double magnitude = std::abs(fd.img[y * fd.width + x]);
            if (magnitude > maxMagnitude) maxMagnitude = magnitude;
        }
    }

    double logMax = std::log(1 + maxMagnitude);

    for (int y = 0; y < fd.imgHeight; y++) {
        for (int x = 0; x < fd.imgWidth; x++) {
            double magnitude = std::abs(fd.img[y * fd.width + x]);
            double logMagnitude = std::log(1 + magnitude);
            int color = static_cast<int>(255 * logMagnitude / logMax);
            int i = (y * fd.imgWidth + x) * (fd.image->bitDepth / BYTE);
            buf[i] = color;
            buf[i + 1] = color;
            buf[i + 2] = color;
        }
    }

    FILE *fo = fopen(fileName.data(), "wb");
    if (fo == nullptr) {
        std::cout << "Unable to create file" << std::endl;
        delete[] buf;
        return false;
    }

    fwrite(fd.image->header, sizeof(unsigned char), BMP_HEADER_SIZE, fo);
    if (fd.image->bitDepth <= 8) {
        fwrite(fd.image->colorTable, sizeof(unsigned char), BMP_COLOR_TABLE_SIZE, fo);
    }

    for (int i = 0; i < fd.imgHeight; i++) {
        fwrite(buf + (i * fd.imgWidth * (fd.image->bitDepth / BYTE)), 
               sizeof(unsigned char), 
               (fd.imgWidth * (fd.image->bitDepth / BYTE)), 
               fo);
        if ((fd.imgWidth * (fd.image->bitDepth / BYTE)) % 4 != 0) {
            unsigned char padding[3] = {0, 0, 0};
            fwrite(padding, sizeof(unsigned char), 
                   (4 - (fd.imgWidth * (fd.image->bitDepth / BYTE)) % 4), fo);
        }
    }

    std::cout << "Image " << fileName << " has been written!" << std::endl;
    fclose(fo);
    delete[] buf;
    return true;
}

bool FdSystem::writePhase(const Fd& fd, std::string_view fileName) noexcept {
    std::cout << "Starting to write phase..." << std::endl;

    unsigned char *buf = new unsigned char[fd.imgHeight * fd.imgWidth * (fd.image->bitDepth / BYTE)];

    double max = DBL_MIN, min = DBL_MAX;
    for (int y = 0; y < fd.imgHeight; y++) {
        for (int x = 0; x < fd.imgWidth; x++) {
            double phase = std::arg(fd.img[y * fd.width + x]);
            if (phase > max) max = phase;
            if (phase < min) min = phase;
        }
    }

    for (int y = 0; y < fd.imgHeight; y++) {
        for (int x = 0; x < fd.imgWidth; x++) {
            double phase = std::arg(fd.img[y * fd.width + x]);
            phase = ((phase - min) * 255 / (max - min));
            int color = static_cast<int>(phase);
            int i = (y * fd.imgWidth + x) * (fd.image->bitDepth / BYTE);
            buf[i] = color;
            buf[i + 1] = color;
            buf[i + 2] = color;
        }
    }

    FILE *fo = fopen(fileName.data(), "wb");
    if (fo == nullptr) {
        std::cout << "Unable to create file" << std::endl;
        delete[] buf;
        return false;
    }

    fwrite(fd.image->header, sizeof(unsigned char), BMP_HEADER_SIZE, fo);
    if (fd.image->bitDepth <= 8) {
        fwrite(fd.image->colorTable, sizeof(unsigned char), BMP_COLOR_TABLE_SIZE, fo);
    }

    for (int i = 0; i < fd.imgHeight; i++) {
        fwrite(buf + (i * fd.imgWidth * (fd.image->bitDepth / BYTE)), 
               sizeof(unsigned char), 
               (fd.imgWidth * (fd.image->bitDepth / BYTE)), 
               fo);
        if ((fd.imgWidth * (fd.image->bitDepth / BYTE)) % 4 != 0) {
            unsigned char padding[3] = {0, 0, 0};
            fwrite(padding, sizeof(unsigned char), 
                   (4 - (fd.imgWidth * (fd.image->bitDepth / BYTE)) % 4), fo);
        }
    }

    std::cout << "Image " << fileName << " has been written!" << std::endl;
    fclose(fo);
    delete[] buf;
    return true;
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