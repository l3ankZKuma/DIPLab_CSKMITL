#include "pch.h"
#include "FrequencyDomainManager.h"
#include "ImageManager.h"

#define NEXT_POWER_OF_2(x) ((x) & ((x) - 1)) ? (1 << (32 - __builtin_clz((x) - 1))) : (x)

void FdSystem::initFd(Fd& fd, const Image& im) noexcept {
    fd.image = const_cast<Image*>(&im);
    fd.imgWidth = im.width;
    fd.imgHeight = im.height;
    fd.width = NEXT_POWER_OF_2(fd.imgWidth);
    fd.height = NEXT_POWER_OF_2(fd.imgHeight);
    fd.img = new Complex[fd.height * fd.width];
    fd.original = new Complex[fd.height * fd.width];

    transformToFrequencyDomain(fd);
    shifting(fd);
}

void FdSystem::destroyFd(Fd& fd) noexcept {
    delete[] fd.img;
    delete[] fd.original;
}

void FdSystem::fft(Complex* x, int size, bool invert) noexcept {
    if (size <= 1) return;

    std::vector<Complex> even(size / 2);
    std::vector<Complex> odd(size / 2);

    for (int i = 0; i < size / 2; i++) {
        even[i] = x[2 * i];
        odd[i] = x[2 * i + 1];
    }

    fft(even.data(), size / 2, invert);
    fft(odd.data(), size / 2, invert);

    double angle = 2 * M_PI / size * (invert ? -1 : 1);
    Complex w(1);
    Complex wn(cos(angle), sin(angle));

    for (int i = 0; i < size / 2; i++) {
        x[i] = even[i] + w * odd[i];
        x[i + size / 2] = even[i] - w * odd[i];
        if (invert) {
            x[i] /= 2;
            x[i + size / 2] /= 2;
        }
        w *= wn;
    }
}

void FdSystem::fft2d(Fd& fd, bool invert) noexcept {
    for (int y = 0; y < fd.height; y++) {
        fft(&fd.img[y * fd.width], fd.width, invert);
    }

    std::vector<Complex> column(fd.height);
    for (int x = 0; x < fd.width; x++) {
        for (int y = 0; y < fd.height; y++) {
            column[y] = fd.img[y * fd.width + x];
        }
        fft(column.data(), fd.height, invert);
        for (int y = 0; y < fd.height; y++) {
            fd.img[y * fd.width + x] = column[y];
        }
    }
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
    
    std::copy_n(fd.img, fd.height * fd.width, fd.original);
}

bool FdSystem::writeSpectrumLogScale(Fd& fd, std::string_view fileName) noexcept {
    const int byteDepth = 3;
    size_t bufferSize = fd.height * fd.width * byteDepth;
    std::vector<unsigned char> buf(bufferSize);

    double max = -std::numeric_limits<double>::infinity();
    double min = std::numeric_limits<double>::infinity();

    for (int i = 0; i < fd.height * fd.width; ++i) {
        double magnitude = std::abs(fd.img[i]);
        double logMagnitude = magnitude > 1.0 ? std::log10(magnitude) : 0.0;
        max = std::max(max, logMagnitude);
        min = std::min(min, logMagnitude);
    }

    double scale = 255.0 / (max - min);
    for (int i = 0; i < fd.height * fd.width; ++i) {
        double magnitude = std::abs(fd.img[i]);
        double logMagnitude = magnitude > 1.0 ? std::log10(magnitude) : 0.0;
        int color = static_cast<int>((logMagnitude - min) * scale);
        color = std::clamp(color, 0, 255);
        buf[i * byteDepth] = buf[i * byteDepth + 1] = buf[i * byteDepth + 2] = color;
    }

    return writeBufferToBMP(fd, fileName, buf.data(), bufferSize);
}

