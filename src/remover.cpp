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
#include <include/json.hpp>
#include <curl/curl.h>
#include <string>
#include <utility>
#include <vector>
#include <stdexcept>
#include <chrono>
#include <thread>
#include <unordered_set>

using nlohmann::json;
using Query = std::pair<std::string, std::string>;

struct Message {
    std::string CHANNEL_ID;
    std::string ID;
    int type;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Message, ID, type);
};

json search(unsigned short offset) {
    debug(IS_DEBUG, std::string("[Search] Parameters: offset = " + std::to_string(offset)));
    log(IS_VERBOSE, "Search: Making API URL...");
    std::string apiURL = isDMGuild(GUILD_ID)
                            ? "https://discord.com/api/v9/channels/" + CHANNEL_ID + "/messages/"
                            : "https://discord.com/api/v9/guilds/" + GUILD_ID + "/messages/";

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
    CURL* curl = curl_easy_init();

    if (!curl)
        throw std::runtime_error("Failed to send search request.");

    std::string response;
    struct curl_slist* headers = nullptr;
    std::string authHeader = "Authorization: " + DISCORD_TOKEN;
    headers = curl_slist_append(headers, authHeader.c_str());

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    CURLcode result = curl_easy_perform(curl);
    if (result != CURLE_OK)
        throw std::runtime_error("Failed to send search request.");
    debug(IS_DEBUG, "Response: " + response);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    return json::parse(response);
}

void deleteMessage(const Message& message) {
    debug(IS_DEBUG, std::string("[Delete Message] Parameters: Message (ID) = " + message.ID));

    using json = nlohmann::json;

    if (isSystemMessage(message.type)) {
        log(IS_VERBOSE, "Delete Message: System message. Skipping...");
        return; // Cannot remove system message
    }

    log(IS_VERBOSE, "Delete Message: Making API URL...");
    std::string deleteAPIURL = "https://discord.com/api/v9/channels/" + message.CHANNEL_ID + "/messages/" + message.ID;
    debug(IS_DEBUG, "Full URL: " + deleteAPIURL);

    log(IS_VERBOSE, "Delete Message: Sending request...");
    CURL* curl = curl_easy_init();
    if (!curl)
        throw std::runtime_error("Failed to send delete message request.");

    std::string response;
    struct curl_slist* headers = nullptr;
    std::string authHeader = "Authorization: " + DISCORD_TOKEN;
    headers = curl_slist_append(headers, authHeader.c_str());

    curl_easy_setopt(curl, CURLOPT_URL, deleteAPIURL.c_str());
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    CURLcode result = curl_easy_perform(curl);
    long httpCode = 0;

    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    debug(IS_DEBUG, "Response: " + response + ", Code: " + std::to_string(httpCode));

    if (result != CURLE_OK)
        throw std::runtime_error("Failed to send delete message request.");

    if (httpCode == 429) { // Rate limited by Discord API
        if (!response.empty()) {
            try {
                json j = json::parse(response);
                log(IS_VERBOSE, "Delete Message: Rate limited by Discord API! Trying again later...");
                DELETE_DELAY_IN_SECONDS = static_cast<unsigned int>(j["retry_after"].get<double>());
                std::this_thread::sleep_for(std::chrono::seconds(DELETE_DELAY_IN_SECONDS * 2)); // Wait twice as long to ensure not hit rate limit again
                DELETE_DELAY_IN_SECONDS = DELETE_DELAY_IN_SECONDS_DEFAULT; // Reset back
                return;
            } catch (...) {
                throw std::runtime_error("Failed to parse rate limit JSON.");
            }
        } else {
            throw std::runtime_error("Failed to parse rate limit JSON.");
        }
    } else if (httpCode == 400 && !response.empty()) {
        try {
            json j = json::parse(response);
            if (j.contains("code") && j["code"] == 50083) {
                log(IS_VERBOSE, "Delete Message: Cannot remove archived thread. Skipping...");
                return;
            }
        } catch (...) {
            throw std::runtime_error("Failed to parse error JSON.");
        }
    } else if (httpCode < 200 || httpCode >= 300) {
        throw std::runtime_error("Failed to delete message request.");
    }

    log(IS_VERBOSE, "Delete Message: Message deleted successfully!");
}

REMOVER_STATUS discordRM() {
    log(IS_VERBOSE, "Remover: Searching for messages to delete...");

    unsigned short offset = 0;
    json messages;

    while (true) { // While loop to remove all pages
        try {
            messages = search(offset);
            debug(IS_DEBUG, std::string("Messages [JSON]:\n") + messages.dump());
        } catch (...) {
            log(IS_VERBOSE, "Search failed! Try again later.");
            return REMOVER_STATUS::SEARCH_FAILED;
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
                        continue;
                    }

                    msgs.push_back(m);
                }
            }
        }

        if (messages["total_results"].get<int>() - skippedMessages <= 0) break; // If total_results (without system messages) is zero, then no messages remain to delete

        if (msgs.empty() && skippedMessages == 0) {
            offset += PAGE_LIMIT - offset; // Offset to the next page, not just by the page size
            continue;
        }
        else if (msgs.empty() && skippedMessages > 0) {
            offset += skippedMessages;
            continue;
        }

        for (const auto& msg: msgs) {
            try {
                std::this_thread::sleep_for(std::chrono::seconds(DELETE_DELAY_IN_SECONDS)); // Delay to not hit rate limit
                deleteMessage(msg);
            } catch (...) {
                if (IS_SKIP_IF_FAIL) {
                    log(IS_VERBOSE, "Delete Message failed! Skipping...");
                    continue;
                }

                log(IS_VERBOSE, "Delete Message failed! Please try again later.");
                return REMOVER_STATUS::DELETE_MESSAGE_FAILED;
            }
        }
    }

    return REMOVER_STATUS::OK;
}
