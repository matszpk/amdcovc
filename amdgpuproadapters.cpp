#include "amdgpuproadapters.h"

void AmdGpuProAdapters::PrintInfo(AMDGPUAdapterHandle& handle, const std::vector<int>& choosenAdapters, bool useChoosen)
{
    int adaptersNum = handle.getAdaptersNum();
    auto choosenIter = choosenAdapters.begin();
    int i = 0;

    for (int ai = 0; ai < adaptersNum; ai++)
    {
        if (useChoosen && (choosenIter==choosenAdapters.end() || *choosenIter != i ))
        {
            i++;
            continue;
        }

        const AMDGPUAdapterInfo adapterInfo = handle.parseAdapterInfo(ai);

        std::cout << "Adapter " << i << ": " << adapterInfo.name << "\n  Core: " << adapterInfo.coreClock << " MHz, Mem: " <<
                adapterInfo.memoryClock << " MHz, CoreOD: " << adapterInfo.coreOD << ", MemOD: " << adapterInfo.memoryOD << ", ";

        if (adapterInfo.gpuLoad>=0)
        {
            std::cout << "Load: " << adapterInfo.gpuLoad << "%, ";
        }

        std::cout << "Temp: " << adapterInfo.temperature/1000.0 << " C, Fan: " <<
                double(adapterInfo.fanSpeed-adapterInfo.minFanSpeed) / double(adapterInfo.maxFanSpeed - adapterInfo.minFanSpeed) * 100.0 <<
                "%" << std::endl;

        if (!adapterInfo.coreClocks.empty())
        {
            std::cout << "  Core clocks: ";

            for (uint32_t v: adapterInfo.coreClocks)
            {
                std::cout << " " << v;
            }

            std::cout << std::endl;
        }

        if (!adapterInfo.memoryClocks.empty())
        {
            std::cout << "  Memory Clocks: ";

            for (uint32_t v: adapterInfo.memoryClocks)
            {
                std::cout << " " << v;
            }

            std::cout << std::endl;
        }

        if (useChoosen)
        {
            ++choosenIter;
        }

        i++;
    }
}

void AmdGpuProAdapters::PrintInfoVerbose(AMDGPUAdapterHandle& handle, const std::vector<int>& choosenAdapters, bool useChoosen)
{
    int adaptersNum = handle.getAdaptersNum();
    auto choosenIter = choosenAdapters.begin();
    int i = 0;

    for (int ai = 0; ai < adaptersNum; ai++)
    {
        if (useChoosen && (choosenIter == choosenAdapters.end() || *choosenIter != i ) )
        {
            i++;
            continue;
        }

        const AMDGPUAdapterInfo adapterInfo = handle.parseAdapterInfo(ai);

        std::cout << "Adapter " << i << ": " << adapterInfo.name << "\n"
            "  Device Topology: " << adapterInfo.busNo << ':' << adapterInfo.deviceNo << ":" << adapterInfo.funcNo << "\n"
            "  Vendor ID: " << adapterInfo.vendorId << "\n"
            "  Device ID: " << adapterInfo.deviceId << "\n"
            "  Current CoreClock: " << adapterInfo.coreClock << " MHz\n"
            "  Current MemoryClock: " << adapterInfo.memoryClock << " MHz\n"
            "  Core Overdrive: " << adapterInfo.coreOD << "\n"
            "  Memory Overdrive: " << adapterInfo.memoryOD << "\n";

        if (adapterInfo.gpuLoad>=0)
        {
            std::cout << "  GPU Load: " << adapterInfo.gpuLoad << "%\n";
        }

        std::cout << "  Current BusSpeed: " << adapterInfo.busSpeed << "\n"
            "  Current BusLanes: " << adapterInfo.busLanes << "\n"
            "  Temperature: " << adapterInfo.temperature/1000.0 << " C\n"
            "  Critical temperature: " << adapterInfo.tempCritical/1000.0 << " C\n"
            "  FanSpeed Min (Value): " << adapterInfo.minFanSpeed << "\n"
            "  FanSpeed Max (Value): " << adapterInfo.maxFanSpeed << "\n"
            "  Current FanSpeed: " <<
                (double(adapterInfo.fanSpeed-adapterInfo.minFanSpeed)/ double(adapterInfo.maxFanSpeed-adapterInfo.minFanSpeed)*100.0) << "%\n"
            "  Controlled FanSpeed: " << ( adapterInfo.defaultFanSpeed ? "yes" : "no" ) << "\n";

        // print available core clocks
        if (!adapterInfo.coreClocks.empty())
        {
            std::cout << "  Core clocks:\n";

            for (uint32_t v: adapterInfo.coreClocks)
            {
                std::cout << "    " << v << "MHz\n";
            }
        }

        if (!adapterInfo.memoryClocks.empty())
        {
            std::cout << "  Memory Clocks:\n";

            for (uint32_t v: adapterInfo.memoryClocks)
            {
                std::cout << "    " << v << "MHz\n";
            }
        }

        if (useChoosen)
        {
            ++choosenIter;
        }

        i++;
        std::cout << std::endl;
    }
}
