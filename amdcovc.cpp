/*
 *  AMDCOVC - AMD Console OVerdrive control utility
 *  Copyright (C) 2016 Mateusz Szpakowski
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#define _DEFAULT_SOURCE
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
#ifdef HAVE_ADLSDK
#include <CL/cl.h>
#endif
extern "C" {
#include <pci/pci.h>
}

#ifdef __linux__
#define LINUX 1
#endif
#ifdef HAVE_ADLSDK
#include <adl_sdk.h>
#endif

#define AMDCOVC_VERSION "0.3.9.2"

#ifdef HAVE_ADLSDK
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
#endif

class Error: public std::exception
{
private:
    std::string description;
public:
    explicit Error(const char* _description) : description(_description)
    { }
    Error(int error, const char* _description)
    {
        char errorBuf[32];
        snprintf(errorBuf, 32, "code %d: ", error);
        description = errorBuf;
        description += _description;
    }
    virtual ~Error() noexcept
    { }
    const char* what() const noexcept
    { return description.c_str(); }
};

#ifdef HAVE_ADLSDK
class ATIADLHandle
{
private:
    typedef int (*ADL_ConsoleMode_FileDescriptor_Set_T)(int fileDescriptor);
    typedef int (*ADL_Main_Control_Create_T)(ADL_MAIN_MALLOC_CALLBACK, int);
    typedef int (*ADL_Main_Control_Destroy_T)();
    typedef int (*ADL_Adapter_NumberOfAdapters_Get_T)(int* numAdapters);
    typedef int (*ADL_Adapter_Active_Get_T)(int adapterIndex, int *status);
    typedef int (*ADL_Adapter_AdapterInfo_Get_T)(LPAdapterInfo info, int inputSize);
    
    typedef int (*ADL_Overdrive5_CurrentActivity_Get_T)(int adapterIndex,
                ADLPMActivity* activity);
    typedef int (*ADL_Overdrive5_Temperature_Get_T)(int adapterIndex, int thermalCtrlIndex,
                ADLTemperature *temperature);
    typedef int (*ADL_Overdrive5_FanSpeedInfo_Get_T)(int adapterIndex, int thermalCtrlIndex,
                ADLFanSpeedInfo* fanSpeedInfo);
    typedef int (*ADL_Overdrive5_FanSpeed_Get_T)(int adapterIndex, int thermalCtrlIndex,
                ADLFanSpeedValue* fanSpeedValue);
    typedef int (*ADL_Overdrive5_ODParameters_Get_T)(int adapterIndex,
                ADLODParameters* odParameters);
    typedef int (*ADL_Overdrive5_ODPerformanceLevels_Get_T)(int adapterIndex, int idefault,
                ADLODPerformanceLevels* odPerformanceLevels);
    
    typedef int (*ADL_Overdrive5_FanSpeed_Set_T)(int adapterIndex, int thermalCtrlIndex,
                ADLFanSpeedValue* fanSpeedValue);
    typedef int (*ADL_Overdrive5_FanSpeedToDefault_Set_T)(int adapterIndex,
                int thermalCtrlIndex);
    typedef int (*ADL_Overdrive5_ODPerformanceLevels_Set_T)(int adapterIndex,
                ADLODPerformanceLevels* odPerformanceLevels);
    
    void* handle;
    void* getSym(const char* name);
    
    ADL_Main_Control_Create_T pADL_Main_Control_Create;
    ADL_Main_Control_Destroy_T pADL_Main_Control_Destroy;
    ADL_ConsoleMode_FileDescriptor_Set_T pADL_ConsoleMode_FileDescriptor_Set;
    ADL_Adapter_NumberOfAdapters_Get_T pADL_Adapter_NumberOfAdapters_Get;
    ADL_Adapter_Active_Get_T pADL_Adapter_Active_Get;
    ADL_Adapter_AdapterInfo_Get_T pADL_Adapter_AdapterInfo_Get;
    ADL_Overdrive5_CurrentActivity_Get_T pADL_Overdrive5_CurrentActivity_Get;
    ADL_Overdrive5_Temperature_Get_T pADL_Overdrive5_Temperature_Get;
    ADL_Overdrive5_FanSpeedInfo_Get_T pADL_Overdrive5_FanSpeedInfo_Get;
    ADL_Overdrive5_FanSpeed_Get_T pADL_Overdrive5_FanSpeed_Get;
    ADL_Overdrive5_ODParameters_Get_T pADL_Overdrive5_ODParameters_Get;
    ADL_Overdrive5_ODPerformanceLevels_Get_T pADL_Overdrive5_ODPerformanceLevels_Get;
    ADL_Overdrive5_FanSpeed_Set_T pADL_Overdrive5_FanSpeed_Set;
    ADL_Overdrive5_FanSpeedToDefault_Set_T pADL_Overdrive5_FanSpeedToDefault_Set;
    ADL_Overdrive5_ODPerformanceLevels_Set_T pADL_Overdrive5_ODPerformanceLevels_Set;
    
public:
    ATIADLHandle();
    bool open();
    ~ATIADLHandle();
    
    void Main_Control_Create(ADL_MAIN_MALLOC_CALLBACK callback,
                             int iEnumConnectedAdapters) const;
    
    void Main_Control_Destroy() const;
    void ConsoleMode_FileDescriptor_Set(int fileDescriptor) const;
    void Adapter_NumberOfAdapters_Get(int* number) const;
    void Adapter_Active_Get(int adapterIndex, int* status) const;
    void Adapter_Info_Get(LPAdapterInfo info, int inputSize) const;
    void Overdrive5_CurrentActivity_Get(int adapterIndex, ADLPMActivity* activity) const;
    void Overdrive5_Temperature_Get(int adapterIndex, int thermalCtrlIndex,
                ADLTemperature* temperature) const;
    void Overdrive5_FanSpeedInfo_Get(int adapterIndex, int thermalCtrlIndex,
                ADLFanSpeedInfo* fanSpeedInfo) const;
    void Overdrive5_FanSpeed_Get(int adapterIndex, int thermalCtrlIndex,
                ADLFanSpeedValue* fanSpeedValue) const;
    void Overdrive5_ODParameters_Get(int adapterIndex, ADLODParameters* odParameters) const;
    void Overdrive5_ODPerformanceLevels_Get(int adapterIndex, int idefault,
                ADLODPerformanceLevels* odPerformanceLevels) const;
    void Overdrive5_FanSpeed_Set(int adapterIndex, int thermalCtrlIndex,
                ADLFanSpeedValue* fanSpeedValue) const;
    void Overdrive5_FanSpeedToDefault_Set(int adapterIndex, int thermalCtrlIndex) const;
    void Overdrive5_ODPerformanceLevels_Set(int adapterIndex,
                ADLODPerformanceLevels* odPerformanceLevels) const;
};

ATIADLHandle::ATIADLHandle() 
    : handle(nullptr),
    pADL_Main_Control_Create(nullptr), pADL_Main_Control_Destroy(nullptr),
    pADL_ConsoleMode_FileDescriptor_Set(nullptr),
    pADL_Adapter_NumberOfAdapters_Get(nullptr), pADL_Adapter_Active_Get(nullptr),
    pADL_Adapter_AdapterInfo_Get(nullptr), pADL_Overdrive5_CurrentActivity_Get(nullptr),
    pADL_Overdrive5_Temperature_Get(nullptr), pADL_Overdrive5_FanSpeedInfo_Get(nullptr),
    pADL_Overdrive5_FanSpeed_Get(nullptr), pADL_Overdrive5_ODParameters_Get(nullptr),
    pADL_Overdrive5_ODPerformanceLevels_Get(nullptr),
    pADL_Overdrive5_FanSpeed_Set(nullptr),
    pADL_Overdrive5_FanSpeedToDefault_Set(nullptr),
    pADL_Overdrive5_ODPerformanceLevels_Set(nullptr)
{ }

bool ATIADLHandle::open()
try
{
    dlerror(); // clear old errors
    handle = dlopen("libatiadlxx.so", RTLD_LAZY|RTLD_GLOBAL);
    if (handle == nullptr)
        return false;
    
    pADL_Main_Control_Create = (ADL_Main_Control_Create_T)
                getSym("ADL_Main_Control_Create");
    pADL_Main_Control_Destroy = (ADL_Main_Control_Destroy_T)
                getSym("ADL_Main_Control_Destroy");
    pADL_ConsoleMode_FileDescriptor_Set = (ADL_ConsoleMode_FileDescriptor_Set_T)
                getSym("ADL_ConsoleMode_FileDescriptor_Set");
    pADL_Adapter_NumberOfAdapters_Get = (ADL_Adapter_NumberOfAdapters_Get_T)
                getSym("ADL_Adapter_NumberOfAdapters_Get");
    pADL_Adapter_Active_Get = (ADL_Adapter_Active_Get_T)
                getSym("ADL_Adapter_Active_Get");
    pADL_Adapter_AdapterInfo_Get = (ADL_Adapter_AdapterInfo_Get_T)
                getSym("ADL_Adapter_AdapterInfo_Get");
    pADL_Overdrive5_CurrentActivity_Get = (ADL_Overdrive5_CurrentActivity_Get_T)
                getSym("ADL_Overdrive5_CurrentActivity_Get");
    pADL_Overdrive5_Temperature_Get = (ADL_Overdrive5_Temperature_Get_T)
                getSym("ADL_Overdrive5_Temperature_Get");
    pADL_Overdrive5_FanSpeedInfo_Get = (ADL_Overdrive5_FanSpeedInfo_Get_T)
                getSym("ADL_Overdrive5_FanSpeedInfo_Get");
    pADL_Overdrive5_FanSpeed_Get = (ADL_Overdrive5_FanSpeed_Get_T)
                getSym("ADL_Overdrive5_FanSpeed_Get");
    pADL_Overdrive5_ODParameters_Get = (ADL_Overdrive5_ODParameters_Get_T)
                getSym("ADL_Overdrive5_ODParameters_Get");
    pADL_Overdrive5_ODPerformanceLevels_Get = (ADL_Overdrive5_ODPerformanceLevels_Get_T)
                getSym("ADL_Overdrive5_ODPerformanceLevels_Get");
    pADL_Overdrive5_FanSpeed_Set = (ADL_Overdrive5_FanSpeed_Set_T)
                getSym("ADL_Overdrive5_FanSpeed_Set");
    pADL_Overdrive5_FanSpeedToDefault_Set = (ADL_Overdrive5_FanSpeedToDefault_Set_T)
                getSym("ADL_Overdrive5_FanSpeedToDefault_Set");
    pADL_Overdrive5_ODPerformanceLevels_Set = (ADL_Overdrive5_ODPerformanceLevels_Set_T)
                getSym("ADL_Overdrive5_ODPerformanceLevels_Set");
    return true;
}
catch(...)
{
    if (handle != nullptr)
    {
        dlerror(); // clear old errors
        if (dlclose(handle)) // if closing failed
        {
            handle = nullptr;
            throw Error(dlerror());
        }
        handle = nullptr;
    }
    throw;
}

ATIADLHandle::~ATIADLHandle()
{
    if (handle != nullptr)
    {
        dlerror();
        if (dlclose(handle))
        {
            handle = nullptr;
            throw Error(dlerror());
        }
        handle = nullptr;
    }
}

void* ATIADLHandle::getSym(const char* symbolName)
{
    void* symbol = nullptr;
    dlerror(); // clear old errors
    symbol = dlsym(handle, symbolName);
    const char* error = dlerror();
    if (symbol == nullptr && error != nullptr)
        throw Error(error);
    return symbol;
}

void ATIADLHandle::Main_Control_Create(ADL_MAIN_MALLOC_CALLBACK callback,
                            int iEnumConnectedAdapters) const
{
    int error = pADL_Main_Control_Create(callback, iEnumConnectedAdapters);
    if (error != ADL_OK)
        throw Error(error, "ADL_Main_Control_Create error");
}

void ATIADLHandle::Main_Control_Destroy() const
{
    int error = pADL_Main_Control_Destroy();
    if (error != ADL_OK)
        throw Error(error, "ADL_Main_Control_Destroy error");
}

void ATIADLHandle::ConsoleMode_FileDescriptor_Set(int fileDescriptor) const
{
    int error = pADL_ConsoleMode_FileDescriptor_Set(fileDescriptor);
    if (error != ADL_OK)
        throw Error(error, "ADL_ConsoleMode_FileDescriptor_Set error");
}

void ATIADLHandle::Adapter_NumberOfAdapters_Get(int* number) const
{
    int error = pADL_Adapter_NumberOfAdapters_Get(number);
    if (error != ADL_OK)
        throw Error(error, "ADL_Adapter_NumberOfAdapters_Get error");
}

void ATIADLHandle::Adapter_Active_Get(int adapterIndex, int* status) const
{
    int error = pADL_Adapter_Active_Get(adapterIndex, status);
    if (error != ADL_OK)
        throw Error(error, "ADL_Adapter_Active_Get error");
}

void ATIADLHandle::Adapter_Info_Get(LPAdapterInfo info, int inputSize) const
{
    int error = pADL_Adapter_AdapterInfo_Get(info, inputSize);
    if (error != ADL_OK)
        throw Error(error, "ADL_AdapterInfo_Get error");
}

void ATIADLHandle::Overdrive5_CurrentActivity_Get(int adapterIndex,
                ADLPMActivity* activity) const
{
    int error = pADL_Overdrive5_CurrentActivity_Get(adapterIndex, activity);
    if (error != ADL_OK)
        throw Error(error, "ADL_Overdrive5_CurrentActivity_Get error");
}

void ATIADLHandle::Overdrive5_Temperature_Get(int adapterIndex, int thermalCtrlIndex,
                ADLTemperature *temperature) const
{
    int error = pADL_Overdrive5_Temperature_Get(adapterIndex, thermalCtrlIndex,
                    temperature);
    if (error != ADL_OK)
        throw Error(error, "ADL_Overdrive5_Temperature_Get error");
}

void ATIADLHandle::Overdrive5_FanSpeedInfo_Get(int adapterIndex, int thermalCtrlIndex,
                ADLFanSpeedInfo* fanSpeedInfo) const
{
    int error = pADL_Overdrive5_FanSpeedInfo_Get(adapterIndex, thermalCtrlIndex,
                    fanSpeedInfo);
    if (error != ADL_OK)
        throw Error(error, "ADL_Overdrive5_FanSpeedInfo_Get error");
}

void ATIADLHandle::Overdrive5_FanSpeed_Get(int adapterIndex, int thermalCtrlIndex,
                ADLFanSpeedValue* fanSpeedValue) const
{
    int error = pADL_Overdrive5_FanSpeed_Get(adapterIndex, thermalCtrlIndex,
                    fanSpeedValue);
    if (error != ADL_OK)
        throw Error(error, "ADL_Overdrive5_FanSpeed_Get error");
}

void ATIADLHandle::Overdrive5_ODParameters_Get(int adapterIndex,
                ADLODParameters* odParameters) const
{
    int error = pADL_Overdrive5_ODParameters_Get(adapterIndex, odParameters);
    if (error != ADL_OK)
        throw Error(error, "ADL_Overdrive5_ODParameters_Get error");
}

void ATIADLHandle::Overdrive5_ODPerformanceLevels_Get(int adapterIndex, int idefault,
                ADLODPerformanceLevels* odPerformanceLevels) const
{
    int error = pADL_Overdrive5_ODPerformanceLevels_Get(adapterIndex, idefault,
                    odPerformanceLevels);
    if (error != ADL_OK)
        throw Error(error, "ADL_Overdrive5_ODPerformanceLevels_Get error");
}

void ATIADLHandle::Overdrive5_FanSpeed_Set(int adapterIndex, int thermalCtrlIndex,
                ADLFanSpeedValue* fanSpeedValue) const
{
    int error = pADL_Overdrive5_FanSpeed_Set(adapterIndex, thermalCtrlIndex,
                    fanSpeedValue);
    if (error != ADL_OK)
        throw Error(error, "ADL_Overdrive5_FanSpeed_Set error");
}

void ATIADLHandle::Overdrive5_FanSpeedToDefault_Set(int adapterIndex,
                int thermalCtrlIndex) const
{
    int error = pADL_Overdrive5_FanSpeedToDefault_Set(adapterIndex, thermalCtrlIndex);
    if (error != ADL_OK)
        throw Error(error, "ADL_Overdrive5_FanSpeedToDefault_Set error");
}

void ATIADLHandle::Overdrive5_ODPerformanceLevels_Set(int adapterIndex,
                ADLODPerformanceLevels* odPerformanceLevels) const
{
    int error = pADL_Overdrive5_ODPerformanceLevels_Set(adapterIndex, odPerformanceLevels);
    if (error != ADL_OK)
        throw Error(error, "ADL_Overdrive5_ODPerformanceLevels_Set error");
}

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
    void getFanSpeedInfo(int adapterIndex, int thermalCtrlIndex,
            ADLFanSpeedInfo& info) const;
    int getFanSpeed(int adapterIndex, int thermalCtrlIndex) const;
    void getODParameters(int adapterIndex, ADLODParameters& odParameters) const;
    void getODPerformanceLevels(int adapterIndex, bool isDefault, int perfLevelsNum,
            ADLODPerformanceLevel* perfLevels) const;
    void setFanSpeed(int adapterIndex, int thermalCtrlIndex, int fanSpeed) const;
    void setFanSpeedToDefault(int adapterIndex, int thermalCtrlIndex) const;
    void setODPerformanceLevels(int adapterIndex, int perfLevelsNum,
            ADLODPerformanceLevel* perfLevels) const;
};

ADLMainControl::ADLMainControl(const ATIADLHandle& _handle, int devId)
try : handle(_handle), fd(-1), mainControlCreated(false), withX(true)
{
    try
    { handle.Main_Control_Create(ADL_Main_Memory_Alloc, 0); }
    catch(const Error& error)
    {
        if (getuid()!=0)
            std::cout << "IMPORTANT: This program requires root privileges to be "
                    "working correctly\nif no running X11 server." << std::endl;
    
        withX = false;
        char devName[64];
        snprintf(devName, 64, "/dev/ati/card%u", devId);
        errno = 0;
        fd = open(devName, O_RDWR);
        if (fd==-1)
        {
            cl_uint platformsNum;
            /// force initializing these stupid devices
            clGetPlatformIDs(0, nullptr, &platformsNum);
            errno = 0;
            fd = open(devName, O_RDWR);
            if (fd==-1)
                throw Error(errno, "Can't open GPU device");
        }
        
        handle.ConsoleMode_FileDescriptor_Set(fd);
        handle.Main_Control_Create(ADL_Main_Memory_Alloc, 0);
    }
}
catch(...)
{
    if (mainControlCreated)
        handle.Main_Control_Destroy();
    if (fd!=-1)
        close(fd);
    throw;
}

ADLMainControl::~ADLMainControl()
{
    if (fd!=-1)
        close(fd);
}

int ADLMainControl::getAdaptersNum() const
{
    int num = 0;
    handle.Adapter_NumberOfAdapters_Get(&num);
    return num;
}

bool ADLMainControl::isAdapterActive(int adapterIndex) const
{
    if (!withX) return true;
    int status = 0;
    handle.Adapter_Active_Get(adapterIndex, &status);
    return status == ADL_TRUE;
}

void ADLMainControl::getAdapterInfo(AdapterInfo* infos) const
{
    int num;
    handle.Adapter_NumberOfAdapters_Get(&num);
    for (int i = 0; i < num; i++)
        infos[i].iSize = sizeof(AdapterInfo);
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

void ADLMainControl::getFanSpeedInfo(int adapterIndex, int thermalCtrlIndex,
                ADLFanSpeedInfo& info) const
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

void ADLMainControl::getODPerformanceLevels(int adapterIndex, bool isDefault,
            int perfLevelsNum, ADLODPerformanceLevel* perfLevels) const
{
    const size_t odPLBufSize = sizeof(ADLODPerformanceLevels)+
                    sizeof(ADLODPerformanceLevel)*(perfLevelsNum-1);
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

void ADLMainControl::setODPerformanceLevels(int adapterIndex, int perfLevelsNum,
            ADLODPerformanceLevel* perfLevels) const
{
    const size_t odPLBufSize = sizeof(ADLODPerformanceLevels)+
                    sizeof(ADLODPerformanceLevel)*(perfLevelsNum-1);
    std::unique_ptr<char[]> odPlBuf(new char[odPLBufSize]);
    ADLODPerformanceLevels* odPLevels = (ADLODPerformanceLevels*)odPlBuf.get();
    odPLevels->iSize = odPLBufSize;
    odPLevels->iReserved = 0;
    std::copy(perfLevels, perfLevels+perfLevelsNum, odPLevels->aLevels);
    handle.Overdrive5_ODPerformanceLevels_Set(adapterIndex, odPLevels);
}
#endif

/*
 * AMD-GPU infos
 */

