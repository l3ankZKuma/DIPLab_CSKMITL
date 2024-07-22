#ifndef __FREQUENCYDOMAIN_MANAGER_
#define __FREQUENCYDOMAIN_MANAGER_

#include"ImageManager.h"

#include<complex>

using Complex = std::complex<double>;


struct FrequencyComponent{

    Image im;
    Complex *img;
    Complex *original;

};

class FrequencyDomainSystem{

    void fdInit(FrequencyComponent & fc);
    void fdDestroy(FrequencyComponent & fc);
    

}














#endif


