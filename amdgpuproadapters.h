#ifndef AMDGPUPROADAPTERS_H
#define AMDGPUPROADAPTERS_H

#include <vector>

#include "adlmaincontrol.h"
#include "amdgpuadapterhandle.h"

class AmdGpuProAdapters
{

private:

  static void printMemoryClocks(const AMDGPUAdapterInfo adapterInfo);

  static void printCoreClocks(const AMDGPUAdapterInfo adapterInfo);

  static void printTemperature(const AMDGPUAdapterInfo adapterInfo);

  static void printGpuLoad(const AMDGPUAdapterInfo adapterInfo);

  static void printAdapterSummary(const AMDGPUAdapterInfo adapterInfo, int i);

public:

  static void PrintInfo(AMDGPUAdapterHandle& handle, const std::vector<int>& choosenAdapters, bool useChoosen);

  static void PrintInfoVerbose(AMDGPUAdapterHandle& handle, const std::vector<int>& choosenAdapters, bool useChoosen);

};

#endif /* AMDGPUPROADAPTERS_H */
