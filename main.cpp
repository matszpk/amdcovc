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

#include "cliparameters.h"

int main(int argc, const char** argv)
try
{
    CliParameters *cli = new CliParameters();
    bool printHelp = false;
    bool printVersion = false;
    bool printVerbose = false;
    bool useAdaptersList = false;
    bool failed = false;

    for (int i = 1; i < argc; i++)
    {
        printHelp |= cli.SetPrintHelp(argv[i]);
        printVersion |= cli.SetPrintVersion(argv[i]);
        printVerbose |= cli.SetPrintVerbose(argv[i]);
        useAdaptersList |= cli.SetUseAdaptersListEquals(argv[i]);
        useAdaptersList |= cli.SetUseAdaptersList(argv, argc, i);
        useAdaptersList |= cli.ParseAdaptersList(argv, argc, i);

        if( !( printVersion | printHelp | printVerbose | useAdaptersList ) )
        {
            failed |= cli.ParseParametersOrFail(argv[i]);
        }
    }

    cli.CheckFailed(failed);
    cli.CheckPrintVersion(printVersion);
    cli.CheckPrintHelp(printHelp);

    if( printVersion | printHelp )
    {
        return 0;
    }

    cli.ProcessParameters(useAdaptersList, printVerbose);
    cli.CleanupPciAccess();

    return 0;
}
catch(const std::exception& ex)
{
    cli.CleanupPciAccess();

    std::cerr << ex.what() << std::endl;

    return 1;
}
