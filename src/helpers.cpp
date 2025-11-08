/*
 * DISCORD-RM
 * --------------------------------
 * CLI removal tool for Discord chats,
 * using an authorization token and
 * the Discord API.
 */

#include <include/helpers.hpp>
#include <include/config.hpp>
#include <curl/curl.h>
#include <string>
#include <sstream>
#include <iomanip>
#include <vector>
#include <utility>
#include <cctype>

size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp) {
    /*
     * The WriteCallback function is needed because libcurl calls it whenever data is received from the server during a transfer.
     * By setting CURLOPT_WRITEFUNCTION to WriteCallback, we tell libcurl how to handle incoming data: instead of writing it to a file or stdout (the default),
     * the callback allows us to process or store the data in a custom way, such as appending it to a std::string in memory.
     * This is essential if you want to capture the HTTP response for further use in your program, rather than just printing or saving it directly.
     */
    static_cast<std::string *>(userp)->append(static_cast<char *>(contents), size * nmemb);
    return size * nmemb;
}

std::string url_encode(const std::string& value) {
    /*
     * urlEncode is needed because URLs can only contain certain ASCII characters.
     * Characters like spaces, quotes, and special symbols must be converted into a safe, transmittable format (percent-encoding).
     * Without encoding, these characters could break the URL structure, cause errors, or introduce security vulnerabilities.
     * URL encoding ensures valid, consistent, and secure transmission of data in URLs across different browsers and servers.
     */
    std::ostringstream escaped;

    escaped.fill('0');
    escaped << std::hex;

    for (char c : value) {
        if (isalnum(static_cast<unsigned char>(c)) || c == '-' || c == '_' || c == '.' || c == '~') {
            escaped << c;
            continue;
        }

        escaped << '%' << std::setw(2) << static_cast<int>(static_cast<unsigned char>(c));
    }
    return escaped.str();
}

std::string build_query_string(const std::vector<Query>& params) {
    std::ostringstream oss;
    bool first = true;
    for (const auto& [key, value] : params) {
        if (!value.empty()) {
            if (!first) oss << "&";
            oss << url_encode(key) << "=" << url_encode(value);
            first = false;
        }
    }
    return oss.str();
}

std::vector<Query> construct_query_params(const std::string& offset) {
    constexpr unsigned long long int SNOWFLAKE_ID_1_DAY = 362387865600000ULL;

    std::vector<Query> params = {
        {"author_id", SENDER_ID},
        {"channel_id", CHANNEL_ID},
        {"offset", offset},
        {"limit", std::to_string(PAGE_LIMIT)}
    };

    if (!MENTIONS.empty()) {
        for (const auto& mention : MENTIONS) params.emplace_back("mentions", mention);
    }

    if (!REMOVE_PINNED) params.emplace_back("pinned", "false");

    if (!BEFORE_DATE.empty()) {
        params.emplace_back("max_id", BEFORE_DATE);
    }
    if (!AFTER_DATE.empty()) {
        params.emplace_back("min_id", std::to_string(std::stoull(AFTER_DATE) + SNOWFLAKE_ID_1_DAY));
    }
    if (!DURING_DATE.empty()) {
        params.emplace_back("min_id", DURING_DATE);
        params.emplace_back("max_id", std::to_string(std::stoull(DURING_DATE) + SNOWFLAKE_ID_1_DAY));
    }
    // The `has` parameter will be processed during parsing.

    return params;
}

std::string convert_to_snowflake_id(const std::string& iso8601) {
    // Discord uses its own timestamp system instead of the traditional Unix timestamps
    constexpr unsigned long long int DISCORD_EPOCH = 1420070400000ULL;

    std::tm tm = {};
    std::istringstream ss(iso8601);

    ss >> std::get_time(&tm, "%Y-%m-%d");
    if (ss.fail()) throw std::runtime_error("Failed to parse ISO8601 date");
    tm.tm_isdst = -1;

    const std::time_t time = std::mktime(&tm);
    if (time == -1) throw std::runtime_error("Failed to parse ISO8601 date");

    const auto time_point = std::chrono::system_clock::from_time_t(time);
    const auto ms_since_epoch = std::chrono::duration_cast<std::chrono::milliseconds>(time_point.time_since_epoch()).count();
    const unsigned long long int timestamp = ms_since_epoch - DISCORD_EPOCH;
    const unsigned long long int snowflake = timestamp << 22;

    return std::to_string(snowflake);
}

std::pair<CURL*, CURLcode> send_request(std::string& response,
                                       const std::string& _headers,
                                       const std::string& url,
                                       const std::string& method) {
    CURL* curl = curl_easy_init();
    if (!curl) throw std::runtime_error("Failed to send request.");
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, _headers.c_str());
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method.c_str());
    CURLcode result = curl_easy_perform(curl);
    curl_slist_free_all(headers);

    return {curl, result};
}
