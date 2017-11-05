#ifndef PCIACCESS_H
#define PCIACCESS_H

#ifdef __linux__
#define LINUX 1
#endif

#include "amdgpuadapterinfo.h"

class PCIAccess
{

private:

public:

    static void InitializePCIAccess();

    static void GetFromPCI_AMDGPU(const char* rlink, AMDGPUAdapterInfo& adapterInfo);

    static void GetFromPCI(int deviceIndex, AdapterInfo& adapterInfo);

};

#endif /* PCIACCESS_H */
