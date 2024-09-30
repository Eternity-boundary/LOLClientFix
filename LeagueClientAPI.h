//Created by Eternity-boundary on Oct.1 2024
//This is an open source project
#pragma once
#include <string>

using namespace std;

std::string base64_encode(const std::string& in);

class LeagueClientAPI {
public:
    LeagueClientAPI(int Port, const string& Token);
    ~LeagueClientAPI();

    double GetClientZoom();
    bool RestartClientUx();
    bool LobbyPlayAgain();

    struct CommandLineArgs {
        bool Available;
        int Port;
        string Token;
        string Protocol;
    };

    static CommandLineArgs CommandLineParser(const string& command);

private:
    string BaseAddress;
    string AuthorizationHeader;
};
