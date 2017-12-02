#ifndef AMDGPUPROOVC_H
#define AMDGPUPROOVC_H

#include <vector>

#include "amdgpuadapterhandle.h"
#include "structs.h"
#include "conststrings.h"

class AmdGpuProOvc
{

private:

    static void setFanSpeeds(int adaptersNum, std::vector<FanSpeedSetup> fanSpeedSetups, AMDGPUAdapterHandle& Handle_);

    static boid checkFanSpeeds(std::vector<OVCParameter>& ovcParams, bool& failed);

public:

    static void Set(AMDGPUAdapterHandle& Handle_, const std::vector<OVCParameter>& OvcParams, const std::vector<PerfClocks>& PerfClocksList);

};

#endif /* AMDGPUPROOVC_H */
