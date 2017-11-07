#ifndef CATALYSTCRIMSONOVC_H
#define CATALYSTCRIMSONOVC_H

#include <vector>
#include <cmath>

#include "adlmaincontrol.h"
#include "structs.h"
#include "conststrings.h"

class CatalystCrimsonOvc
{

private:

public:

  static void Set(ADLMainControl& MainControl, const std::vector<int>& ActiveAdapters, const std::vector<OVCParameter>& OvcParams);

};

#endif /* CATALYSTCRIMSONOVC_H */
