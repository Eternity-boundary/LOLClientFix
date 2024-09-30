//Created by Eternity-boundary on Oct.1 2024
//This is an open source project
#include "LeagueClientAPI.h"
#include <curl/curl.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <iostream>
#include <regex>
#define DEBUG
LeagueClientAPI::LeagueClientAPI(int Port, const std::string& Token) {
    BaseAddress = "https://127.0.0.1:" + std::to_string(Port);
    std::string auth = "riot:" + Token;
    AuthorizationHeader = "Authorization: Basic " + base64_encode(auth);
#ifdef DEBUG
    // �������
    std::cout << "AuthorizationHeader: " << AuthorizationHeader << std::endl; 
#endif // DEBUG
}




LeagueClientAPI::~LeagueClientAPI() {}

size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

double LeagueClientAPI::GetClientZoom() {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (curl) {
        std::string url = BaseAddress + "/riotclient/zoom-scale";
        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, AuthorizationHeader.c_str());
        headers = curl_slist_append(headers, "Accept: application/json");

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false); // ����֤�����
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);

        res = curl_easy_perform(curl);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        curl_global_cleanup();

        if (res != CURLE_OK) {
            std::cout << "cURL ����" << curl_easy_strerror(res) << std::endl;
            return -1;
        }
        else {
            try {
                return std::stod(readBuffer);
            }
            catch (...) {
                return -1;
            }
        }
    }
    return -1;
}


bool LeagueClientAPI::RestartClientUx() {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (curl) {
        std::string url = BaseAddress + "/riotclient/kill-and-restart-ux";
        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, AuthorizationHeader.c_str());
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        // �������󷽷�Ϊ POST
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        // ����������Ϊ��
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "");

        // ����֤�����
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);
#ifdef DEBUG
        // ���õ������
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
#endif // DEBUG
        // ������Ӧ����
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        res = curl_easy_perform(curl);

        // �����Ӧ��
        long response_code;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        curl_global_cleanup();

        if (res != CURLE_OK) {
            std::cout << "cURL ����" << curl_easy_strerror(res) << std::endl;
            return false;
        }
        else if (response_code != 204) {
            std::cout << "HTTP ��Ӧ�룺" << response_code << std::endl;
            std::cout << "��Ӧ���ݣ�" << readBuffer << std::endl;
            return false;
        }
        else {
            return true;
        }
    }
    return false;
}



bool LeagueClientAPI::LobbyPlayAgain() {
    CURL* curl;
    CURLcode res;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (curl) {
        std::string url = BaseAddress + "/lol-lobby/v2/play-again";
        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, AuthorizationHeader.c_str());
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POST, 1L);

        // Ԥ�����ܣ������Ҫ���������壬�����ڴ�����
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "");

        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false); // ����֤�����
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);

        res = curl_easy_perform(curl);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        curl_global_cleanup();

        if (res != CURLE_OK) {
            std::cout << "cURL ����" << curl_easy_strerror(res) << std::endl;
            return false;
        }
        else {
            return true;
        }
    }
    return false;
}


LeagueClientAPI::CommandLineArgs LeagueClientAPI::CommandLineParser(const std::string& command) {
    std::regex authTokenRegex("--remoting-auth-token=(\"[^\"]*\"|[^\\s]+)");
    std::regex appPortRegex("--app-port=(\"[^\"]*\"|\\d+)");
    std::smatch tokenMatch;
    std::smatch portMatch;

    bool tokenFound = std::regex_search(command, tokenMatch, authTokenRegex);
    bool portFound = std::regex_search(command, portMatch, appPortRegex);

    if (tokenFound && portFound) {
        std::string token = tokenMatch[1].str();
        std::string portStr = portMatch[1].str();

        // �������
        std::cout << "��ȡ�� Token��ԭʼ��: [" << token << "]" << std::endl;

        // �Ƴ���ͷ�����ţ�����У�
        if (!token.empty() && token.front() == '"') {
            token.erase(0, 1);
        }
        // �Ƴ���β�����ţ�����У�
        if (!token.empty() && token.back() == '"') {
            token.pop_back();
        }

        // �������
        std::cout << "��ȡ�� Token�������: [" << token << "]" << std::endl;

        // �Զ˿ڽ���ͬ���Ĵ���
        if (!portStr.empty() && portStr.front() == '"') {
            portStr.erase(0, 1);
        }
        if (!portStr.empty() && portStr.back() == '"') {
            portStr.pop_back();
        }

        int port = std::stoi(portStr);

        return { true, port, token, "https" };
    } else {
        return { false, 0, "", "" };
    }
}





// ʹ��OpenSSl����base64����
std::string base64_encode(const std::string& in) {
    std::string out;
    int encoded_length = 4 * ((in.length() + 2) / 3);
    out.resize(encoded_length);

    int out_len = EVP_EncodeBlock(reinterpret_cast<unsigned char*>(&out[0]),
        reinterpret_cast<const unsigned char*>(in.c_str()),
        in.length());
    if (out_len != encoded_length) {
        // ��������ݲ�ʵ��
        return "";
    }

    return out;
}
