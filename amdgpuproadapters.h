#ifndef AMDGPUPROADAPTERS_H
#define AMDGPUPROADAPTERS_H

#include <vector>

#include "adlmaincontrol.h"
#include "amdgpuadapterhandle.h"

class AmdGpuProAdapters
{

private:

  static void printMemoryClocks(const AMDGPUAdapterInfo adapterInfo)

public:

  static void PrintInfo(AMDGPUAdapterHandle& handle, const std::vector<int>& choosenAdapters, bool useChoosen);

  static void PrintInfoVerbose(AMDGPUAdapterHandle& handle, const std::vector<int>& choosenAdapters, bool useChoosen);

};

#endif /* AMDGPUPROADAPTERS_H */
