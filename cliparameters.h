#ifndef CLIPARAMETERS_H
#define CLIPARAMETERS_H

#include "parameters.h"
#include "amdgpuproprocessing.h"
#include "catalystcrimsonprocessing.h"
#include "conststrings.h"

class CliParameters
{

private:

  pci_access* pciAccess;

  pci_filter pciFilter;

  std::vector<OVCParameter> ovcParameters;

  std::vector<int> chosenAdapters;

  bool chooseAllAdapters;

public:

  bool SetPrintHelp(const char* Argvi);

  bool CheckPrintHelp(bool PrintHelp);

  bool SetPrintVersion(const char* Argvi);

  void CheckPrintVersion(bool PrintVersion);

  void CheckFailed(bool Failed);

  void ProcessParameters(bool UseAdaptersList, bool PrintVerbose);

  void CleanupPciAccess();

  bool SetPrintVerbose(const char* Argvi);

  bool SetUseAdaptersListEquals(const char* Argvi);

  bool SetUseAdaptersList(const char** Argv, int Argc, int I);

  bool ParseParametersOrFail(const char* Argvi);

  bool ParseAdaptersList(const char** Argv, int Argc, int I);
}

#endif /* CLIPARAMETERS_H */
