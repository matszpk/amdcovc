#ifndef PARAMETERS_H
#define PARAMETERS_H

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

struct PerfClocks
{
    unsigned int coreClock;
    unsigned int memoryClock;
};

enum class OVCParamType
{
    CORE_CLOCK,
    MEMORY_CLOCK,
    VDDC_VOLTAGE,
    FAN_SPEED,
    CORE_OD,
    MEMORY_OD
};

struct OVCParameter
{
    OVCParamType type;
    std::vector<int> adapters;
    bool allAdapters;
    int partId;
    double value;
    bool useDefault;
    std::string argText;
};

class Parameters
{
private:

public:

    static void SetOVCParameters(AMDGPUAdapterHandle& handle, const std::vector<OVCParameter>& ovcParams, const std::vector<PerfClocks>& perfClocks);

    static void SetOVCParameters(ADLMainControl& mainControl, int adaptersNum, const std::vector<int>& activeAdapters,
                               const std::vector<OVCParameter>& ovcParams);

    static bool ParseOVCParameter(const char* string, OVCParameter& param);

}

#endif /* PARAMETERS_H */
