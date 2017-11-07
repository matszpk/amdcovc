#include "catalystcrimsonadapters.h"

void CatalystCrimsonAdapters::PrintInfo(ADLMainControl& mainControl, int adaptersNum, const std::vector<int>& activeAdapters,
                                        const std::vector<int>& choosenAdapters, bool useChoosen)
{
    std::unique_ptr<AdapterInfo[]> adapterInfos(new AdapterInfo[adaptersNum]);
    ::memset(adapterInfos.get(), 0, sizeof(AdapterInfo)*adaptersNum);
    mainControl.getAdapterInfo(adapterInfos.get());

    int i = 0;
    auto choosenIter = choosenAdapters.begin();

    for (int ai = 0; ai < adaptersNum; ai++)
    {
        if (!mainControl.isAdapterActive(ai))
        {
            continue;
        }

        if (useChoosen && (choosenIter==choosenAdapters.end() || *choosenIter!=i))
        {
            i++;
            continue;
        }

        if (adapterInfos[ai].strAdapterName[0] == 0)
        {
            PCIAccess::GetFromPCI(adapterInfos[ai].iAdapterIndex, adapterInfos[ai]);
        }

        ADLPMActivity activity;
        mainControl.getCurrentActivity(ai, activity);

        std::cout << "Adapter " << i << ": " << adapterInfos[ai].strAdapterName << "\n"
                "  Core: " << activity.iEngineClock/100.0 << " MHz, "
                "Mem: " << activity.iMemoryClock/100.0 << " MHz, "
                "Vddc: " << activity.iVddc/1000.0 << " V, "
                "Load: " << activity.iActivityPercent << "%, "
                "Temp: " << mainControl.getTemperature(ai, 0)/1000.0 << " C, "
                "Fan: " << mainControl.getFanSpeed(ai, 0) << "%" << std::endl;

        ADLODParameters odParams;
        mainControl.getODParameters(ai, odParams);

        std::cout << "  Max Ranges: Core: " << odParams.sEngineClock.iMin/100.0 << " - " << odParams.sEngineClock.iMax/100.0 << " MHz, "
            "Mem: " << odParams.sMemoryClock.iMin/100.0 << " - " << odParams.sMemoryClock.iMax/100.0 << " MHz, " <<
            "Vddc: " <<  odParams.sVddc.iMin/1000.0 << " - " << odParams.sVddc.iMax/1000.0 << " V\n";

        int levelsNum = odParams.iNumberOfPerformanceLevels;

        std::unique_ptr<ADLODPerformanceLevel[]> odPLevels(new ADLODPerformanceLevel[levelsNum]);

        mainControl.getODPerformanceLevels(ai, false, levelsNum, odPLevels.get());

        std::cout << "  PerfLevels: Core: " << odPLevels[0].iEngineClock/100.0 << " - " << odPLevels[levelsNum-1].iEngineClock/100.0 << " MHz, "
            "Mem: " << odPLevels[0].iMemoryClock/100.0 << " - " << odPLevels[levelsNum-1].iMemoryClock/100.0 << " MHz, "
            "Vddc: " << odPLevels[0].iVddc/1000.0 << " - " << odPLevels[levelsNum-1].iVddc/1000.0 << " V\n";

        if (useChoosen)
        {
            ++choosenIter;
        }

        i++;
        std::cout << std::endl;
    }
}

