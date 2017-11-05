#include "amdgpuadapterhandle.h"
#include "pciaccess.h"

static void writeFileContentValue(const char* filename, unsigned int value)
{
    std::ofstream ofs(filename, std::ios::binary);

    try
    {
        ofs.exceptions(std::ios::failbit);
        ofs << value << std::endl;
    }
    catch(const std::exception& ex)
    {
        throw Error( (std::string("Unable to write to file '") + filename + "'").c_str() );
    }
}

static bool getFileContentValue(const char* filename, unsigned int& value)
{
    value = 0;

    std::ifstream ifs(filename, std::ios::binary);

    ifs.exceptions(std::ios::failbit);

    std::string line;
    std::getline(ifs, line);

    char* p = (char*)line.c_str();
    char* p2;

    errno = 0;

    value = strtoul(p, &p2, 0);

    if (errno != 0)
    {
        throw Error("Unable to parse value from file");
    }

    return (p != p2);
}

AMDGPUAdapterHandle::AMDGPUAdapterHandle() : totDeviceCount(0)
{
    errno = 0;
    DIR* dirp = opendir("/sys/class/drm");

    if (dirp == nullptr)
    {
        throw Error(errno, "Unable to open 'sys/class/drm'");
    }

    errno = 0;
    struct dirent* dire;

    while ((dire = readdir(dirp)) != nullptr)
    {
        if (::strncmp(dire->d_name, "card", 4) != 0)
        {
            continue; // is not card directory
        }

        const char* p;

        for (p = dire->d_name + 4; ::isdigit(*p); p++);

        if (*p != 0)
        {
            continue; // is not card directory
        }

        errno = 0;

        unsigned int v = ::strtoul(dire->d_name + 4, nullptr, 10);

        totDeviceCount = std::max(totDeviceCount, v + 1);
    }

    if (errno != 0)
    {
        closedir(dirp);
        throw Error(errno, "Unable to read directory 'sys/class/drm'");
    }

    closedir(dirp);

    // filter AMD GPU cards
    char dbuf[120];

    for (unsigned int i = 0; i < totDeviceCount; i++)
    {
        snprintf(dbuf, 120, "/sys/class/drm/card%u/device/vendor", i);

        unsigned int vendorId = 0;

        if (!getFileContentValue(dbuf, vendorId))
        {
            continue;
        }

        if (vendorId != 4098) // if not AMD
        {
            continue;
        }

        amdDevices.push_back(i);
    }

    /* hwmon indices */
    for (unsigned int cardIndex: amdDevices)
    {
        // search hwmon
        errno = 0;

        snprintf(dbuf, 120, "/sys/class/drm/card%u/device/hwmon", cardIndex);
        DIR* dirp = opendir(dbuf);

        if (dirp == nullptr)
        {
            throw Error(errno, "Unable to open directory 'sys/class/drm/card?/device/hwmon'");
        }

        errno = 0;
        struct dirent* dire;
        unsigned int hwmonIndex = UINT_MAX;

        while ( (dire = readdir(dirp)) != nullptr)
        {
            if (::strncmp(dire->d_name, "hwmon", 5) != 0)
            {
                continue; // is not hwmon directory
            }

            const char* p;
            for (p = dire->d_name + 5; ::isdigit(*p); p++);

            if (*p != 0)
            {
                continue; // is not hwmon directory
            }

            errno = 0;
            unsigned int v = ::strtoul(dire->d_name + 5, nullptr, 10);
            hwmonIndex = std::min(hwmonIndex, v);
        }

        if (errno != 0)
        {
            closedir(dirp);
            throw Error(errno, "Unable to open directory 'sys/class/drm/card?/hwmon'");
        }

        closedir(dirp);

        if (hwmonIndex == UINT_MAX)
        {
            throw Error("Unable to find hwmon? directory");
        }

        hwmonIndices.push_back(hwmonIndex);
    }
}

