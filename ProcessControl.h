//Created by Eternity-boundary on Oct.1 2024
//This is an open source project
#pragma once
#include <windows.h>
#include <string>

using namespace std;

string GetCommandLineByProcessId(DWORD processId);
uint32_t GetProcessId(const string& processName);
bool IsMinimized(HWND hWnd);
void PatchDpiChangedMessage(HWND hWnd);
