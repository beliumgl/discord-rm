/*
 * DISCORD-RM
 * --------------------------------
 * CLI removal tool for Discord chats,
 * using an authorization token and
 * the Discord API.
 */

#pragma once

#include <nlohmann/json.hpp>
#include <curl/curl.h>
#include <fmt/base.h>
#include <fmt/color.h>
#include <utility>
#include <string>
#include <string_view>
#include <algorithm>
#include <cctype>
#include <iostream>
#include <vector>
#include <thread>

using Query = std::pair<std::string, std::string>;

enum MessageType {
    OK,
    WARNING,
    ERROR
};

inline std::string format_string(const std::string_view& s) {
    auto str = static_cast<std::string>(s);
    std::erase_if(str,
                  [](const unsigned char c) { return std::isspace(c); }); // Remove all whitespace
    std::ranges::transform(str, str.begin(),
                           [](const unsigned char c) { return std::tolower(c); }); // Convert to lowercase
    return str;
}

inline bool is_dm_guild(const std::string_view& guild_id) { return guild_id == "@me"; }
inline bool parse_input(const std::string_view& in) { return format_string(in) == "y" || in == "Y"; }

inline void ask(const std::string_view& question, std::string& save) {
    fmt::print("{}", question);
    std::cin >> save;
}

inline void input(const std::string_view& msg, std::string& save) {
    ask(msg, save);
}

inline void log(const bool is_verbose, const std::string_view& msg, const MessageType type = OK) {
    if (!is_verbose)
        return;

    switch (type) {
        case OK:
            fmt::print("{}\n", msg);
            break;
        case WARNING:
            fmt::print(fg(fmt::color::yellow), "{}\n", msg);
            break;
        case ERROR:
            fmt::print(fg(fmt::color::red), "{}\n", msg);
            break;
    }
}

inline void debug(const bool is_debug, const std::string_view& msg, const MessageType type = OK) { // Rename for `log` function
    log(is_debug, msg, type);
}

inline bool is_system_message(const int type) { return (type < 6 || type > 21) && type != 0; }
inline bool is_http_error(const long code) { return code < 200 || code >= 300; }

inline void handle_rate_limit(nlohmann::json response) {
    constexpr unsigned int DELAY_MULTIPLIER = 2;

    const auto new_delay = static_cast<unsigned int>(response["retry_after"].get<double>());
    std::this_thread::sleep_for(std::chrono::seconds(new_delay * DELAY_MULTIPLIER)); // Wait a little longer to ensure not hit rate limit again
}

size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp);
std::string url_encode(const std::string& value);
std::string build_query_string(const std::vector<Query>& params);
std::string convert_to_snowflake_id(const std::string& iso8601);
std::vector<Query> construct_query_params(const std::string& offset);
std::pair<CURL*, CURLcode> send_request(std::string& response,
                                        const std::string& _headers,
                                        const std::string& url,
                                        const std::string& method);
