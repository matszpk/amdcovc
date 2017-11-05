#ifndef ADLMAINCONTROL_H
#define ADLMAINCONTROL_H

#include <iostream>
#include <memory>
#include <unistd.h>
#include <fcntl.h>
#include <CL/cl.h>

#ifdef __linux__
#define LINUX 1
#endif

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
