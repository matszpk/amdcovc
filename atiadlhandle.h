#ifndef ATIADLHANDLE_H
#define ATIADLHANDLE_H

class ATIADLHandle
{

private:

    typedef int (*ADL_ConsoleMode_FileDescriptor_Set_T)(int fileDescriptor);
    typedef int (*ADL_Main_Control_Create_T)(ADL_MAIN_MALLOC_CALLBACK, int);
    typedef int (*ADL_Main_Control_Destroy_T)();
    typedef int (*ADL_Adapter_NumberOfAdapters_Get_T)(int* numAdapters);
    typedef int (*ADL_Adapter_Active_Get_T)(int adapterIndex, int *status);
    typedef int (*ADL_Adapter_AdapterInfo_Get_T)(LPAdapterInfo info, int inputSize);

    typedef int (*ADL_Overdrive5_CurrentActivity_Get_T)(int adapterIndex, ADLPMActivity* activity);
    typedef int (*ADL_Overdrive5_Temperature_Get_T)(int adapterIndex, int thermalCtrlIndex, ADLTemperature *temperature);
    typedef int (*ADL_Overdrive5_FanSpeedInfo_Get_T)(int adapterIndex, int thermalCtrlIndex, ADLFanSpeedInfo* fanSpeedInfo);
    typedef int (*ADL_Overdrive5_FanSpeed_Get_T)(int adapterIndex, int thermalCtrlIndex, ADLFanSpeedValue* fanSpeedValue);
    typedef int (*ADL_Overdrive5_ODParameters_Get_T)(int adapterIndex, ADLODParameters* odParameters);
    typedef int (*ADL_Overdrive5_ODPerformanceLevels_Get_T)(int adapterIndex, int idefault, ADLODPerformanceLevels* odPerformanceLevels);

    typedef int (*ADL_Overdrive5_FanSpeed_Set_T)(int adapterIndex, int thermalCtrlIndex, ADLFanSpeedValue* fanSpeedValue);
    typedef int (*ADL_Overdrive5_FanSpeedToDefault_Set_T)(int adapterIndex, int thermalCtrlIndex);
    typedef int (*ADL_Overdrive5_ODPerformanceLevels_Set_T)(int adapterIndex, ADLODPerformanceLevels* odPerformanceLevels);

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

    void Main_Control_Create(ADL_MAIN_MALLOC_CALLBACK callback, int iEnumConnectedAdapters) const;
    void Main_Control_Destroy() const;

    void ConsoleMode_FileDescriptor_Set(int fileDescriptor) const;
    void Adapter_NumberOfAdapters_Get(int* number) const;
    void Adapter_Active_Get(int adapterIndex, int* status) const;
    void Adapter_Info_Get(LPAdapterInfo info, int inputSize) const;

    void Overdrive5_CurrentActivity_Get(int adapterIndex, ADLPMActivity* activity) const;
    void Overdrive5_Temperature_Get(int adapterIndex, int thermalCtrlIndex, ADLTemperature* temperature) const;
    void Overdrive5_FanSpeedInfo_Get(int adapterIndex, int thermalCtrlIndex, ADLFanSpeedInfo* fanSpeedInfo) const;
    void Overdrive5_FanSpeed_Get(int adapterIndex, int thermalCtrlIndex, ADLFanSpeedValue* fanSpeedValue) const;
    void Overdrive5_ODParameters_Get(int adapterIndex, ADLODParameters* odParameters) const;
    void Overdrive5_ODPerformanceLevels_Get(int adapterIndex, int idefault, ADLODPerformanceLevels* odPerformanceLevels) const;
    void Overdrive5_FanSpeed_Set(int adapterIndex, int thermalCtrlIndex, ADLFanSpeedValue* fanSpeedValue) const;
    void Overdrive5_FanSpeedToDefault_Set(int adapterIndex, int thermalCtrlIndex) const;
    void Overdrive5_ODPerformanceLevels_Set(int adapterIndex, ADLODPerformanceLevels* odPerformanceLevels) const;

};

#endif /* ATIADLHANDLE_H */
