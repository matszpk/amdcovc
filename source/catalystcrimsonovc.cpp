#include "catalystcrimsonovc.h"

void CatalystCrimsonOvc::Set(ADLMainControl& MainControl, const std::vector<int>& ActiveAdapters, const std::vector<OVCParameter>& OvcParams)
{
    std::cout << ConstStrings::OverdriveWarning << std::endl;

    const int realAdaptersNum = ActiveAdapters.size();

    std::vector<ADLODParameters> odParams(realAdaptersNum);
    std::vector<std::vector<ADLODPerformanceLevel> > perfLevels(realAdaptersNum);
    std::vector<std::vector<ADLODPerformanceLevel> > defaultPerfLevels(realAdaptersNum);
    std::vector<bool> changedDevices(realAdaptersNum);
    std::fill(changedDevices.begin(), changedDevices.end(), false);

    bool failed = false;

    for (OVCParameter param: OvcParams)
    {
        if (!param.allAdapters)
        {
            bool listFailed = false;

            for (int adapterIndex: param.adapters)
            {
                if (!listFailed && (adapterIndex >= realAdaptersNum || adapterIndex <0 ))
                {
                    std::cerr << "Some adapter indices are out of range in '" << param.argText << "'!" << std::endl;
                    listFailed = failed = true;
                }
            }
        }
    }

    // check fanspeed
    for (OVCParameter param: OvcParams)
    {
        if (param.type == OVCParamType::FAN_SPEED)
        {
            if(param.partId != 0)
            {
                std::cerr << "Thermal Control Index is not 0 in '" << param.argText << "'!" << std::endl;
                failed = true;
            }
            if(!param.useDefault && (param.value < 0.0 || param.value > 100.0))
            {
                std::cerr << "FanSpeed value out of range in '" << param.argText << "'!" << std::endl;
                failed = true;
            }
        }
    }

    for (int ai = 0; ai < realAdaptersNum; ai++)
    {
        int i = ActiveAdapters[ai];

        MainControl.getODParameters(i, odParams[ai]);

        perfLevels[ai].resize(odParams[ai].iNumberOfPerformanceLevels);
        defaultPerfLevels[ai].resize(odParams[ai].iNumberOfPerformanceLevels);

        MainControl.getODPerformanceLevels(i, 0, odParams[ai].iNumberOfPerformanceLevels, perfLevels[ai].data());
        MainControl.getODPerformanceLevels(i, 1, odParams[ai].iNumberOfPerformanceLevels, defaultPerfLevels[ai].data());
    }

    // check other params
    for (OVCParameter param: OvcParams)
    {
        if (param.type!=OVCParamType::FAN_SPEED)
        {
            for (AdapterIterator ait(param.adapters, param.allAdapters, realAdaptersNum); ait; ++ait)
            {
                int i = *ait;

                if (i >= realAdaptersNum)
                {
                    continue;
                }

                int partId = (param.partId != LAST_PERFLEVEL) ? param.partId : odParams[i].iNumberOfPerformanceLevels - 1;

                if (partId >= odParams[i].iNumberOfPerformanceLevels || partId < 0)
                {
                    std::cerr << "Performance level out of range in '" << param.argText << "'!" << std::endl;
                    failed = true;
                    continue;
                }
                switch(param.type)
                {
                    case OVCParamType::CORE_CLOCK:

                        if (!param.useDefault && (param.value < odParams[i].sEngineClock.iMin/100.0 || param.value > odParams[i].sEngineClock.iMax/100.0))
                        {
                            std::cerr << "Core clock out of range in '" << param.argText << "'!" << std::endl;
                            failed = true;
                        }
                        break;

                    case OVCParamType::MEMORY_CLOCK:

                        if (!param.useDefault && (param.value < odParams[i].sMemoryClock.iMin/100.0 || param.value > odParams[i].sMemoryClock.iMax/100.0))
                        {
                            std::cerr << "Memory clock out of range in '" << param.argText << "'!" << std::endl;
                            failed = true;
                        }
                        break;

                    case OVCParamType::VDDC_VOLTAGE:

                        if (!param.useDefault && (param.value < odParams[i].sVddc.iMin/1000.0 || param.value > odParams[i].sVddc.iMax/1000.0))
                        {
                            std::cerr << "Voltage out of range in '" << param.argText << "'!" << std::endl;
                            failed = true;
                        }
                        break;

                    default:
                        break;
                }
            }
        }
    }

    if (failed)
    {
        std::cerr << "No settings applied. Error in parameters!" << std::endl;
        throw Error("Wrong parameters!");
    }

    // print what has been changed
    for (OVCParameter param: OvcParams)
    {
        if (param.type==OVCParamType::FAN_SPEED)
        {
            for (AdapterIterator ait(param.adapters, param.allAdapters, realAdaptersNum); ait; ++ait)
            {
                std::cout << "Setting fanspeed to ";

                if (param.useDefault)
                {
                    std::cout << "default";
                }
                else
                {
                    std::cout << param.value << "%";
                }

                std::cout << " for adapter " << *ait << " at thermal controller " << param.partId << std::endl;
            }
        }
    }

    for (OVCParameter param: OvcParams)
    {
        if (param.type != OVCParamType::FAN_SPEED)
        {
            for (AdapterIterator ait(param.adapters, param.allAdapters, realAdaptersNum); ait; ++ait)
            {
                int i = *ait;
                int partId = (param.partId != LAST_PERFLEVEL) ? param.partId: odParams[i].iNumberOfPerformanceLevels - 1;

                switch(param.type)
                {
                    case OVCParamType::CORE_CLOCK:

                        std::cout << "Setting core clock to ";

                        if (param.useDefault)
                        {
                            std::cout << "default";
                        }
                        else
                        {
                            std::cout << param.value << " MHz";
                        }

                        std::cout << " for adapter " << i << " at performance level " << partId << std::endl;
                        break;

                    case OVCParamType::MEMORY_CLOCK:

                        std::cout << "Setting memory clock to ";

                        if (param.useDefault)
                        {
                            std::cout << "default";
                        }
                        else
                        {
                            std::cout << param.value << " MHz";
                        }

                        std::cout << " for adapter " << i << " at performance level " << partId << std::endl;
                        break;

                    case OVCParamType::CORE_OD:

                        std::cout << "Core OD available only for AMDGPU-(PRO) drivers." << std::endl;
                        break;

                    case OVCParamType::MEMORY_OD:

                        std::cout << "Memory OD available only for AMDGPU-(PRO) drivers." << std::endl;
                        break;

                    case OVCParamType::VDDC_VOLTAGE:

                        std::cout << "Setting Vddc voltage to ";

                        if (param.useDefault)
                        {
                            std::cout << "default";
                        }
                        else
                        {
                            std::cout << param.value << " V";
                        }

                        std::cout << " for adapter " << i << " at performance level " << partId << std::endl;
                        break;

                    default:

                        break;
                }
            }
        }
    }

    std::vector<FanSpeedSetup> fanSpeedSetups(realAdaptersNum);
    std::fill(fanSpeedSetups.begin(), fanSpeedSetups.end(), FanSpeedSetup{ 0.0, false, false });

    for (OVCParameter param: OvcParams)
    {
        if (param.type==OVCParamType::FAN_SPEED)
        {
            for (AdapterIterator ait(param.adapters, param.allAdapters, realAdaptersNum); ait; ++ait)
            {
                fanSpeedSetups[*ait].value = param.value;
                fanSpeedSetups[*ait].useDefault = param.useDefault;
                fanSpeedSetups[*ait].isSet = true;
            }
        }
    }

    for (OVCParameter param: OvcParams)
    {
        if (param.type!=OVCParamType::FAN_SPEED)
        {
            for (AdapterIterator ait(param.adapters, param.allAdapters, realAdaptersNum); ait; ++ait)
            {
                int i = *ait;
                int partId = (param.partId != LAST_PERFLEVEL) ? param.partId : odParams[i].iNumberOfPerformanceLevels - 1;
                ADLODPerformanceLevel& perfLevel = perfLevels[i][partId];
                const ADLODPerformanceLevel& defaultPerfLevel = defaultPerfLevels[i][partId];

                switch(param.type)
                {
                    case OVCParamType::CORE_CLOCK:

                        if (param.useDefault)
                        {
                            perfLevel.iEngineClock = defaultPerfLevel.iEngineClock;
                        }
                        else
                        {
                            perfLevel.iEngineClock = int(round(param.value * 100.0));
                        }

                        break;

                    case OVCParamType::MEMORY_CLOCK:

                        if (param.useDefault)
                        {
                            perfLevel.iMemoryClock = defaultPerfLevel.iMemoryClock;
                        }
                        else
                        {
                            perfLevel.iMemoryClock = int(round(param.value * 100.0));
                        }

                        break;

                    case OVCParamType::VDDC_VOLTAGE:

                        if (param.useDefault)
                        {
                            perfLevel.iVddc = defaultPerfLevel.iVddc;
                        }
                        else if (perfLevel.iVddc == 0)
                        {
                            std::cout << "Voltage for adapter " << i << " is not set!" << std::endl;
                        }
                        else
                        {
                            perfLevel.iVddc = int(round(param.value * 1000.0));
                        }
                        break;

                    default:

                        break;
                }

                changedDevices[i] = true;
            }
        }
    }

    /// set fan speeds
    for (int i = 0; i < realAdaptersNum; i++)
    {
        if (fanSpeedSetups[i].isSet)
        {
            if (!fanSpeedSetups[i].useDefault)
            {
                MainControl.setFanSpeed(ActiveAdapters[i], 0 /* must be zero */, int(round(fanSpeedSetups[i].value)));
            }
            else
            {
                MainControl.setFanSpeedToDefault(ActiveAdapters[i], 0);
            }
        }
    }

    // set od perflevels
    for (int i = 0; i < realAdaptersNum; i++)
    {
        if (changedDevices[i])
        {
            MainControl.setODPerformanceLevels(ActiveAdapters[i], odParams[i].iNumberOfPerformanceLevels, perfLevels[i].data());
        }
    }
}