enum class AMDGPUPerfControl {
    AUTO = 0,
    LOW,
    MANUAL,
    HIGH,
    UNKNOWN
};

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
    AMDGPUPerfControl perfControl;
    unsigned int temperature;
    unsigned int temperature2;
    unsigned int temperature3;
    unsigned int tempCritical;
    unsigned int busLanes;
    unsigned int busSpeed;
    int gpuLoad;
    unsigned int extraTemperatures;
};

static pci_access* pciAccess = nullptr;
static pci_filter pciFilter;

static void pciAccessError(char* msg, ...)
{
    va_list ap;
    va_start(ap, msg);
    vprintf(msg, ap);
    va_end(ap);
    exit(-1);
}

static void initializePCIAccess()
{
    pciAccess = pci_alloc();
    if (pciAccess==nullptr)
        throw Error("Can't allocate PCIAccess");
    pciAccess->error = pciAccessError;
    pci_filter_init(pciAccess, &pciFilter);
    pci_init(pciAccess);
    pci_scan_bus(pciAccess);
}

#ifdef HAVE_ADLSDK
static void getFromPCI(int deviceIndex, AdapterInfo& adapterInfo)
{
    if (pciAccess==nullptr)
        initializePCIAccess();
    char fnameBuf[64];
    snprintf(fnameBuf, 64, "/proc/ati/%u/name", deviceIndex);
    std::string tmp, pciBusStr;
    {
        std::ifstream procNameIs(fnameBuf);
        procNameIs.exceptions(std::ios::badbit|std::ios::failbit);
        procNameIs >> tmp >> tmp >> pciBusStr;
    }
    unsigned int busNum, devNum, funcNum;
    if (pciBusStr.size() < 9)
        throw Error("Wrong PCI Bus string");
    char* pciStrPtr = (char*)pciBusStr.data()+4;
    char* pciStrPtrNew;
    errno  = 0;
    busNum = strtoul(pciStrPtr, &pciStrPtrNew, 10);
    if (errno!=0 || pciStrPtr==pciStrPtrNew)
        throw Error(errno, "Can't parse BusID");
    pciStrPtr = pciStrPtrNew+1;
    errno  = 0;
    devNum = strtoul(pciStrPtr, &pciStrPtrNew, 10);
    if (errno!=0 || pciStrPtr==pciStrPtrNew)
        throw Error(errno, "Can't parse DevID");
    pciStrPtr = pciStrPtrNew+1;
    errno  = 0;
    funcNum = strtoul(pciStrPtr, &pciStrPtrNew, 10);
    if (errno!=0 || pciStrPtr==pciStrPtrNew)
        throw Error(errno, "Can't parse FuncID");
    pci_dev* dev = pciAccess->devices;
    for (; dev!=nullptr; dev=dev->next)
        if (dev->bus==busNum && dev->dev==devNum && dev->func==funcNum)
        {
            char deviceBuf[128];
            deviceBuf[0] = 0;
            pci_lookup_name(pciAccess, deviceBuf, 128, PCI_LOOKUP_DEVICE,
                    dev->vendor_id, dev->device_id);
            adapterInfo.iBusNumber = busNum;
            adapterInfo.iDeviceNumber = devNum;
            adapterInfo.iFunctionNumber = funcNum;
            adapterInfo.iVendorID = dev->vendor_id;
            strcpy(adapterInfo.strAdapterName, deviceBuf);
            break;
        }
}
#endif

/* AMDGPU code */

static void getFromPCI_AMDGPU(const char* rlink, AMDGPUAdapterInfo& adapterInfo)
{
    if (pciAccess==nullptr)
        initializePCIAccess();
    unsigned int busNum, devNum, funcNum;
    size_t rlinkLen = strlen(rlink);
    if (rlinkLen < 18 || ::strncmp(rlink, "../../../", 9)!=0)
        throw Error("Wrong PCI Bus string");
    char* pciStrPtr = (char*)rlink+9;
    char* pciStrPtrNew;
    while (isdigit(*pciStrPtr)) pciStrPtr++;
    if (*pciStrPtr!=':')
        throw Error(errno, "Can't parse PCI location");
    pciStrPtr++;
    errno  = 0;
    busNum = strtoul(pciStrPtr, &pciStrPtrNew, 16);
    if (errno!=0 || pciStrPtr==pciStrPtrNew)
        throw Error(errno, "Can't parse BusID");
    pciStrPtr = pciStrPtrNew+1;
    errno  = 0;
    devNum = strtoul(pciStrPtr, &pciStrPtrNew, 16);
    if (errno!=0 || pciStrPtr==pciStrPtrNew)
        throw Error(errno, "Can't parse DevID");
    pciStrPtr = pciStrPtrNew+1;
    errno  = 0;
    funcNum = strtoul(pciStrPtr, &pciStrPtrNew, 16);
    if (errno!=0 || pciStrPtr==pciStrPtrNew)
        throw Error(errno, "Can't parse FuncID");
    pci_dev* dev = pciAccess->devices;
    for (; dev!=nullptr; dev=dev->next)
        if (dev->bus==busNum && dev->dev==devNum && dev->func==funcNum)
        {
            char deviceBuf[128];
            deviceBuf[0] = 0;
            pci_lookup_name(pciAccess, deviceBuf, 128, PCI_LOOKUP_DEVICE,
                    dev->vendor_id, dev->device_id);
            adapterInfo.busNo = busNum;
            adapterInfo.deviceNo  = devNum;
            adapterInfo.funcNo = funcNum;
            adapterInfo.vendorId = dev->vendor_id;
            adapterInfo.deviceId = dev->device_id;
            adapterInfo.name = deviceBuf;
            break;
        }
}


