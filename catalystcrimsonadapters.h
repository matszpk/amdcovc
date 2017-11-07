#ifndef CATALYSTCRIMSONADAPTERS_H
#define CATALYSTCRIMSONADAPTERS_H

class CatalystCrimsonAdapters
{

private:

public:

  static void CatalystCrimsonAdapters::PrintInfo(AMDGPUAdapterHandle& handle, const std::vector<int>& choosenAdapters, bool useChoosen);

  static void CatalystCrimsonAdapters::PrintInfoVerbose(AMDGPUAdapterHandle& handle, const std::vector<int>& choosenAdapters, bool useChoosen)

  static void CatalystCrimsonAdapters::GetActiveAdaptersIndices(ADLMainControl& mainControl, int adaptersNum, std::vector<int>& activeAdapters);

};

#endif /* CATALYSTCRIMSONADAPTERS_H */
