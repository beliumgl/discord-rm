/*
 * DISCORD-RM
 * --------------------------------
 * CLI removal tool for Discord chats,
 * using an authorization token and
 * the Discord API.
 */

#pragma once

#include "json.hpp"
#include <curl/curl.h>
#include <utility>
#include <string>
#include <string_view>
#include <algorithm>
#include <cctype>
#include <iostream>
#include <vector>
#include <thread>

using Query = std::pair<std::string, std::string>;

inline std::string formatString(const std::string_view& s) {
    std::string str = static_cast<std::string>(s);
    str.erase(std::remove_if(str.begin(), str.end(),
                             [](unsigned char c) { return std::isspace(c); }), str.end()); // Remove all whitespace
    std::transform(str.begin(), str.end(), str.begin(),
                   [](unsigned char c) { return std::tolower(c); }); // Convert to lowercase
    return str;
}

inline bool isDMGuild(const std::string_view& guildID) { return guildID == "@me"; };
inline bool parseInput(const std::string_view& in) { return formatString(in) == "y" || in == "Y"; };

inline void ask(const std::string_view& question, std::string& save) {
    std::cout << question;
    std::cin >> save;
}

inline void input(const std::string_view& msg, std::string& save) {
    ask(msg, save);
}

inline void log(bool isVerbose, const std::string_view& msg) {
    if (!isVerbose)
        return;

    std::cout << msg << '\n';
}

inline void debug(bool isDebug, const std::string_view& msg) { // Rename for `log` function
    log(isDebug, msg);
}

inline bool isSystemMessage(int type) { return (type < 6 || type > 21) && type != 0; }

inline void handleRateLimit(nlohmann::json response) {
    constexpr unsigned int DELAY_MULTIPLIER = 2;

    unsigned int newDelay = static_cast<unsigned int>(response["retry_after"].get<double>());
    std::this_thread::sleep_for(std::chrono::seconds(newDelay * DELAY_MULTIPLIER)); // Wait a little longer to ensure not hit rate limit again
}

size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp);
std::string urlEncode(const std::string& value);
std::string buildQueryString(const std::vector<Query>& params);
std::pair<CURL*, CURLcode> sendRequest(std::string& response,
                                       const std::string& _headers,
                                       const std::string& url,
                                       const std::string& method);
