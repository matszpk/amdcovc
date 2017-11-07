#include "adapterslist.h"


void AdaptersList::Parse(const char* string, std::vector<int>& adapters, bool& allAdapters)
{
    adapters.clear();
    allAdapters = false;

    if (::strcmp(string, "all") == 0)
    {
        allAdapters = true;
        return;
    }

    while (true)
    {
        char* endptr;
        errno = 0;
        int adapterIndex = strtol(string, &endptr, 10);

        if (errno != 0 || endptr==string)
        {
            throw Error("Unable to parse adapter index");
        }

        string = endptr;

        if (*string == '-')
        {
            // if range
            string++;
            errno = 0;
            int adapterIndexEnd = strtol(string, &endptr, 10);

            if (errno != 0 || endptr==string)
            {
                throw Error("Unable to parse adapter index");
            }

            string = endptr;

            if (adapterIndex>adapterIndexEnd)
            {
                throw Error("Wrong range of adapter indices in adapter list");
            }

            for (int i = adapterIndex; i <= adapterIndexEnd; i++)
            {
                adapters.push_back(i);
            }
        }
        else
        {
            adapters.push_back(adapterIndex);
        }

        if (*string == 0)
        {
            break;
        }

        if (*string==',')
        {
            string++;
        }
        else
        {
            throw Error("Invalid data in adapter list");
        }
    }

    std::sort(adapters.begin(), adapters.end());

    adapters.resize(std::unique(adapters.begin(), adapters.end()) - adapters.begin());
}
