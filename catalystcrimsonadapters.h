#ifndef CATALYSTCRIMSONADAPTERS_H
#define CATALYSTCRIMSONADAPTERS_H

#include <vector>

#include "amdgpuadapterhandle.h"
#include "adlmaincontrol.h"

class CatalystCrimsonAdapters
{

private:

public:

  static void PrintInfo(ADLMainControl& mainControl, int adaptersNum, const std::vector<int>& activeAdapters, const std::vector<int>& choosenAdapters,
                                bool useChoosen);

  static void PrintInfoVerbose(ADLMainControl& mainControl, int adaptersNum, const std::vector<int>& activeAdapters,
                                       const std::vector<int>& choosenAdapters, bool useChoosen);

  static void GetActiveAdaptersIndices(ADLMainControl& mainControl, int adaptersNum, std::vector<int>& activeAdapters);

};

#endif /* CATALYSTCRIMSONADAPTERS_H */
