#include "adlmaincontrol.h"

// Memory allocation function
void* __stdcall ADL_Main_Memory_Alloc (int iSize)
{
    void* lpBuffer = malloc (iSize);
    return lpBuffer;
}

// Optional Memory de-allocation function
void __stdcall ADL_Main_Memory_Free (void** lpBuffer)
{
    if (nullptr != *lpBuffer)
    {
        free (*lpBuffer);
        *lpBuffer = nullptr;
    }
}

ADLMainControl::ADLMainControl(const ATIADLHandle& _handle, int devId)

try : handle(_handle), fd(-1), mainControlCreated(false), withX(true)
{
    try
    {
        handle.Main_Control_Create(ADL_Main_Memory_Alloc, 0);
    }
    catch(const Error& error)
    {
        if (getuid() != 0)
        {
            std::cout << "This program requires root privileges to run if X11 server is not running." << std::endl;
        }

        withX = false;
        char devName[64];

        snprintf(devName, 64, "/dev/ati/card%u", devId);

        errno = 0;
        fd = open(devName, O_RDWR);

        if (fd == -1)
        {
            cl_uint platformsNum;

            /// force initialization of devices
            clGetPlatformIDs(0, nullptr, &platformsNum);
            errno = 0;
            fd = open(devName, O_RDWR);

            if (fd == -1)
            {
                throw Error(errno, "Cannot open GPU device");
            }
        }

        handle.ConsoleMode_FileDescriptor_Set(fd);
        handle.Main_Control_Create(ADL_Main_Memory_Alloc, 0);
    }
}
catch(...)
{
    if (mainControlCreated)
    {
        handle.Main_Control_Destroy();
    }

    if (fd!=-1)
    {
        close(fd);
    }

    throw;
}

ADLMainControl::~ADLMainControl()
{
    if (fd!=-1)
    {
        close(fd);
    }
}

int ADLMainControl::getAdaptersNum() const
{
    int num = 0;

    handle.Adapter_NumberOfAdapters_Get(&num);

    return num;
}

bool ADLMainControl::isAdapterActive(int adapterIndex) const
{
    if (!withX)
    {
        return true;
    }

    int status = 0;

    handle.Adapter_Active_Get(adapterIndex, &status);

    return status == ADL_TRUE;
}

void ADLMainControl::getAdapterInfo(AdapterInfo* infos) const
{
    int num;
    handle.Adapter_NumberOfAdapters_Get(&num);

    for (int i = 0; i < num; i++)
    {
        infos[i].iSize = sizeof(AdapterInfo);
    }

    handle.Adapter_Info_Get(infos, num*sizeof(AdapterInfo));
}

void ADLMainControl::getCurrentActivity(int adapterIndex, ADLPMActivity& activity) const
{
    activity.iSize = sizeof(ADLPMActivity);
    handle.Overdrive5_CurrentActivity_Get(adapterIndex, &activity);
}

int ADLMainControl::getTemperature(int adapterIndex, int thermalCtrlIndex) const
{
    ADLTemperature temp;
    temp.iSize = sizeof(ADLTemperature);
    handle.Overdrive5_Temperature_Get(adapterIndex, thermalCtrlIndex, &temp);

    return temp.iTemperature;
}

void ADLMainControl::getFanSpeedInfo(int adapterIndex, int thermalCtrlIndex, ADLFanSpeedInfo& info) const
{
    info.iSize = sizeof(ADLFanSpeedInfo);
    handle.Overdrive5_FanSpeedInfo_Get(adapterIndex, thermalCtrlIndex, &info);
}

int ADLMainControl::getFanSpeed(int adapterIndex, int thermalCtrlIndex) const
{
    ADLFanSpeedValue fanSpeedValue;
    fanSpeedValue.iSpeedType = ADL_DL_FANCTRL_SPEED_TYPE_PERCENT;
    fanSpeedValue.iFlags = 0;
    fanSpeedValue.iSize = sizeof(ADLFanSpeedValue);

    handle.Overdrive5_FanSpeed_Get(adapterIndex, thermalCtrlIndex, &fanSpeedValue);

    return fanSpeedValue.iFanSpeed;
}

void ADLMainControl::getODParameters(int adapterIndex, ADLODParameters& odParameters) const
{
    odParameters.iSize = sizeof(ADLODParameters);
    handle.Overdrive5_ODParameters_Get(adapterIndex, &odParameters);
}

void ADLMainControl::getODPerformanceLevels(int adapterIndex, bool isDefault, int perfLevelsNum, ADLODPerformanceLevel* perfLevels) const
{
    const size_t odPLBufSize = sizeof(ADLODPerformanceLevels) + sizeof(ADLODPerformanceLevel)*(perfLevelsNum - 1);
    std::unique_ptr<char[]> odPlBuf(new char[odPLBufSize]);

    ADLODPerformanceLevels* odPLevels = (ADLODPerformanceLevels*)odPlBuf.get();
    odPLevels->iSize = odPLBufSize;

    handle.Overdrive5_ODPerformanceLevels_Get(adapterIndex, isDefault, odPLevels);

    std::copy(odPLevels->aLevels, odPLevels->aLevels+perfLevelsNum, perfLevels);
}

void ADLMainControl::setFanSpeed(int adapterIndex, int thermalCtrlIndex, int fanSpeed) const
{
    ADLFanSpeedValue fanSpeedValue;
    fanSpeedValue.iSize = sizeof(ADLFanSpeedValue);
    fanSpeedValue.iSpeedType = ADL_DL_FANCTRL_SPEED_TYPE_PERCENT;
    fanSpeedValue.iFanSpeed = fanSpeed;

    handle.Overdrive5_FanSpeed_Set(adapterIndex, thermalCtrlIndex, &fanSpeedValue);
}

void ADLMainControl::setFanSpeedToDefault(int adapterIndex, int thermalCtrlIndex) const
{
    handle.Overdrive5_FanSpeedToDefault_Set(adapterIndex, thermalCtrlIndex);
}

void ADLMainControl::setODPerformanceLevels(int adapterIndex, int perfLevelsNum, ADLODPerformanceLevel* perfLevels) const
{
    const size_t odPLBufSize = sizeof(ADLODPerformanceLevels) + sizeof(ADLODPerformanceLevel) * (perfLevelsNum - 1);
    std::unique_ptr<char[]> odPlBuf(new char[odPLBufSize]);

    ADLODPerformanceLevels* odPLevels = (ADLODPerformanceLevels*)odPlBuf.get();
    odPLevels->iSize = odPLBufSize;
    odPLevels->iReserved = 0;

    std::copy(perfLevels, perfLevels + perfLevelsNum, odPLevels->aLevels);

    handle.Overdrive5_ODPerformanceLevels_Set(adapterIndex, odPLevels);
}