static std::vector<unsigned int> parseDPMFile(const char* filename, uint32_t& choosen)
{
    std::vector<uint32_t> out;
    std::ifstream ifs(filename, std::ios::binary);

    choosen = UINT32_MAX;

    while (ifs)
    {
        std::string line;
        std::getline(ifs, line);

        if (line.empty())
        {
            break;
        }

        char* p = (char*)line.c_str();
        char* p2 = (char*)line.c_str();
        errno = 0;
        unsigned int index = strtoul(p, &p2, 10);

        if (errno !=0 || p == p2)
        {
            throw Error(errno, "Unable to parse index");
        }

        p = p2;

        if (*p != ':' || p[1] != ' ')
        {
            throw Error(errno, "Unable to parse next part of line");
        }

        p += 2;

        unsigned int clock = strtoul(p, &p2, 10);

        if (errno != 0 || p == p2)
        {
            throw Error(errno, "Unable to parse clock");
        }

        p = p2;

        if (::strncmp(p, "Mhz", 3) != 0)
        {
            throw Error(errno, "Unable to parse next part of line");
        }

        p += 3;

        if (*p == ' ' && p[1] == '*')
        {
            choosen = index;
        }

        out.resize(index + 1);
        out[index] = clock;
    }

    return out;
}

static void parseDPMPCIEFile(const char* filename, unsigned int& pcieMB, unsigned int& lanes)
{
    std::ifstream ifs(filename, std::ios::binary);

    unsigned int ilanes = 0, ipcieMB = 0;

    while (ifs)
    {
        std::string line;
        std::getline(ifs, line);

        if (line.empty())
        {
            break;
        }

        char* p = (char*)line.c_str();
        char* p2 = (char*)line.c_str();

        errno = 0;

        strtoul(p, &p2, 10);

        if (errno!=0 || p==p2)
        {
            throw Error(errno, "Unable to parse index");
        }

        p = p2;

        if (*p != ':' || p[1] != ' ')
        {
            throw Error(errno, "Unable to parse next part of line");
        }

        p += 2;
        double bandwidth = strtod(p, &p2);

        if (errno != 0 || p == p2)
        {
            throw Error(errno, "Unable to parse bandwidth");
        }

        p = p2;

        if (*p == 'G' && p2[1] == 'B')
        {
            ipcieMB = bandwidth * 1000;
        }
        else if (*p == 'M' && p2[1] == 'B')
        {
            ipcieMB = bandwidth;
        }
        else if (*p == 'M' && p2[1] == 'B')
        {
            ipcieMB = bandwidth / 1000;
        }
        else
        {
            throw Error(errno, "Invalid bandwidth specified");
        }

        p += 2;

        if (::strncmp(p, ", x", 3) != 0)
        {
            throw Error(errno, "Unable to parse next part of line");
        }

        errno = 0;
        ilanes = strtoul(p, &p2, 10);

        if (errno != 0 || p == p2)
        {
            throw Error(errno, "Unable to parse lanes");
        }

        if (*p == ' ' && p[1] == '*')
        {
            lanes = ilanes;
            pcieMB = ipcieMB;
            break;
        }

    }
}

void AMDGPUAdapterHandle::getPerformanceClocks(int adapterIndex, unsigned int& coreClock, unsigned int& memoryClock) const
{
    char dbuf[120];
    unsigned int cardIndex = amdDevices[adapterIndex];

    unsigned int coreOD = 0;
    snprintf(dbuf, 120, "/sys/class/drm/card%u/device/pp_sclk_od", cardIndex);

    getFileContentValue(dbuf, coreOD);

    unsigned int memoryOD = 0;
    snprintf(dbuf, 120, "/sys/class/drm/card%u/device/pp_mclk_od", cardIndex);

    getFileContentValue(dbuf, memoryOD);

    snprintf(dbuf, 120, "/sys/class/drm/card%u/device/pp_dpm_sclk", cardIndex);

    unsigned int activeClockIndex;
    std::vector<unsigned int> clocks = parseDPMFile(dbuf, activeClockIndex);
    coreClock = 0;

    if (!clocks.empty())
    {
        coreClock = int(ceil(double(clocks.back()) / (1.0 + coreOD * 0.01)));
    }

    snprintf(dbuf, 120, "/sys/class/drm/card%u/device/pp_dpm_mclk", cardIndex);
    clocks = parseDPMFile(dbuf, activeClockIndex);
    memoryClock = 0;

    if (!clocks.empty())
    {
        memoryClock = int(ceil(double(clocks.back()) / (1.0 + memoryOD * 0.01)));
    }
}

