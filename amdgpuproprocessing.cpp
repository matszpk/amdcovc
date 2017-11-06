#include "amdgpuproprocessing.h"

void Process(std::vector<OVCParameter> OvcParameters, bool UseAdaptersList, std::vector<int> ChosenAdapters, bool ChooseAllAdapters,
                                bool PrintVerbose)
{
    if (!OvcParameters.empty())
    {
        setOvcParametersAmdGpuPro(OvcParameters);
    }
    else
    {
        validateAdapterListAmdGpuPro(UseAdaptersList, ChosenAdapters);

        printAdapterInfoAmdGpuPro(PrintVerbose, ChosenAdapters, UseAdaptersList, ChooseAllAdapters);
    }
}

void setOvcParameters(std::vector<OVCParameter> ovcParameters)
{
    AMDGPUAdapterHandle handle;
    std::vector<PerfClocks> perfClocks;

    for (unsigned int i = 0; i < handle.getAdaptersNum(); i++)
    {
        unsigned int coreClock, memoryClock;
        handle.getPerformanceClocks(i, coreClock, memoryClock);
        perfClocks.push_back(PerfClocks{ coreClock, memoryClock });
    }

    Parameters::SetOVCParameters(handle, ovcParameters, perfClocks);
}

void printAdapterInfo(bool printVerbose, std::std::vector<int> chosenAdapters, bool useAdaptersList, bool chooseAllAdapters)
{
    if (printVerbose)
    {
        Adapters::PrintAdaptersInfoVerbose(handle, chosenAdapters, useAdaptersList && !chooseAllAdapters);
    }
    else
    {
        Adapters::PrintAdaptersInfo(handle, chosenAdapters, useAdaptersList && !chooseAllAdapters);
    }
}

void validateAdapterList(bool useAdaptersList, std::vector<int> chosenAdapters)
{
    if (useAdaptersList)
    {
        for (int adapterIndex: chosenAdapters)
        {
            if (adapterIndex >= int(handle.getAdaptersNum()) || adapterIndex < 0)
            {
                throw Error("Some adapter indices are out of range");
            }
        }
    }
}
