#ifndef CLIPARAMETERS_H
#define CLIPARAMETERS_H

#include "amdgpuproprocessing.h"
#include "catalystcrimsonprocessing.h"
#include "conststrings.h"
#include "structs.h"
#include "adapterslist.h"

class CliParameters
{

private:

  pci_access* pciAccess;

  pci_filter pciFilter;

  std::vector<OVCParameter> ovcParameters;

  std::vector<int> chosenAdapters;

  bool chooseAllAdapters;

  bool parseOVCParameter(const char* string, OVCParameter& param);

public:

  bool SetPrintHelp(const char* Argvi);

  void CheckPrintHelp(bool PrintHelp);

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
};

#endif /* CLIPARAMETERS_H */
