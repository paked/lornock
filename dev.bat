@echo off

cd C:\Users\harrison\code\lornock

set path=%PATH%;C:\Users\harrison\code\lornock\bin

call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
call vim
