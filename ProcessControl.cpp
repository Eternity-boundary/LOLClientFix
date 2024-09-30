//Created by Eternity-boundary on Oct.1 2024
//This is an open source project
#include "ProcessControl.h"
#include <TlHelp32.h>
#include <psapi.h>
#include <comdef.h>
#include <Wbemidl.h>
#include <vector>
#include <windows.h>
#include <winternl.h>
#pragma comment(lib, "ntdll.lib")
#pragma comment(lib, "wbemuuid.lib")

bool IsMinimized(HWND hWnd) {
    WINDOWPLACEMENT placement = {};
    placement.length = sizeof(WINDOWPLACEMENT);
    if (GetWindowPlacement(hWnd, &placement)) {
        return placement.showCmd == SW_SHOWMINIMIZED;
    }
    else {
        // 错误处理，返回false，也可抛出异常
        return false;
    }
}

uint32_t GetProcessId(const std::string& processName) {
    PROCESSENTRY32W entry;
    entry.dwSize = sizeof(PROCESSENTRY32W);
    uint32_t processId = 0;

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
    if (snapshot == INVALID_HANDLE_VALUE) {
        return 0;
    }

    if (Process32FirstW(snapshot, &entry)) {
        do {
            std::wstring wsProcessName(processName.begin(), processName.end());
            if (_wcsicmp(entry.szExeFile, wsProcessName.c_str()) == 0) {
                processId = entry.th32ProcessID;
                break;
            }
        } while (Process32NextW(snapshot, &entry));
    }
    CloseHandle(snapshot);
    return processId;
}


void PatchDpiChangedMessage(HWND hWnd) {
    UINT dpi = GetDpiForWindow(hWnd);
    WPARAM wParam = MAKEWPARAM(dpi, dpi);
    RECT rect;
    GetWindowRect(hWnd, &rect);
    LPARAM lParam = reinterpret_cast<LPARAM>(&rect);
    SendMessage(hWnd, WM_DPICHANGED, wParam, lParam);
}

typedef NTSTATUS(NTAPI* _NtQueryInformationProcess)(
    HANDLE ProcessHandle,
    PROCESSINFOCLASS ProcessInformationClass,
    PVOID ProcessInformation,
    ULONG ProcessInformationLength,
    PULONG ReturnLength
    );

std::string GetCommandLineByProcessId(DWORD processId) {
    std::string commandLine;
    HRESULT hr;
    IWbemLocator* pLocator = NULL;
    IWbemServices* pServices = NULL;
    IEnumWbemClassObject* pEnumerator = NULL;
    IWbemClassObject* pClassObject = NULL;
    ULONG uReturn = 0;

    hr = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hr)) {
        return commandLine;
    }

    hr = CoInitializeSecurity(
        NULL,
        -1,
        NULL,
        NULL,
        RPC_C_AUTHN_LEVEL_DEFAULT,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL,
        EOAC_NONE,
        NULL
    );
    if (FAILED(hr)) {
        CoUninitialize();
        return commandLine;
    }

    hr = CoCreateInstance(
        CLSID_WbemLocator,
        0,
        CLSCTX_INPROC_SERVER,
        IID_IWbemLocator,
        (LPVOID*)&pLocator
    );
    if (FAILED(hr)) {
        CoUninitialize();
        return commandLine;
    }

    hr = pLocator->ConnectServer(
        _bstr_t(L"ROOT\\CIMV2"),
        NULL,
        NULL,
        0,
        NULL,
        0,
        0,
        &pServices
    );
    if (FAILED(hr)) {
        pLocator->Release();
        CoUninitialize();
        return commandLine;
    }

    hr = CoSetProxyBlanket(
        pServices,
        RPC_C_AUTHN_WINNT,
        RPC_C_AUTHZ_NONE,
        NULL,
        RPC_C_AUTHN_LEVEL_CALL,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL,
        EOAC_NONE
    );
    if (FAILED(hr)) {
        pServices->Release();
        pLocator->Release();
        CoUninitialize();
        return commandLine;
    }

    std::wstring query = L"SELECT CommandLine FROM Win32_Process WHERE ProcessId = " + std::to_wstring(processId);
    hr = pServices->ExecQuery(
        bstr_t(L"WQL"),
        bstr_t(query.c_str()),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        NULL,
        &pEnumerator
    );
    if (FAILED(hr)) {
        pServices->Release();
        pLocator->Release();
        CoUninitialize();
        return commandLine;
    }

    while (pEnumerator) {
        hr = pEnumerator->Next(WBEM_INFINITE, 1, &pClassObject, &uReturn);
        if (0 == uReturn) {
            break;
        }
        VARIANT vtProp;
        hr = pClassObject->Get(L"CommandLine", 0, &vtProp, 0, 0);
        if (SUCCEEDED(hr) && vtProp.vt == VT_BSTR) {
            // 将 BSTR 转换为 std::wstring
            std::wstring ws(vtProp.bstrVal, SysStringLen(vtProp.bstrVal));

            // 将 std::wstring 转换为 std::string
            int size_needed = WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), -1, NULL, 0, NULL, NULL);
            if (size_needed > 0) {
                std::vector<char> buffer_str(size_needed);
                WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), -1, &buffer_str[0], size_needed, NULL, NULL);
                commandLine = std::string(buffer_str.data());
            }

            VariantClear(&vtProp);
            pClassObject->Release();
            break;
        }
        VariantClear(&vtProp);
        pClassObject->Release();
    }

    if (pEnumerator) pEnumerator->Release();
    if (pServices) pServices->Release();
    if (pLocator) pLocator->Release();
    CoUninitialize();

    return commandLine;
}
