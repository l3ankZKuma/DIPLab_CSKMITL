#include"ImageManager.h"

#define PATH_IMAGES "/Users/bankzkuma/Desktop/CSKMITL/DIP/Lab/Lab5/images/"

int main(){

    Image images;
    ImageSystem::initImage(images);
    ImageSystem::readImage(images,PATH_IMAGES"mandril.bmp");


    










    ImageSystem::destroyImage(images);



    return 0;
}