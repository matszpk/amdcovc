#include "atiadlhandle.h"

ATIADLHandle::ATIADLHandle() : handle(nullptr),
    pADL_Main_Control_Create(nullptr), pADL_Main_Control_Destroy(nullptr), pADL_ConsoleMode_FileDescriptor_Set(nullptr),
    pADL_Adapter_NumberOfAdapters_Get(nullptr), pADL_Adapter_Active_Get(nullptr), pADL_Adapter_AdapterInfo_Get(nullptr),
    pADL_Overdrive5_CurrentActivity_Get(nullptr), pADL_Overdrive5_Temperature_Get(nullptr), pADL_Overdrive5_FanSpeedInfo_Get(nullptr),
    pADL_Overdrive5_FanSpeed_Get(nullptr), pADL_Overdrive5_ODParameters_Get(nullptr), pADL_Overdrive5_ODPerformanceLevels_Get(nullptr),
    pADL_Overdrive5_FanSpeed_Set(nullptr), pADL_Overdrive5_FanSpeedToDefault_Set(nullptr), pADL_Overdrive5_ODPerformanceLevels_Set(nullptr)
{

}

bool ATIADLHandle::open()
try
{
    dlerror(); // clear old errors
    handle = dlopen("libatiadlxx.so", RTLD_LAZY|RTLD_GLOBAL);

    if (handle == nullptr)
    {
        return false;
    }

    pADL_Main_Control_Create = (ADL_Main_Control_Create_T) getSym("ADL_Main_Control_Create");
    pADL_Main_Control_Destroy = (ADL_Main_Control_Destroy_T) getSym("ADL_Main_Control_Destroy");
    pADL_ConsoleMode_FileDescriptor_Set = (ADL_ConsoleMode_FileDescriptor_Set_T) getSym("ADL_ConsoleMode_FileDescriptor_Set");
    pADL_Adapter_NumberOfAdapters_Get = (ADL_Adapter_NumberOfAdapters_Get_T) getSym("ADL_Adapter_NumberOfAdapters_Get");
    pADL_Adapter_Active_Get = (ADL_Adapter_Active_Get_T) getSym("ADL_Adapter_Active_Get");
    pADL_Adapter_AdapterInfo_Get = (ADL_Adapter_AdapterInfo_Get_T) getSym("ADL_Adapter_AdapterInfo_Get");
    pADL_Overdrive5_CurrentActivity_Get = (ADL_Overdrive5_CurrentActivity_Get_T) getSym("ADL_Overdrive5_CurrentActivity_Get");
    pADL_Overdrive5_Temperature_Get = (ADL_Overdrive5_Temperature_Get_T) getSym("ADL_Overdrive5_Temperature_Get");
    pADL_Overdrive5_FanSpeedInfo_Get = (ADL_Overdrive5_FanSpeedInfo_Get_T) getSym("ADL_Overdrive5_FanSpeedInfo_Get");
    pADL_Overdrive5_FanSpeed_Get = (ADL_Overdrive5_FanSpeed_Get_T) getSym("ADL_Overdrive5_FanSpeed_Get");
    pADL_Overdrive5_ODParameters_Get = (ADL_Overdrive5_ODParameters_Get_T) getSym("ADL_Overdrive5_ODParameters_Get");
    pADL_Overdrive5_ODPerformanceLevels_Get = (ADL_Overdrive5_ODPerformanceLevels_Get_T) getSym("ADL_Overdrive5_ODPerformanceLevels_Get");
    pADL_Overdrive5_FanSpeed_Set = (ADL_Overdrive5_FanSpeed_Set_T) getSym("ADL_Overdrive5_FanSpeed_Set");
    pADL_Overdrive5_FanSpeedToDefault_Set = (ADL_Overdrive5_FanSpeedToDefault_Set_T) getSym("ADL_Overdrive5_FanSpeedToDefault_Set");
    pADL_Overdrive5_ODPerformanceLevels_Set = (ADL_Overdrive5_ODPerformanceLevels_Set_T) getSym("ADL_Overdrive5_ODPerformanceLevels_Set");

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
    {
        throw Error(error);
    }

    return symbol;
}

void ATIADLHandle::Main_Control_Create(ADL_MAIN_MALLOC_CALLBACK callback, int iEnumConnectedAdapters) const
{
    int error = pADL_Main_Control_Create(callback, iEnumConnectedAdapters);

    if (error != ADL_OK)
    {
        throw Error(error, "ADL_Main_Control_Create error");
    }
}

void ATIADLHandle::Main_Control_Destroy() const
{
    int error = pADL_Main_Control_Destroy();

    if (error != ADL_OK)
    {
        throw Error(error, "ADL_Main_Control_Destroy error");
    }
}

