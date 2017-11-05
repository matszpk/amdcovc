#ifndef ADLMAINCONTROL_H
#define ADLMAINCONTROL_H

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

class ADLMainControl
{

private:

    const ATIADLHandle& handle;

    int fd;

    bool mainControlCreated;

    bool withX;

public:

    explicit ADLMainControl(const ATIADLHandle& handle, int devId);

    ~ADLMainControl();

    int getAdaptersNum() const;

    bool isAdapterActive(int adapterIndex) const;

    void getAdapterInfo(AdapterInfo* infos) const;

    void getCurrentActivity(int adapterIndex, ADLPMActivity& activity) const;

    int getTemperature(int adapterIndex, int thermalCtrlIndex) const;

    void getFanSpeedInfo(int adapterIndex, int thermalCtrlIndex, ADLFanSpeedInfo& info) const;

    int getFanSpeed(int adapterIndex, int thermalCtrlIndex) const;

    void getODParameters(int adapterIndex, ADLODParameters& odParameters) const;

    void getODPerformanceLevels(int adapterIndex, bool isDefault, int perfLevelsNum, ADLODPerformanceLevel* perfLevels) const;

    void setFanSpeed(int adapterIndex, int thermalCtrlIndex, int fanSpeed) const;

    void setFanSpeedToDefault(int adapterIndex, int thermalCtrlIndex) const;

    void setODPerformanceLevels(int adapterIndex, int perfLevelsNum, ADLODPerformanceLevel* perfLevels) const;

};

#endif /* ADLMAINCONTROL_H */
