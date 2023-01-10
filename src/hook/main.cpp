#include "crash_report.hpp"
#include "dictionary.h"
#include "hooks.h"
#include "logger.hpp"

extern "C" __declspec(dllexport) VOID NullExport(VOID)
{
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
  if (Config::Setting::crash_report) {
    CrashReport::Install();
  }

  switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH: {
      InitLogger();

      if (Config::Metadata::name != "dfint localization hook") {
        logger::critical("unable to find config file");
        MessageBoxA(nullptr, "unable to find config file", "dfint hook error", MB_ICONERROR);
        exit(2);
      }
      logger::info("pe checksum: 0x{:x}", Config::Metadata::checksum);
      logger::info("offsets version: {}", Config::Metadata::version);

      Dictionary::GetSingleton()->LoadCsv("./dfint_data/dfint_dictionary.csv");

      DetourRestoreAfterWith();
      DetourTransactionBegin();
      DetourUpdateThread(GetCurrentThread());

      InstallTranslation();
      // InstallTTFInjection();
      // InstallStateManager();

      DetourTransactionCommit();
      logger::info("hooks installed");
      break;
    }
    case DLL_PROCESS_DETACH: {
      DetourTransactionBegin();
      DetourUpdateThread(GetCurrentThread());

      UninstallTranslation();
      // UninstallTTFInjection();
      // UninstallStateManager();
      logger::info("hooks uninstalled");

      DetourTransactionCommit();
      break;
    }
    case DLL_THREAD_ATTACH:
      break;
    case DLL_THREAD_DETACH:
      break;
  }
  return TRUE;
}