void ATIADLHandle::ConsoleMode_FileDescriptor_Set(int fileDescriptor) const
{
    int error = pADL_ConsoleMode_FileDescriptor_Set(fileDescriptor);

    if (error != ADL_OK)
    {
        throw Error(error, "ADL_ConsoleMode_FileDescriptor_Set error");
    }
}

void ATIADLHandle::Adapter_NumberOfAdapters_Get(int* number) const
{
    int error = pADL_Adapter_NumberOfAdapters_Get(number);

    if (error != ADL_OK)
    {
        throw Error(error, "ADL_Adapter_NumberOfAdapters_Get error");
    }
}

void ATIADLHandle::Adapter_Active_Get(int adapterIndex, int* status) const
{
    int error = pADL_Adapter_Active_Get(adapterIndex, status);

    if (error != ADL_OK)
    {
        throw Error(error, "ADL_Adapter_Active_Get error");
    }
}

void ATIADLHandle::Adapter_Info_Get(LPAdapterInfo info, int inputSize) const
{
    int error = pADL_Adapter_AdapterInfo_Get(info, inputSize);

    if (error != ADL_OK)
    {
        throw Error(error, "ADL_AdapterInfo_Get error");
    }
}

void ATIADLHandle::Overdrive5_CurrentActivity_Get(int adapterIndex, ADLPMActivity* activity) const
{
    int error = pADL_Overdrive5_CurrentActivity_Get(adapterIndex, activity);

    if (error != ADL_OK)
    {
        throw Error(error, "ADL_Overdrive5_CurrentActivity_Get error");
    }
}

void ATIADLHandle::Overdrive5_Temperature_Get(int adapterIndex, int thermalCtrlIndex, ADLTemperature *temperature) const
{
    int error = pADL_Overdrive5_Temperature_Get(adapterIndex, thermalCtrlIndex, temperature);

    if (error != ADL_OK)
    {
        throw Error(error, "ADL_Overdrive5_Temperature_Get error");
    }
}

void ATIADLHandle::Overdrive5_FanSpeedInfo_Get(int adapterIndex, int thermalCtrlIndex, ADLFanSpeedInfo* fanSpeedInfo) const
{
    int error = pADL_Overdrive5_FanSpeedInfo_Get(adapterIndex, thermalCtrlIndex, fanSpeedInfo);

    if (error != ADL_OK)
    {
        throw Error(error, "ADL_Overdrive5_FanSpeedInfo_Get error");
    }
}

void ATIADLHandle::Overdrive5_FanSpeed_Get(int adapterIndex, int thermalCtrlIndex, ADLFanSpeedValue* fanSpeedValue) const
{
    int error = pADL_Overdrive5_FanSpeed_Get(adapterIndex, thermalCtrlIndex, fanSpeedValue);

    if (error != ADL_OK)
    {
        throw Error(error, "ADL_Overdrive5_FanSpeed_Get error");
    }
}

void ATIADLHandle::Overdrive5_ODParameters_Get(int adapterIndex, ADLODParameters* odParameters) const
{
    int error = pADL_Overdrive5_ODParameters_Get(adapterIndex, odParameters);

    if (error != ADL_OK)
    {
        throw Error(error, "ADL_Overdrive5_ODParameters_Get error");
    }
}

void ATIADLHandle::Overdrive5_ODPerformanceLevels_Get(int adapterIndex, int idefault, ADLODPerformanceLevels* odPerformanceLevels) const
{
    int error = pADL_Overdrive5_ODPerformanceLevels_Get(adapterIndex, idefault, odPerformanceLevels);

    if (error != ADL_OK)
    {
        throw Error(error, "ADL_Overdrive5_ODPerformanceLevels_Get error");
    }
}

void ATIADLHandle::Overdrive5_FanSpeed_Set(int adapterIndex, int thermalCtrlIndex, ADLFanSpeedValue* fanSpeedValue) const
{
    int error = pADL_Overdrive5_FanSpeed_Set(adapterIndex, thermalCtrlIndex, fanSpeedValue);

    if (error != ADL_OK)
    {
        throw Error(error, "ADL_Overdrive5_FanSpeed_Set error");
    }
}

void ATIADLHandle::Overdrive5_FanSpeedToDefault_Set(int adapterIndex, int thermalCtrlIndex) const
{
    int error = pADL_Overdrive5_FanSpeedToDefault_Set(adapterIndex, thermalCtrlIndex);

    if (error != ADL_OK)
    {
        throw Error(error, "ADL_Overdrive5_FanSpeedToDefault_Set error");
    }
}

void ATIADLHandle::Overdrive5_ODPerformanceLevels_Set(int adapterIndex, ADLODPerformanceLevels* odPerformanceLevels) const
{
    int error = pADL_Overdrive5_ODPerformanceLevels_Set(adapterIndex, odPerformanceLevels);

    if (error != ADL_OK)
    {
        throw Error(error, "ADL_Overdrive5_ODPerformanceLevels_Set error");
    }
}
