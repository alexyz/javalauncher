#pragma once
#ifdef LAUNCHER_EXPORTS
#define LAUNCHER_API extern "C" __declspec(dllexport)
#else
#define LAUNCHER_API extern "C" __declspec(dllimport)
#endif

LAUNCHER_API void AlexSetLog (int l);
LAUNCHER_API int AlexCreateVm (LPWSTR jredir, LPWSTR *jreargs);
LAUNCHER_API int AlexRunMain(LPWSTR mainclassname, LPWSTR *mainargs);
LAUNCHER_API int AlexDestroyVm ();
