﻿//Created by Eternity-boundary on Oct.1 2024
//This is an open source project
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <conio.h>
#include "ProcessControl.h"
#include "LeagueClientAPI.h"
#include "FixLeagueClientWindowError.h"
#pragma warning(disable:6031)//抑制警告6031，目前不需要返回值
#include <sstream>


using namespace std;

class Program {
public:
    static LeagueClientAPI* GetLCU() {
        uint32_t LeagueClientId = GetProcessId("LeagueClientUx.exe");
        string LeagueClientUxCommandLine = GetCommandLineByProcessId(LeagueClientId);
        auto LeagueClientUxArgs = LeagueClientAPI::CommandLineParser(LeagueClientUxCommandLine);

        if (!LeagueClientUxArgs.Available) {
            return nullptr;
        }

        // 实例化LeagueClientAPI并返回
        return new LeagueClientAPI(LeagueClientUxArgs.Port, LeagueClientUxArgs.Token);
    }

    static pair<HWND, HWND> GetLeagueClientWindowHandle() {
        HWND LeagueClientWindowHWnd = FindWindowA("RCLIENT", "League of Legends");
        HWND LeagueClientWindowCefHWnd = FindWindowExA(LeagueClientWindowHWnd, NULL, "CefBrowserWindow", NULL);

        // 返回两个窗口句柄
        return make_pair(LeagueClientWindowHWnd, LeagueClientWindowCefHWnd);
    }

    static bool needResize(RECT rect) {
        double aspectRatio = static_cast<double>(rect.bottom - rect.top) / (rect.right - rect.left);
        return abs(aspectRatio - 0.5625) > 0.01; 
    }

    static int FixLeagueClientWindow(bool forced = false) {
        auto LeagueClientWindow = GetLeagueClientWindowHandle();
        RECT LeagueClientWindowRect = {};
        RECT LeagueClientWindowCefRect = {};

        if (LeagueClientWindow.first == NULL || LeagueClientWindow.second == NULL) {
            return -1; //获取窗口句柄失败
        }

        if (IsMinimized(LeagueClientWindow.first)) {
            return 0;
        }

        GetWindowRect(LeagueClientWindow.first, &LeagueClientWindowRect);
        GetWindowRect(LeagueClientWindow.second, &LeagueClientWindowCefRect);

        if (!needResize(LeagueClientWindowRect) && !needResize(LeagueClientWindowCefRect) && !forced) {
            return 0;
        }

        LeagueClientAPI* LeagueClientAPIClient = GetLCU();

        if (LeagueClientAPIClient == nullptr) {
            return -2; // leagueclient api实例化失败
        }

        double LeagueClientZoom = LeagueClientAPIClient->GetClientZoom();

        int PrimaryScreenWidth = GetSystemMetrics(SM_CXSCREEN);
        int PrimaryScreenHeight = GetSystemMetrics(SM_CYSCREEN);
        double PrimaryScreenDpi = GetDpiForWindow(LeagueClientWindow.first) / static_cast<double>(GetDpiForSystem());

        if (LeagueClientZoom == -1) {
            delete LeagueClientAPIClient;
            return -3; // 无法获取窗口大小
        }

        int TargetLeagueClientWindowWidth = static_cast<int>(1280 * LeagueClientZoom);
        int TargetLeagueClientWindowHeight = static_cast<int>(720 * LeagueClientZoom);

        PatchDpiChangedMessage(LeagueClientWindow.first);
        PatchDpiChangedMessage(LeagueClientWindow.second);

        SetWindowPos(
            LeagueClientWindow.first,
            NULL,
            (PrimaryScreenWidth - TargetLeagueClientWindowWidth) / 2,
            (PrimaryScreenHeight - TargetLeagueClientWindowHeight) / 2,
            TargetLeagueClientWindowWidth, TargetLeagueClientWindowHeight,
            SWP_NOZORDER | SWP_NOSENDCHANGING
        );

        SetWindowPos(
            LeagueClientWindow.second,
            NULL,
            0,
            0,
            TargetLeagueClientWindowWidth,
            TargetLeagueClientWindowHeight,
            SWP_NOZORDER | SWP_NOSENDCHANGING
        );

        delete LeagueClientAPIClient;
        return 1;
    }

