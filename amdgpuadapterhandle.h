#ifndef AMDGPUADAPTERHANDLE_H
#define AMDGPUADAPTERHANDLE_H

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
#include "amdgpuadapterinfo.h"

class AMDGPUAdapterHandle
{

private:

    unsigned int totDeviceCount;

    std::vector<uint32_t> amdDevices;

    std::vector<uint32_t> hwmonIndices;

public:

    AMDGPUAdapterHandle();

    unsigned int getAdaptersNum() const
    {
        return amdDevices.size();
    }

    AMDGPUAdapterInfo parseAdapterInfo(int index);

    void setFanSpeed(int index, int fanSpeed) const;

    void setFanSpeedToDefault(int adapterIndex) const;

    void setOverdriveCoreParam(int adapterIndex, unsigned int coreOD) const;

    void setOverdriveMemoryParam(int adapterIndex, unsigned int memoryOD) const;

    void getPerformanceClocks(int adapterIndex, unsigned int& coreClock, unsigned int& memoryClock) const;
};

#endif /* AMDGPUADAPTERHANDLE_H */
