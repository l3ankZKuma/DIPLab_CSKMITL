#include"pch.h"

#include "ImageManager.h"
#include "FrequencyDomainManager.h"

#define PATH_IMAGES "/Users/bankzkuma/Desktop/CSKMITL/DIP/Lab/Lab5/images/"

int main() {
    Image image;
    ImageSystem::initImage(image);
    ImageSystem::readImage(image, PATH_IMAGES "mandril.bmp");


    Fd fd = ImageSystem::getFrequencyDomain(image);
    
    FdSystem::writeSpectrumLogScale(fd, PATH_IMAGES "spectrum.bmp");
    FdSystem::writePhase(fd, PATH_IMAGES "phase.bmp");







    ImageSystem::destroyImage(image);
    FdSystem::destroyFd(fd);


    return 0;
}