class AMDGPUAdapterHandle
{
private:
    unsigned int totDeviceCount;
    std::vector<uint32_t> amdDevices;
    std::vector<uint32_t> hwmonIndices;
public:
    AMDGPUAdapterHandle();
    unsigned int getAdaptersNum() const
    { return amdDevices.size(); }
    AMDGPUAdapterInfo parseAdapterInfo(int index);
    
    void setFanSpeed(int index, int fanSpeed) const;
    void setFanSpeedToDefault(int adapterIndex) const;
    void setOverdriveCoreParam(int adapterIndex, unsigned int coreOD) const;
    void setOverdriveMemoryParam(int adapterIndex, unsigned int memoryOD) const;
    void setPerfCoreLevel(int adapterIndex, unsigned int corePLevel) const;
    void setPerfMemoryLevel(int adapterIndex, unsigned int memoryPLevel) const;
    void setPerformanceControl(int adapterIndex, AMDGPUPerfControl perfControl) const;
    void getPerformanceClocks(int adapterIndex, unsigned int& coreClock,
                    unsigned int& memoryClock) const;
};

static bool getFileContentValue(const char* filename, unsigned int& value)
try
{
    value = 0;
    std::ifstream ifs(filename, std::ios::binary);
    ifs.exceptions(std::ios::failbit);
    std::string line;
    std::getline(ifs, line);
    char* p = (char*)line.c_str();
    char* p2;
    errno = 0;
    value = strtoul(p, &p2, 0);
    if (errno != 0)
        throw Error("Can't parse value from file");
    return (p != p2);
}
catch(const std::exception& ex)
{
    return false;
}

static void writeFileContentValue(const char* filename, unsigned int value)
{
    if (access(filename, F_OK|W_OK))
    {
        throw Error((std::string("Can't write file '")+filename+"'").c_str());
    }
    std::ofstream ofs(filename, std::ios::binary);
    try
    {
        ofs.exceptions(std::ios::failbit);
        ofs << value << std::endl;
    }
    catch(const std::exception& ex)
    {
        throw Error((std::string("Can't write file '")+filename+"'").c_str());
    }
}

static void writeFileContent(const char* filename, const char* content)
{
    if (access(filename, F_OK|W_OK))
    {
        throw Error((std::string("Can't write file '")+filename+"'").c_str());
    }
    std::ofstream ofs(filename, std::ios::binary);
    try
    {
        ofs.exceptions(std::ios::failbit);
        ofs << content << std::endl;
    }
    catch(const std::exception& ex)
    {
        throw Error((std::string("Can't write file '")+filename+"'").c_str());
    }
}

AMDGPUAdapterHandle::AMDGPUAdapterHandle()
        : totDeviceCount(0)
{
    errno = 0;
    DIR* dirp = opendir("/sys/class/drm");
    if (dirp == nullptr)
        throw Error(errno, "Can't open 'sys/class/drm' directory");
    errno = 0;
    struct dirent* dire;
    while ((dire = readdir(dirp)) != nullptr)
    {
        if (::strncmp(dire->d_name, "card", 4) != 0)
            continue; // is not card directory
        const char* p;
        for (p = dire->d_name + 4; ::isdigit(*p); p++);
        if (*p != 0)
            continue; // is not card directory
        errno = 0;
        unsigned int v = ::strtoul(dire->d_name + 4, nullptr, 10);
        totDeviceCount = std::max(totDeviceCount, v+1);
    }
    if (errno != 0)
    {
        closedir(dirp);
        throw Error(errno, "Can't read 'sys/class/drm' directory");
    }
    closedir(dirp);
    
    // filter AMD GPU cards
    char dbuf[120];
    for (unsigned int i = 0; i < totDeviceCount; i++)
    {
        snprintf(dbuf, 120, "/sys/class/drm/card%u/device/vendor", i);
        unsigned int vendorId = 0;
        if (!getFileContentValue(dbuf, vendorId))
            continue;
        if (vendorId != 4098) // if not AMD
            continue;
        amdDevices.push_back(i);
    }
    
    /* hwmon indices */
    for (unsigned int cardIndex: amdDevices)
    {
        // search hwmon
        errno = 0;
        snprintf(dbuf, 120, "/sys/class/drm/card%u/device/hwmon", cardIndex);
        DIR* dirp = opendir(dbuf);
        if (dirp == nullptr)
            throw Error(errno, "Can't open 'sys/class/drm/card?/device/hwmon' directory");
        errno = 0;
        struct dirent* dire;
        unsigned int hwmonIndex = UINT_MAX;
        while ((dire = readdir(dirp)) != nullptr)
        {
            if (::strncmp(dire->d_name, "hwmon", 5) != 0)
                continue; // is not hwmon directory
            const char* p;
            for (p = dire->d_name + 5; ::isdigit(*p); p++);
            if (*p != 0)
                continue; // is not hwmon directory
            errno = 0;
            unsigned int v = ::strtoul(dire->d_name + 5, nullptr, 10);
            hwmonIndex = std::min(hwmonIndex, v);
        }
        if (errno != 0)
        {
            closedir(dirp);
            throw Error(errno, "Can't open 'sys/class/drm/card?/hwmon' directory");
        }
        closedir(dirp);
        if (hwmonIndex == UINT_MAX)
            throw Error("Can't find hwmon? directory");
        hwmonIndices.push_back(hwmonIndex);
    }
}

static std::vector<unsigned int> parseDPMFile(const char* filename, uint32_t& choosen)
{
    std::vector<uint32_t> out;
    std::ifstream ifs(filename, std::ios::binary);
    choosen = UINT32_MAX;
    while (ifs)
    {
        std::string line;
        std::getline(ifs, line);
        if (line.empty())
            break;
        char* p = (char*)line.c_str();
        char* p2 = (char*)line.c_str();
        errno = 0;
        unsigned int index = strtoul(p, &p2, 10);
        if (errno!=0 || p==p2)
            throw Error(errno, "Can't parse index");
        p = p2;
        if (*p!=':' || p[1]!=' ')
            throw Error(errno, "Can't parse next part of line");
        p += 2;
        unsigned int clock = strtoul(p, &p2, 10);
        if (errno!=0 || p==p2)
            throw Error(errno, "Can't parse clock");
        p = p2;
        if (::strncmp(p, "Mhz", 3) != 0)
            throw Error(errno, "Can't parse next part of line");
        p += 3;
        if (*p==' ' && p[1]=='*')
            choosen = index;
        out.resize(index+1);
        out[index] = clock;
    }
    return out;
}

static AMDGPUPerfControl parsePerfControl(const char* filename)
try
{
    std::ifstream ifs(filename, std::ios::binary);
    ifs.exceptions(std::ios::failbit);
    std::string line;
    std::getline(ifs, line);
    if (line=="auto")
        return AMDGPUPerfControl::AUTO;
    else if (line=="low")
        return AMDGPUPerfControl::LOW;
    else if (line=="high")
        return AMDGPUPerfControl::HIGH;
    else if (line=="manual")
        return AMDGPUPerfControl::MANUAL;
    else
        return AMDGPUPerfControl::UNKNOWN;
}
catch(const std::exception& ex)
{
    return AMDGPUPerfControl::UNKNOWN;
}

static void parseDPMPCIEFile(const char* filename, unsigned int& pcieMB, unsigned int& lanes)
{
    std::ifstream ifs(filename, std::ios::binary);
    unsigned int ilanes = 0, ipcieMB = 0;
    while (ifs)
    {
        std::string line;
        std::getline(ifs, line);
        if (line.empty())
            break;
        char* p = (char*)line.c_str();
        char* p2 = (char*)line.c_str();
        errno = 0;
        strtoul(p, &p2, 10);
        if (errno!=0 || p==p2)
            throw Error(errno, "Can't parse index");
        p = p2;
        if (*p!=':' || p[1]!=' ')
            throw Error(errno, "Can't parse next part of line");
        p += 2;
        double bandwidth = strtod(p, &p2);
        if (errno!=0 || p==p2)
            throw Error(errno, "Can't parse bandwidth");
        p = p2;
        if (*p=='G' && p2[1]=='B')
            ipcieMB = bandwidth*1000;
        else if (*p=='M' && p2[1]=='B')
            ipcieMB = bandwidth;
        else if (*p=='M' && p2[1]=='B')
            ipcieMB = bandwidth/1000;
        else 
            throw Error(errno, "Wrong bandwidth specifier");
        p += 2;
        if (::strncmp(p, ", x", 3)!=0)
            throw Error(errno, "Can't parse next part of line");
        errno = 0;
        ilanes = strtoul(p, &p2, 10);
        if (errno!=0 || p==p2)
            throw Error(errno, "Can't parse lanes");
        if (*p==' ' && p[1]=='*')
        {
            lanes = ilanes;
            pcieMB = ipcieMB;
            break;
        }
    }
}

void AMDGPUAdapterHandle::getPerformanceClocks(int adapterIndex, unsigned int& coreClock,
                    unsigned int& memoryClock) const
{
    char dbuf[120];
    unsigned int cardIndex = amdDevices[adapterIndex];
    
    unsigned int coreOD = 0;
    snprintf(dbuf, 120, "/sys/class/drm/card%u/device/pp_sclk_od", cardIndex);
    getFileContentValue(dbuf, coreOD);
    unsigned int memoryOD = 0;
    snprintf(dbuf, 120, "/sys/class/drm/card%u/device/pp_mclk_od", cardIndex);
    getFileContentValue(dbuf, memoryOD);
    
    snprintf(dbuf, 120, "/sys/class/drm/card%u/device/pp_dpm_sclk", cardIndex);
    unsigned int activeClockIndex;
    std::vector<unsigned int> clocks = parseDPMFile(dbuf, activeClockIndex);
    coreClock = 0;
    if (!clocks.empty())
        coreClock = int(ceil(double(clocks.back()) / (1.0 + coreOD*0.01)));
    snprintf(dbuf, 120, "/sys/class/drm/card%u/device/pp_dpm_mclk", cardIndex);
    clocks = parseDPMFile(dbuf, activeClockIndex);
    memoryClock = 0;
    if (!clocks.empty())
        memoryClock = int(ceil(double(clocks.back()) / (1.0 + memoryOD*0.01)));
}

