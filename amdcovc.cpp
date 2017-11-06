/*
 *  AMDCOVC - AMD Console OVerdrive Control utility
 *  Copyright (C) 2016 Mateusz Szpakowski
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifdef __linux__
#define LINUX 1
#endif

#include "parameters.h"
#include "amdgpuproprocessing.h"
#include "catalystcrimsonprocessing.h"
#include "conststrings.h"

#define AMDCOVC_VERSION "0.4.0"

pci_access* pciAccess = nullptr;

pci_filter pciFilter;

std::vector<OVCParameter> ovcParameters;

std::vector<int> chosenAdapters;

bool chooseAllAdapters = false;


bool checkPrintHelp(bool printHelp)
{
    if (printHelp)
    {
        std::cout << ConstStrings::HelpAndUsage;
        std::cout.flush();
        return true;
    }

    return false;
}

void checkFailed(bool failed)
{
    if (failed)
    {
        throw Error("Unable to parse parameters");
    }
}

void processParameters(bool useAdaptersList, std::std::vector<int> chosenAdapters, std::std::vector<OVCParameter> ovcParameters, bool chooseAllAdapters
                      bool printVerbose)
{
    ATIADLHandle handle;

    if (handle.open())
    {
        CatalystCrimsonProcessing *processor = new CatalystCrimsonProcessing();
        processor->Process(handle, useAdaptersList, chosenAdapters, ovcParameters, chooseAllAdapters, printVerbose);
        delete processor;
    }
    else
    {
        AmdGpuProProcessing *processor = new AmdGpuProProcessing();
        processor->Process(ovcParameters, useAdaptersList, chosenAdapters, chooseAllAdapters, printVerbose);
        delete processor;
    }
}

void cleanupPciAccess()
{
    if (pciAccess != nullptr)
    {
        pci_cleanup(pciAccess);
    }
}

bool setPrintHelp(char** argv, int i)
{
    if (::strcmp(argv[i], "--help") == 0 || ::strcmp(argv[i], "-?") == 0)
    {
        return true;
    }

    return false;
}

bool setPrintVerbose(char** argv, int i)
{
    if (::strcmp(argv[i], "--verbose") == 0 || ::strcmp(argv[i], "-v") == 0)
    {
        return true;
    }

    return false;
}

bool setUseAdaptersListEquals(char** argv, int i)
{
    if (::strncmp(argv[i], "--adapters=", 11) == 0)
    {
        Adapters::ParseAdaptersList(argv[i] + 11, chosenAdapters, chooseAllAdapters);
        return true;
    }
    return false;
}

bool setUseAdaptersList(char** argv, int i);
{
  if (::strcmp(argv[i], "--adapters") == 0)
  {
      if ( i + 1 < argc)
      {
          Adapters::ParseAdaptersList(argv[++i], chosenAdapters, chooseAllAdapters);
          return true;
      }
      else
      {
          throw Error("Adapter list not supplied");
      }
  }

  return false;
}

bool parseParametersOrFail(char** argv, int i)
{
    OVCParameter param;

    if (Parameters::ParseOVCParameter(argv[i], param))
    {
        ovcParameters.push_back(param);
    }
    else
    {
        return true;
    }

    return false;
}

bool parseAdaptersList(char** argv, int i)
{
    if (::strncmp(argv[i], "-a", 2)==0)
    {
        if (argv[i][2] != 0)
        {
            Adapters::ParseAdaptersList(argv[i] + 2, chosenAdapters, chooseAllAdapters);
        }
        else if (i + 1 < argc)
        {
            Adapters::ParseAdaptersList(argv[++i], chosenAdapters, chooseAllAdapters);
        }
        else
        {
            throw Error("Adapter list not supplied");
        }

        return true;
    }

    return false;
}

bool checkPrintVersion(char** argv, int i)
{
    if (::strcmp(argv[i], "--version") == 0)
    {
        std::cout << ConstStrings::Version;
        std::cout.flush();
        return true;
    }

    return false;
}

int main(int argc, const char** argv)
try
{
    bool printHelp = false;
    bool printVerbose = false;
    bool useAdaptersList = false;
    bool failed = false;

    for (int i = 1; i < argc; i++)
    {
        printHelp = setPrintHelp(argc, i);

        printVerbose = setPrintVerbose(argc, i);

        useAdaptersList = setUseAdaptersListEquals(argv, i);

        useAdaptersList |= setUseAdaptersList(argv, i);

        useAdaptersList |= parseAdaptersList(argv, i);

        if(checkPrintVersion(argv, i))
        {
            return 0;
        }

        failed = parseParametersOrFail(argv, i);
    }

    if(checkPrintHelp(printHelp))
    {
        return 0;
    }

    checkFailed(failed);

    processParameters(useAdaptersList, chosenAdapters, ovcParameters, chooseAllAdapters, printVerbose);

    cleanupPciAccess();

    return 0;
}
catch(const std::exception& ex)
{
    cleanupPciAccess();

    std::cerr << ex.what() << std::endl;

    return 1;
}