AMDGPUAdapterInfo AMDGPUAdapterHandle::parseAdapterInfo(int index)
{
    AMDGPUAdapterInfo adapterInfo;
    unsigned int cardIndex = amdDevices[index];
    char dbuf[120];
    char rlink[120];

    snprintf(dbuf, 120, "/sys/class/drm/card%u/device", cardIndex);

    // currently throws a warning at compile time
    ::readlink(dbuf, rlink, 120);
    rlink[119] = 0;

    getFromPCI_AMDGPU(rlink, adapterInfo);

    // parse pp_dpm_sclk
    snprintf(dbuf, 120, "/sys/class/drm/card%u/device/pp_dpm_sclk", cardIndex);

    unsigned int activeCoreClockIndex;
    adapterInfo.coreClocks = parseDPMFile(dbuf, activeCoreClockIndex);

    if (activeCoreClockIndex!=UINT_MAX)
    {
      adapterInfo.coreClock = adapterInfo.coreClocks[activeCoreClockIndex];
    }
    else
    {
      adapterInfo.coreClock = 0;
    }

    // parse pp_dpm_mclk
    snprintf(dbuf, 120, "/sys/class/drm/card%u/device/pp_dpm_mclk", cardIndex);

    unsigned int activeMemoryClockIndex;
    adapterInfo.memoryClocks = parseDPMFile(dbuf, activeMemoryClockIndex);

    if (activeMemoryClockIndex!=UINT_MAX)
    {
      adapterInfo.memoryClock = adapterInfo.memoryClocks[activeMemoryClockIndex];
    }
    else
    {
      adapterInfo.memoryClock = 0;
    }

    unsigned int hwmonIndex = hwmonIndices[index];

    snprintf(dbuf, 120, "/sys/class/drm/card%u/device/pp_sclk_od", cardIndex);

    getFileContentValue(dbuf, adapterInfo.coreOD);

    snprintf(dbuf, 120, "/sys/class/drm/card%u/device/pp_mclk_od", cardIndex);

    getFileContentValue(dbuf, adapterInfo.memoryOD);

    // get fanspeed
    snprintf(dbuf, 120, "/sys/class/drm/card%u/device/hwmon/hwmon%u/pwm1_min", cardIndex, hwmonIndex);

    getFileContentValue(dbuf, adapterInfo.minFanSpeed);

    snprintf(dbuf, 120, "/sys/class/drm/card%u/device/hwmon/hwmon%u/pwm1_max", cardIndex, hwmonIndex);

    getFileContentValue(dbuf, adapterInfo.maxFanSpeed);

    snprintf(dbuf, 120, "/sys/class/drm/card%u/device/hwmon/hwmon%u/pwm1", cardIndex, hwmonIndex);

    getFileContentValue(dbuf, adapterInfo.fanSpeed);

    snprintf(dbuf, 120, "/sys/class/drm/card%u/device/hwmon/hwmon%u/pwm1_enable", cardIndex, hwmonIndex);

    unsigned int pwmEnable = 0;

    getFileContentValue(dbuf, pwmEnable);

    adapterInfo.defaultFanSpeed = pwmEnable==2;

    snprintf(dbuf, 120, "/sys/class/drm/card%u/device/hwmon/hwmon%u/temp1_input", cardIndex, hwmonIndex);

    getFileContentValue(dbuf, adapterInfo.temperature);

    snprintf(dbuf, 120, "/sys/class/drm/card%u/device/hwmon/hwmon%u/temp1_crit", cardIndex, hwmonIndex);

    getFileContentValue(dbuf, adapterInfo.tempCritical);

    // parse GPU load
    snprintf(dbuf, 120, "/sys/kernel/debug/dri/%u/amdgpu_pm_info", cardIndex);
    {
        adapterInfo.gpuLoad = -1;

        std::ifstream ifs(dbuf, std::ios::binary);

        while (ifs)
        {
            std::string line;
            std::getline(ifs, line);

            if (line.compare(0, 10, "GPU load: ") == 0 || line.compare(0, 10, "GPU Load: ") == 0)
            {
                errno = 0;
                char* endp;
                adapterInfo.gpuLoad = strtoul(line.c_str()+10, &endp, 10);

                if (errno != 0 || endp == line.c_str()+10)
                {
                    throw Error("Unable to parse GPU load");
                }

                break;
            }
        }
    }

    snprintf(dbuf, 120, "/sys/class/drm/card%u/pp_dpm_pcie", cardIndex);

    parseDPMPCIEFile(dbuf, adapterInfo.busLanes, adapterInfo.busSpeed);

    return adapterInfo;
}

