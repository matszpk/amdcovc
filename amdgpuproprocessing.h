#ifndef AMDGPUPROPARAMETERS_H
#define AMDGPUPROPARAMETERS_H

class AmdGpuProProcessing
{

private:

    static void setOvcParameters(std::vector<OVCParameter> ovcParameters);

    static void printAdapterInfo(bool printVerbose, std::std::vector<int> chosenAdapters, bool useAdaptersList, bool chooseAllAdapters);

    static void validateAdapterList(bool useAdaptersList, std::vector<int> chosenAdapters);

public:

    static void Process(std::vector<OVCParameter> OvcParameters, bool UseAdaptersList, std::vector<int> ChosenAdapters, bool ChooseAllAdapters,
                        bool printVerbose);

}

#endif /* AMDGPUPROPARAMETERS_H */
