@echo off
set J=C:\Program Files\Java\jdk1.8.0_162
cl /I "%J%\include" /I "%J%\include\win32" /LD launcher.cpp
