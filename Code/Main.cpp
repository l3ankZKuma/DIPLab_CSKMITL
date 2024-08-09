#include"pch.h"
#include"ImageManager.h"

#define PATH_IMAGES "/Users/bankzkuma/Desktop/CSKMITL/DIP/Lab/Code/images/"

int main(){

    ImageManager::Image img;



    ImageManager::read(img,PATH_IMAGES"mandril.bmp");

    ImageManager::write(img,PATH_IMAGES"mandril_copy.bmp");

    


    return 0;
}