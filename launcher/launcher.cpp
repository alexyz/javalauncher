
// you'll probably need this
// https://docs.microsoft.com/en-us/windows/desktop/Debug/system-error-codes

#include <SDKDDKVer.h>
#include <windows.h>
#include <jni.h>

#define LAUNCHER_EXPORTS
#include "launcher.h"

// jint JNI_CreateJavaVM(JavaVM **p_vm, void **p_env, void *vm_args);
typedef jint(__cdecl CREATEVM)(JavaVM **p_vm, JNIEnv **p_env, JavaVMInitArgs *vm_args);

// jint JNI_GetDefaultJavaVMInitArgs(void *vm_args);
typedef jint(__cdecl DEFAULTVM)(JavaVMInitArgs *vm_args);

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {
    return TRUE;
}

char* tochar(WCHAR *ws) {
	int slen = wcslen(ws);
	int ulen = WideCharToMultiByte(CP_UTF8, 0, ws, slen, NULL, 0,    NULL, NULL);
	char *us = new char[ulen];
	WideCharToMultiByte(CP_UTF8, 0, ws, -1,   us,   ulen, NULL, NULL);
	//printf("tochar: slen=%d ulen=%d vlen=%d strlen(us)=%d\n", slen, ulen, vlen, strlen(us));
	//tochar: slen=25 ulen=25 vlen=0 strlen(us)=25
	return us;
}

BOOL isempty(WCHAR *s) {
	return !s || wcslen(s) == 0;
}

int argcount(WCHAR **a) {
	int c = 0;
	for (int n = 0; a[n]; n++) {
		c++;
	}
	return c;
}

// As of JDK/JRE 1.2 , creation of multiple VMs in a single process is not supported.
LAUNCHER_API const WCHAR *LaunchImpl(WCHAR *jredir, WCHAR **jreargs, WCHAR *mainclassname, WCHAR **mainargs, int log) {

	if (isempty(jredir) || !jreargs || isempty(mainclassname) || !mainargs) {
		return L"launchimpl: invalid arguments";
	}

	DWORD jredirattr = GetFileAttributes(jredir);
	if (jredirattr == INVALID_FILE_ATTRIBUTES || !(jredirattr & FILE_ATTRIBUTE_DIRECTORY)) {
		if (log) {
			wprintf(L"launchimpl: GetFileAttributes(%s): %x\n", jredir, jredirattr);
		}
		return L"launchimpl: jredir is not a directory";
	}

	WCHAR dlldir[MAX_PATH];
	swprintf(dlldir, MAX_PATH, L"%s\\bin", jredir);
	BOOL set = SetDllDirectory(dlldir);
	if (!set) {
		if (log) {
			int e = GetLastError();
			wprintf(L"launchimpl: SetDllDirectory(%s): %d\n", dlldir, e);
		}
		return L"launchimpl: could not set dll directory";
	}

	WCHAR *jvmfile = L"server\\jvm.dll";
	HMODULE jvmdll = LoadLibrary(jvmfile);
	if (!jvmdll) {
		if (log) {
			int e = GetLastError();
			wprintf(L"launchimpl: LoadLibrary(%s): %d\n", jvmfile, e);
		}
		return L"launchimpl: could not load jvm.dll (32/64 bit mismatch?)";
	}

	DEFAULTVM *defaultvm = (DEFAULTVM*)GetProcAddress(jvmdll, "JNI_GetDefaultJavaVMInitArgs");
	if (!defaultvm) {
		if (log) {
			wprintf(L"launchimpl: GetProcAddress: %d\n", GetLastError());
		}
		return L"launchimpl: could not find default function";
	}

	CREATEVM *createvm = (CREATEVM*)GetProcAddress(jvmdll, "JNI_CreateJavaVM");
	if (!createvm) {
		if (log) {
			wprintf(L"launchimpl: GetProcAddress: %d\n", GetLastError());
		}
		return L"launchimpl: could not find create function";
	}

	JavaVMInitArgs init;
	init.version = JNI_VERSION_1_8;

	int defok = defaultvm(&init);
	if (defok != JNI_OK) {
		if (log) {
			wprintf(L"launchimpl: GetDefaultJavaVMInitArgs: %d\n", defok);
		}
		return L"launchimpl: could not default jvm";
	}

	init.nOptions = argcount(jreargs);

	init.options = new JavaVMOption[init.nOptions];
	for (int n = 0; n < init.nOptions; n++) {
		init.options[n].optionString = tochar(jreargs[n]);
	}

	init.ignoreUnrecognized = false;

	JavaVM *jvm;
	JNIEnv *env;
	int createok = createvm(&jvm, &env, &init);

	for (int n = 0; n < init.nOptions; n++) {
		delete init.options[n].optionString;
	}

	delete init.options;

	if (createok != JNI_OK) {
		if (log) {
			wprintf(L"launchimpl: CreateJavaVM: %d\n", createok);
		}
		return L"launchimpl: could not create jvm";
	}

	_jclass *stringclass = env->FindClass("java/lang/String");

	int mainargscount = argcount(mainargs);

	_jobjectArray *jmainargs = env->NewObjectArray(mainargscount, stringclass, NULL);

	for (int n = 0; n < mainargscount; n++) {
		//wprintf(L"mainargs[%d]=%s\n", n, mainargs[n]);
		char *t = tochar(mainargs[n]);
		//printf("t=%s\n", t);
		env->SetObjectArrayElement(jmainargs, n, env->NewStringUTF(t));
		delete t;
	}

	_jclass *mainclass;
	{
		char *t = tochar(mainclassname);
		mainclass = env->FindClass(t);
		delete t;
	}

	if (!mainclass) {
		if (log) {
			wprintf(L"launchimpl: could not find main class %s\n", mainclassname);
		}
		return L"launchimpl: could not find main class";
	}

	_jmethodID *mainmethod = env->GetStaticMethodID(mainclass, "main", "([Ljava/lang/String;)V");

	if (!mainmethod) {
		if (log) {
			wprintf(L"launchimpl: could not find main method in %s\n", mainclassname);
		}
		return L"launchimpl: could not find main method";
	}

	env->CallStaticVoidMethod(mainclass, mainmethod, jmainargs);

	int destroyok = jvm->DestroyJavaVM();

	if (destroyok != JNI_OK) {
		if (log) {
			wprintf(L"launchimpl: destroy: %d\n", destroyok);
		}
		return L"launchimpl: could not destroy jvm";
	}

	return NULL;
}


