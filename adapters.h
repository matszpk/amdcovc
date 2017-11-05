#ifndef ADAPTERS_H
#define ADAPTERS_H

#include "amdgpuadapterhandle.h"
#include "pciaccess.h"

class Adapters
{
private:

public:

    static void ParseAdaptersList(const char* string, std::vector<int>& adapters, bool& allAdapters);

    static void PrintAdaptersInfoVerbose(ADLMainControl& mainControl, int adaptersNum, const std::vector<int>& activeAdapters,
                                         const std::vector<int>& choosenAdapters, bool useChoosen);

    static void PrintAdaptersInfo(ADLMainControl& mainControl, int adaptersNum, const std::vector<int>& activeAdapters, const std::vector<int>& choosenAdapters,
                                  bool useChoosen);

    static void GetActiveAdaptersIndices(ADLMainControl& mainControl, int adaptersNum, std::vector<int>& activeAdapters);

    static void PrintAdaptersInfoVerbose(AMDGPUAdapterHandle& handle, const std::vector<int>& choosenAdapters, bool useChoosen);

    static void PrintAdaptersInfo(AMDGPUAdapterHandle& handle, const std::vector<int>& choosenAdapters, bool useChoosen);

};

struct AdapterIterator
{
    const std::vector<int>& adapters;
    bool allAdapters;
    int allAdaptersNum;
    int position;

    AdapterIterator(const std::vector<int>& _adapters, bool _allAdapters, int _allAdaptersNum) :
        adapters(_adapters), allAdapters(_allAdapters), allAdaptersNum(_allAdaptersNum), position(0)
    {

    }

    AdapterIterator& operator++()
    {
        position++;
        return *this;
    }

    operator bool() const
    {
        return ( !allAdapters && position < int(adapters.size())) || (allAdapters && position < allAdaptersNum );
    }
    bool operator!() const
    {
        return ! ( ( !allAdapters && position < int(adapters.size()) ) || (allAdapters && position < allAdaptersNum) );
    }
    int operator*() const
    {
        return allAdapters ? position : adapters[position];
    }
};

#endif /* ADAPTERS_H */
