#pragma once
#ifdef LAUNCHER_EXPORTS
#define LAUNCHER_API extern "C" __declspec(dllexport)
#else
#define LAUNCHER_API extern "C" __declspec(dllimport)
#endif

LAUNCHER_API const wchar_t *Create(WCHAR *jredir, WCHAR **jreargs, int log);
LAUNCHER_API const wchar_t *Run(WCHAR *mainclassname, WCHAR **mainargs, int log);
LAUNCHER_API const wchar_t *Destroy(int log);
