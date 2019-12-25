// javatest.cpp : Defines the entry point for the console application.
//

#include <SDKDDKVer.h>
#include <windows.h>
#include <stdio.h>

#include "..\Project1\launcher.h"

bool startswith(const wchar_t* str, const wchar_t* pre)
{
	return wcsncmp(pre, str, wcslen(pre)) == 0;
}

// x64\Release\LauncherTest.exe "c:\Dev\openjdk-11+28" java Hello 1 2 3
int wmain(int argc, wchar_t** argv)
{
	printf("wmain start\n");
	wchar_t* dir = 0;
	wchar_t** optionsv = new wchar_t* [255];
	int optionsc = 0;
	wchar_t* mainclassname = 0;
	wchar_t** mainargsv = new wchar_t* [255];
	int mainargsc = 0;
	wchar_t classpath[255] = { 0 };

	for (int n = 1; n < argc; n++) {
		if (n == 1) {
			printf("wmain: jvm dir = %S\n", argv[n]);
			dir = argv[n];
		}
		else if (n == 2) {
			swprintf(classpath, 255, L"-Djava.class.path=%s", argv[n]);
			optionsv[optionsc++] = classpath;
		}
		else if (startswith(argv[n], L"-D")) {
			printf("wmain: jvm option = %S\n", argv[n]);
			optionsv[optionsc++] = argv[n];
		}
		else if (!mainclassname) {
			printf("wmain: main class = %S\n", argv[n]);
			mainclassname = argv[n];
		}
		else {
			printf("wmain: main class arg = %S\n", argv[n]);
			mainargsv[mainargsc++] = argv[n];
		}
	}

	optionsv[optionsc] = 0;
	mainargsv[mainargsc] = 0;

	AlexSetLog(1);

	int e = AlexCreateVm(dir, optionsv);
	if (e) {
		printf("wmain: create error: %d\n", e);
		return 1;
	}

	for (int n = 0; n < 2; n++) {
		printf("wmain: run %d\n", n);
		e = AlexRunMain(mainclassname, mainargsv);
		if (e) {
			printf("wmain: run error: %d\n", e);
			return 1;
		}
	}

	e = AlexDestroyVm();
	if (e) {
		printf("wmain: destroy error: %d\n", e);
		return 1;
	}

	printf("wmain exit\n");
	return 0;
}
