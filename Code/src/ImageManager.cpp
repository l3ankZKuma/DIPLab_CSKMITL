#include"pch.h"
#include"ImageManager.h"

namespace ImageManager{


    bool read(Image& img, std::string_view fileName) noexcept {
        std::ifstream file(fileName.data(), std::ios::binary);
        if (!file) {
            std::cout << "Unable to open file" << '\n';
            return false;
        }

        file.read(reinterpret_cast<char*>(img.header), BMP_HEADER_SIZE);

        img.width = *reinterpret_cast<int*>(&img.header[18]);
        img.height = *reinterpret_cast<int*>(&img.header[22]);
        img.bitDepth = *reinterpret_cast<int*>(&img.header[28]);

        if (img.bitDepth <= 8) {
            file.read(reinterpret_cast<char*>(img.colorTable), BMP_COLOR_TABLE_SIZE);
        }

        img.buf = new uint8_t[img.height * img.width * (img.bitDepth / BYTE)];
        file.read(reinterpret_cast<char*>(img.buf), img.height * img.width * (img.bitDepth / BYTE));

        img.original = new uint8_t[img.height * img.width * (img.bitDepth / BYTE)];
        for (int i = 0; i < (img.height * img.width * (img.bitDepth / BYTE)); i++) {
            img.original[i] = img.buf[i];
        }

        file.close();
        return true;
    }

    bool write(Image& img, std::string_view fileName) noexcept {
        std::ofstream file(fileName.data(), std::ios::binary);
        if (!file) {
            std::cout << "Unable to create file" << std::endl;
            return false;
        }

        int size = img.width * img.height * (img.bitDepth / BYTE);
        int fileSize = size + 54;
        img.header[18] = static_cast<uint8_t>(img.width & 0xff);
        img.header[19] = static_cast<uint8_t>((img.width >> 8) & 0xff);
        img.header[20] = static_cast<uint8_t>((img.width >> 16) & 0xff);
        img.header[21] = static_cast<uint8_t>((img.width >> 24) & 0xff);
        img.header[22] = static_cast<uint8_t>(img.height & 0xff);
        img.header[23] = static_cast<uint8_t>((img.height >> 8) & 0xff);
        img.header[24] = static_cast<uint8_t>((img.height >> 16) & 0xff);
        img.header[25] = static_cast<uint8_t>((img.height >> 24) & 0xff);
        file.write(reinterpret_cast<char*>(img.header), BMP_HEADER_SIZE);

        if (img.bitDepth <= 8) {
            file.write(reinterpret_cast<char*>(img.colorTable), BMP_COLOR_TABLE_SIZE);
        }

        for (int i = 0; i < img.height; i++) {
            file.write(reinterpret_cast<char*>(img.buf + (i * img.width * (img.bitDepth / BYTE))), (img.width * (img.bitDepth / BYTE)));

            if ((img.width * (img.bitDepth / BYTE)) % 4 != 0) {
                uint8_t padding = 0;
                file.write(reinterpret_cast<char*>(&padding), (4 - (img.width * (img.bitDepth / BYTE)) % 4));
            }
        }
        

        file.close();
        return true;
    }


}