#include "ImageManager.h"




void ImageSystem::initImage(Image& img) noexcept {
    img.header = new uint8_t[BMP_HEADER_SIZE];
    img.colorTable = new uint8_t[BMP_COLOR_TABLE_SIZE];
    img.buf = nullptr;
    img.original = nullptr;
}


void ImageSystem::destroyImage(Image& img) noexcept {
    delete[] img.header;
    delete[] img.colorTable;
    delete[] img.buf;
    delete[] img.original;
}


bool ImageSystem::readImage(Image& img, std::string_view fileName) noexcept {
    FILE* fi = fopen(fileName.data(), "rb");
    if (fi == nullptr) {
        std::cout << "Unable to open file" << '\n';
        return false;
    }
    fread(img.header, sizeof(uint8_t), BMP_HEADER_SIZE, fi);

    img.width = *reinterpret_cast<int*>(&img.header[18]);
    img.height = *reinterpret_cast<int*>(&img.header[22]);
    img.bitDepth = *reinterpret_cast<int*>(&img.header[28]);

    if (img.bitDepth <= 8) {
        fread(img.colorTable, sizeof(uint8_t), BMP_COLOR_TABLE_SIZE, fi);
    }

    img.buf = new uint8_t[img.height * img.width * (img.bitDepth / BYTE)];
    fread(img.buf, sizeof(uint8_t), img.height * img.width * (img.bitDepth / BYTE), fi);

    img.original = new uint8_t[img.height * img.width * (img.bitDepth / BYTE)];
    for (int i = 0; i < (img.height * img.width * (img.bitDepth / BYTE)); i++) {
        img.original[i] = img.buf[i];
    }

    std::cout << "Image " << fileName << " with " << img.width << " x " << img.height << " pixels (" << img.bitDepth << " bits per pixel) has been read!" << std::endl;
    fclose(fi);
    return true;
}

bool ImageSystem::writeImage(const Image& img,std::string_view fileName) noexcept {

    FILE* fo = fopen(fileName.data(), "wb");
    if (fo == nullptr) {
        std::cout << "Unable to create file" << std::endl;
        return false;
    }
    fwrite(img.header, sizeof(uint8_t), BMP_HEADER_SIZE, fo);

    if (img.bitDepth <= 8) {
        fwrite(img.colorTable, sizeof(uint8_t), BMP_COLOR_TABLE_SIZE, fo);
    }
    fwrite(img.buf, sizeof(uint8_t), img.height * img.width * (img.bitDepth / BYTE), fo);
    std::cout << "Image " << fileName << " has been written!" << std::endl;
    fclose(fo);
    return true;
}


void ImageSystem::convertToRed(Image& img) noexcept {
    for (uint32_t i = 0; i < img.height * img.width; ++i) {
        img.buf[i * 3 + 1] = 0;
        img.buf[i * 3 + 2] = 0;
    }
}


void ImageSystem::convertToGreen(Image& img) noexcept {
    for (uint32_t i = 0; i < img.height * img.width; ++i) {
        img.buf[i * 3] = 0;
        img.buf[i * 3 + 2] = 0;
    }
}


void ImageSystem::convertToBlue(Image& img) noexcept {
    for (uint32_t i = 0; i < img.height * img.width; ++i) {
        img.buf[i * 3] = 0;
        img.buf[i * 3 + 1] = 0;
    }
}


void ImageSystem::convertToGrayscale(Image& img) noexcept {
    for (uint32_t i = 0; i < img.height * img.width; ++i) {
        uint8_t gray = 0.3 * img.buf[i * 3] + 0.59 * img.buf[i * 3 + 1] + 0.11 * img.buf[i * 3 + 2];
        img.buf[i * 3] = gray;
        img.buf[i * 3 + 1] = gray;
        img.buf[i * 3 + 2] = gray;
    }
}


void ImageSystem::restoreToOriginal(Image& img) noexcept {
    for (uint32_t i = 0; i < img.height * img.width * (img.bitDepth / BYTE); ++i) {
        img.buf[i] = img.original[i];
    }
}

int ImageSystem::getRGB(const Image& img,int x,int y) noexcept {
    int index = (y * img.width + x) * 3;
    return (img.buf[index] << 16) | (img.buf[index + 1] << 8) | img.buf[index + 2];
}


void ImageSystem::setRGB(Image& img,int x,int y,int color) noexcept {
    int index = (y * img.width + x) * 3;
    img.buf[index] = (color >> 16) & 0xFF;
    img.buf[index + 1] = (color >> 8) & 0xFF;
    img.buf[index + 2] = color & 0xFF;
}

template<int brightness>
void ImageSystem::adjustBrightness(Image& img) noexcept {
    for (uint32_t i = 0; i < img.height * img.width * 3; ++i) {
        int value = img.buf[i] + brightness;
        img.buf[i] = std::max(0, std::min(255, value));
    }
}

void ImageSystem::invert(Image& img) noexcept {
    for (uint32_t i = 0; i < img.height * img.width * 3; ++i) {
        img.buf[i] = 255 - img.buf[i];
    }
}


