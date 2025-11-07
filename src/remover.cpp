/*
 * DISCORD-RM
 * --------------------------------
 * CLI removal tool for Discord chats,
 * using an authorization token and
 * the Discord API.
 */

#include <include/remover.hpp>
#include <include/helpers.hpp>
#include <include/config.hpp>
#include <nlohmann/json.hpp>
#include <curl/curl.h>
#include <string>
#include <vector>
#include <stdexcept>
#include <chrono>
#include <thread>
#include <utility>
#include <unordered_set>

using nlohmann::json;
using Query = std::pair<std::string, std::string>;

struct Message {
    std::string CHANNEL_ID;
    std::string ID;
    int type;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Message, ID, type, CHANNEL_ID);

    bool operator==(const Message& other) const { // Required for unordered_set
        return ID == other.ID &&
        type == other.type &&
        CHANNEL_ID == other.CHANNEL_ID;
    }
};

namespace std {
    template <>
    struct hash<Message> { // Required for unordered_set
        std::size_t operator()(const Message& m) const noexcept {
            std::size_t h1 = std::hash<std::string>{}(m.ID);
            std::size_t h2 = std::hash<int>{}(m.type);
            std::size_t h3 = std::hash<std::string>{}(m.CHANNEL_ID);
            return h1 ^ (h2 << 1) ^ (h3 << 2);
        }
    };
}

const std::string DISCORD_API_URL_BASE = "https://discord.com/api/";
const std::string DISCORD_API_VERSION = "v10";
const std::string DISCORD_API_AUTHORIZATION_KEY = "Authorization: ";
const std::string CURL_GET_METHOD = "GET";
const std::string CURL_DELETE_METHOD = "DELETE";

json search(unsigned short offset) {
    debug(IS_DEBUG, std::string("[Search] Parameters: offset = " + std::to_string(offset)));
    log(IS_VERBOSE, "Search: Making API URL...");
    std::string apiURL = isDMGuild(GUILD_ID)
                            ? DISCORD_API_URL_BASE + DISCORD_API_VERSION + "/channels/" + CHANNEL_ID + "/messages/"
                            : DISCORD_API_URL_BASE + DISCORD_API_VERSION + "/guilds/" + GUILD_ID + "/messages/";

    std::vector<Query> params = {
        {"author_id", SENDER_ID},
        {"channel_id", CHANNEL_ID},
        {"offset", std::to_string(offset)},
        {"limit", std::to_string(PAGE_LIMIT)}
    };

    log(IS_VERBOSE, "Search: Making request parameters...");
    std::string query = buildQueryString(params);
    debug(IS_DEBUG, "Query Parameters: " + query);

    log(IS_VERBOSE, "Search: Making full API URL...");
    std::string url = apiURL + "search?" + query;
    debug(IS_DEBUG, "Full URL: " + url);

    log(IS_VERBOSE, "Search: Sending request...");
    std::string response;
    std::string authHeader = DISCORD_API_AUTHORIZATION_KEY + DISCORD_TOKEN;

    auto [curl, result] = sendRequest(response, authHeader, url, CURL_GET_METHOD);

    if (result != CURLE_OK)
        throw std::runtime_error("Failed to send search request.");
    debug(IS_DEBUG, "Response: " + response);

    curl_easy_cleanup(curl);

    json jsonResponse = json::parse(response);

    if (jsonResponse.contains("retry_after")) { // Rate limited by discord
        log(IS_VERBOSE, "Search: Rate limited by Discord API! Trying again later...");
        handleRateLimit(jsonResponse);
        log(IS_VERBOSE, "Search: Retrying...");
        jsonResponse = search(offset); // Retry
    }

    return jsonResponse;
}

