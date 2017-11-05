# include "parameters.h"

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

bool Parameters::ParseOVCParameter(const char* string, OVCParameter& param)
{
    const char* afterName = strchr(string, ':');

    if (afterName==nullptr)
    {
        afterName = strchr(string, '=');

        if (afterName==nullptr)
        {
            std::cerr << "This is not parameter: '" << string << "'!" << std::endl;

            return false;
        }
    }

    std::string name(string, afterName);
    param.argText = string;
    param.adapters.clear();
    param.adapters.push_back(0); // default is 0
    param.allAdapters = false;
    param.partId = 0;
    param.useDefault = false;
    bool partIdSet = false;

    if (name=="coreclk")
    {
        param.type = OVCParamType::CORE_CLOCK;
        param.partId = LAST_PERFLEVEL;
    }
    else if (name=="memclk")
    {
        param.type = OVCParamType::MEMORY_CLOCK;
        param.partId = LAST_PERFLEVEL;
    }
    else if (name=="coreod")
    {
        param.type = OVCParamType::CORE_OD;
        param.partId = LAST_PERFLEVEL;
    }
    else if (name=="memod")
    {
        param.type = OVCParamType::MEMORY_OD;
        param.partId = LAST_PERFLEVEL;
    }
    else if (name=="vcore")
    {
        param.type = OVCParamType::VDDC_VOLTAGE;
        param.partId = LAST_PERFLEVEL;
    }
    else if (name=="fanspeed")
    {
        param.type = OVCParamType::FAN_SPEED;
        partIdSet = false;
    }
    else if (name=="icoreclk")
    {
        param.type = OVCParamType::CORE_CLOCK;
        partIdSet = true;
    }
    else if (name=="imemclk")
    {
        param.type = OVCParamType::MEMORY_CLOCK;
        partIdSet = true;
    }
    else if (name=="ivcore")
    {
        param.type = OVCParamType::VDDC_VOLTAGE;
        partIdSet = true;
    }
    else
    {
        std::cout << "Wrong parameter name in '" << string << "'!" << std::endl;
        return false;
    }

    char* next;

    if (*afterName == ':')
    {
        // if is
        afterName++;

        try
        {
            const char* afterList = ::strchr(afterName, ':');

            if (afterList==nullptr)
            {
                afterList = ::strchr(afterName, '=');
            }

            if (afterList==nullptr)
            {
                afterList += strlen(afterName); // to end
            }

            if (afterList!=afterName)
            {
                std::string listString(afterName, afterList);
                Adapters::ParseAdaptersList(listString.c_str(), param.adapters, param.allAdapters);
                afterName = afterList;
            }
        }
        catch(const Error& error)
        {
            std::cerr << "Unable to parse adapter list for '" << string << "': " << error.what() << std::endl;
            return false;
        }
    }
    else if (*afterName==0)
    {
        std::cerr << "Unterminated parameter '" << string << "'!" << std::endl;
        return false;
    }

    if (*afterName==':' && !partIdSet)
    {
        afterName++;
        errno = 0;
        int value = strtol(afterName, &next, 10);

        if (errno!=0)
        {
            std::cerr << "Unable to parse partId in '" << string << "'!" << std::endl;
            return false;
        }

        if (afterName != next)
        {
            param.partId = value;
        }

        afterName = next;
    }
    else if (*afterName==0)
    {
        std::cerr << "Unterminated parameter '" << string << "'!" << std::endl;
        return false;
    }

    if (*afterName=='=')
    {
        afterName++;
        errno = 0;

        if (::strcmp(afterName, "default") == 0)
        {
            param.useDefault = true;
            afterName += 7;
        }
        else
        {
            param.value = strtod(afterName, &next);

            if (errno!=0 || afterName==next)
            {
                std::cerr << "Unable to parse value in '" << string << "'!" << std::endl;
                return false;
            }
            if (std::isinf(param.value) || std::isnan(param.value))
            {
                std::cerr << "Value of '" << string << "' is not finite!" << std::endl;
                return false;
            }
            afterName = next;
        }
        if (*afterName!=0)
        {
            std::cerr << "Garbage in '" << string << "'!" << std::endl;
            return false;
        }
    }
    else
    {
        std::cerr << "Unterminated parameter '" << string << "'!" << std::endl;
        return false;
    }

    return true;
}

