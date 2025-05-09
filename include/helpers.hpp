/*
 * DISCORD-RM
 * --------------------------------
 * CLI removal tool for Discord chats,
 * using an authorization token and
 * the Discord API.
 */

#pragma once

#include <utility>
#include <string>
#include <string_view>
#include <algorithm>
#include <cctype>
#include <iostream>
#include <vector>

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

size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp);
std::string urlEncode(const std::string& value);
std::string buildQueryString(const std::vector<Query>& params);
