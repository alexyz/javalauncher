
// you'll probably need this
// https://docs.microsoft.com/en-us/windows/desktop/Debug/system-error-codes

#include <SDKDDKVer.h>
#include <windows.h>
#include <jni.h>

#define LAUNCHER_EXPORTS
#include "launcher.h"

static JavaVM *jvm = 0;
static JNIEnv *env = 0;
static int _log = 0;

#define LOG(...) if (_log) { printf(__VA_ARGS__); }
#define E GetLastError()

// jint JNI_CreateJavaVM(JavaVM **p_vm, void **p_env, void *vm_args);
typedef jint(__cdecl CREATEVM)(JavaVM **p_vm, JNIEnv **p_env, JavaVMInitArgs *vm_args);

// jint JNI_GetDefaultJavaVMInitArgs(void *vm_args);
typedef jint(__cdecl DEFAULTVM)(JavaVMInitArgs *vm_args);

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {
    return TRUE;
}

static LPSTR wstrToUtf8Str(LPWSTR ws) {
	int slen = wcslen(ws);
	int ulen = WideCharToMultiByte(CP_UTF8, 0, ws, slen, NULL, 0,    NULL, NULL) + 1;
	char *us = new char[ulen];
	int vlen = WideCharToMultiByte(CP_UTF8, 0, ws, -1,   us,   ulen, NULL, NULL);
	us[ulen-1] = 0;
	//wprintf(L"tochar: slen=%d ulen=%d vlen=%d strlen(us)=%d\n", slen, ulen, vlen, strlen(us));
	return us;
}

static int wstrArrayCount(LPWSTR* a) {
	int c = 0;
	for (int n = 0; a[n]; n++) {
		c++;
	}
	return c;
}

LAUNCHER_API void AlexSetLog (int l) {
	_log = l;
	LOG("log: %d\n", _log);
}

// As of JDK/JRE 1.2 , creation of multiple VMs in a single process is not supported.
LAUNCHER_API int AlexCreateVm (LPWSTR jredir, LPWSTR* jreargs) {

	// TODO log args
	LOG("create: jredir=%S\n", jredir);
	for (int n = 0; jreargs && jreargs[n]; n++) {
		LOG("create: jrearg[%d]=%S\n", n, jreargs[n]);
	}

	if (jvm) {
		LOG("create: jvm already exists\n")
		return -1;
	}

	if (!jredir || wcslen(jredir) == 0) {
		LOG("create: invalid jre dir\n")
		return -2;
	}
	
	if (!jreargs) {
		LOG("create: invalid jre args\n");
		return -3;
	}

	DWORD jredirattr = GetFileAttributesW(jredir);
	if (jredirattr == INVALID_FILE_ATTRIBUTES || !(jredirattr & FILE_ATTRIBUTE_DIRECTORY)) {
		LOG("create: could not get file attr: %x\n", jredirattr);
		return -4;
	}

	WCHAR dlldir[MAX_PATH];
	swprintf(dlldir, MAX_PATH, L"%s\\bin", jredir);
	BOOL set = SetDllDirectoryW(dlldir);
	if (!set) {
		LOG("create: could not set dll dir: %d\n", E);
		return -5;
	}

	HMODULE jvmdll = LoadLibraryW(L"server\\jvm.dll");
	if (!jvmdll) {
		LOG("create: could not load library (32/64 bit mismatch?) %d\n", E);
		return -6;
	}

	DEFAULTVM *defaultvm = (DEFAULTVM*)GetProcAddress(jvmdll, "JNI_GetDefaultJavaVMInitArgs");
	if (!defaultvm) {
		LOG("create: could not get default vm address: %d\n", E);
		return -7;
	}

	CREATEVM *createvm = (CREATEVM*)GetProcAddress(jvmdll, "JNI_CreateJavaVM");
	if (!createvm) {
		LOG("create: could not get create vm address: %d\n", E);
		return -8;
	}

	JavaVMInitArgs init;
	init.version = JNI_VERSION_1_8;

	int defok = defaultvm(&init);
	if (defok != JNI_OK) {
		LOG("create: could not default vm: %d\n", defok);
		return -9;
	}

	init.nOptions = wstrArrayCount(jreargs);
	init.options = new JavaVMOption[init.nOptions];
	init.ignoreUnrecognized = false;
	for (int n = 0; n < init.nOptions; n++) {
		init.options[n].optionString = wstrToUtf8Str(jreargs[n]);
	}
	
	int createok = createvm(&jvm, &env, &init);

	for (int n = 0; n < init.nOptions; n++) {
		delete init.options[n].optionString;
	}
	delete init.options;

	if (createok == JNI_OK) {
		LOG("create: ok\n");
		return 0;
		
	} else {
		LOG("create: could not create jvm: %d\n", createok);
		return -10;
	}
	
}

// this may return quickly if main spawns other threads and exits
// this may never return if jvm calls System.exit
LAUNCHER_API int AlexRunMain(LPWSTR mainclassname, LPWSTR* mainargs) {
	
	LOG("run: mainclass = % S\n", mainclassname);
	for (int n = 0; mainargs[n]; n++) {
		LOG("mainarg[%d] = %S\n", n, mainargs[n]);
	}

	if (!jvm) {
		LOG("run: not created\n");
		return -1;
	}

	if (!mainclassname || wcslen(mainclassname) == 0 || !mainargs) {
		LOG("run: invalid arguments\n");
		return -2;
	}
	
	_jclass *stringclass = env->FindClass("java/lang/String");
	if (!stringclass) {
		LOG("run: could not find string class\n");
		return -3;
	}

	int mainargscount = wstrArrayCount(mainargs);

	_jobjectArray *jmainargs = env->NewObjectArray(mainargscount, stringclass, NULL);

	for (int n = 0; n < mainargscount; n++) {
		char *t = wstrToUtf8Str(mainargs[n]);
		env->SetObjectArrayElement(jmainargs, n, env->NewStringUTF(t));
		delete t;
	}

	_jclass *mainclass;
	{
		char *t = wstrToUtf8Str(mainclassname);
		mainclass = env->FindClass(t);
		delete t;
	}

	if (!mainclass) {
		// make sure is separated with /
		LOG("run: could not find main class\n");
		return -4;
	}

	_jmethodID *mainmethod = env->GetStaticMethodID(mainclass, "main", "([Ljava/lang/String;)V");

	if (!mainmethod) {
		LOG("run: could not find main method\n");
		return -5;
	}

	env->CallStaticVoidMethod(mainclass, mainmethod, jmainargs);
	
	LOG("run: ok\n");
	return 0;
}

// this might never return, if jvm never exits or calls System.exit
LAUNCHER_API int AlexDestroyVm () {

	LOG("destroy\n");

	if (!jvm) {
		LOG("destroy: not created\n");
		return -1;
	}

	int ok = jvm->DestroyJavaVM();
	if (ok == JNI_OK) {
		LOG("destroy: ok\n");
		return 0;
		
	} else {
		LOG("destroy: could not destroy: %d\n", ok);
		return -2;
	}
	
}