    static void Main(int argc, char* argv[]) {
        if (argc >= 3 && string(argv[1]) == "--mode") {
            int mode = stoi(argv[2]);
            Run(true, mode);
        }
        else {
            Run(false);
            cout << "> 按任意键退出..." << endl;
            _getch();//已抑制警告：此处不需要返回值
        }
    }

    static void PrintMenu() {
        cout << "------" << endl;
        cout << "> 功能菜单: " << endl;
        cout << "| [ 0 ]: 退出" << endl;
        cout << "| [ 1 ]: 立即恢复客户端窗口到正常大小 " << endl;
        cout << "| [ 2 ]: 自动恢复客户端窗口到正常大小(背景执行,不要关闭终端机)" << endl;
        cout << "| [ 3 ]: 直接跳过结算页面" << endl;
        cout << "| [ 4 ]: 热重载客户端" << endl;
        cout << "> " << endl;
        cout << "> 请输入功能序号并回车以执行 (1): ";
    }

    static void PrintCopyright() {
        cout << "------" << endl;
        cout << "> [ 英雄联盟客户端修复 ]" << endl;
        cout << "原项目地址：https://github.com/LeagueTavern/fix-lcu-window" << endl;
        cout << "此项目由@Eternity-boundary重构" << endl;
        cout << "> 修复客户端窗口 版本 -- " << GetVersion() << endl;
        cout << "------" << endl;
    }

    static string GetVersion() {
        return "1.0.0"; //可以根据需要调整版本号
    }

    static int GetUserChoice() {
        string UserOriginalInput;
        getline(cin, UserOriginalInput);
        int UserInput = 0;
        if (UserOriginalInput.empty() || !(istringstream(UserOriginalInput) >> UserInput)) {
            UserInput = 1; 
        }
        return UserInput;
    }

    static string Plan1() {
        int Result = FixLeagueClientWindow(true);
        string ErrorMessage = FixLeagueClientWindowError::ErrorDict[Result];
        return ErrorMessage;
    }

    static void Plan2() {
        int CurrentTriggerCount = 0;

        cout << "> 程序将在背景执行" << endl;
        cout << "> 当客户端尺寸出现异常时，程序将会自动修复" << endl;
        cout << "> 按下 [Ctrl] + [C]即可结束本程序" << endl;
        cout << "> ------" << endl;

        while (true) {
            int CurrentResult = FixLeagueClientWindow();
            if (CurrentResult == 1) {
                CurrentTriggerCount++;
                cout << "> 检测到窗口尺寸异常，已自动处理 (" << CurrentTriggerCount << ")" << endl;
            }
            this_thread::sleep_for(chrono::milliseconds(1500));
        }
    }

    static string Plan3() {
        LeagueClientAPI* LeagueClientAPIClient = GetLCU();
        if (LeagueClientAPIClient == nullptr) {
            return "未能获取到LCUAPI必要参数";
        }
        bool Result = LeagueClientAPIClient->LobbyPlayAgain();
        delete LeagueClientAPIClient;
        return Result ? "成功向客户端发送指令" : "向客户端发送指令时出现问题";
    }

    static string Plan4() {
        LeagueClientAPI* LeagueClientAPIClient = GetLCU();
        if (LeagueClientAPIClient == nullptr) {
            return "未能获取到LCUAPI必要参数";
        }
        bool Result = LeagueClientAPIClient->RestartClientUx();
        delete LeagueClientAPIClient;
        return Result ? "成功向客户端发送指令" : "向客户端发送指令时出现问题";
    }

    static void Run(bool withArgs, int mode = 0) {
        if (!withArgs) {
            PrintCopyright();
            PrintMenu();
        }

        int UserChoice = withArgs ? mode : GetUserChoice();

        if (withArgs) {
            cout << "> 正在执行功能: " << mode << endl;
        }
        else {
            cout << "> ------" << endl;
        }

        switch (UserChoice) {
        case 0: // 退出
            return;
        case 1: // Fix League Client Window
            cout << "> " << Plan1() << endl;
            break;
        case 2: // 自动修复窗口
            Plan2();
            break;
        case 3: // 跳过结算
            cout << "> " << Plan3() << endl;
            break;
        case 4: // 重启客户端
            cout << "> " << Plan4() << endl;
            break;
        default: // 未知选择
            cout << "> 未知的功能序号" << endl;
            break;
        }

        return;
    }
};



int main(int argc, char* argv[]) {
    Program::Main(argc, argv);
    return 0;
}
