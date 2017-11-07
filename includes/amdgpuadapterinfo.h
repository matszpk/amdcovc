#ifndef AMDGPUADAPTERINFO_H
#define AMDGPUADAPTERINFO_H

#include <fstream>
#include <algorithm>
#include <cstring>
#include <cstdarg>

extern "C" {
#include <pci/pci.h>
}

#include "adlmaincontrol.h"

struct AMDGPUAdapterInfo
{
    unsigned int busNo;
    unsigned int deviceNo;
    unsigned int funcNo;
    unsigned int vendorId;
    unsigned int deviceId;
    std::string name;
    std::vector<unsigned int> memoryClocks;
    std::vector<unsigned int> coreClocks;
    unsigned int minFanSpeed;
    unsigned int maxFanSpeed;
    bool defaultFanSpeed;
    unsigned int fanSpeed;
    unsigned int coreClock;
    unsigned int memoryClock;
    unsigned int coreOD;
    unsigned int memoryOD;
    unsigned int temperature;
    unsigned int tempCritical;
    unsigned int busLanes;
    unsigned int busSpeed;
    int gpuLoad;
};

#endif /* AMDGPUADAPTERINFO_H */
