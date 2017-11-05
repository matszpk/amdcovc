#include "pciaccess.h"

extern pci_access* pciAccess = nullptr;

extern pci_filter pciFilter;

void PCIAccess::pciAccessError(char* msg, ...)
{
    va_list ap;

    va_start(ap, msg);
    vprintf(msg, ap);
    va_end(ap);

    exit(-1);
}

void PCIAccess::initializePCIAccess()
{
    pciAccess = pci_alloc();

    if (pciAccess == nullptr)
    {
        throw Error("Unable to allocate memory for PCIAccess");
    }

    pciAccess->error = pciAccessError;

    pci_filter_init(pciAccess, &pciFilter);
    pci_init(pciAccess);
    pci_scan_bus(pciAccess);
}

void PCIAccess::getFromPCI_AMDGPU(const char* rlink, AMDGPUAdapterInfo& adapterInfo)
{
    if (pciAccess==nullptr)
    {
        initializePCIAccess();
    }

    unsigned int busNum, devNum, funcNum;
    size_t rlinkLen = strlen(rlink);

    if (rlinkLen < 18 || ::strncmp(rlink, "../../../", 9) != 0)
    {
        throw Error("Invalid PCI Bus string");
    }

    char* pciStrPtr = (char*)rlink+9;
    char* pciStrPtrNew;

    while (isdigit(*pciStrPtr))
    {
        pciStrPtr++;
    }

    if (*pciStrPtr != ':')
    {
        throw Error(errno, "Unable to parse PCI location");
    }

    pciStrPtr++;
    errno  = 0;

    busNum = strtoul(pciStrPtr, &pciStrPtrNew, 10);

    if (errno != 0 || pciStrPtr == pciStrPtrNew)
    {
        throw Error(errno, "Unable to parse BusID");
    }

    pciStrPtr = pciStrPtrNew+1;
    errno  = 0;
    devNum = strtoul(pciStrPtr, &pciStrPtr, 10);

    if (errno !=0 || pciStrPtr == pciStrPtrNew)
    {
        throw Error(errno, "Unable to parse DevID");
    }

    pciStrPtr = pciStrPtrNew + 1;
    errno  = 0;

    funcNum = strtoul(pciStrPtr, &pciStrPtr, 10);

    if (errno != 0 || pciStrPtr == pciStrPtrNew)
    {
        throw Error(errno, "Unable to parse FuncID");
    }

    pci_dev* dev = pciAccess->devices;

    for (; dev != nullptr; dev = dev->next)
    {
        if (dev->bus == busNum && dev->dev == devNum && dev->func == funcNum)
        {
            char deviceBuf[128];
            deviceBuf[0] = 0;

            pci_lookup_name(pciAccess, deviceBuf, 128, PCI_LOOKUP_DEVICE, dev->vendor_id, dev->device_id);
            adapterInfo.busNo = busNum;
            adapterInfo.deviceNo  = devNum;
            adapterInfo.funcNo = funcNum;
            adapterInfo.vendorId = dev->vendor_id;
            adapterInfo.deviceId = dev->device_id;
            adapterInfo.name = deviceBuf;

            break;
        }
    }
}
