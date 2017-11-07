#ifndef STRUCTS_H
#define STRUCTS_H

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

enum: int
{
    LAST_PERFLEVEL = -1
};

struct FanSpeedSetup
{
    double value;
    bool useDefault;
    bool isSet;
};

struct PerfClocks
{
    unsigned int coreClock;
    unsigned int memoryClock;
};

enum class OVCParamType
{
    CORE_CLOCK,
    MEMORY_CLOCK,
    VDDC_VOLTAGE,
    FAN_SPEED,
    CORE_OD,
    MEMORY_OD
};

struct OVCParameter
{
    OVCParamType type;
    std::vector<int> adapters;
    bool allAdapters;
    int partId;
    double value;
    bool useDefault;
    std::string argText;
};

#endif /* STRUCTS_H */
