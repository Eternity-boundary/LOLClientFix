//Created by Eternity-boundary on Oct.1 2024
//This is an open source project
#include "FixLeagueClientWindowError.h"

unordered_map<int, string> FixLeagueClientWindowError::ErrorDict = {
    { 1, "成功恢复客户端窗口大小" },
    { 0, "未发现客户端窗口出现异常" },
    { -1, "未检测到客户端窗口，可能客户端正处于最小化状态" },
    { -2, "未能获取到LCUAPI必要参数，检查是否以管理员身份运行" },
    { -3, "未能获取到客户端缩放比例" }
};
