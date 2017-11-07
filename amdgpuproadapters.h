#ifndef AMDGPUPROADAPTERS_H
#define AMDGPUPROADAPTERS_H

#include <vector>

#include "adlmaincontrol.h"

class AmdGpuProAdapters
{

private:

public:

  static void PrintInfo(AMDGPUAdapterHandle& handle, const std::vector<int>& choosenAdapters, bool useChoosen);

  static void PrintInfoVerbose(AMDGPUAdapterHandle& handle, const std::vector<int>& choosenAdapters, bool useChoosen);

};

#endif /* AMDGPUPROADAPTERS_H */
