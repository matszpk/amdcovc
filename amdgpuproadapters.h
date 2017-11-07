#ifndef AMDGPUPROADAPTERS_H
#define AMDGPUPROADAPTERS_H

#include <vector>

#include "adlmaincontrol.h"

class AmdGpuProAdapters
{

private:

public:

  static void PrintInfo(ADLMainControl& mainControl, int adaptersNum, const std::vector<int>& activeAdapters, const std::vector<int>& choosenAdapters,
                                bool useChoosen);

  static void PrintInfoVerbose(ADLMainControl& mainControl, int adaptersNum, const std::vector<int>& activeAdapters,
                                       const std::vector<int>& choosenAdapters, bool useChoosen);

};

#endif /* AMDGPUPROADAPTERS_H */
