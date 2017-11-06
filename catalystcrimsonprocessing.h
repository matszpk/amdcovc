#ifndef CATALYSTCRIMSONPARAMETERS_H
#define CATALYSTCRIMSONPARAMETERS_H

#define _DEFAULT_SOURCE
#include <iostream>
#include <exception>
#include <vector>
#include <dlfcn.h>
#include <fstream>
#include <algorithm>
#include <cstdio>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <string>
#include <memory>
#include <cmath>
#include <cstdarg>
#include <stdint.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <fcntl.h>
#include <CL/cl.h>
extern "C" {
#include <pci/pci.h>
}

#ifdef __linux__
#define LINUX 1
#endif

class CatalystCrimsonProcessing
{

private:

    static void checkAdapterList(bool useAdaptersList, std::vector<int> chosenAdapters, std::vector<int> activeAdapters);

public:

    static void Process(ATIADLHandle Handle_, bool UseAdaptersList, std::vector<int> ChosenAdapters, std::vector<OVCParameter> OvcParameters,
                        bool ChooseAllAdapters, bool PrintVerbose, bool UseAdaptersList);

}

#endif /* CATALYSTCRIMSONPARAMETERS_H */