AMDGPUAdapterInfo AMDGPUAdapterHandle::parseAdapterInfo(int index)
{
    AMDGPUAdapterInfo adapterInfo{};
    unsigned int cardIndex = amdDevices[index];
    char dbuf[120];
    char rlink[120];
    snprintf(dbuf, 120, "/sys/class/drm/card%u/device", cardIndex);
    auto linkRead = ::readlink(dbuf, rlink, sizeof(rlink) - 1);
    if (linkRead < 0) {
//         throw Error(errno, "Can't readlink 'sys/class/drm/card?/device'");
    }
    rlink[linkRead] = 0;
    getFromPCI_AMDGPU(rlink, adapterInfo);
    // parse pp_dpm_sclk
    snprintf(dbuf, 120, "/sys/class/drm/card%u/device/pp_dpm_sclk", cardIndex);
    unsigned int activeCoreClockIndex;
    adapterInfo.coreClocks = parseDPMFile(dbuf, activeCoreClockIndex);
    if (activeCoreClockIndex!=UINT_MAX)
      adapterInfo.coreClock = adapterInfo.coreClocks[activeCoreClockIndex];
    else
      adapterInfo.coreClock = 0;
    // parse pp_dpm_mclk
    snprintf(dbuf, 120, "/sys/class/drm/card%u/device/pp_dpm_mclk", cardIndex);
    unsigned int activeMemoryClockIndex;
    adapterInfo.memoryClocks = parseDPMFile(dbuf, activeMemoryClockIndex);
    if (activeMemoryClockIndex!=UINT_MAX)
      adapterInfo.memoryClock = adapterInfo.memoryClocks[activeMemoryClockIndex];
    else
      adapterInfo.memoryClock = 0;
    
    unsigned int hwmonIndex = hwmonIndices[index];
    
    snprintf(dbuf, 120, "/sys/class/drm/card%u/device/pp_sclk_od", cardIndex);
    getFileContentValue(dbuf, adapterInfo.coreOD);
    snprintf(dbuf, 120, "/sys/class/drm/card%u/device/pp_mclk_od", cardIndex);
    getFileContentValue(dbuf, adapterInfo.memoryOD);
    // get fanspeed
    snprintf(dbuf, 120, "/sys/class/drm/card%u/device/hwmon/hwmon%u/pwm1_min",
             cardIndex, hwmonIndex);
    getFileContentValue(dbuf, adapterInfo.minFanSpeed);
    snprintf(dbuf, 120, "/sys/class/drm/card%u/device/hwmon/hwmon%u/pwm1_max",
             cardIndex, hwmonIndex);
    getFileContentValue(dbuf, adapterInfo.maxFanSpeed);
    snprintf(dbuf, 120, "/sys/class/drm/card%u/device/hwmon/hwmon%u/pwm1",
             cardIndex, hwmonIndex);
    getFileContentValue(dbuf, adapterInfo.fanSpeed);
    snprintf(dbuf, 120, "/sys/class/drm/card%u/device/hwmon/hwmon%u/pwm1_enable",
             cardIndex, hwmonIndex);
    unsigned int pwmEnable = 0;
    getFileContentValue(dbuf, pwmEnable);
    adapterInfo.defaultFanSpeed = pwmEnable==2;
    snprintf(dbuf, 120, "/sys/class/drm/card%u/device/hwmon/hwmon%u/temp1_input",
             cardIndex, hwmonIndex);
    getFileContentValue(dbuf, adapterInfo.temperature);
    snprintf(dbuf, 120, "/sys/class/drm/card%u/device/hwmon/hwmon%u/temp1_crit",
             cardIndex, hwmonIndex);
    getFileContentValue(dbuf, adapterInfo.tempCritical);
    
    snprintf(dbuf, 120, "/sys/class/drm/card%u/device/hwmon/hwmon%u/temp2_input",
             cardIndex, hwmonIndex);
    if (getFileContentValue(dbuf, adapterInfo.temperature2))
        adapterInfo.extraTemperatures |= 1;
    snprintf(dbuf, 120, "/sys/class/drm/card%u/device/hwmon/hwmon%u/temp3_input",
             cardIndex, hwmonIndex);
    if (getFileContentValue(dbuf, adapterInfo.temperature3))
        adapterInfo.extraTemperatures |= 2;
    
    // parse GPU load
    snprintf(dbuf, 120, "/sys/kernel/debug/dri/%u/amdgpu_pm_info", cardIndex);
    {
        adapterInfo.gpuLoad = -1;
        std::ifstream ifs(dbuf, std::ios::binary);
        while (ifs)
        {
            std::string line;
            std::getline(ifs, line);
            if (line.compare(0, 10, "GPU load: ")==0 ||
                line.compare(0, 10, "GPU Load: ")==0)
            {
                errno = 0;
                char* endp;
                adapterInfo.gpuLoad = strtoul(line.c_str()+10, &endp, 10);
                if (errno != 0 || endp == line.c_str()+10)
                    throw Error("Can't parse GPU load");
                break;
            }
        }
    }
    snprintf(dbuf, 120, "/sys/class/drm/card%u/pp_dpm_pcie", cardIndex);
    parseDPMPCIEFile(dbuf, adapterInfo.busLanes, adapterInfo.busSpeed);
    
    snprintf(dbuf, 120, "/sys/class/drm/card%u/device/power_dpm_force_performance_level",
             cardIndex);
    adapterInfo.perfControl = parsePerfControl(dbuf);
    return adapterInfo;
}

void AMDGPUAdapterHandle::setFanSpeed(int index, int fanSpeed) const
{
    char dbuf[120];
    unsigned int cardIndex = amdDevices[index];
    unsigned int hwmonIndex = hwmonIndices[index];
    snprintf(dbuf, 120, "/sys/class/drm/card%u/device/hwmon/hwmon%u/pwm1_enable",
             cardIndex, hwmonIndex);
    writeFileContentValue(dbuf, 1);
    
    unsigned int minFanSpeed, maxFanSpeed;
    snprintf(dbuf, 120, "/sys/class/drm/card%u/device/hwmon/hwmon%u/pwm1_min",
             cardIndex, hwmonIndex);
    getFileContentValue(dbuf, minFanSpeed);
    snprintf(dbuf, 120, "/sys/class/drm/card%u/device/hwmon/hwmon%u/pwm1_max",
             cardIndex, hwmonIndex);
    getFileContentValue(dbuf, maxFanSpeed);
    
    snprintf(dbuf, 120, "/sys/class/drm/card%u/device/hwmon/hwmon%u/pwm1",
             cardIndex, hwmonIndex);
    writeFileContentValue(dbuf, int(round(
            fanSpeed/100.0 * (maxFanSpeed-minFanSpeed) + minFanSpeed)));
}

void AMDGPUAdapterHandle::setFanSpeedToDefault(int index) const
{
    char dbuf[120];
    unsigned int cardIndex = amdDevices[index];
    unsigned int hwmonIndex = hwmonIndices[index];
    snprintf(dbuf, 120, "/sys/class/drm/card%u/device/hwmon/hwmon%u/pwm1_enable",
             cardIndex, hwmonIndex);
    writeFileContentValue(dbuf, 2);
}

void AMDGPUAdapterHandle::setOverdriveCoreParam(int index, unsigned int coreOD) const
{
    char dbuf[120];
    unsigned int cardIndex = amdDevices[index];
    snprintf(dbuf, 120, "/sys/class/drm/card%u/device/pp_sclk_od", cardIndex);
    writeFileContentValue(dbuf, coreOD);
}

void AMDGPUAdapterHandle::setOverdriveMemoryParam(int index, unsigned int memoryOD) const
{
    char dbuf[120];
    unsigned int cardIndex = amdDevices[index];
    snprintf(dbuf, 120, "/sys/class/drm/card%u/device/pp_mclk_od", cardIndex);
    writeFileContentValue(dbuf, memoryOD);
}

void AMDGPUAdapterHandle::setPerfCoreLevel(int index, unsigned int corePLevel) const
{
    char dbuf[120];
    unsigned int cardIndex = amdDevices[index];
    snprintf(dbuf, 120, "/sys/class/drm/card%u/device/pp_dpm_sclk", cardIndex);
    
    if (access(dbuf, F_OK))
    {
        throw Error((std::string("File '")+dbuf+"' doesn't exist").c_str());
    }
    
    setPerformanceControl(index, AMDGPUPerfControl::MANUAL);
    writeFileContentValue(dbuf, corePLevel);
}

void AMDGPUAdapterHandle::setPerfMemoryLevel(int index, unsigned int memoryPLevel) const
{
    char dbuf[120];
    unsigned int cardIndex = amdDevices[index];
    snprintf(dbuf, 120, "/sys/class/drm/card%u/device/pp_dpm_mclk", cardIndex);
    
    if (access(dbuf, F_OK))
    {
        throw Error((std::string("File '")+dbuf+"' doesn't exist").c_str());
    }
    
    setPerformanceControl(index, AMDGPUPerfControl::MANUAL);
    writeFileContentValue(dbuf, memoryPLevel);
}

static const char* perfControlNames[] =
{
    "auto", "low", "manual", "high", "unknown"
};

void AMDGPUAdapterHandle::setPerformanceControl(int index,
                    AMDGPUPerfControl perfControl) const
{
    char dbuf[120];
    unsigned int cardIndex = amdDevices[index];
    snprintf(dbuf, 120, "/sys/class/drm/card%u/device/power_dpm_force_performance_level",
             cardIndex);
    writeFileContent(dbuf, perfControlNames[int(perfControl)]);
}

static void printAdaptersInfo(AMDGPUAdapterHandle& handle,
            const std::vector<int>& choosenAdapters, bool useChoosen)
{
    int adaptersNum = handle.getAdaptersNum();
    auto choosenIter = choosenAdapters.begin();
    int i = 0;
    for (int ai = 0; ai < adaptersNum; ai++)
    {
        if (useChoosen && (choosenIter==choosenAdapters.end() || *choosenIter!=i))
        { i++; continue; }
        const AMDGPUAdapterInfo adapterInfo = handle.parseAdapterInfo(ai);
        
        std::cout << "Adapter " << i << ": PCI " <<
                adapterInfo.busNo << ":" << adapterInfo.deviceNo << ":" <<
                adapterInfo.funcNo << ": " << adapterInfo.name << "\n"
                "  Core: " << adapterInfo.coreClock << " MHz, "
                "Mem: " << adapterInfo.memoryClock << " MHz, "
                "CoreOD: " << adapterInfo.coreOD << ", "
                "MemOD: " << adapterInfo.memoryOD << "\n  "
                "PerfCtrl: " << perfControlNames[int(adapterInfo.perfControl)] << ", ";
        if (adapterInfo.gpuLoad>=0)
            std::cout << "Load: " << adapterInfo.gpuLoad << "%, ";
        std::cout << "Temp: " << adapterInfo.temperature/1000.0 << " C";
        if ((adapterInfo.extraTemperatures&1) != 0)
            std::cout << ", T2: " << adapterInfo.temperature2/1000.0 << " C";
        if ((adapterInfo.extraTemperatures&2) != 0)
            std::cout << ", T3: " << adapterInfo.temperature3/1000.0 << " C";
        std::cout << ", Fan: " << double(adapterInfo.fanSpeed-adapterInfo.minFanSpeed)/
                double(adapterInfo.maxFanSpeed-adapterInfo.minFanSpeed)*100.0 <<
                "%" << std::endl;
        if (!adapterInfo.coreClocks.empty())
        {
            std::cout << "  Core Clocks:";
            for (uint32_t v: adapterInfo.coreClocks)
                std::cout << " " << v;
            std::cout << std::endl;
        }
        if (!adapterInfo.memoryClocks.empty())
        {
            std::cout << "  Memory Clocks:";
            for (uint32_t v: adapterInfo.memoryClocks)
                std::cout << " " << v;
            std::cout << std::endl;
        }
        if (useChoosen)
            ++choosenIter;
        i++;
    }
}

static void printAdaptersInfoVerbose(AMDGPUAdapterHandle& handle,
            const std::vector<int>& choosenAdapters, bool useChoosen)
{
    int adaptersNum = handle.getAdaptersNum();
    auto choosenIter = choosenAdapters.begin();
    int i = 0;
    for (int ai = 0; ai < adaptersNum; ai++)
    {
        if (useChoosen && (choosenIter==choosenAdapters.end() || *choosenIter!=i))
        { i++; continue; }
        const AMDGPUAdapterInfo adapterInfo = handle.parseAdapterInfo(ai);
        
        std::cout << "Adapter " << i << ": " << adapterInfo.name << "\n"
                "  Device Topology: " << adapterInfo.busNo << ":" <<
                adapterInfo.deviceNo << ":" << adapterInfo.funcNo << "\n"
                "  Vendor ID: " << adapterInfo.vendorId << " (0x" << std::hex <<
                        adapterInfo.vendorId << std::dec << ")" << "\n"
                "  Device ID: " << adapterInfo.deviceId << " (0x" << std::hex <<
                        adapterInfo.deviceId << std::dec << ")" << "\n"
                "  Current CoreClock: " << adapterInfo.coreClock << " MHz\n"
                "  Current MemoryClock: " << adapterInfo.memoryClock << " MHz\n"
                "  Core Overdrive: " << adapterInfo.coreOD << "\n"
                "  Memory Overdrive: " << adapterInfo.memoryOD << "\n"
                "  Performance Control: " <<
                    perfControlNames[int(adapterInfo.perfControl)] << "\n";
        if (adapterInfo.gpuLoad>=0)
            std::cout << "  GPU Load: " << adapterInfo.gpuLoad << "%\n";
        std::cout << "  Current BusSpeed: " << adapterInfo.busSpeed << "\n"
                "  Current BusLanes: " << adapterInfo.busLanes << "\n"
                "  Temperature: " << adapterInfo.temperature/1000.0 << " C\n";
        if ((adapterInfo.extraTemperatures&1) != 0)
            std::cout << "  Temperature2: " << adapterInfo.temperature2/1000.0 << " C\n";
        if ((adapterInfo.extraTemperatures&2) != 0)
            std::cout << "  Temperature3: " << adapterInfo.temperature3/1000.0 << " C\n";
        std::cout <<
                "  Critical temperature: " << adapterInfo.tempCritical/1000.0 << " C\n"
                "  FanSpeed Min (Value): " << adapterInfo.minFanSpeed << "\n"
                "  FanSpeed Max (Value): " << adapterInfo.maxFanSpeed << "\n"
                "  Current FanSpeed: " <<
                    (double(adapterInfo.fanSpeed-adapterInfo.minFanSpeed)/
                    double(adapterInfo.maxFanSpeed-adapterInfo.minFanSpeed)*100.0)
                    << "%\n"
                "  Controlled FanSpeed: " <<
                    (adapterInfo.defaultFanSpeed?"yes":"no") << "\n";
            // print available core clocks
        if (!adapterInfo.coreClocks.empty())
        {
            std::cout << "  Core Clocks:\n";
            for (uint32_t v: adapterInfo.coreClocks)
                std::cout << "    " << v << "MHz\n";
        }
        if (!adapterInfo.memoryClocks.empty())
        {
            std::cout << "  Memory Clocks:\n";
            for (uint32_t v: adapterInfo.memoryClocks)
                std::cout << "    " << v << "MHz\n";
        }
        if (useChoosen)
            ++choosenIter;
        i++;
    }
}

