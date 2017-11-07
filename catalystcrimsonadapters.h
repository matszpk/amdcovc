#ifndef CATALYSTCRIMSONADAPTERS_H
#define CATALYSTCRIMSONADAPTERS_H

#include <vector>

#include "amdgpuadapterhandle.h"
#include "adlmaincontrol.h"

class CatalystCrimsonAdapters
{

private:

public:

  static void PrintInfo(AMDGPUAdapterHandle& handle, const std::vector<int>& choosenAdapters, bool useChoosen);

  static void PrintInfoVerbose(AMDGPUAdapterHandle& handle, const std::vector<int>& choosenAdapters, bool useChoosen);

  static void GetActiveAdaptersIndices(ADLMainControl& mainControl, int adaptersNum, std::vector<int>& activeAdapters);

};

#endif /* CATALYSTCRIMSONADAPTERS_H */
