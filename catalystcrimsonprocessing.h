#ifndef CATALYSTCRIMSONPARAMETERS_H
#define CATALYSTCRIMSONPARAMETERS_H

class CatalystCrimsonProcessing
{

private:

    static void checkAdapterList(bool useAdaptersList, std::vector<int> chosenAdapters, std::vector<int> activeAdapters);

public:

    static void Process(ATIADLHandle Handle_, bool UseAdaptersList, std::vector<int> ChosenAdapters, std::vector<OVCParameter> OvcParameters,
                        bool ChooseAllAdapters, bool PrintVerbose, bool UseAdaptersList);

}

#endif /* CATALYSTCRIMSONPARAMETERS_H */
