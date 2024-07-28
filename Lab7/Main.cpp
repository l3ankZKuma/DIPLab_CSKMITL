#include "pch.h"
#include "ImageManager.h"
#include "FrequencyDomainManager.h"

#define PATH_IMAGES "/Users/bankzkuma/Desktop/CSKMITL/DIP/Lab/Lab7/images/"

int main() {


    Image image;
    ImageSystem::initImage(image);
    ImageSystem::readImage(image, PATH_IMAGES "mandril.bmp");

    Image image2;
    ImageSystem::initImage(image2);
    ImageSystem::readImage(image2, PATH_IMAGES "mandril.bmp");

    ImageSystem::ResizeBilinear(image,3.5,3.5);
    ImageSystem::write(image, PATH_IMAGES "mandril_resize3_5f.bmp");

    ImageSystem::ResizeBilinear(image2,0.35,0.35);
    ImageSystem::write(image2, PATH_IMAGES "mandril_resize0_35f.bmp");










    ImageSystem::destroyImage(image);
    ImageSystem::destroyImage(image2);





    return 0;
}