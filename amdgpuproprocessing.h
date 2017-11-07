#ifndef AMDGPUPROPARAMETERS_H
#define AMDGPUPROPARAMETERS_H

#include "amdgpuproovcparameters.h"
#include "structs.h"

class AmdGpuProProcessing
{

private:

    AMDGPUAdapterHandle handle;

    void setOvcParameters(std::vector<OVCParameter> ovcParameters);

    void printAdapterInfo(bool printVerbose, std::vector<int> chosenAdapters, bool useAdaptersList, bool chooseAllAdapters);

    void validateAdapterList(bool useAdaptersList, std::vector<int> chosenAdapters);

public:

    void Process(std::vector<OVCParameter> OvcParameters, bool UseAdaptersList, std::vector<int> ChosenAdapters, bool ChooseAllAdapters, bool PrintVerbose);

};

#endif /* AMDGPUPROPARAMETERS_H */