void deleteMessage(const Message& message) {
    constexpr long RATE_LIMITED_HTTP_CODE = 429;
    constexpr unsigned int ARCHIVED_THREAD_CODE = 50083;
    using json = nlohmann::json;

    debug(IS_DEBUG, std::string("[Delete Message] Parameters: Message (ID) = " + message.ID));

    if (isSystemMessage(message.type)) {
        log(IS_VERBOSE, "Delete Message: System message. Skipping...");
        return; // Cannot remove system message
    }

    log(IS_VERBOSE, "Delete Message: Making API URL...");
    std::string deleteAPIURL = DISCORD_API_URL_BASE + DISCORD_API_VERSION + "/channels/" + message.CHANNEL_ID + "/messages/" + message.ID;
    debug(IS_DEBUG, "Full URL: " + deleteAPIURL);

    log(IS_VERBOSE, "Delete Message: Sending request...");
    std::string response;
    std::string authHeader = DISCORD_API_AUTHORIZATION_KEY + DISCORD_TOKEN;

    auto [curl, result] = sendRequest(response, authHeader, deleteAPIURL, CURL_DELETE_METHOD);
    long httpCode = 0;

    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
    curl_easy_cleanup(curl);

    debug(IS_DEBUG, "Response: " + response + ", Code: " + std::to_string(httpCode));

    if (result != CURLE_OK)
        throw std::runtime_error("Failed to send delete message request.");

    if (httpCode == RATE_LIMITED_HTTP_CODE) { // Rate limited by Discord API
        if (response.empty()) {
            throw std::runtime_error("Failed to parse rate limit JSON.");
        }

        try {
            json j = json::parse(response);
            log(IS_VERBOSE, "Delete Message: Rate limited by Discord API! Trying again later...");
            handleRateLimit(j);
            log(IS_VERBOSE, "Delete Message: Retrying...");
            deleteMessage(message); // Retry
        } catch (...) {
            throw std::runtime_error("Failed to parse rate limit JSON.");
        }
    }

    if (httpCode == 400 && !response.empty()) {
        try {
            json j = json::parse(response);
            if (j.contains("code") && j["code"] == ARCHIVED_THREAD_CODE) {
                log(IS_VERBOSE, "Delete Message: Cannot remove archived thread. Skipping...");
                return;
            }
        } catch (...) {
            throw std::runtime_error("Failed to parse error JSON.");
        }
    }

    if (httpCode < 200 || httpCode >= 300) {
        throw std::runtime_error("Failed to delete message request.");
    }

    log(IS_VERBOSE, "Delete Message: Message deleted successfully!");
}

void discordRM() {
    log(IS_VERBOSE, "Remover: Searching for messages to delete...");

    unsigned short offset = 0;
    std::unordered_set<Message> skippedMessagesSet; // fix infinite loop (maybe, finally???)
    json messages;

    while (true) { // While loop to remove all pages
        std::this_thread::sleep_for(std::chrono::milliseconds(DELAY_IN_MS)); // Delay to not hit rate limit
        try {
            messages = search(offset);
            debug(IS_DEBUG, std::string("Messages [JSON]:\n") + messages.dump());
        } catch (...) {
            if (IS_SKIP_IF_FAIL) {
                log(IS_VERBOSE, "Search failed! Skipping...");
                continue;
            }

            throw std::runtime_error("Search failed! Try again later.");
        }

        // Parse Messages
        log(IS_VERBOSE, "Remover: Parsing the messages...");
        std::vector<Message> msgs;
        unsigned char skippedMessages = 0;
        for (const auto& group : messages["messages"]) {
            for (const auto& msg : group) {
                if (msg.contains("id")) {
                    Message m;
                    m.ID = msg["id"].get<std::string>();
                    m.CHANNEL_ID = CHANNEL_ID;
                    m.type = msg["type"].get<int>();

                    if (isSystemMessage(m.type)) {
                        ++skippedMessages;
                        skippedMessagesSet.insert(m); // Insert only if the element does not already exist
                        continue;
                    }

                    msgs.push_back(m);
                }
            }
        }

        if (msgs.empty() && messages["total_results"].get<int>() <= 0) break; // If messages are empty and total is 0, then no messages remain to delete
        if (msgs.empty() && skippedMessages > 0 && skippedMessagesSet.size() >= messages["total_results"].get<int>()) break; // If total_results are system messages, then no messages remain to delete

        if (msgs.empty() && skippedMessages == 0) {
            offset = 0; // Reset back to first page
            continue;
        }
        else if (msgs.empty() && skippedMessages > 0) {
            offset += skippedMessages;
            continue;
        }

        for (const auto& msg: msgs) {
            try {
                std::this_thread::sleep_for(std::chrono::seconds(DELAY_IN_MS_DEFAULT)); // Delay to not hit rate limit
                deleteMessage(msg);
            } catch (...) {
                if (IS_SKIP_IF_FAIL) {
                    log(IS_VERBOSE, "WARNING: Delete Message failed! Skipping...");
                    continue;
                }

                throw std::runtime_error("Delete Message failed! Please try again later.");
            }
        }
    }
}
