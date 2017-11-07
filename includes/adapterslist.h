#ifndef ADAPTERSLIST_H
#define ADAPTERSLIST_H

#include "amdgpuadapterhandle.h"
#include "pciaccess.h"
#include "structs.h"

class AdaptersList
{

private:

public:

    static void Parse(const char* string, std::vector<int>& adapters, bool& allAdapters);

};

#endif /* ADAPTERSLIST_H */
