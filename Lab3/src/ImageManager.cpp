#include "ImageManager.h"

ImageManager::ImageManager()noexcept
{
    header = new unsigned char[BMP_HEADER_SIZE];
    colorTable = new unsigned char[BMP_COLOR_TABLE_SIZE];
    buf = nullptr;
    original = nullptr;
}

ImageManager::~ImageManager()noexcept
{
    delete[] header;
    delete[] colorTable;
    delete[] buf;
    delete[] original;
}

bool ImageManager::read(const char* fileName) noexcept
{
    FILE* fi = fopen(fileName, "rb");
    if (fi == nullptr)
    {
        std::cout << "Unable to open file" << '\n';
        return false;
    }
    fread(header, sizeof(uint8_t), BMP_HEADER_SIZE, fi);

    width = *reinterpret_cast<int*>(&header[18]);
    height = *reinterpret_cast<int*>(&header[22]);
    bitDepth = *reinterpret_cast<int*>(&header[28]);

    if (bitDepth <= 8)
    {
        fread(colorTable, sizeof(uint8_t), BMP_COLOR_TABLE_SIZE, fi);
    }

    buf = new uint8_t[height * width * (bitDepth / BYTE)];
    fread(buf, sizeof(uint8_t), height * width * (bitDepth / BYTE), fi);
    
    original = new uint8_t[height * width * (bitDepth / BYTE)];
    for (int i = 0; i < (height * width * (bitDepth / BYTE)); i++)
    {
        original[i] = buf[i];
    }
    
    std::cout << "Image " << fileName << " with " << width << " x " << height << " pixels (" << bitDepth << " bits per pixel) has been read!" << std::endl;
    fclose(fi);
    return true;
}

bool ImageManager::write(const char* fileName) noexcept
{
    FILE* fo = fopen(fileName, "wb");
    if (fo == nullptr)
    {
        std::cout << "Unable to create file" << std::endl;
        return false;
    }
    fwrite(header, sizeof(unsigned char), BMP_HEADER_SIZE, fo);
    
    if (bitDepth <= 8)
    {
        fwrite(colorTable, sizeof(unsigned char), BMP_COLOR_TABLE_SIZE, fo);
    }
    fwrite(buf, sizeof(unsigned char), height * width * (bitDepth / BYTE), fo);
    std::cout << "Image " << fileName << " has been written!" << std::endl;
    fclose(fo);
    return true;
}

int ImageManager::getRGB(int x, int y) noexcept
{
    int i = y * width * (bitDepth / BYTE) + x * (bitDepth / BYTE);
    int b = buf[i];
    int g = buf[i + 1];
    int r = buf[i + 2];

    int color = (r << 16) | (g << 8) | b;
    return color;
}

void ImageManager::setRGB(int x, int y, int color) noexcept
{
    int r = (color >> 16) & 0xff;
    int g = (color >> 8) & 0xff;
    int b = color & 0xff;

    int i = y * width * (bitDepth / BYTE) + x * (bitDepth / BYTE);
    buf[i] = b;
    buf[i + 1] = g;
    buf[i + 2] = r;
}

void ImageManager::convertToRed() noexcept
{
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            int color = getRGB(x, y);
            int r = (color >> 16) & 0xff;
            color = (r << 16) | 0 | 0;
            setRGB(x, y, color);
        }
    }
}

void ImageManager::convertToGreen() noexcept
{
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            int color = getRGB(x, y);
            int g = (color >> 8) & 0xff;
            color = 0 | (g << 8) | 0;
            setRGB(x, y, color);
        }
    }
}

void ImageManager::convertToBlue() noexcept
{
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            int color = getRGB(x, y);
            int b = color & 0xff;
            color = (0 << 16) | (0 << 8) | b;
            setRGB(x, y, color);
        }
    }
}

void ImageManager::convertToGrayscale() noexcept
{
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            uint32_t color = getRGB(x, y);
            uint32_t r = (color >> 16) & 0xff;
            uint32_t g = (color >> 8) & 0xff;
            uint32_t b = color & 0xff;
            uint32_t gray = (r + g + b) / 3;
            color = (gray << 16) | (gray << 8) | gray;
            setRGB(x, y, color);
        }
    }
}

void ImageManager::restoreToOriginal() noexcept
{
    for (int i = 0; i < (height * width * (bitDepth / BYTE)); i++)
    {
        buf[i] = original[i];
    }
}