bool FdSystem::writePhase(Fd& fd, std::string_view fileName) noexcept {
    const int byteDepth = 3;
    size_t bufferSize = fd.height * fd.width * byteDepth;
    std::vector<unsigned char> buf(bufferSize);

    const double min = -M_PI;
    const double max = M_PI;

    double scale = 255.0 / (max - min);
    for (int i = 0; i < fd.height * fd.width; ++i) {
        double phase = std::arg(fd.img[i]);
        int color = static_cast<int>((phase - min) * scale);
        color = std::clamp(color, 0, 255);
        buf[i * byteDepth] = buf[i * byteDepth + 1] = buf[i * byteDepth + 2] = color;
    }

    return writeBufferToBMP(fd, fileName, buf.data(), bufferSize);
}

bool FdSystem::writeBufferToBMP(Fd& fd, std::string_view fileName, const unsigned char* buf, size_t bufferSize) noexcept {
    FILE* fo = fopen(fileName.data(), "wb");
    if (!fo) {
        return false;
    }

    if (!fd.image->header) {
        fclose(fo);
        return false;
    }

    if (fwrite(fd.image->header, sizeof(uint8_t), BMP_HEADER_SIZE, fo) != BMP_HEADER_SIZE) {
        fclose(fo);
        return false;
    }

    if (fd.image->bitDepth <= 8 && fd.image->colorTable) {
        if (fwrite(fd.image->colorTable, sizeof(uint8_t), BMP_COLOR_TABLE_SIZE, fo) != BMP_COLOR_TABLE_SIZE) {
            fclose(fo);
            return false;
        }
    }

    const int byteDepth = 3;
    for (int y = 0; y < fd.height; ++y) {
        size_t rowSize = fd.width * byteDepth;
        if (fwrite(buf + (y * rowSize), sizeof(unsigned char), rowSize, fo) != rowSize) {
            fclose(fo);
            return false;
        }

        size_t paddingSize = (4 - (rowSize % 4)) % 4;
        if (paddingSize > 0) {
            unsigned char padding[3] = {0};
            if (fwrite(padding, sizeof(unsigned char), paddingSize, fo) != paddingSize) {
                fclose(fo);
                return false;
            }
        }
    }

    fclose(fo);
    return true;
}

void FdSystem::shifting(Fd& fd) noexcept {
    const int halfWidth = fd.width / 2;
    const int halfHeight = fd.height / 2;

    std::vector<Complex> temp(std::max(fd.width, fd.height));

    for (int y = 0; y < fd.height; y++) {
        std::copy_n(&fd.img[y * fd.width], fd.width, temp.begin());
        std::rotate(temp.begin(), temp.begin() + halfWidth, temp.end());
        std::copy_n(temp.begin(), fd.width, &fd.img[y * fd.width]);
    }

    for (int x = 0; x < fd.width; x++) {
        for (int y = 0; y < fd.height; y++) {
            temp[y] = fd.img[y * fd.width + x];
        }
        std::rotate(temp.begin(), temp.begin() + halfHeight, temp.begin() + fd.height);
        for (int y = 0; y < fd.height; y++) {
            fd.img[y * fd.width + x] = temp[y];
        }
    }
}

void FdSystem::getInverse(Fd& fd) noexcept {
    shifting(fd);
    fft2d(fd, true);

    for (int y = 0; y < fd.height; y++) {
        for (int x = 0; x < fd.width; x++) {
            int gray = static_cast<int>(fd.img[y * fd.width + x].real());
            gray = std::clamp(gray, 0, 255);
            int color = (gray << 16) | (gray << 8) | gray;
            ImageSystem::setRGB(*fd.image, x, y, color);
        }
    }
}

void FdSystem::ILPF(Fd &fd, double radius) noexcept {
    if (radius <= 0 || radius > std::min(fd.width/2, fd.height/2)) {
        return;
    }
    int centerX = fd.width/2;
    int centerY = fd.height/2;
    for (int x = 0; x < fd.height; x++) {
        for (int y = 0; y < fd.width; y++) {
            if ((x - centerX) * (x - centerX) + (y - centerY) * (y - centerY) > radius * radius) {
                fd.img[y * fd.width + x] = Complex(0, 0);
            }
        }
    }
}