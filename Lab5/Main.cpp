#include "pch.h"
#include "ImageManager.h"
#include "FrequencyDomainManager.h"

#define PATH_IMAGES "/Users/bankzkuma/Desktop/CSKMITL/DIP/Lab/Lab5/images/"

int main() {
    Image image;
    ImageSystem::initImage(image);
    ImageSystem::readImage(image, PATH_IMAGES "mandril.bmp");

    const Fd &fd_bind = ImageSystem::getFrequencyDomain(image);
    auto fd = const_cast<Fd&>(fd_bind);

    FdSystem::writeSpectrumLogScale(fd, PATH_IMAGES "mandril_spectrum.bmp");
    FdSystem::writePhase(fd, PATH_IMAGES "mandril_phase.bmp");

    std::array<double, 4> radius{3, 5, 10, 20};

    for (int i = 0; i < radius.size(); ++i) {
        std::ostringstream ss;
        
        FdSystem::ILPF(fd, radius[i]);
        
        ss << PATH_IMAGES << "mandril_spectrum_ILPF" << static_cast<int>(radius[i]) << ".bmp";
        FdSystem::writeSpectrumLogScale(fd, ss.str());
        
        ss.str("");
        ss.clear();
        ss << PATH_IMAGES << "mandril_phase_ILPF" << static_cast<int>(radius[i]) << ".bmp";
        FdSystem::writePhase(fd, ss.str());
        
        std::copy_n(fd.original, fd.height * fd.width, fd.img);
    }

    

    ImageSystem::destroyImage(image);
    FdSystem::destroyFd(fd);

    return 0;
}