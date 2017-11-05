#ifndef AMDGPUADAPTERINFO_H
#define AMDGPUADAPTERINFO_H

#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE
#endif

#include <iostream>
#include <exception>
#include <vector>
#include <dlfcn.h>
#include <fstream>
#include <algorithm>
#include <cstdio>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <string>
#include <memory>
#include <cmath>
#include <cstdarg>
#include <stdint.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <fcntl.h>
#include <CL/cl.h>

extern "C" {
#include <pci/pci.h>
}

#ifdef __linux__
#define LINUX 1
#endif
#include "./dependencies/ADL_SDK_V10.2/include/adl_sdk.h"

#include "error.h"
#include "atiadlhandle.h"
#include "adlmaincontrol.h"

/*
 * AMD-GPU information
 */

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