/* end of AMDGPU code */

#ifdef HAVE_ADLSDK
static void getActiveAdaptersIndices(ADLMainControl& mainControl, int adaptersNum,
                    std::vector<int>& activeAdapters)
{
    activeAdapters.clear();
    for (int i = 0; i < adaptersNum; i++)
        if (mainControl.isAdapterActive(i))
            activeAdapters.push_back(i);
}

static void printAdaptersInfo(ADLMainControl& mainControl, int adaptersNum,
            const std::vector<int>& activeAdapters,
            const std::vector<int>& choosenAdapters, bool useChoosen)
{
    std::unique_ptr<AdapterInfo[]> adapterInfos(new AdapterInfo[adaptersNum]);
    ::memset(adapterInfos.get(), 0, sizeof(AdapterInfo)*adaptersNum);
    mainControl.getAdapterInfo(adapterInfos.get());
    int i = 0;
    auto choosenIter = choosenAdapters.begin();
    for (int ai = 0; ai < adaptersNum; ai++)
    {
        if (!mainControl.isAdapterActive(ai))
            continue;
        if (useChoosen && (choosenIter==choosenAdapters.end() || *choosenIter!=i))
        { i++; continue; }
        
        if (adapterInfos[ai].strAdapterName[0]==0)
            getFromPCI(adapterInfos[ai].iAdapterIndex, adapterInfos[ai]);
        
        ADLPMActivity activity;
        mainControl.getCurrentActivity(ai, activity);
        std::cout << "Adapter " << i << ": PCI " <<
                adapterInfos[ai].iBusNumber << ":" <<
                adapterInfos[ai].iDeviceNumber << ":" <<
                adapterInfos[ai].iFunctionNumber <<
                ": " << adapterInfos[ai].strAdapterName << "\n"
                "  Core: " << activity.iEngineClock/100.0 << " MHz, "
                "Mem: " << activity.iMemoryClock/100.0 << " MHz, "
                "Vddc: " << activity.iVddc/1000.0 << " V, "
                "Load: " << activity.iActivityPercent << "%, "
                "Temp: " << mainControl.getTemperature(ai, 0)/1000.0 << " C, "
                "Fan: " << mainControl.getFanSpeed(ai, 0) << "%" << std::endl;
        ADLODParameters odParams;
        mainControl.getODParameters(ai, odParams);
        std::cout << "  Max Ranges: Core: " << odParams.sEngineClock.iMin/100.0 << " - " <<
            odParams.sEngineClock.iMax/100.0 << " MHz, "
            "Mem: " << odParams.sMemoryClock.iMin/100.0 << " - " <<
                odParams.sMemoryClock.iMax/100.0 << " MHz, " <<
            "Vddc: " <<  odParams.sVddc.iMin/1000.0 << " - " <<
                odParams.sVddc.iMax/1000.0 << " V\n";
        int levelsNum = odParams.iNumberOfPerformanceLevels;
        std::unique_ptr<ADLODPerformanceLevel[]> odPLevels(
                new ADLODPerformanceLevel[levelsNum]);
        mainControl.getODPerformanceLevels(ai, false, levelsNum, odPLevels.get());
        std::cout << "  PerfLevels: Core: " << odPLevels[0].iEngineClock/100.0 << " - " <<
            odPLevels[levelsNum-1].iEngineClock/100.0 << " MHz, "
            "Mem: " << odPLevels[0].iMemoryClock/100.0 << " - " <<
            odPLevels[levelsNum-1].iMemoryClock/100.0 << " MHz, "
            "Vddc: " << odPLevels[0].iVddc/1000.0 << " - " <<
            odPLevels[levelsNum-1].iVddc/1000.0 << " V\n";
        if (useChoosen)
            ++choosenIter;
        i++;
    }
}

static void printAdaptersInfoVerbose(ADLMainControl& mainControl, int adaptersNum,
            const std::vector<int>& activeAdapters,
            const std::vector<int>& choosenAdapters, bool useChoosen)
{
    std::unique_ptr<AdapterInfo[]> adapterInfos(new AdapterInfo[adaptersNum]);
    ::memset(adapterInfos.get(), 0, sizeof(AdapterInfo)*adaptersNum);
    mainControl.getAdapterInfo(adapterInfos.get());
    int i = 0;
    auto choosenIter = choosenAdapters.begin();
    for (int ai = 0; ai < adaptersNum; ai++)
    {
        if (!mainControl.isAdapterActive(ai))
            continue;
        if (useChoosen && (choosenIter==choosenAdapters.end() || *choosenIter!=i))
        { i++; continue; }
        if (adapterInfos[ai].strAdapterName[0]==0)
            getFromPCI(adapterInfos[ai].iAdapterIndex, adapterInfos[ai]);
        std::cout << "Adapter " << i << ": " << adapterInfos[ai].strAdapterName << "\n"
                "  Device Topology: " << adapterInfos[ai].iBusNumber << ":" <<
                adapterInfos[ai].iDeviceNumber << ":" <<
                adapterInfos[ai].iFunctionNumber << "\n"
                "  Vendor ID: " << adapterInfos[ai].iVendorID << " (0x" << std::hex <<
                        adapterInfos[ai].iVendorID << std::dec << ")" << std::endl;
        ADLFanSpeedInfo fsInfo;
        ADLPMActivity activity;
        mainControl.getCurrentActivity(ai, activity);
        std::cout << "  Current CoreClock: " << activity.iEngineClock/100.0 << " MHz\n"
                "  Current MemoryClock: " << activity.iMemoryClock/100.0 << " MHz\n"
                "  Current Voltage: " << activity.iVddc/1000.0 << " V\n"
                "  GPU Load: " << activity.iActivityPercent << "%\n"
                "  Current PerfLevel: " << activity.iCurrentPerformanceLevel << "\n"
                "  Current BusSpeed: " << activity.iCurrentBusSpeed << "\n"
                "  Current BusLanes: " << activity.iCurrentBusLanes<< "\n";
        
        int temperature = mainControl.getTemperature(ai, 0);
        std::cout << "  Temperature: " << temperature/1000.0 << " C\n";
        mainControl.getFanSpeedInfo(ai, 0, fsInfo);
        std::cout << "  FanSpeed Min: " << fsInfo.iMinPercent << "%\n"
                "  FanSpeed Max: " << fsInfo.iMaxPercent << "%\n"
                "  FanSpeed MinRPM: " << fsInfo.iMinRPM << " RPM\n"
                "  FanSpeed MaxRPM: " << fsInfo.iMaxRPM << " RPM" << "\n";
        std::cout << "  Current FanSpeed: " << mainControl.getFanSpeed(ai, 0) << "%\n";
        ADLODParameters odParams;
        mainControl.getODParameters(ai, odParams);
        std::cout << "  CoreClock: " << odParams.sEngineClock.iMin/100.0 << " - " <<
                odParams.sEngineClock.iMax/100.0 << " MHz, step: " <<
                odParams.sEngineClock.iStep/100.0 << " MHz\n"
                "  MemClock: " << odParams.sMemoryClock.iMin/100.0 << " - " <<
                odParams.sMemoryClock.iMax/100.0 << " MHz, step: " <<
                odParams.sMemoryClock.iStep/100.0 << " MHz\n"
                "  Voltage: " << odParams.sVddc.iMin/1000.0 << " - " <<
                odParams.sVddc.iMax/1000.0 << " V, step: " <<
                odParams.sVddc.iStep/1000.0 << " V\n";
        std::unique_ptr<ADLODPerformanceLevel[]> odPLevels(
                new ADLODPerformanceLevel[odParams.iNumberOfPerformanceLevels]);
        mainControl.getODPerformanceLevels(ai, false, odParams.iNumberOfPerformanceLevels,
                                odPLevels.get());
        std::cout << "  Performance levels: " << odParams.iNumberOfPerformanceLevels << "\n";
        for (int j = 0; j < odParams.iNumberOfPerformanceLevels; j++)
            std::cout << "    Performance Level: " << j << "\n"
                "      CoreClock: " << odPLevels[j].iEngineClock/100.0 << " MHz\n"
                "      MemClock: " << odPLevels[j].iMemoryClock/100.0 << " MHz\n"
                "      Voltage: " << odPLevels[j].iVddc/1000.0 << " V\n";
        mainControl.getODPerformanceLevels(ai, true, odParams.iNumberOfPerformanceLevels,
                                odPLevels.get());
        std::cout << "  Default Performance levels: " <<
                        odParams.iNumberOfPerformanceLevels << "\n";
        for (int j = 0; j < odParams.iNumberOfPerformanceLevels; j++)
            std::cout << "    Performance Level: " << j << "\n"
                "      CoreClock: " << odPLevels[j].iEngineClock/100.0 << " MHz\n"
                "      MemClock: " << odPLevels[j].iMemoryClock/100.0 << " MHz\n"
                "      Voltage: " << odPLevels[j].iVddc/1000.0 << " V\n";
        std::cout.flush();
        if (useChoosen)
            ++choosenIter;
        i++;
    }
}
#endif

static void parseAdaptersList(const char* string, std::vector<int>& adapters,
                              bool& allAdapters)
{
    adapters.clear();
    allAdapters = false;
    if (::strcmp(string, "all")==0)
    {
        allAdapters = true;
        return;
    }
    while (true)
    {
        char* endptr;
        errno = 0;
        int adapterIndex = strtol(string, &endptr, 10);
        if (errno!=0 || endptr==string)
            throw Error("Can't parse adapter index");
        
        string = endptr;
        if (*string == '-')
        {   // if range
            string++;
            errno = 0;
            int adapterIndexEnd = strtol(string, &endptr, 10);
            if (errno!=0 || endptr==string)
                throw Error("Can't parse adapter index");
            string = endptr;
            if (adapterIndex>adapterIndexEnd)
                throw Error("Wrong range of adapter indices in adapter list");
            for (int i = adapterIndex; i <= adapterIndexEnd; i++)
                adapters.push_back(i);
        }
        else
            adapters.push_back(adapterIndex);
        if (*string==0)
            break;
        if (*string==',')
            string++;
        else
            throw Error("Garbages at adapter list");
    }
    std::sort(adapters.begin(), adapters.end());
    adapters.resize(std::unique(adapters.begin(), adapters.end()) - adapters.begin());
}

enum class OVCParamType
{
    CORE_CLOCK,
    MEMORY_CLOCK,
    VDDC_VOLTAGE,
    FAN_SPEED,
    CORE_OD,
    MEMORY_OD,
    CUR_CORE_CLOCK,
    CUR_MEMORY_CLOCK,
    CORE_PERF_LEVEL,
    MEMORY_PERF_LEVEL
};

