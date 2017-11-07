#include "cliparameters.h"

pci_access* pciAccess = nullptr;

pci_filter pciFilter;

std::vector<OVCParameter> ovcParameters;

std::vector<int> chosenAdapters;

bool chooseAllAdapters = false;


bool CliParameters::SetPrintHelp(const char* Argvi)
{
    if (::strcmp(Argvi, "--help") == 0 || ::strcmp(Argvi, "-?") == 0)
    {
        return true;
    }

    return false;
}

void CliParameters::CheckPrintHelp(bool PrintHelp)
{
    if (PrintHelp)
    {
        std::cout << ConstStrings::HelpAndUsage << "\n" << ConstStrings::OverdriveWarning;
        std::cout.flush();
    }
}

bool CliParameters::SetPrintVersion(const char* Argvi)
{
    if (::strcmp(Argvi, "--version") == 0)
    {
        return true;
    }

    return false;
}

void CliParameters::CheckPrintVersion(bool PrintVersion)
{
    if (PrintVersion)
    {
        std::cout << ConstStrings::Version;
        std::cout.flush();
    }
}

void CliParameters::CheckFailed(bool failed)
{
    if (failed)
    {
        throw Error("Unable to parse parameters");
    }
}

void CliParameters::ProcessParameters(bool UseAdaptersList, bool PrintVerbose)
{
    ATIADLHandle handle;

    if (handle.open())
    {
        CatalystCrimsonProcessing *processor = new CatalystCrimsonProcessing();
        processor->Process(handle, UseAdaptersList, chosenAdapters, ovcParameters, chooseAllAdapters, PrintVerbose);
        delete processor;
    }
    else
    {
        AmdGpuProProcessing *processor = new AmdGpuProProcessing();
        processor->Process(ovcParameters, UseAdaptersList, chosenAdapters, chooseAllAdapters, PrintVerbose);
        delete processor;
    }
}

void CliParameters::CleanupPciAccess()
{
    if (pciAccess != nullptr)
    {
        pci_cleanup(pciAccess);
    }
}


bool CliParameters::SetPrintVerbose(const char* Argvi)
{
    if (::strcmp(Argvi, "--verbose") == 0 || ::strcmp(Argvi, "-v") == 0)
    {
        return true;
    }

    return false;
}

bool CliParameters::SetUseAdaptersListEquals(const char* Argvi)
{
    if (::strncmp(Argvi, "--adapters=", 11) == 0)
    {
        AdaptersList::Parse(Argvi + 11, chosenAdapters, chooseAllAdapters);
        return true;
    }
    return false;
}

bool CliParameters::SetUseAdaptersList(const char** Argv, int Argc, int I)
{
  if (::strcmp(Argv[I], "--adapters") == 0)
  {
      if ( I + 1 < Argc)
      {
          AdaptersList::Parse(Argv[++I], chosenAdapters, chooseAllAdapters);
          return true;
      }
      else
      {
          throw Error("Adapter list not supplied.");
      }
  }

  return false;
}

bool CliParameters::ParseParametersOrFail(const char* Argvi)
{
    OVCParameter param;

    if (this->parseOVCParameter(Argvi, param))
    {
        ovcParameters.push_back(param);
    }
    else
    {
        return true;
    }

    return false;
}

bool CliParameters::ParseAdaptersList(const char** Argv, int Argc, int I)
{
    if (::strncmp(Argv[I], "-a", 2) == 0)
    {
        if (Argv[I][2] != 0)
        {
            AdaptersList::Parse(Argv[I] + 2, chosenAdapters, chooseAllAdapters);
        }
        else if (I + 1 < Argc)
        {
            AdaptersList::Parse(Argv[++I], chosenAdapters, chooseAllAdapters);
        }
        else
        {
            throw Error("Adapter list not supplied.");
        }

        return true;
    }

    return false;
}

bool CliParameters::parseOVCParameter(const char* string, OVCParameter& param)
{
    const char* afterName = strchr(string, ':');

    if (afterName==nullptr)
    {
        afterName = strchr(string, '=');

        if (afterName==nullptr)
        {
            std::cerr << "Invalid parameter specified: '" << string << "'!" << std::endl;

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
                AdaptersList::Parse(listString.c_str(), param.adapters, param.allAdapters);
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
            std::cerr << "Invalid data in '" << string << "'!" << std::endl;
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
