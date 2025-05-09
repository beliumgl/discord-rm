/*
 * DISCORD-RM
 * --------------------------------
 * CLI removal tool for Discord chats,
 * using an authorization token and
 * the Discord API.
 */

#include <include/helpers.hpp>
#include <cstddef>
#include <string>
#include <sstream>
#include <iomanip>
#include <vector>
#include <utility>
#include <cctype>
#include <regex>

size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    /*
     * The WriteCallback function is needed because libcurl calls it whenever data is received from the server during a transfer.
     * By setting CURLOPT_WRITEFUNCTION to WriteCallback, we tell libcurl how to handle incoming data: instead of writing it to a file or stdout (the default),
     * the callback allows us to process or store the data in a custom way, such as appending it to a std::string in memory.
     * This is essential if you want to capture the HTTP response for further use in your program, rather than just printing or saving it directly.
     */
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::string urlEncode(const std::string& value) {
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
        } else {
            escaped << '%' << std::setw(2) << int((unsigned char) c);
        }
    }
    return escaped.str();
}

std::string buildQueryString(const std::vector<Query>& params) {
    std::ostringstream oss;
    bool first = true;
    for (const auto& [key, value] : params) {
        if (!value.empty()) {
            if (!first) oss << "&";
            oss << urlEncode(key) << "=" << urlEncode(value);
            first = false;
        }
    }
    return oss.str();
}
