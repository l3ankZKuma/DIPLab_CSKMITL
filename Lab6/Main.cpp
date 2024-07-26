#include "pch.h"
#include "ImageManager.h"
#include "FrequencyDomainManager.h"

#define PATH_IMAGES "/Users/bankzkuma/Desktop/CSKMITL/DIP/Lab/Lab6/images/"

int main() {
    Image image;
    ImageSystem::initImage(image);
    ImageSystem::readImage(image, PATH_IMAGES "mandril.bmp");


    ImageSystem::addSaltNoise<10>(image);
    ImageSystem::writeImage(image, PATH_IMAGES "mandril_salt.bmp");
    ImageSystem::restoreToOriginal(image);

    ImageSystem::addPepperNoise<10>(image);
    ImageSystem::writeImage(image, PATH_IMAGES "mandril_pepper.bmp");
    ImageSystem::restoreToOriginal(image);

    Image mandril_salt;
    ImageSystem::initImage(mandril_salt);
    ImageSystem::readImage(mandril_salt, PATH_IMAGES "mandril_salt.bmp");
    ImageSystem::contraharmonicFilter<3>(mandril_salt,-1.5);
    ImageSystem::writeImage(mandril_salt, PATH_IMAGES "mandril_salt_contra.bmp");

    Image mandril_pepper;
    ImageSystem::initImage(mandril_pepper);
    ImageSystem::readImage(mandril_pepper, PATH_IMAGES "mandril_pepper.bmp");
    ImageSystem::contraharmonicFilter<3>(mandril_pepper,1.5);
    ImageSystem::writeImage(mandril_pepper, PATH_IMAGES "mandril_pepper_contra.bmp");

    ImageSystem::destroyImage(image);
    ImageSystem::destroyImage(mandril_salt);
    ImageSystem::destroyImage(mandril_pepper);




    return 0;
}