void AMDGPUAdapterHandle::setFanSpeed(int index, int fanSpeed) const
{
    char dbuf[120];
    unsigned int cardIndex = amdDevices[index];
    unsigned int hwmonIndex = hwmonIndices[index];

    snprintf(dbuf, 120, "/sys/class/drm/card%u/device/hwmon/hwmon%u/pwm1_enable", cardIndex, hwmonIndex);

    writeFileContentValue(dbuf, 1);

    unsigned int minFanSpeed, maxFanSpeed;

    snprintf(dbuf, 120, "/sys/class/drm/card%u/device/hwmon/hwmon%u/pwm1_min", cardIndex, hwmonIndex);

    getFileContentValue(dbuf, minFanSpeed);

    snprintf(dbuf, 120, "/sys/class/drm/card%u/device/hwmon/hwmon%u/pwm1_max", cardIndex, hwmonIndex);

    getFileContentValue(dbuf, maxFanSpeed);

    snprintf(dbuf, 120, "/sys/class/drm/card%u/device/hwmon/hwmon%u/pwm1", cardIndex, hwmonIndex);

    writeFileContentValue(dbuf, int( round( fanSpeed/100.0 * (maxFanSpeed-minFanSpeed) + minFanSpeed) ) );
}

void AMDGPUAdapterHandle::setFanSpeedToDefault(int index) const
{
    char dbuf[120];
    unsigned int cardIndex = amdDevices[index];
    unsigned int hwmonIndex = hwmonIndices[index];

    snprintf(dbuf, 120, "/sys/class/drm/card%u/device/hwmon/hwmon%u/pwm1_enable", cardIndex, hwmonIndex);

    writeFileContentValue(dbuf, 2);
}

void AMDGPUAdapterHandle::setOverdriveCoreParam(int index, unsigned int coreOD) const
{
    char dbuf[120];
    unsigned int cardIndex = amdDevices[index];

    snprintf(dbuf, 120, "/sys/class/drm/card%u/device/pp_sclk_od", cardIndex);

    writeFileContentValue(dbuf, coreOD);
}

void AMDGPUAdapterHandle::setOverdriveMemoryParam(int index, unsigned int memoryOD) const
{
    char dbuf[120];
    unsigned int cardIndex = amdDevices[index];

    snprintf(dbuf, 120, "/sys/class/drm/card%u/device/pp_mclk_od", cardIndex);

    writeFileContentValue(dbuf, memoryOD);
}
