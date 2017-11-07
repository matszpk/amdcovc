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
        Adapters::ParseAdaptersList(Argvi + 11, chosenAdapters, chooseAllAdapters);
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
          Adapters::ParseAdaptersList(Argv[++I], chosenAdapters, chooseAllAdapters);
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

    if (Parameters::ParseOVCParameter(Argvi, param))
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
            Adapters::ParseAdaptersList(Argv[I] + 2, chosenAdapters, chooseAllAdapters);
        }
        else if (I + 1 < Argc)
        {
            Adapters::ParseAdaptersList(Argv[++I], chosenAdapters, chooseAllAdapters);
        }
        else
        {
            throw Error("Adapter list not supplied.");
        }

        return true;
    }

    return false;
}
