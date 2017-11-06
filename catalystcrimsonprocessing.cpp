#include "catalystcrimsonprocessing.h"

void Process(ATIADLHandle Handle_, bool UseAdaptersList, std::vector<int> ChosenAdapters, std::vector<OVCParameter> OvcParameters, bool ChooseAllAdapters,
             bool PrintVerbose, bool UseAdaptersList)
{
    ADLMainControl mainControl(Handle_, 0);
    int adaptersNum = mainControl.getAdaptersNum();

    std::vector<int> activeAdapters;
    Adapters::GetActiveAdaptersIndices(mainControl, adaptersNum, activeAdapters);

    checkAdapterList(UseAdaptersList, ChosenAdapters, activeAdapters);

    if (!OvcParameters.empty())
    {
        Parameters::SetOVCParameters(mainControl, adaptersNum, activeAdapters, OvcParameters);
        return;
    }

    if (PrintVerbose)
    {
        Adapters::PrintAdaptersInfoVerbose(mainControl, adaptersNum, activeAdapters, ChosenAdapters, UseAdaptersList && !ChooseAllAdapters);
        return;
    }

    Adapters::PrintAdaptersInfo(mainControl, adaptersNum, activeAdapters, ChosenAdapters, UseAdaptersList && !ChooseAllAdapters);
}

void checkAdapterList(bool useAdaptersList, std::vector<int> chosenAdapters, std::vector<int> activeAdapters)
{
    if (useAdaptersList)
    {
        for (int adapterIndex: chosenAdapters)
        {
            if (adapterIndex >= int(activeAdapters.size()) || adapterIndex < 0)
            {
                throw Error("Some adapter indices are out of range");
            }
        }
    }
}