void Parameters::SetOVCParameters(ADLMainControl& mainControl, int adaptersNum, const std::vector<int>& activeAdapters,
                                  const std::vector<OVCParameter>& ovcParams)
{
    std::cout << "WARNING: Setting AMD Overdrive parameters!" << std::endl;
    std::cout <<
        "\nIMPORTANT NOTICE: Before any setting of AMD Overdrive parameters,\nplease stop all GPU computations and GPU renderings.\n"
        "Please use this utility carefully, as it can damage your hardware.\n" << std::endl;

    const int realAdaptersNum = activeAdapters.size();

    std::vector<ADLODParameters> odParams(realAdaptersNum);
    std::vector<std::vector<ADLODPerformanceLevel> > perfLevels(realAdaptersNum);
    std::vector<std::vector<ADLODPerformanceLevel> > defaultPerfLevels(realAdaptersNum);
    std::vector<bool> changedDevices(realAdaptersNum);
    std::fill(changedDevices.begin(), changedDevices.end(), false);

    bool failed = false;

    for (OVCParameter param: ovcParams)
    {
        if (!param.allAdapters)
        {
            bool listFailed = false;

            for (int adapterIndex: param.adapters)
            {
                if (!listFailed && (adapterIndex>=realAdaptersNum || adapterIndex <0 ))
                {
                    std::cerr << "Some adapter indices are out of range in '" << param.argText << "'!" << std::endl;
                    listFailed = failed = true;
                }
            }
        }
    }

    // check fanspeed
    for (OVCParameter param: ovcParams)
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
        int i = activeAdapters[ai];

        mainControl.getODParameters(i, odParams[ai]);

        perfLevels[ai].resize(odParams[ai].iNumberOfPerformanceLevels);
        defaultPerfLevels[ai].resize(odParams[ai].iNumberOfPerformanceLevels);

        mainControl.getODPerformanceLevels(i, 0, odParams[ai].iNumberOfPerformanceLevels, perfLevels[ai].data());
        mainControl.getODPerformanceLevels(i, 1, odParams[ai].iNumberOfPerformanceLevels, defaultPerfLevels[ai].data());
    }

    // check other params
    for (OVCParameter param: ovcParams)
    {
        if (param.type!=OVCParamType::FAN_SPEED)
        {
            for (AdapterIterator ait(param.adapters, param.allAdapters, realAdaptersNum); ait; ++ait)
            {
                int i = *ait;

                if (i>=realAdaptersNum)
                {
                    continue;
                }

                int partId = (param.partId!=LAST_PERFLEVEL) ? param.partId : odParams[i].iNumberOfPerformanceLevels - 1;

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
        std::cerr << "NO ANY settings applied. Error in parameters!" << std::endl;
        throw Error("Wrong parameters!");
    }

    // print what has been changed
    for (OVCParameter param: ovcParams)
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

    for (OVCParameter param: ovcParams)
    {
        if (param.type!=OVCParamType::FAN_SPEED)
        {
            for (AdapterIterator ait(param.adapters, param.allAdapters, realAdaptersNum); ait; ++ait)
            {
                int i = *ait;
                int partId = (param.partId!=LAST_PERFLEVEL) ? param.partId: odParams[i].iNumberOfPerformanceLevels -1;

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

    for (OVCParameter param: ovcParams)
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

    for (OVCParameter param: ovcParams)
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
                        else if (perfLevel.iVddc==0)
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
                mainControl.setFanSpeed(activeAdapters[i], 0 /* must be zero */, int(round(fanSpeedSetups[i].value)));
            }
            else
            {
                mainControl.setFanSpeedToDefault(activeAdapters[i], 0);
            }
        }
    }

    // set od perflevels
    for (int i = 0; i < realAdaptersNum; i++)
    {
        if (changedDevices[i])
        {
            mainControl.setODPerformanceLevels(activeAdapters[i], odParams[i].iNumberOfPerformanceLevels, perfLevels[i].data());
        }
    }
}

void Parameters::SetOVCParameters(AMDGPUAdapterHandle& handle, const std::vector<OVCParameter>& ovcParams, const std::vector<PerfClocks>& perfClocks)
{
    std::cout << "WARNING: setting AMD Overdrive parameters!" << std::endl;
    std::cout << "\nIMPORTANT NOTICE: Before any setting of AMD Overdrive parameters,\nplease STOP ANY GPU computations and GPU renderings.\n"
        "Please use this utility CAREFULLY, because it can DAMAGE your hardware!\n" << std::endl;

    bool failed = false;
    int adaptersNum = handle.getAdaptersNum();

    for (OVCParameter param: ovcParams)
    {
        if (!param.allAdapters)
        {
            bool listFailed = false;
            for (int adapterIndex: param.adapters)
            {
                if (!listFailed && (adapterIndex>=adaptersNum || adapterIndex < 0))
                {
                    std::cerr << "Some adapter indices out of range in '" << param.argText << "'!" << std::endl;
                    listFailed = failed = true;
                }
            }
        }
    }

    // check fanspeed
    for (OVCParameter param: ovcParams)
    {
        if (param.type == OVCParamType::FAN_SPEED)
        {
            if(param.partId != 0)
            {
                std::cerr << "Thermal Control Index is not 0 in '" << param.argText << "'!" << std::endl;
                failed = true;
            }
            if(!param.useDefault && (param.value < 0.0 || param.value>100.0))
            {
                std::cerr << "FanSpeed value out of range in '" << param.argText << "'!" << std::endl;
                failed = true;
            }
        }
    }

    // check other params
    for (OVCParameter param: ovcParams)
    {
        if (param.type!=OVCParamType::FAN_SPEED)
        {
            for (AdapterIterator ait(param.adapters, param.allAdapters, adaptersNum); ait; ++ait)
            {
                int i = *ait;
                if (i>=adaptersNum)
                {
                    continue;
                }

                int partId = (param.partId != LAST_PERFLEVEL) ? param.partId : 0;

                if (partId != 0)
                {
                    std::cerr << "Performance level out of range in '" << param.argText << "'!" << std::endl;
                    failed = true;
                    continue;
                }

                const PerfClocks& perfClks = perfClocks[i];

                switch(param.type)
                {
                    case OVCParamType::CORE_CLOCK:

                        if (!param.useDefault && (param.value < perfClks.coreClock || param.value > perfClks.coreClock * 1.20))
                        {
                            std::cerr << "Core clock out of range in '" << param.argText << "'!" << std::endl;
                            failed = true;
                        }
                        break;

                    case OVCParamType::MEMORY_CLOCK:

                        if (!param.useDefault && (param.value < perfClks.memoryClock || param.value > perfClks.memoryClock * 1.20))
                        {
                            std::cerr << "Memory clock out of range in '" << param.argText << "'!" << std::endl;
                            failed = true;
                        }
                        break;

                    case OVCParamType::CORE_OD:

                        if (!param.useDefault && (param.value < 0.0 || param.value > 20.0))
                        {
                            std::cerr << "Core Overdrive out of range in '" << param.argText << "'!" << std::endl;
                            failed = true;
                        }
                        break;

                    case OVCParamType::MEMORY_OD:

                        if (!param.useDefault && (param.value < 0.0 || param.value > 20.0))
                        {
                            std::cerr << "Memory Overdrive out of range in '" << param.argText << "'!" << std::endl;
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
        std::cerr << "Error in parameters. No settings have been applied." << std::endl;
        throw Error("Invalid parameters.");
    }

    // print what has been changed
    for (OVCParameter param: ovcParams)
    {
        if (param.type==OVCParamType::FAN_SPEED)
        {
            for (AdapterIterator ait(param.adapters, param.allAdapters, adaptersNum); ait; ++ait)
            {
                std::cout << "Setting fan speed to ";

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

    for (OVCParameter param: ovcParams)
    {
        if (param.type!=OVCParamType::FAN_SPEED)
        {
            for (AdapterIterator ait(param.adapters, param.allAdapters, adaptersNum); ait; ++ait)
            {
                int i = *ait;
                int partId = (param.partId!=LAST_PERFLEVEL)?param.partId:0;

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

                        std::cout << "Setting core overdrive to ";

                        if (param.useDefault)
                        {
                            std::cout << "default";
                        }
                        else
                        {
                            std::cout << param.value;
                        }

                        std::cout << " for adapter " << i << " at performance level " << partId << std::endl;
                        break;

                    case OVCParamType::MEMORY_OD:

                        std::cout << "Setting memory overdrive to ";

                        if (param.useDefault)
                        {
                            std::cout << "default";
                        }
                        else
                        {
                            std::cout << param.value;
                        }

                        std::cout << " for adapter " << i << " at performance level " << partId << std::endl;
                        break;

                    case OVCParamType::VDDC_VOLTAGE:

                        std::cout << "VDDC voltage available only for AMD Catalyst/Crimson drivers." << std::endl;
                        break;

                    default:

                        break;
                }
            }
        }
    }

    std::vector<FanSpeedSetup> fanSpeedSetups(adaptersNum);
    std::fill(fanSpeedSetups.begin(), fanSpeedSetups.end(), FanSpeedSetup{ 0.0, false, false });

    for (OVCParameter param: ovcParams)
    {
        if (param.type==OVCParamType::FAN_SPEED)
        {
            for (AdapterIterator ait(param.adapters, param.allAdapters, adaptersNum); ait; ++ait)
            {
                fanSpeedSetups[*ait].value = param.value;
                fanSpeedSetups[*ait].useDefault = param.useDefault;
                fanSpeedSetups[*ait].isSet = true;
            }
        }
    }

    for (OVCParameter param: ovcParams)
    {
        if (param.type!=OVCParamType::FAN_SPEED)
        {
            for (AdapterIterator ait(param.adapters, param.allAdapters, adaptersNum); ait; ++ait)
            {
                int i = *ait;

                const PerfClocks& perfClks = perfClocks[i];

                switch(param.type)
                {
                    case OVCParamType::CORE_CLOCK:

                        if (param.useDefault)
                        {
                            handle.setOverdriveCoreParam(i, 0);
                        }
                        else
                        {
                            handle.setOverdriveCoreParam(i, int(round((double(param.value - perfClks.coreClock) / perfClks.coreClock) * 100.0)));
                        }
                        break;

                    case OVCParamType::MEMORY_CLOCK:

                        if (param.useDefault)
                        {
                            handle.setOverdriveMemoryParam(i, 0);
                        }
                        else
                        {
                            handle.setOverdriveMemoryParam(i, int(round((double(param.value - perfClks.memoryClock) / perfClks.memoryClock) * 100.0)));
                        }
                        break;

                    case OVCParamType::CORE_OD:

                        if (param.useDefault)
                        {
                            handle.setOverdriveCoreParam(i, 0);
                        }
                        else
                        {
                            handle.setOverdriveCoreParam(i, int(round(param.value)));
                        }
                        break;

                    case OVCParamType::MEMORY_OD:

                        if (param.useDefault)
                        {
                            handle.setOverdriveMemoryParam(i, 0);
                        }
                        else
                        {
                            handle.setOverdriveMemoryParam(i, int(round(param.value)));
                        }
                        break;

                    default:

                        break;
                }
            }
        }
    }

    /// set fan speeds
    for (int i = 0; i < adaptersNum; i++)
    {
        if (fanSpeedSetups[i].isSet)
        {
            if (!fanSpeedSetups[i].useDefault)
            {
                handle.setFanSpeed(i, int(round(fanSpeedSetups[i].value)));
            }
            else
            {
                handle.setFanSpeedToDefault(i);
            }
        }
    }
}
