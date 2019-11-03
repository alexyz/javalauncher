
// you'll probably need this
// https://docs.microsoft.com/en-us/windows/desktop/Debug/system-error-codes

#include <SDKDDKVer.h>
#include <windows.h>
#include <jni.h>

#define LAUNCHER_EXPORTS
#include "launcher.h"

static JavaVM *jvm = 0;
static JNIEnv *env = 0;
static int log = 0;

#define LOG(...) if (log) { wprintf(__VA_ARGS__); }
#define E GetLastError()

// jint JNI_CreateJavaVM(JavaVM **p_vm, void **p_env, void *vm_args);
typedef jint(__cdecl CREATEVM)(JavaVM **p_vm, JNIEnv **p_env, JavaVMInitArgs *vm_args);

// jint JNI_GetDefaultJavaVMInitArgs(void *vm_args);
typedef jint(__cdecl DEFAULTVM)(JavaVMInitArgs *vm_args);

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {
    return TRUE;
}

static LPSTR wstrToStr(LPWSTR ws) {
	int slen = wcslen(ws);
	int ulen = WideCharToMultiByte(CP_UTF8, 0, ws, slen, NULL, 0,    NULL, NULL) + 1;
	char *us = new char[ulen];
	int vlen = WideCharToMultiByte(CP_UTF8, 0, ws, -1,   us,   ulen, NULL, NULL);
	us[ulen-1] = 0;
	//wprintf(L"tochar: slen=%d ulen=%d vlen=%d strlen(us)=%d\n", slen, ulen, vlen, strlen(us));
	return us;
}

static BOOL wstrIsEmpty(LPWSTR s) {
	return !s || wcslen(s) == 0;
}

static int wstrArrayCount(LPWSTR *a) {
	if (a) {
		int c = 0;
		for (int n = 0; a[n]; n++) {
			c++;
		}
		return c;
	} else {
		return -1;
	}
}

LAUNCHER_API void AlexSetLog (int l) {
	log = l;
	LOG(L"log: %d\n", log);
}

// As of JDK/JRE 1.2 , creation of multiple VMs in a single process is not supported.
LAUNCHER_API int AlexCreateVm (LPWSTR jredir, LPWSTR* jreargs) {
	
	if (jvm) {
		LOG(L"create: jvm already exists\n")
		return -10;
	}

	if (wstrIsEmpty(jredir)) {
		LOG(L"create: invalid jre dir\n")
		return -20;
	}
	
	if (wstrArrayCount(jreargs) <= 0) {
		LOG(L"create: invalid jre args\n");
		return -30;
	}

	DWORD jredirattr = GetFileAttributes(jredir);
	if (jredirattr == INVALID_FILE_ATTRIBUTES || !(jredirattr & FILE_ATTRIBUTE_DIRECTORY)) {
		LOG(L"create: could not get file attr %s: %x\n", jredir, jredirattr);
		return -40;
	}

	WCHAR dlldir[MAX_PATH];
	swprintf(dlldir, MAX_PATH, L"%s\\bin", jredir);
	BOOL set = SetDllDirectory(dlldir);
	if (!set) {
		LOG(L"create: could not set dll dir %s: %d\n", dlldir, E);
		return -50;
	}

	WCHAR *jvmfile = L"server\\jvm.dll";
	HMODULE jvmdll = LoadLibrary(jvmfile);
	if (!jvmdll) {
		LOG(L"create: could not load jvm (32/64 bit mismatch?) %s: %d\n", jvmfile, E);
		return -60;
	}

	DEFAULTVM *defaultvm = (DEFAULTVM*)GetProcAddress(jvmdll, "JNI_GetDefaultJavaVMInitArgs");
	if (!defaultvm) {
		LOG(L"create: could not get default vm address: %d\n", E);
		return -70;
	}

	CREATEVM *createvm = (CREATEVM*)GetProcAddress(jvmdll, "JNI_CreateJavaVM");
	if (!createvm) {
		LOG(L"create: could not get create vm address: %d\n", E);
		return -80;
	}

	JavaVMInitArgs init;
	init.version = JNI_VERSION_1_8;

	int defok = defaultvm(&init);
	if (defok != JNI_OK) {
		LOG(L"create: could not default vm: %d\n", defok);
		return -90;
	}

	init.nOptions = wstrArrayCount(jreargs);
	init.options = new JavaVMOption[init.nOptions];
	init.ignoreUnrecognized = false;
	for (int n = 0; n < init.nOptions; n++) {
		init.options[n].optionString = wstrToStr(jreargs[n]);
	}
	
	int createok = createvm(&jvm, &env, &init);

	for (int n = 0; n < init.nOptions; n++) {
		delete init.options[n].optionString;
	}
	delete init.options;

	if (createok == JNI_OK) {
		LOG(L"create: ok\n");
		return 0;
		
	} else {
		LOG(L"create: could not create jvm: %d\n", createok);
		return -100;
	}
	
}

// this may return quickly if main spawns other threads and exits
// this may never return if jvm calls System.exit
LAUNCHER_API int AlexRunMain(LPWSTR mainclassname, LPWSTR* mainargs) {
	
	if (!jvm) {
		LOG(L"run: not created\n");
		return -1;
	}

	if (wstrIsEmpty(mainclassname) || !mainargs) {
		LOG(L"run: invalid arguments\n");
		return -2;
	}
	
	_jclass *stringclass = env->FindClass("java/lang/String");
	if (!stringclass) {
		LOG(L"run: could not find string class\n");
		return -3;
	}

	int mainargscount = wstrArrayCount(mainargs);

	_jobjectArray *jmainargs = env->NewObjectArray(mainargscount, stringclass, NULL);

	for (int n = 0; n < mainargscount; n++) {
		char *t = wstrToStr(mainargs[n]);
		env->SetObjectArrayElement(jmainargs, n, env->NewStringUTF(t));
		delete t;
	}

	_jclass *mainclass;
	{
		char *t = wstrToStr(mainclassname);
		mainclass = env->FindClass(t);
		delete t;
	}

	if (!mainclass) {
		// make sure is separated with /
		LOG(L"run: could not find main class: %s\n", mainclassname);
		return -4;
	}

	_jmethodID *mainmethod = env->GetStaticMethodID(mainclass, "main", "([Ljava/lang/String;)V");

	if (!mainmethod) {
		LOG(L"run: could not find main method: %s\n", mainclassname);
		return -5;
	}

	env->CallStaticVoidMethod(mainclass, mainmethod, jmainargs);
	
	LOG(L"run: ok\n");
	return 0;
}

// this might never return, if jvm never exits or calls System.exit
LAUNCHER_API int AlexDestroyVm () {

	if (!jvm) {
		LOG(L"destroy: not created\n");
		return -1;
	}

	int ok = jvm->DestroyJavaVM();
	if (ok == JNI_OK) {
		LOG(L"destroy: ok\n");
		return 0;
		
	} else {
		LOG(L"destroy: could not destroy: %d\n", ok);
		return -2;
	}
	
}
