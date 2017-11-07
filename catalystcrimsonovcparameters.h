#ifndef CATALYSTCRIMSONOVCPARAMETERS_H
#define CATALYSTCRIMSONOVCPARAMETERS_H

#include <vector>
#include <cmath>

#include "adlmaincontrol.h"
#include "structs.h"
#include "conststrings.h"

class CatalystCrimsonOvcParameters
{

private:

public:

  static void Set(ADLMainControl& MainControl, const std::vector<int>& ActiveAdapters, const std::vector<OVCParameter>& OvcParams);

};

#endif /* CATALYSTCRIMSONOVCPARAMETERS_H */
