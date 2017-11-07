#ifndef AMDGPUPROOVCPARAMETERS_H
#define AMDGPUPROOVCPARAMETERS_H

class AmdGpuProOvcParameters
{

private:

public:

    static void Set(AMDGPUAdapterHandle& Handle_, const std::vector<OVCParameter>& OvcParams, const std::vector<PerfClocks>& PerfClocks);

};

#endif /* AMDGPUPROOVCPARAMETERS_H */
