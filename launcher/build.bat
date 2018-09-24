@echo off
rem compiler 32/64 bit has to match jvm.dll 32/64 bit!!
rem "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvars64.bat"
rem make sure JAVA_HOME is set
cl.exe /D "_DEBUG" /D "_CONSOLE" /D "_UNICODE" /D "UNICODE"  /I "%JAVA_HOME%\include" /I "%JAVA_HOME%\include\win32" /LD launcher.cpp
