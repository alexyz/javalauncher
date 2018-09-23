// javatest.cpp : Defines the entry point for the console application.
//

#define UNICODE

#include <SDKDDKVer.h>
#include <stdio.h>
#include <tchar.h>
#include <windows.h>

#include "..\launcher\launcher.h"

bool startswith(const wchar_t *str, const wchar_t *pre)
{
	return wcsncmp(pre, str, wcslen(pre)) == 0;
}

int wmain(int argc, wchar_t **argv)
{
	wprintf(L"wmain start\n");
	wchar_t *dir = 0;
	wchar_t **optionsv = new wchar_t*[255];
	int optionsc = 0;
	wchar_t *mainclassname = 0;
	wchar_t **mainargsv = new wchar_t*[255];
	int mainargsc = 0;

	for (int n = 1; n < argc; n++) {
		if (n == 1) {
			wprintf(L"wmain: jvm dir = %s\n", argv[n]);
			dir = argv[n];
		}
		else if (startswith(argv[n], L"-D")) {
			wprintf(L"wmain: jvm option = %s\n", argv[n]);
			optionsv[optionsc++] = argv[n];
		}
		else if (!mainclassname) {
			wprintf(L"wmain: main class = %s\n", argv[n]);
			mainclassname = argv[n];
		}
		else {
			wprintf(L"wmain: main class arg = %s\n", argv[n]);
			mainargsv[mainargsc++] = argv[n];
		}
	}

	optionsv[optionsc] = 0;
	mainargsv[mainargsc] = 0;

	const wchar_t *error = LaunchImpl(dir, optionsv, mainclassname, mainargsv, 1);

	if (error) {
		wprintf(L"wmain: launch error: %s\n", error);
		return 1;
	}

	wprintf(L"wmain exit\n");
	return 0;
}
