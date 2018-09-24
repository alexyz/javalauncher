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

	const wchar_t *e = Create(dir, optionsv, 1);
	if (e) {
		wprintf(L"wmain: create error: %s\n", e);
		return 1;
	}
	
	for (int n = 0; n < 2; n++) {
		wprintf(L"wmain: run %d\n", n);
		e = Run(mainclassname, mainargsv, 1);
		if (e) {
			wprintf(L"wmain: run error: %s\n", e);
			return 1;
		}
	}
	
	e = Destroy(1);
	if (e) {
		wprintf(L"wmain: destroy error: %s\n", e);
		return 1;
	}
	
	wprintf(L"wmain exit\n");
	return 0;
}
