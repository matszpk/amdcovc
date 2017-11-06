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

#define AMDCOVC_VERSION "0.4.0"

pci_access* pciAccess = nullptr;

pci_filter pciFilter;

static const char* helpAndUsageString =
    "amdcovc " AMDCOVC_VERSION " by Mateusz Szpakowski (matszpk@interia.pl)\n"
    "This program is distributed under terms of the GPLv2.\n"
    "and is available at https://github.com/matszpk/amdcovc.\n"
    "\n"
    "Usage: amdcovc [--help|-?] [--verbose|-v] [-a LIST|--adapters=LIST] [PARAM ...]\n"
    "Prints AMD Overdrive information if no parameters are given.\n"
    "Sets AMD Overdrive parameters (clocks, fanspeeds,...) if any parameters are given.\n"
    "\n"
    "List of options:\n"
    "  -a, --adapters=LIST       print informations only for these adapters\n"
    "  -v, --verbose             print verbose informations\n"
    "      --version             print version\n"
    "  -?, --help                print help\n"
    "\n"
    "List of parameters:\n"
    "  coreclk[:[ADAPTERS][:LEVEL]]=CLOCK    set core clock in MHz\n"
    "  memclk[:[ADAPTERS][:LEVEL]]=CLOCK     set memory clock in MHz\n"
    "  coreod[:[ADAPTERS][:LEVEL]]=PERCENT   set core Overdrive in percent (AMDGPU)\n"
    "  memod[:[ADAPTERS][:LEVEL]]=PERCENT    set memory Overdrive in percent (AMDGPU)\n"
    "  vcore[:[ADAPTERS][:LEVEL]]=VOLTAGE    set Vddc voltage in Volts\n"
    "  icoreclk[:ADAPTERS]=CLOCK             set core clock in MHz for idle level\n"
    "  imemclk[:ADAPTERS]=CLOCK              set memory clock in MHz for idle level\n"
    "  ivcore[:ADAPTERS]=VOLTAGE             set Vddc voltage in Volts for idle level\n"
    "  fanspeed[:[ADAPTERS][:THID]]=PERCENT  set fanspeed by percentage\n"
    "\n"
    "Extra specifiers in parameters:\n"
    "  ADAPTERS                  adapter (devices) index list (default is 0)\n"
    "  LEVEL                     performance level (typically 0 or 1, default is last)\n"
    "  THID                      thermal controller index (must be 0)\n"
    "You can use 'default' in place of a value to set default value.\n"
    "For fanspeed the 'default' value forces automatic speed setup.\n"
    "\n"
    "Adapter list specified in the parameters and '--adapter' options are a comma-separated list\n"
    "with ranges 'first-last' or 'all'. e.g. 'all', '0-2', '0,1,3-5'\n"
    "\n"
    "Example usage:\n"
    "\n"
    "amdcovc\n"
    "    print short information about the state of all adapters\n\n"
    "amdcovc -a 1,2,4-6\n"
    "    print short information about adapter 1, 2 and 4 to 6\n\n"
    "amdcovc coreclk:1=900 coreclk=1000\n"
    "    set core clock to 900 for adapter 1, set core clock to 1000 for adapter 0\n\n"
    "amdcovc coreclk:1:0=900 coreclk:0:1=1000\n"
    "    set core clock to 900 for adapter 1 at performance level 0,\n"
    "    set core clock to 1000 for adapter 0 at performance level 1\n\n"
    "amdcovc coreclk:1:0=default coreclk:0:1=default\n"
    "    set core clock to default for adapter 0 and 1\n\n"
    "amdcovc fanspeed=75 fanspeed:2=60 fanspeed:1=default\n"
    "    set fanspeed to 75% for adapter 0 and set fanspeed to 60% for adapter 2\n"
    "    set fanspeed to default for adapter 1\n\n"
    "amdcovc vcore=1.111 vcore::0=0.81\n"
    "    set Vddc voltage to 1.111 V for adapter 0\n"
    "    set Vddc voltage to 0.81 for adapter 0 for performance level 0\n\n"
    "\n"
    "WARNING: Before setting any of the AMD Overdrive parameters,\n"
    "please stop any processes doing GPU computations and renderings.\n"
    "Please use this utility carefully, as it can damage your hardware.\n"
    "\n"
    "If the X11 server is not running, this program will require root privileges.\n";

static const char* versionString =
    "amdcovc " AMDCOVC_VERSION
    " by Mateusz Szpakowski (matszpk@interia.pl)\n"
    "Program is distributed under terms of the GPLv2.\n"
    "Program available at https://github.com/matszpk/amdcovc.\n";

int main(int argc, const char** argv)
try
{
    bool printHelp = false;
    bool printVerbose = false;
    std::vector<OVCParameter> ovcParameters;
    std::vector<int> chosenAdapters;
    bool useAdaptersList = false;
    bool chooseAllAdapters = false;

    bool failed = false;

    for (int i = 1; i < argc; i++)
    {
        if (::strcmp(argv[i], "--help") == 0 || ::strcmp(argv[i], "-?") == 0)
        {
            printHelp = true;
        }
        else if (::strcmp(argv[i], "--verbose") == 0 || ::strcmp(argv[i], "-v") == 0)
        {
            printVerbose = true;
        }
        else if (::strncmp(argv[i], "--adapters=", 11) == 0)
        {
            Adapters::ParseAdaptersList(argv[i] + 11, chosenAdapters, chooseAllAdapters);
            useAdaptersList = true;
        }
        else if (::strcmp(argv[i], "--adapters") == 0)
        {
            if ( i + 1 < argc)
            {
                Adapters::ParseAdaptersList(argv[++i], chosenAdapters, chooseAllAdapters);
                useAdaptersList = true;
            }
            else
            {
                throw Error("Adapter list not supplied");
            }
        }
        else if (::strncmp(argv[i], "-a", 2)==0)
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

            useAdaptersList = true;
        }
        else if (::strcmp(argv[i], "--version") == 0)
        {
            std::cout << versionString;
            std::cout.flush();
            return 0;
        }
        else
        {
            OVCParameter param;

            if (Parameters::ParseOVCParameter(argv[i], param))
            {
                ovcParameters.push_back(param);
            }
            else
            {
                failed = true;
            }
        }
    }

    if (printHelp)
    {
        std::cout << helpAndUsageString;
        std::cout.flush();
        return 0;
    }

    if (failed)
    {
        throw Error("Unable to parse parameters");
    }

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

    if (pciAccess != nullptr)
    {
        pci_cleanup(pciAccess);
    }

    return 0;
}
catch(const std::exception& ex)
{
    if (pciAccess != nullptr)
    {
        pci_cleanup(pciAccess);
    }

    std::cerr << ex.what() << std::endl;

    return 1;
}
