#pragma once
#ifdef LAUNCHER_EXPORTS
#define LAUNCHER_API extern "C" __declspec(dllexport)
#else
#define LAUNCHER_API extern "C" __declspec(dllimport)
#endif

LAUNCHER_API const wchar_t *LaunchImpl(WCHAR *jredir, WCHAR **jreargs, WCHAR *mainclassname, WCHAR **mainargs, int log);
