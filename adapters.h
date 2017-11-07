#ifndef ADAPTERS_H
#define ADAPTERS_H

#include "amdgpuadapterhandle.h"
#include "pciaccess.h"
#include "structs.h"

class Adapters
{

private:

public:

    static void ParseAdaptersList(const char* string, std::vector<int>& adapters, bool& allAdapters);

    static void PrintAdaptersInfoVerbose(ADLMainControl& mainControl, int adaptersNum, const std::vector<int>& activeAdapters,
                                         const std::vector<int>& choosenAdapters, bool useChoosen);

    static void PrintAdaptersInfo(ADLMainControl& mainControl, int adaptersNum, const std::vector<int>& activeAdapters, const std::vector<int>& choosenAdapters,
                                  bool useChoosen);

    static void GetActiveAdaptersIndices(ADLMainControl& mainControl, int adaptersNum, std::vector<int>& activeAdapters);

    static void PrintAdaptersInfoVerbose(AMDGPUAdapterHandle& handle, const std::vector<int>& choosenAdapters, bool useChoosen);

    static void PrintAdaptersInfo(AMDGPUAdapterHandle& handle, const std::vector<int>& choosenAdapters, bool useChoosen);

};

#endif /* ADAPTERS_H */
