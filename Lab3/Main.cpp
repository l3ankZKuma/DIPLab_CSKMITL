#include "ImageManager.h"

#define PATH_IMAGES "/Users/bankzkuma/Desktop/CSKMITL/DIP/Lab/Lab3/images/"

int main() 
{
    ImageManager *imgr = new ImageManager();


    if(imgr->read(PATH_IMAGES"mandril.bmp")){

        imgr->write(PATH_IMAGES"out.bmp");

        imgr->adjustBrightness(0);
        imgr->write(PATH_IMAGES"adjbright_mandril.bmp");
        imgr->restoreToOriginal();

        imgr->invert();
        imgr->write(PATH_IMAGES"invert_mandril.bmp");
        imgr->restoreToOriginal();

        int *data = imgr->getGrayscaleHistogram();
        imgr->writeHistogramToCSV(data,PATH_IMAGES"histogram.csv");


        


        imgr->adjustContrast(-100);
        imgr->write(PATH_IMAGES"mandril_constrastNeg100.bmp");
        imgr->restoreToOriginal();

        imgr->adjustContrast(100);
        imgr->write(PATH_IMAGES"mandril_constrast100.bmp");
        imgr->restoreToOriginal();

        // Adjust gamma to 2.2
        imgr->adjustGamma(2.2);
        imgr->write(PATH_IMAGES "mandril_gamma2.2.bmp");
        imgr->restoreToOriginal();

        // Adjust gamma to 0.4
        imgr->adjustGamma(0.4);
        imgr->write(PATH_IMAGES "mandril_gamma0.4.bmp");
        imgr->restoreToOriginal();


        delete data;

        


    }

    delete imgr;

    return 0;
}