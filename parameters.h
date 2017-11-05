#ifndef PARAMETERS_H
#define PARAMETERS_H

#include "adapters.h"

enum: int
{
    LAST_PERFLEVEL = -1
};

struct FanSpeedSetup
{
    double value;
    bool useDefault;
    bool isSet;
};

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

};

#endif /* PARAMETERS_H */
