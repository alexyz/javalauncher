@echo off
copy ..\launcher\launcher.dll .
cl.exe test.cpp ..\launcher\launcher.lib
