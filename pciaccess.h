#ifndef PCIACCESS_H
#define PCIACCESS_H

#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE
#endif

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
#include "./dependencies/ADL_SDK_V10.2/include/adl_sdk.h"

#include "error.h"
#include "atiadlhandle.h"
#include "adlmaincontrol.h"
#include "amdgpuadapterinfo.h"

class PCIAccess
{

private:

public:

//    void PciAccessError(char* msg, ...);

    static void InitializePCIAccess();

    void GetFromPCI_AMDGPU(const char* rlink, AMDGPUAdapterInfo& adapterInfo);

};



#endif /* PCIACCESS_H */
