#include "ImageManager.h"

#define PATH_IMAGES "/Users/bankzkuma/Desktop/CSKMITL/DIP/Lab/Lab4/images/"

int main() 
{
    ImageManager *imgr = new ImageManager();


    if(imgr->read(PATH_IMAGES"mandril.bmp")){


        imgr->averagingFilter(3);
        imgr->write(PATH_IMAGES"mandril_averaging_3.bmp");
        imgr->restoreToOriginal();

        imgr->averagingFilter(5);
        imgr->write(PATH_IMAGES"mandril_averaging_5.bmp");
        imgr->restoreToOriginal();


        imgr->averagingFilter(7);
        imgr->write(PATH_IMAGES"mandril_averaging_7.bmp");
        imgr->restoreToOriginal();






        


    }

    delete imgr;

    return 0;
}