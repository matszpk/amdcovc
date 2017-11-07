#ifndef AMDGPUPROOVCPARAMETERS_H
#define AMDGPUPROOVCPARAMETERS_H

#include <vector>

#include "amdgpuadapterhandle.h"
#include "structs.h"
#include "conststrings.h"

class AmdGpuProOvcParameters
{

private:

public:

    static void Set(AMDGPUAdapterHandle& Handle_, const std::vector<OVCParameter>& OvcParams, const std::vector<PerfClocks>& PerfClocksList);

};

#endif /* AMDGPUPROOVCPARAMETERS_H */