void CatalystCrimsonAdapters::PrintInfoVerbose(ADLMainControl& mainControl, int adaptersNum, const std::vector<int>& activeAdapters,
                                         const std::vector<int>& choosenAdapters, bool useChoosen)
{
    std::unique_ptr<AdapterInfo[]> adapterInfos(new AdapterInfo[adaptersNum]);
    ::memset(adapterInfos.get(), 0, sizeof(AdapterInfo)*adaptersNum);

    mainControl.getAdapterInfo(adapterInfos.get());

    int i = 0;
    auto choosenIter = choosenAdapters.begin();

    for (int ai = 0; ai < adaptersNum; ai++)
    {
        if (!mainControl.isAdapterActive(ai))
        {
            continue;
        }

        if (useChoosen && (choosenIter==choosenAdapters.end() || *choosenIter!=i))
        {
            i++;
            continue;
        }

        if (adapterInfos[ai].strAdapterName[0] == 0)
        {
            PCIAccess::GetFromPCI(adapterInfos[ai].iAdapterIndex, adapterInfos[ai]);
        }

        std::cout <<
            "Adapter " << i << ": " << adapterInfos[ai].strAdapterName << "\n"
            "  Device Topology: " << adapterInfos[ai].iBusNumber << ':' << adapterInfos[ai].iDeviceNumber << ":" << adapterInfos[ai].iFunctionNumber << "\n"
            "  Vendor ID: " << adapterInfos[ai].iVendorID << std::endl;

        ADLFanSpeedInfo fsInfo;
        ADLPMActivity activity;

        mainControl.getCurrentActivity(ai, activity);

        std::cout << "  Current CoreClock: " << activity.iEngineClock / 100.0 << " MHz\n"
            "  Current MemoryClock: " << activity.iMemoryClock / 100.0 << " MHz\n"
            "  Current Voltage: " << activity.iVddc / 1000.0 << " V\n"
            "  GPU Load: " << activity.iActivityPercent << "%\n"
            "  Current PerfLevel: " << activity.iCurrentPerformanceLevel << "\n"
            "  Current BusSpeed: " << activity.iCurrentBusSpeed << "\n"
            "  Current BusLanes: " << activity.iCurrentBusLanes<< "\n";

        int temperature = mainControl.getTemperature(ai, 0);

        std::cout << "  Temperature: " << temperature / 1000.0 << " C\n";

        mainControl.getFanSpeedInfo(ai, 0, fsInfo);

        std::cout << "  FanSpeed Min: " << fsInfo.iMinPercent << "%\n"
            "  FanSpeed Max: " << fsInfo.iMaxPercent << "%\n"
            "  FanSpeed MinRPM: " << fsInfo.iMinRPM << " RPM\n"
            "  FanSpeed MaxRPM: " << fsInfo.iMaxRPM << " RPM" << "\n";

        std::cout << "  Current FanSpeed: " << mainControl.getFanSpeed(ai, 0) << "%\n";

        ADLODParameters odParams;
        mainControl.getODParameters(ai, odParams);

        std::cout <<
            "  CoreClock: " << odParams.sEngineClock.iMin / 100.0 << " - " << odParams.sEngineClock.iMax / 100.0 <<
            " MHz, step: " << odParams.sEngineClock.iStep / 100.0 << " MHz\n"
            "  MemClock: " << odParams.sMemoryClock.iMin / 100.0 << " - " << odParams.sMemoryClock.iMax / 100.0 <<
            " MHz, step: " << odParams.sMemoryClock.iStep / 100.0 << " MHz\n"
            "  Voltage: " << odParams.sVddc.iMin / 1000.0 << " - " << odParams.sVddc.iMax / 1000.0 <<
            " V, step: " << odParams.sVddc.iStep / 1000.0 << " V\n";

        std::unique_ptr<ADLODPerformanceLevel[]> odPLevels(new ADLODPerformanceLevel[odParams.iNumberOfPerformanceLevels]);

        mainControl.getODPerformanceLevels(ai, false, odParams.iNumberOfPerformanceLevels, odPLevels.get());

        std::cout << "  Performance levels: " << odParams.iNumberOfPerformanceLevels << "\n";

        for (int j = 0; j < odParams.iNumberOfPerformanceLevels; j++)
        {
            std::cout <<
                "    Performance Level: " << j << "\n"
                "      CoreClock: " << odPLevels[j].iEngineClock / 100.0 << " MHz\n"
                "      MemClock: " << odPLevels[j].iMemoryClock / 100.0 << " MHz\n"
                "      Voltage: " << odPLevels[j].iVddc / 1000.0 << " V\n";
        }

        mainControl.getODPerformanceLevels(ai, true, odParams.iNumberOfPerformanceLevels, odPLevels.get());

        std::cout << "  Default Performance levels: " << odParams.iNumberOfPerformanceLevels << "\n";

        for (int j = 0; j < odParams.iNumberOfPerformanceLevels; j++)
        {
            std::cout <<
                "    Performance Level: " << j << "\n"
                "      CoreClock: " << odPLevels[j].iEngineClock / 100.0 << " MHz\n"
                "      MemClock: " << odPLevels[j].iMemoryClock / 100.0 << " MHz\n"
                "      Voltage: " << odPLevels[j].iVddc / 1000.0 << " V\n";
        }

        std::cout.flush();

        if (useChoosen)
        {
            ++choosenIter;
        }

        i++;
        std::cout << std::endl;
    }
}

void CatalystCrimsonAdapters::GetActiveAdaptersIndices(ADLMainControl& mainControl, int adaptersNum, std::vector<int>& activeAdapters)
{
    activeAdapters.clear();

    for (int i = 0; i < adaptersNum; i++)
    {
        if (mainControl.isAdapterActive(i))
        {
            activeAdapters.push_back(i);
        }
    }
}