int* ImageSystem::getGrayscaleHistogram(const Image& img) noexcept {
    int* histogram = new int[256]();
    for (uint32_t i = 0; i < img.height * img.width; ++i) {
        int gray = 0.3 * img.buf[i * 3] + 0.59 * img.buf[i * 3 + 1] + 0.11 * img.buf[i * 3 + 2];
        histogram[gray]++;
    }
    return histogram;
}


void ImageSystem::writeHistogramToCSV(std::string_view src,std::string_view dest) noexcept {
    FILE* fo = fopen(dest.data(), "w");
    if (fo == nullptr) {
        std::cout << "Unable to create file" << '\n';
        return;
    }
    for (int i = 0; i < 256; ++i) {
        fprintf(fo, "%d,%d\n", i, src.at(i));
    }
    fclose(fo);
}


float ImageSystem::getContrast(const Image& img) noexcept {
    float sum = 0;
    float sumSq = 0;
    uint32_t pixelCount = img.height * img.width * 3;

    for (uint32_t i = 0; i < pixelCount; ++i) {
        sum += img.buf[i];
        sumSq += img.buf[i] * img.buf[i];
    }

    float mean = sum / pixelCount;
    float variance = (sumSq / pixelCount) - (mean * mean);
    return std::sqrt(variance);
}


template<int contrast>
void ImageSystem::adjustContrast(Image& img) noexcept {
    float factor = (259 * (contrast + 255)) / (255 * (259 - contrast));
    for (uint32_t i = 0; i < img.height * img.width * 3; ++i) {
        int value = factor * (img.buf[i] - 128) + 128;
        img.buf[i] = std::max(0, std::min(255, value));
    }
}

template<auto gamma>
void ImageSystem::adjustGamma(Image& img) noexcept {
    float inverseGamma = 1 / gamma;
    for (uint32_t i = 0; i < img.height * img.width * 3; ++i) {
        img.buf[i] = std::pow(img.buf[i] / 255.0, inverseGamma) * 255.0;
    }
}


template<int rTemp,int gTemp,int bTemp>
void ImageSystem::setTemperature(Image& img) noexcept {
    for (uint32_t i = 0; i < img.height * img.width; ++i) {
        int r = std::max(0, std::min(255, img.buf[i * 3] + rTemp));
        int g = std::max(0, std::min(255, img.buf[i * 3 + 1] + gTemp));
        int b = std::max(0, std::min(255, img.buf[i * 3 + 2] + bTemp));
        img.buf[i * 3] = r;
        img.buf[i * 3 + 1] = g;
        img.buf[i * 3 + 2] = b;
    }
}

template<int size>
void ImageSystem::averagingFilter(Image& img) noexcept {

        std::vector<int> buffer(img.width * img.height * 3);
        int halfSize = size / 2;
        for (int y = halfSize; y < img.height - halfSize; ++y) {
        for (int x = halfSize; x < img.width - halfSize; ++x) {
        int r = 0, g = 0, b = 0;
        for (int ky = -halfSize; ky <= halfSize; ++ky) {
            for (int kx = -halfSize; kx <= halfSize; ++kx) {
                int index = ((y + ky) * img.width + (x + kx)) * 3;
                r += img.buf[index];
                g += img.buf[index + 1];
                b += img.buf[index + 2];
                }
                }
                int area = size * size;
                int index = (y * img.width + x) * 3;
                buffer[index] = r / area;
                buffer[index + 1] = g / area;
                buffer[index + 2] = b / area;
                }
            }
        std::copy(buffer.begin(), buffer.end(), img.buf);
    }

    template<int size>
    void ImageSystem::medianFilter(Image& img) noexcept {
    // Implementation of applying a median filter
    std::vector<int> buffer(img.width * img.height * 3);
    int halfSize = size / 2;
    for (int y = halfSize; y < img.height - halfSize; ++y) {
    for (int x = halfSize; x < img.width - halfSize; ++x) {
    std::vector<int> r, g, b;
    for (int ky = -halfSize; ky <= halfSize; ++ky) {
    for (int kx = -halfSize; kx <= halfSize; ++kx) {
    int index = ((y + ky) * img.width + (x + kx)) * 3;
    r.push_back(img.buf[index]);
    g.push_back(img.buf[index + 1]);
    b.push_back(img.buf[index + 2]);
    }
    }
    std::sort(r.begin(), r.end());
    std::sort(g.begin(), g.end());
    std::sort(b.begin(), b.end());
    int index = (y * img.width + x) * 3;
    buffer[index] = r[r.size() / 2];
    buffer[index + 1] = g[g.size() / 2];
    buffer[index + 2] = b[b.size() / 2];
    }
    }
    std::copy(buffer.begin(), buffer.end(), img.buf);
}

template<int k, int size>
void ImageSystem::unsharpMasking(Image& img) noexcept {

    Image blurred;
    initImage(blurred);
    blurred.width = img.width;
    blurred.height = img.height;
    blurred.bitDepth = img.bitDepth;
    blurred.buf = new uint8_t[img.width * img.height * 3];

    averagingFilter<size>(blurred);

    for (uint32_t i = 0; i < img.width * img.height * 3; ++i) {
        int value = img.buf[i] + k * (img.buf[i] - blurred.buf[i]);
        img.buf[i] = std::max(0, std::min(255, value));
    }

    destroyImage(blurred);

}


