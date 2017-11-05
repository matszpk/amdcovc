#ifndef AMDGPUADAPTERHANDLE_H
#define AMDGPUADAPTERHANDLE_H

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