enum: int {
    LAST_PERFLEVEL = -1
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

static bool parseOVCParameter(const char* string, OVCParameter& param)
{
    const char* afterName = strchr(string, ':');
    if (afterName==nullptr)
    {
        afterName = strchr(string, '=');
        if (afterName==nullptr)
        {
            std::cerr << "This is not parameter: '" << string << "'!" << std::endl;
            return false;
        }
    }
    std::string name(string, afterName);
    param.argText = string;
    param.adapters.clear();
    param.adapters.push_back(0); // default is 0
    param.allAdapters = false;
    param.partId = 0;
    param.useDefault = false;
    bool partIdSet = false;
    if (name=="coreclk")
    {
        param.type = OVCParamType::CORE_CLOCK;
        param.partId = LAST_PERFLEVEL;
    }
    else if (name=="memclk")
    {
        param.type = OVCParamType::MEMORY_CLOCK;
        param.partId = LAST_PERFLEVEL;
    }
    else if (name=="ccoreclk")
    {
        param.type = OVCParamType::CUR_CORE_CLOCK;
        param.partId = LAST_PERFLEVEL;
    }
    else if (name=="cmemclk")
    {
        param.type = OVCParamType::CUR_MEMORY_CLOCK;
        param.partId = LAST_PERFLEVEL;
    }
    else if (name=="coreod")
    {
        param.type = OVCParamType::CORE_OD;
        param.partId = LAST_PERFLEVEL;
    }
    else if (name=="memod")
    {
        param.type = OVCParamType::MEMORY_OD;
        param.partId = LAST_PERFLEVEL;
    }
    else if (name=="corepl")
    {
        param.type = OVCParamType::CORE_PERF_LEVEL;
        param.partId = LAST_PERFLEVEL;
    }
    else if (name=="mempl")
    {
        param.type = OVCParamType::MEMORY_PERF_LEVEL;
        param.partId = LAST_PERFLEVEL;
    }
    else if (name=="vcore")
    {
        param.type = OVCParamType::VDDC_VOLTAGE;
        param.partId = LAST_PERFLEVEL;
    }
    else if (name=="fanspeed")
    {
        param.type = OVCParamType::FAN_SPEED;
        partIdSet = false;
    }
    else if (name=="icoreclk")
    {
        param.type = OVCParamType::CORE_CLOCK;
        partIdSet = true;
    }
    else if (name=="imemclk")
    {
        param.type = OVCParamType::MEMORY_CLOCK;
        partIdSet = true;
    }
    else if (name=="ivcore")
    {
        param.type = OVCParamType::VDDC_VOLTAGE;
        partIdSet = true;
    }
    else
    {
        std::cout << "Wrong parameter name in '" << string << "'!" << std::endl;
        return false;
    }
    
    char* next;
    if (*afterName==':')
    {   // if is
        afterName++;
        try
        {
            const char* afterList = ::strchr(afterName, ':');
            if (afterList==nullptr)
                afterList = ::strchr(afterName, '=');
            if (afterList==nullptr)
                afterList = afterName + strlen(afterName); // to end
            if (afterList!=afterName)
            {
                std::string listString(afterName, afterList);
                parseAdaptersList(listString.c_str(), param.adapters, param.allAdapters);
                afterName = afterList;
            }
        }
        catch(const Error& error)
        {
            std::cerr << "Can't parse adapter list for '" << string << "': " <<
                        error.what() << std::endl;
            return false;
        }
    }
    else if (*afterName==0)
    {
        std::cerr << "Unterminated parameter '" << string << "'!" << std::endl;
        return false;
    }
    
    if (*afterName==':' && !partIdSet)
    {
        afterName++;
        errno = 0;
        int value = strtol(afterName, &next, 10);
        if (errno!=0)
        {
            std::cerr << "Can't parse partId in '" << string << "'!" << std::endl;
            return false;
        }
        if (afterName != next)
            param.partId = value;
        afterName = next;
    }
    else if (*afterName==0)
    {
        std::cerr << "Unterminated parameter '" << string << "'!" << std::endl;
        return false;
    }
    
    if (*afterName=='=')
    {
        afterName++;
        errno = 0;
        if (::strcmp(afterName, "default")==0)
        {
            param.useDefault = true;
            afterName += 7;
        }
        else
        {
            param.value = strtod(afterName, &next);
            if (errno!=0 || afterName==next)
            {
                std::cerr << "Can't parse value in '" << string << "'!" << std::endl;
                return false;
            }
            if (std::isinf(param.value) || std::isnan(param.value))
            {
                std::cerr << "Value of '" << string << "' is not finite!" << std::endl;
                return false;
            }
            afterName = next;
        }
        if (*afterName!=0)
        {
            std::cerr << "Garbages in '" << string << "'!" << std::endl;
            return false;
        }
    }
    else
    {
        std::cerr << "Unterminated parameter '" << string << "'!" << std::endl;
        return false;
    }
    /*std::cout << "param: " << int(param.type) << ", dev: " << param.adapterIndex <<
            ", pid: " << param.partId << ", value=" << param.value << std::endl;*/
    return true;
}

struct FanSpeedSetup
{
    double value;
    bool useDefault;
    bool isSet;
};

struct AdapterIterator
{
    const std::vector<int>& adapters;
    bool allAdapters;
    int allAdaptersNum;
    int position;
    
    AdapterIterator(const std::vector<int>& _adapters, bool _allAdapters,
            int _allAdaptersNum) : adapters(_adapters), allAdapters(_allAdapters),
            allAdaptersNum(_allAdaptersNum), position(0)
    { }
    
    AdapterIterator& operator++()
    {
        position++;
        return *this;
    }
    
