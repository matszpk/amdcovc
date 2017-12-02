#include "amdgpuproovc.h"

void AmdGpuProOvc::Set(AMDGPUAdapterHandle& Handle_, const std::vector<OVCParameter>& OvcParams, const std::vector<PerfClocks>& PerfClocksList)
{
    std::cout << ConstStrings::OverdriveWarning << std::endl;

    bool failed = false;
    int adaptersNum = Handle_.getAdaptersNum();

    for (OVCParameter param: OvcParams)
    {
        if (!param.allAdapters)
        {
            bool listFailed = false;
            for (int adapterIndex: param.adapters)
            {
                if ( !listFailed && (adapterIndex >= adaptersNum || adapterIndex < 0) )
                {
                    std::cerr << "Some adapter indices out of range in '" << param.argText << "'!" << std::endl;
                    listFailed = failed = true;
                }
            }
        }
    }

    checkFanSpeeds(OvcParams, adaptersNum, failed);

    // check other params
    for (OVCParameter param: OvcParams)
    {
        if (param.type != OVCParamType::FAN_SPEED)
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

                const PerfClocks& perfClks = PerfClocksList[i];

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
    for (OVCParameter param: OvcParams)
    {
        if (param.type == OVCParamType::FAN_SPEED)
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

    for (OVCParameter param: OvcParams)
    {
        if (param.type != OVCParamType::FAN_SPEED)
        {
            for (AdapterIterator ait(param.adapters, param.allAdapters, adaptersNum); ait; ++ait)
            {
                int i = *ait;
                int partId = (param.partId != LAST_PERFLEVEL) ? param.partId : 0;

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

    for (OVCParameter param: OvcParams)
    {
        if (param.type == OVCParamType::FAN_SPEED)
        {
            for (AdapterIterator ait(param.adapters, param.allAdapters, adaptersNum); ait; ++ait)
            {
                fanSpeedSetups[*ait].value = param.value;
                fanSpeedSetups[*ait].useDefault = param.useDefault;
                fanSpeedSetups[*ait].isSet = true;
            }
        }
    }

    for (OVCParameter param: OvcParams)
    {
        if (param.type != OVCParamType::FAN_SPEED)
        {
            for (AdapterIterator ait(param.adapters, param.allAdapters, adaptersNum); ait; ++ait)
            {
                int i = *ait;

                const PerfClocks& perfClks = PerfClocksList[i];

                switch(param.type)
                {
                    case OVCParamType::CORE_CLOCK:

                        if (param.useDefault)
                        {
                            Handle_.setOverdriveCoreParam(i, 0);
                        }
                        else
                        {
                            Handle_.setOverdriveCoreParam(i, int(round((double(param.value - perfClks.coreClock) / perfClks.coreClock) * 100.0)));
                        }
                        break;

                    case OVCParamType::MEMORY_CLOCK:

                        if (param.useDefault)
                        {
                            Handle_.setOverdriveMemoryParam(i, 0);
                        }
                        else
                        {
                            Handle_.setOverdriveMemoryParam(i, int(round((double(param.value - perfClks.memoryClock) / perfClks.memoryClock) * 100.0)));
                        }
                        break;

                    case OVCParamType::CORE_OD:

                        if (param.useDefault)
                        {
                            Handle_.setOverdriveCoreParam(i, 0);
                        }
                        else
                        {
                            Handle_.setOverdriveCoreParam(i, int(round(param.value)));
                        }
                        break;

                    case OVCParamType::MEMORY_OD:

                        if (param.useDefault)
                        {
                            Handle_.setOverdriveMemoryParam(i, 0);
                        }
                        else
                        {
                            Handle_.setOverdriveMemoryParam(i, int(round(param.value)));
                        }
                        break;

                    default:

                        break;
                }
            }
        }
    }

    setFanSpeeds(adaptersNum, fanSpeedSetups, Handle_);
}

void AmdGpuProOvc::setFanSpeeds(int adaptersNum, std::vector<FanSpeedSetup> fanSpeedSetups, AMDGPUAdapterHandle& Handle_)
{
    for (int i = 0; i < adaptersNum; i++)
    {
        if (fanSpeedSetups[i].isSet)
        {
            if (!fanSpeedSetups[i].useDefault)
            {
                Handle_.setFanSpeed(i, int(round(fanSpeedSetups[i].value)));
            }
            else
            {
                Handle_.setFanSpeedToDefault(i);
            }
        }
    }
}

void AmdGpuProOvc::checkFanSpeeds(const std::vector<OVCParameter>& ovcParams, bool& failed)
{
    for (OVCParameter param: ovcParams)
    {
        if (param.type == OVCParamType::FAN_SPEED)
        {
            if(param.partId != 0)
            {
                std::cerr << "Thermal Control Index is not 0 in '" << param.argText << "'!" << std::endl;
                failed = true;
            }
            if( !param.useDefault && (param.value < 0.0 || param.value > 100.0) )
            {
                std::cerr << "FanSpeed value out of range in '" << param.argText << "'!" << std::endl;
                failed = true;
            }
        }
    }
}

void AmdGpuProOvc::checkAdapterIndicies(const std::vector<OVCParameter>& ovcParams, int adaptersNum, bool& failed)
{
    for (OVCParameter param: ovcParams)
    {
        if (!param.allAdapters)
        {
            bool listFailed = false;
            for (int adapterIndex: param.adapters)
            {
                if ( !listFailed && (adapterIndex >= adaptersNum || adapterIndex < 0) )
                {
                    std::cerr << "Some adapter indices out of range in '" << param.argText << "'!" << std::endl;
                    listFailed = failed = true;
                }
            }
        }
    }
}
