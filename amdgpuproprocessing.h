#ifndef AMDGPUPROPARAMETERS_H
#define AMDGPUPROPARAMETERS_H

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