    operator bool() const
    {
        return (!allAdapters && position < int(adapters.size())) ||
                (allAdapters && position < allAdaptersNum);
    }
    bool operator!() const
    {
        return !((!allAdapters && position < int(adapters.size())) ||
                (allAdapters && position < allAdaptersNum));
    }
    int operator*() const
    { return allAdapters ? position : adapters[position]; }
};

#ifdef HAVE_ADLSDK
static void setOVCParameters(ADLMainControl& mainControl, int adaptersNum,
            const std::vector<int>& activeAdapters,
            const std::vector<OVCParameter>& ovcParams)
{
    std::cout << "WARNING: setting AMD Overdrive parameters!" << std::endl;
    std::cout <<
        "\nIMPORTANT NOTICE: Before any setting of AMD Overdrive parameters,\n"
        "please STOP ANY GPU computations and GPU renderings.\n"
        "Please use this utility CAREFULLY, because it can DAMAGE your hardware!\n" 
        << std::endl;
    
    const int realAdaptersNum = activeAdapters.size();
    std::vector<ADLODParameters> odParams(realAdaptersNum);
    std::vector<std::vector<ADLODPerformanceLevel> > perfLevels(realAdaptersNum);
    std::vector<std::vector<ADLODPerformanceLevel> > defaultPerfLevels(realAdaptersNum);
    std::vector<bool> changedDevices(realAdaptersNum);
    std::fill(changedDevices.begin(), changedDevices.end(), false);
    
    bool failed = false;
    for (OVCParameter param: ovcParams)
        if (!param.allAdapters)
        {
            bool listFailed = false;
            for (int adapterIndex: param.adapters)
                if (!listFailed && (adapterIndex>=realAdaptersNum || adapterIndex<0))
                {
                    std::cerr << "Some adapter indices out of range in '" <<
                                    param.argText << "'!" << std::endl;
                    listFailed = failed = true;
                }
        }
    
    // check fanspeed
    for (OVCParameter param: ovcParams)
        if (param.type==OVCParamType::FAN_SPEED)
        {
            if(param.partId!=0)
            {
                std::cerr << "Thermal Control Index is not 0 in '" <<
                        param.argText << "'!" << std::endl;
                failed = true;
            }
            if(!param.useDefault && (param.value<0.0 || param.value>100.0))
            {
                std::cerr << "FanSpeed value out of range in '" <<
                        param.argText << "'!" << std::endl;
                failed = true;
            }
        }
    
    for (int ai = 0; ai < realAdaptersNum; ai++)
    {
        int i = activeAdapters[ai];
        mainControl.getODParameters(i, odParams[ai]);
        perfLevels[ai].resize(odParams[ai].iNumberOfPerformanceLevels);
        defaultPerfLevels[ai].resize(odParams[ai].iNumberOfPerformanceLevels);
        mainControl.getODPerformanceLevels(i, 0, odParams[ai].iNumberOfPerformanceLevels,
                    perfLevels[ai].data());
        mainControl.getODPerformanceLevels(i, 1, odParams[ai].iNumberOfPerformanceLevels,
                    defaultPerfLevels[ai].data());
    }
    
    // check other params
    for (OVCParameter param: ovcParams)
        if (param.type!=OVCParamType::FAN_SPEED)
            for (AdapterIterator ait(param.adapters, param.allAdapters, realAdaptersNum);
                        ait; ++ait)
            {
                int i = *ait;
                if (i>=realAdaptersNum)
                    continue;
                int partId = (param.partId!=LAST_PERFLEVEL)?param.partId:
                        odParams[i].iNumberOfPerformanceLevels-1;
                if (partId >= odParams[i].iNumberOfPerformanceLevels || partId < 0)
                {
                    std::cerr << "Performance level out of range in '" <<
                            param.argText << "'!" << std::endl;
                    failed = true;
                    continue;
                }
                switch(param.type)
                {
                    case OVCParamType::CORE_CLOCK:
                        if (!param.useDefault &&
                            (param.value < odParams[i].sEngineClock.iMin/100.0 ||
                            param.value > odParams[i].sEngineClock.iMax/100.0))
                        {
                            std::cerr << "Core clock out of range in '" <<
                                    param.argText << "'!" << std::endl;
                            failed = true;
                        }
                        break;
                    case OVCParamType::MEMORY_CLOCK:
                        if (!param.useDefault &&
                            (param.value < odParams[i].sMemoryClock.iMin/100.0 ||
                            param.value > odParams[i].sMemoryClock.iMax/100.0))
                        {
                            std::cerr << "Memory clock out of range in '" <<
                                    param.argText << "'!" << std::endl;
                            failed = true;
                        }
                        break;
                    case OVCParamType::VDDC_VOLTAGE:
                        if (!param.useDefault &&
                            (param.value < odParams[i].sVddc.iMin/1000.0 ||
                            param.value > odParams[i].sVddc.iMax/1000.0))
                        {
                            std::cerr << "Voltage out of range in '" <<
                                    param.argText << "'!" << std::endl;
                            failed = true;
                        }
                        break;
                    default:
                        break;
                }
            }
    if (failed)
    {
        std::cerr << "NO ANY settings applied. Error in parameters!" << std::endl;
        throw Error("Wrong parameters!");
    }
    // print what has been changed
    for (OVCParameter param: ovcParams)
        if (param.type==OVCParamType::FAN_SPEED)
            for (AdapterIterator ait(param.adapters, param.allAdapters, realAdaptersNum);
                        ait; ++ait)
            {
                std::cout << "Setting fanspeed to ";
                if (param.useDefault)
                    std::cout << "default";
                else
                    std::cout << param.value << "%";
                std::cout << " for adapter " << *ait << " at thermal controller " <<
                        param.partId << std::endl;
            }
    for (OVCParameter param: ovcParams)
        if (param.type!=OVCParamType::FAN_SPEED)
            for (AdapterIterator ait(param.adapters, param.allAdapters, realAdaptersNum);
                        ait; ++ait)
            {
                int i = *ait;
                int partId = (param.partId!=LAST_PERFLEVEL)?param.partId:
                        odParams[i].iNumberOfPerformanceLevels-1;
                switch(param.type)
                {
                    case OVCParamType::CORE_CLOCK:
                        std::cout << "Setting core clock to ";
                        if (param.useDefault)
                            std::cout << "default";
                        else
                            std::cout << param.value << " MHz";
                        std::cout << " for adapter " << i <<
                                " at performance level " << partId << std::endl;
                        break;
                    case OVCParamType::MEMORY_CLOCK:
                        std::cout << "Setting memory clock to ";
                        if (param.useDefault)
                            std::cout << "default";
                        else
                            std::cout << param.value << " MHz";
                        std::cout << " for adapter " << i <<
                                " at performance level " << partId << std::endl;
                        break;
                    case OVCParamType::CUR_CORE_CLOCK:
                        std::cout << "Current core clock available only for "
                                "AMDGPU-(PRO) drivers." << std::endl;
                        break;
                    case OVCParamType::CUR_MEMORY_CLOCK:
                        std::cout << "Current memory clock available only for "
                                "AMDGPU-(PRO) drivers." << std::endl;
                        break;
                    case OVCParamType::CORE_OD:
                        std::cout << "Core OD available only for "
                                "AMDGPU-(PRO) drivers." << std::endl;
                        break;
                    case OVCParamType::MEMORY_OD:
                        std::cout << "Memory OD available only for "
                                "AMDGPU-(PRO) drivers." << std::endl;
                        break;
                    case OVCParamType::VDDC_VOLTAGE:
                        std::cout << "Setting Vddc voltage to ";
                        if (param.useDefault)
                            std::cout << "default";
                        else
                            std::cout << param.value << " V";
                        std::cout << " for adapter " << i <<
                                " at performance level " << partId << std::endl;
                        break;
                    case OVCParamType::CORE_PERF_LEVEL:
                        std::cout << "Core perf level available only for "
                                "AMDGPU-(PRO) drivers." << std::endl;
                        break;
                    case OVCParamType::MEMORY_PERF_LEVEL:
                        std::cout << "Memory perf level available only for "
                                "AMDGPU-(PRO) drivers." << std::endl;
                        break;
                    default:
                        break;
                }
            }
    
    std::vector<FanSpeedSetup> fanSpeedSetups(realAdaptersNum);
    std::fill(fanSpeedSetups.begin(), fanSpeedSetups.end(),
              FanSpeedSetup{ 0.0, false, false });
    for (OVCParameter param: ovcParams)
        if (param.type==OVCParamType::FAN_SPEED)
            for (AdapterIterator ait(param.adapters, param.allAdapters, realAdaptersNum);
                        ait; ++ait)
            {
                fanSpeedSetups[*ait].value = param.value;
                fanSpeedSetups[*ait].useDefault = param.useDefault;
                fanSpeedSetups[*ait].isSet = true;
            }
    
    for (OVCParameter param: ovcParams)
        if (param.type!=OVCParamType::FAN_SPEED)
            for (AdapterIterator ait(param.adapters, param.allAdapters, realAdaptersNum);
                        ait; ++ait)
            {
                int i = *ait;
                int partId = (param.partId!=LAST_PERFLEVEL)?param.partId:
                        odParams[i].iNumberOfPerformanceLevels-1;
                ADLODPerformanceLevel& perfLevel = perfLevels[i][partId];
                const ADLODPerformanceLevel& defaultPerfLevel = defaultPerfLevels[i][partId];
                switch(param.type)
                {
                    case OVCParamType::CORE_CLOCK:
                        if (param.useDefault)
                            perfLevel.iEngineClock = defaultPerfLevel.iEngineClock;
                        else
                            perfLevel.iEngineClock = int(round(param.value*100.0));
                        break;
                    case OVCParamType::MEMORY_CLOCK:
                        if (param.useDefault)
                            perfLevel.iMemoryClock = defaultPerfLevel.iMemoryClock;
                        else
                            perfLevel.iMemoryClock = int(round(param.value*100.0));
                        break;
                    case OVCParamType::VDDC_VOLTAGE:
                        if (param.useDefault)
                            perfLevel.iVddc = defaultPerfLevel.iVddc;
                        else if (perfLevel.iVddc==0)
                            std::cout << "Voltage for adapter " << i <<
                                        " is not set!" << std::endl;
                        else
                            perfLevel.iVddc = int(round(param.value*1000.0));
                        break;
                    default:
                        break;
                }
                changedDevices[i] = true;
            }
    /// set fan speeds
    for (int i = 0; i < realAdaptersNum; i++)
        if (fanSpeedSetups[i].isSet)
        {
            if (!fanSpeedSetups[i].useDefault)
                mainControl.setFanSpeed(activeAdapters[i], 0 /* must be zero */,
                                int(round(fanSpeedSetups[i].value)));
            else
                mainControl.setFanSpeedToDefault(activeAdapters[i], 0);
        }
    
    // set od perflevels
    for (int i = 0; i < realAdaptersNum; i++)
        if (changedDevices[i])
            mainControl.setODPerformanceLevels(activeAdapters[i],
                    odParams[i].iNumberOfPerformanceLevels, perfLevels[i].data());
}
#endif

/* AMDGPU code */

struct PerfClocks
{
    unsigned int coreClock;
    unsigned int memoryClock;
    unsigned int curCoreClock;
    unsigned int curMemoryClock;
};

static void setOVCParameters(AMDGPUAdapterHandle& handle,
            const std::vector<OVCParameter>& ovcParams,
            const std::vector<PerfClocks>& perfClocks,
            const std::vector<AMDGPUAdapterInfo>& adapterInfos)
{
    std::cout << "WARNING: setting AMD Overdrive parameters!" << std::endl;
    std::cout <<
        "\nIMPORTANT NOTICE: Before any setting of AMD Overdrive parameters,\n"
        "please STOP ANY GPU computations and GPU renderings.\n"
        "Please use this utility CAREFULLY, because it can DAMAGE your hardware!\n" 
        << std::endl;
    
    bool failed = false;
    int adaptersNum = handle.getAdaptersNum();
    for (OVCParameter param: ovcParams)
        if (!param.allAdapters)
        {
            bool listFailed = false;
            for (int adapterIndex: param.adapters)
                if (!listFailed && (adapterIndex>=adaptersNum || adapterIndex<0))
                {
                    std::cerr << "Some adapter indices out of range in '" <<
                                    param.argText << "'!" << std::endl;
                    listFailed = failed = true;
                }
        }
    
    // check fanspeed
    for (OVCParameter param: ovcParams)
        if (param.type==OVCParamType::FAN_SPEED)
        {
            if(param.partId!=0)
            {
                std::cerr << "Thermal Control Index is not 0 in '" <<
                        param.argText << "'!" << std::endl;
                failed = true;
            }
            if(!param.useDefault && (param.value<0.0 || param.value>100.0))
            {
                std::cerr << "FanSpeed value out of range in '" <<
                        param.argText << "'!" << std::endl;
                failed = true;
            }
        }
    
    // check other params
    for (OVCParameter param: ovcParams)
        if (param.type!=OVCParamType::FAN_SPEED)
        {
            for (AdapterIterator ait(param.adapters, param.allAdapters, adaptersNum);
                        ait; ++ait)
            {
                int i = *ait;
                if (i>=adaptersNum)
                    continue;
                
                int partId = (param.partId!=LAST_PERFLEVEL)?param.partId:0;
                if (partId != 0)
                {
                    std::cerr << "Performance level out of range in '" <<
                            param.argText << "'!" << std::endl;
                    failed = true;
                    continue;
                }
                
                const PerfClocks& perfClks = perfClocks[i];
                const std::vector<unsigned int>& coreClocks =
                        adapterInfos[i].coreClocks;
                const std::vector<unsigned int>& memoryClocks =
                        adapterInfos[i].memoryClocks;
                
                switch(param.type)
                {
                    case OVCParamType::CORE_CLOCK:
                        if (!param.useDefault &&
                            (param.value < perfClks.coreClock ||
                             param.value > perfClks.coreClock*1.20))
                        {
                            std::cerr << "Core clock out of range in '" <<
                                    param.argText << "'!" << std::endl;
                            failed = true;
                        }
                        break;
                    case OVCParamType::MEMORY_CLOCK:
                        if (!param.useDefault &&
                            (param.value < perfClks.memoryClock ||
                             param.value > perfClks.memoryClock*1.20))
                        {
                            std::cerr << "Memory clock out of range in '" <<
                                    param.argText << "'!" << std::endl;
                            failed = true;
                        }
                        break;
                    case OVCParamType::CUR_CORE_CLOCK:
                        if (!param.useDefault &&
                            (size_t(param.value) != param.value ||
                                std::find(coreClocks.begin(), coreClocks.end(),
                                    (unsigned int)param.value) == coreClocks.end()))
                        {
                            std::cerr << "Wrong current core clock '" <<
                                    param.argText << "'!" << std::endl;
                            failed = true;
                        }
                        break;
                    case OVCParamType::CUR_MEMORY_CLOCK:
                        if (!param.useDefault &&
                            (size_t(param.value) != param.value ||
                                std::find(memoryClocks.begin(), memoryClocks.end(),
                                    (unsigned int)param.value) == memoryClocks.end()))
                        {
                            std::cerr << "Wrong current memory clock '" <<
                                    param.argText << "'!" << std::endl;
                            failed = true;
                        }
                        break;
                    case OVCParamType::CORE_OD:
                        if (!param.useDefault &&
                            (param.value < 0.0 || param.value > 20.0))
                        {
                            std::cerr << "Core Overdrive out of range in '" <<
                                    param.argText << "'!" << std::endl;
                            failed = true;
                        }
                        break;
                    case OVCParamType::MEMORY_OD:
                        if (!param.useDefault &&
                            (param.value < 0.0 || param.value > 20.0))
                        {
                            std::cerr << "Memory Overdrive out of range in '" <<
                                    param.argText << "'!" << std::endl;
                            failed = true;
                        }
                        break;
                    case OVCParamType::CORE_PERF_LEVEL:
                        if (!param.useDefault &&
                            (param.value < 0.0 || param.value >= coreClocks.size() ||
                                size_t(param.value) != param.value))
                        {
                            std::cerr << "Wrong core performance level '" <<
                                    param.argText << "'!" << std::endl;
                            failed = true;
                        }
                        break;
                    case OVCParamType::MEMORY_PERF_LEVEL:
                        if (!param.useDefault &&
                            (param.value < 0.0 || param.value >= memoryClocks.size() ||
                                size_t(param.value) != param.value))
                        {
                            std::cerr << "Wrong memory performance level '" <<
                                    param.argText << "'!" << std::endl;
                            failed = true;
                        }
                        break;
                    default:
                        break;
                }
            }
        }
    
    if (failed)
    {
        std::cerr << "NO ANY settings applied. Error in parameters!" << std::endl;
        throw Error("Wrong parameters!");
    }
    
    // print what has been changed
    for (OVCParameter param: ovcParams)
        if (param.type==OVCParamType::FAN_SPEED)
            for (AdapterIterator ait(param.adapters, param.allAdapters, adaptersNum);
                        ait; ++ait)
            {
                std::cout << "Setting fanspeed to ";
                if (param.useDefault)
                    std::cout << "default";
                else
                    std::cout << param.value << "%";
                std::cout << " for adapter " << *ait << " at thermal controller " <<
                        param.partId << std::endl;
            }
    for (OVCParameter param: ovcParams)
        if (param.type!=OVCParamType::FAN_SPEED)
            for (AdapterIterator ait(param.adapters, param.allAdapters, adaptersNum);
                        ait; ++ait)
            {
                int i = *ait;
                int partId = (param.partId!=LAST_PERFLEVEL)?param.partId:0;
                switch(param.type)
                {
                    case OVCParamType::CORE_CLOCK:
                        std::cout << "Setting core clock to ";
                        if (param.useDefault)
                            std::cout << "default";
                        else
                            std::cout << param.value << " MHz";
                        std::cout << " for adapter " << i <<
                                " at performance level " << partId << std::endl;
                        break;
                    case OVCParamType::MEMORY_CLOCK:
                        std::cout << "Setting memory clock to ";
                        if (param.useDefault)
                            std::cout << "default";
                        else
                            std::cout << param.value << " MHz";
                        std::cout << " for adapter " << i <<
                                " at performance level " << partId << std::endl;
                        break;
                    case OVCParamType::CUR_CORE_CLOCK:
                        std::cout << "Setting current core clock to ";
                        if (param.useDefault)
                            std::cout << "default";
                        else
                            std::cout << param.value << " MHz";
                        std::cout << " for adapter " << i <<
                                " at performance level " << partId << std::endl;
                        break;
                    case OVCParamType::CUR_MEMORY_CLOCK:
                        std::cout << "Setting current memory clock to ";
                        if (param.useDefault)
                            std::cout << "default";
                        else
                            std::cout << param.value << " MHz";
                        std::cout << " for adapter " << i <<
                                " at performance level " << partId << std::endl;
                        break;
                    case OVCParamType::CORE_OD:
                        std::cout << "Setting core overdrive to ";
                        if (param.useDefault)
                            std::cout << "default";
                        else
                            std::cout << param.value;
                        std::cout << " for adapter " << i <<
                                " at performance level " << partId << std::endl;
                        break;
                    case OVCParamType::MEMORY_OD:
                        std::cout << "Setting memory overdrive to ";
                        if (param.useDefault)
                            std::cout << "default";
                        else
                            std::cout << param.value;
                        std::cout << " for adapter " << i <<
                                " at performance level " << partId << std::endl;
                        break;
                    case OVCParamType::VDDC_VOLTAGE:
                        std::cout << "VDDC voltage available only for "
                                "AMD Catalyst/Crimson drivers." << std::endl;
                        break;
                    case OVCParamType::CORE_PERF_LEVEL:
                        std::cout << "Setting core performance level to ";
                        if (param.useDefault)
                            std::cout << "default";
                        else
                            std::cout << param.value;
                        std::cout << " for adapter " << i <<
                                " at performance level " << partId << std::endl;
                        break;
                    case OVCParamType::MEMORY_PERF_LEVEL:
                        std::cout << "Setting memory performance level to ";
                        if (param.useDefault)
                            std::cout << "default";
                        else
                            std::cout << param.value;
                        std::cout << " for adapter " << i <<
                                " at performance level " << partId << std::endl;
                        break;
                    default:
                        break;
                }
            }
    
    std::vector<FanSpeedSetup> fanSpeedSetups(adaptersNum);
    std::fill(fanSpeedSetups.begin(), fanSpeedSetups.end(),
              FanSpeedSetup{ 0.0, false, false });
    for (OVCParameter param: ovcParams)
        if (param.type==OVCParamType::FAN_SPEED)
            for (AdapterIterator ait(param.adapters, param.allAdapters, adaptersNum);
                        ait; ++ait)
            {
                fanSpeedSetups[*ait].value = param.value;
                fanSpeedSetups[*ait].useDefault = param.useDefault;
                fanSpeedSetups[*ait].isSet = true;
            }
    
    for (OVCParameter param: ovcParams)
        if (param.type!=OVCParamType::FAN_SPEED)
            for (AdapterIterator ait(param.adapters, param.allAdapters, adaptersNum);
                        ait; ++ait)
            {
                int i = *ait;
                const PerfClocks& perfClks = perfClocks[i];
                const std::vector<unsigned int>& coreClocks =
                        adapterInfos[i].coreClocks;
                const std::vector<unsigned int>& memoryClocks =
                        adapterInfos[i].memoryClocks;
                
                switch(param.type)
                {
                    case OVCParamType::CORE_CLOCK:
                        if (param.useDefault)
                            handle.setOverdriveCoreParam(i, 0);
                        else
                            handle.setOverdriveCoreParam(i,
                                int(round((double(param.value - perfClks.coreClock) /
                                        perfClks.coreClock)*100.0)));
                        break;
                    case OVCParamType::MEMORY_CLOCK:
                        if (param.useDefault)
                            handle.setOverdriveMemoryParam(i, 0);
                        else
                            handle.setOverdriveMemoryParam(i,
                                int(round((double(param.value - perfClks.memoryClock) /
                                        perfClks.memoryClock)*100.0)));
                        break;
                    case OVCParamType::CUR_CORE_CLOCK:
                        if (param.useDefault)
                            handle.setPerformanceControl(i, AMDGPUPerfControl::AUTO);
                        else
                        {
                            auto ccit = std::find(coreClocks.begin(), coreClocks.end(),
                                    (unsigned int)round(param.value));
                            if (ccit != coreClocks.end())
                                handle.setPerfCoreLevel(i, ccit-coreClocks.begin());
                        }
                        break;
                    case OVCParamType::CUR_MEMORY_CLOCK:
                        if (param.useDefault)
                            handle.setPerformanceControl(i, AMDGPUPerfControl::AUTO);
                        else
                        {
                            auto cmit = std::find(memoryClocks.begin(), memoryClocks.end(),
                                    (unsigned int)round(param.value));
                            if (cmit != memoryClocks.end())
                                handle.setPerfMemoryLevel(i, cmit-memoryClocks.begin());
                        }
                        break;
                    case OVCParamType::CORE_OD:
                        if (param.useDefault)
                            handle.setOverdriveCoreParam(i, 0);
                        else
                            handle.setOverdriveCoreParam(i, int(round(param.value)));
                        break;
                    case OVCParamType::MEMORY_OD:
                        if (param.useDefault)
                            handle.setOverdriveMemoryParam(i, 0);
                        else
                            handle.setOverdriveMemoryParam(i, int(round(param.value)));
                        break;
                    case OVCParamType::CORE_PERF_LEVEL:
                        if (param.useDefault)
                            handle.setPerformanceControl(i, AMDGPUPerfControl::AUTO);
                        else
                            handle.setPerfCoreLevel(i, int(round(param.value)));
                        break;
                    case OVCParamType::MEMORY_PERF_LEVEL:
                        if (param.useDefault)
                            handle.setPerformanceControl(i, AMDGPUPerfControl::AUTO);
                        else
                            handle.setPerfMemoryLevel(i, int(round(param.value)));
                        break;
                    default:
                        break;
                }
            }

    /// set fan speeds
    for (int i = 0; i < adaptersNum; i++)
        if (fanSpeedSetups[i].isSet)
        {
            if (!fanSpeedSetups[i].useDefault)
                handle.setFanSpeed(i, int(round(fanSpeedSetups[i].value)));
            else
                handle.setFanSpeedToDefault(i);
        }
}

/* AMDGPU code */

static const char* helpAndUsageString =
"amdcovc " AMDCOVC_VERSION " by Mateusz Szpakowski (matszpk@interia.pl)\n"
"Program is distributed under terms of the GPLv2.\n"
"Program available at https://github.com/matszpk/amdcovc.\n"
"\n"
"Usage: amdcovc [--help|-?] [--verbose|-v] [-a LIST|--adapters=LIST] [PARAM ...]\n"
"Print AMD Overdrive informations if no parameter given.\n"
"Set AMD Overdrive parameters (clocks, fanspeeds,...) if any parameter given.\n"
"\n"
"List of parameters:\n"
"  coreclk[:[ADAPTERS][:LEVEL]]=CLOCK    set core clock in MHz\n"
"  memclk[:[ADAPTERS][:LEVEL]]=CLOCK     set memory clock in MHz\n"
"  ccoreclk[:[ADAPTERS][:LEVEL]]=CLOCK   set current core clock in MHz (AMDGPU)\n"
"  cmemclk[:[ADAPTERS][:LEVEL]]=CLOCK    set current memory clock in MHz (AMDGPU)\n"
"  coreod[:[ADAPTERS][:LEVEL]]=PERCENT   set core Overdrive in percent (AMDGPU)\n"
"  memod[:[ADAPTERS][:LEVEL]]=PERCENT    set memory Overdrive in percent (AMDGPU)\n"
"  coreod[:[ADAPTERS][:LEVEL]]=LEVEL     set core performance level (AMDGPU)\n"
"  memod[:[ADAPTERS][:LEVEL]]=LEVEL      set memory performance level (AMDGPU)\n"
"  vcore[:[ADAPTERS][:LEVEL]]=VOLTAGE    set Vddc voltage in Volts\n"
"  icoreclk[:ADAPTERS]=CLOCK             set core clock in MHz for idle level\n"
"  imemclk[:ADAPTERS]=CLOCK              set memory clock in MHz for idle level\n"
"  ivcore[:ADAPTERS]=VOLTAGE             set Vddc voltage  in Volts for idle level\n"
"  fanspeed[:[ADAPTERS][:THID]]=PERCENT  set fanspeed in percents\n"
"Extra specifiers in parameters:\n"
"  ADAPTERS                  adapter (devices) index list (default is 0)\n"
"  LEVEL                     performance level (typically 0 or 1, default is last)\n"
"  THID                      thermal controller index (must be 0)\n"
"You can use 'default' in value place to set default value.\n"
"For fanspeed 'default' value force automatic speed setup.\n"
"\n"
"List of options:\n"
"  -a, --adapters=LIST       print informations only for these adapters\n"
"  -v, --verbose             print verbose informations\n"
"      --version             print version\n"
"  -?, --help                print help\n"
"\n"
"Adapter list specified in parameters and '--adapter' option is comma-separated list\n"
"with ranges 'first-last' or 'all'. Examples: 'all', '0-2', '0,1,3-5'\n"
"\n"
"Sample usage:\n"
"amdcovc\n"
"    print short informations about state of the all adapters\n"
"amdcovc -a 1,2,4-6\n"
"    print short informations about adapter 1, 2 and 4 to 6\n"
"amdcovc coreclk:1=900 coreclk=1000\n"
"    set core clock to 900 for adapter 1, set core clock to 1000 for adapter 0\n"
"amdcovc coreclk:1:0=900 coreclk:0:1=1000\n"
"    set core clock to 900 for adapter 1 at performance level 0,\n"
"    set core clock to 1000 for adapter 0 at performance level 1\n"
"amdcovc coreclk:1:0=default coreclk:0:1=default\n"
"    set core clock to default for adapter 0 and 1\n"
"amdcovc fanspeed=75 fanspeed:2=60 fanspeed:1=default\n"
"    set fanspeed to 75% for adapter 0 and set fanspeed to 60% for adapter 2\n"
"    set fanspeed to default for adapter 1\n"
"amdcovc vcore=1.111 vcore::0=0.81\n"
"    set Vddc voltage to 1.111 V for adapter 0\n"
"    set Vddc voltage to 0.81 for adapter 0 for performance level 0\n"
"\n"
"IMPORTANT NOTICE: Before any setting of AMD Overdrive parameters,\n"
"please STOP ANY GPU computations and GPU renderings.\n"
"Please use this utility CAREFULLY, because it can DAMAGE your hardware!\n"
"\n"
"If no X11 server is running, then this program requires root privileges.\n";

int main(int argc, const char** argv)
try
{
    bool printHelp = false;
    bool printVerbose = false;
    std::vector<OVCParameter> ovcParameters;
    std::vector<int> choosenAdapters;
    bool useAdaptersList = false;
    bool chooseAllAdapters = false;
    
    bool failed = false;
    for (int i = 1; i < argc; i++)
        if (::strcmp(argv[i], "--help")==0 || ::strcmp(argv[i], "-?")==0)
            printHelp = true;
        else if (::strcmp(argv[i], "--verbose")==0 || ::strcmp(argv[i], "-v")==0)
            printVerbose = true;
        else if (::strncmp(argv[i], "--adapters=", 11)==0)
        {
            parseAdaptersList(argv[i]+11, choosenAdapters, chooseAllAdapters);
            useAdaptersList = true;
        }
        else if (::strcmp(argv[i], "--adapters")==0)
        {
            if (i+1 < argc)
            {
                parseAdaptersList(argv[++i], choosenAdapters, chooseAllAdapters);
                useAdaptersList = true;
            }
            else
                throw Error("Adapter list not supplied");
        }
        else if (::strncmp(argv[i], "-a", 2)==0)
        {
            if (argv[i][2]!=0)
                parseAdaptersList(argv[i]+2, choosenAdapters, chooseAllAdapters);
            else if (i+1 < argc)
                parseAdaptersList(argv[++i], choosenAdapters, chooseAllAdapters);
            else
                throw Error("Adapter list not supplied");
            useAdaptersList = true;
        }
        else if (::strcmp(argv[i], "--version")==0)
        {
            std::cout << "amdcovc " AMDCOVC_VERSION
            " by Mateusz Szpakowski (matszpk@interia.pl)\n"
            "Program is distributed under terms of the GPLv2.\n"
            "Program available at https://github.com/matszpk/amdcovc.\n"<< std::endl;
            return 0;
        }
        else
        {
            OVCParameter param;
            if (parseOVCParameter(argv[i], param))
                ovcParameters.push_back(param);
            else
                failed = true;
        }
    if (printHelp)
    {
        std::cout << helpAndUsageString;
        std::cout.flush();
        return 0;
    }
    
    if (failed)
        throw Error("Can't parse parameters");
    
#ifdef HAVE_ADLSDK
    ATIADLHandle handle;
    if (handle.open())
    {   // AMD Catalyst/Crimson
        ADLMainControl mainControl(handle, 0);
        int adaptersNum = mainControl.getAdaptersNum();
        /* list for converting user indices to input indices to ADL interface */
        std::vector<int> activeAdapters;
        getActiveAdaptersIndices(mainControl, adaptersNum, activeAdapters);
        
        if (useAdaptersList)
            // sort and check adapter list
            for (int adapterIndex: choosenAdapters)
                if (adapterIndex>=int(activeAdapters.size()) || adapterIndex<0)
                    throw Error("Some adapter indices out of range");
        
        if (!ovcParameters.empty())
            setOVCParameters(mainControl, adaptersNum, activeAdapters, ovcParameters);
        else
        {
            if (printVerbose)
                printAdaptersInfoVerbose(mainControl, adaptersNum, activeAdapters,
                            choosenAdapters, useAdaptersList && !chooseAllAdapters);
            else
                printAdaptersInfo(mainControl, adaptersNum, activeAdapters,
                            choosenAdapters, useAdaptersList && !chooseAllAdapters);
        }
    }
    else
#endif
    {   // AMD GPU(-PRO)
        AMDGPUAdapterHandle handle;
        if (!ovcParameters.empty())
        {
            std::vector<PerfClocks> perfClocks;
            for (unsigned int i = 0; i < handle.getAdaptersNum(); i++)
            {
                unsigned int coreClock, memoryClock;
                handle.getPerformanceClocks(i, coreClock, memoryClock);
                //std::cout << "PerfClocks: " << coreClock << ", " << memoryClock << std::endl;
                perfClocks.push_back(PerfClocks{ coreClock, memoryClock });
            }
            std::vector<AMDGPUAdapterInfo> adapterInfos;
            for (unsigned int i = 0; i < handle.getAdaptersNum(); i++)
                adapterInfos.push_back(handle.parseAdapterInfo(i));
            setOVCParameters(handle, ovcParameters, perfClocks, adapterInfos);
        }
        else
        {
            if (useAdaptersList)
                // sort and check adapter list
                for (int adapterIndex: choosenAdapters)
                    if (adapterIndex>=int(handle.getAdaptersNum()) || adapterIndex<0)
                        throw Error("Some adapter indices out of range");
                    
            if (printVerbose)
                printAdaptersInfoVerbose(handle, choosenAdapters,
                            useAdaptersList && !chooseAllAdapters);
            else
                printAdaptersInfo(handle, choosenAdapters,
                            useAdaptersList && !chooseAllAdapters);
        }
    }
    if (pciAccess!=nullptr)
        pci_cleanup(pciAccess);
    return 0;
}
catch(const std::exception& ex)
{
    if (pciAccess!=nullptr)
        pci_cleanup(pciAccess);
    std::cerr << ex.what() << std::endl;
    return 1;
}
