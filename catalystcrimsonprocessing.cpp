#include "catalystcrimsonprocessing.h"

void CatalystCrimsonProcessing::Process(ATIADLHandle Handle_, bool UseAdaptersList, std::vector<int> ChosenAdapters, std::vector<OVCParameter> OvcParameters, bool ChooseAllAdapters,
             bool PrintVerbose)
{
    ADLMainControl mainControl(Handle_, 0);
    int adaptersNum = mainControl.getAdaptersNum();

    std::vector<int> activeAdapters;
    CatalystCrimsonAdapters::GetActiveAdaptersIndices(mainControl, adaptersNum, activeAdapters);

    checkAdapterList(UseAdaptersList, ChosenAdapters, activeAdapters);

    if (!OvcParameters.empty())
    {
        CatalystCrimsonOvc::Set(mainControl, activeAdapters, OvcParameters);
        return;
    }

    if (PrintVerbose)
    {
        CatalystCrimsonAdapters::PrintInfoVerbose(mainControl, adaptersNum, activeAdapters, ChosenAdapters, UseAdaptersList && !ChooseAllAdapters);
        return;
    }

    CatalystCrimsonAdapters::PrintInfo(mainControl, adaptersNum, activeAdapters, ChosenAdapters, UseAdaptersList && !ChooseAllAdapters);
}

void CatalystCrimsonProcessing::checkAdapterList(bool useAdaptersList, std::vector<int> chosenAdapters, std::vector<int> activeAdapters)
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
