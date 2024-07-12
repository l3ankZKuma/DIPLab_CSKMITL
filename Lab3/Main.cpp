#include "ImageManager.h"

#define PATH_IMAGES "/Users/bankzkuma/Desktop/CSKMITL/DIP/Lab/Lab2/images/"

int main() 
{
    ImageManager *imgr = new ImageManager();


    if(imgr->read(PATH_IMAGES"mandril.bmp")){

        imgr->write(PATH_IMAGES"out.bmp");

        
        imgr->convertToRed();
        imgr->write(PATH_IMAGES"mandril_red.bmp");
        imgr->restoreToOriginal();

        imgr->convertToGreen();
        imgr->write(PATH_IMAGES"mandril_green.bmp");
        imgr->restoreToOriginal();

        imgr->convertToBlue();
        imgr->write(PATH_IMAGES"mandril_blue.bmp");
        imgr->restoreToOriginal();

        imgr->convertToGrayscale();
        imgr->write(PATH_IMAGES"mandril_gray.bmp");
        imgr->restoreToOriginal();

    }

    delete imgr;

    return 0;
}