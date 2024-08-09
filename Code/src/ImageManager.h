#ifndef IMAGE_MANAGER_H
#define IMAGE_MANAGER_H

#define BYTE 8
#define BMP_COLOR_TABLE_SIZE 1024
#define BMP_HEADER_SIZE 54

namespace ImageManager{

    struct Image{

        uint32_t width;
        uint32_t height;
        uint32_t bitDepth;
        uint8_t* header;
        uint8_t* colorTable;
        uint8_t* buf;


        
        uint8_t* original;
        uint32_t originalWidth;
        uint32_t originalHeight;


        Image() noexcept : buf(nullptr), header(nullptr), colorTable(nullptr), original(nullptr) {
            header = new uint8_t[BMP_HEADER_SIZE];
            colorTable = new uint8_t[BMP_COLOR_TABLE_SIZE];
        }

        ~Image() noexcept{
            delete[] header;
            delete[] colorTable;
            delete[] buf;
        }
    
    };


    bool read(Image &img,std::string_view fileName) noexcept;
    bool write(Image &img,std::string_view fileName) noexcept;



}












#endif