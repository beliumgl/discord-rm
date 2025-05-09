/*
 * DISCORD-RM
 * --------------------------------
 * CLI removal tool for Discord chats,
 * using an authorization token and
 * the Discord API.
 */

#include <include/remover.hpp>
#include <include/helpers.hpp>
#include <include/json.hpp>
#include <curl/curl.h>
#include <string>
#include <utility>
#include <vector>
#include <stdexcept>
#include <chrono>
#include <thread>

using nlohmann::json;
using Query = std::pair<std::string, std::string>;

struct Message {
    std::string channelID;
    std::string ID;
    int type;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Message, ID, type);
};

json search(bool isVerbose,
            bool isDebug,
            const std::string& discordToken,
            const std::string& guildID,
            const std::string& channelID,
            const std::string& senderID) {
    debug(isDebug, std::string("[Search] Parameters: isVerbose = ")
        + (isVerbose ? "true" : "false") + ", isDebug = "
        + (isDebug ? "true" : "false") + ", guildID = "
        + guildID + ", channelID = "
        + channelID + ", senderID = " + senderID);
    log(isVerbose, "Search: Making API URL...");
    std::string apiURL = isDMGuild(guildID)
                            ? "https://discord.com/api/v9/channels/" + channelID + "/messages/"
                            : "https://discord.com/api/v9/guilds/" + guildID + "/messages/";

    std::vector<Query> params = {
        {"author_id", senderID},
        {"channel_id", (isDMGuild(guildID) ? channelID : "")}
    };

    log(isVerbose, "Search: Making request parameters...");
    std::string query = buildQueryString(params);
    debug(isDebug, "Query Parameters: " + query);
    log(isVerbose, "Search: Making full API URL...");
    std::string url = apiURL + "search?" + query;
    debug(isDebug, "Full URL: " + url);

    log(isVerbose, "Search: Sending request...");
    CURL* curl = curl_easy_init();

    if (!curl)
        throw std::runtime_error("Failed to send search request.");

    std::string response;
    struct curl_slist* headers = nullptr;
    std::string authHeader = "Authorization: " + discordToken;
    headers = curl_slist_append(headers, authHeader.c_str());

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    CURLcode result = curl_easy_perform(curl);
    if (result != CURLE_OK)
        throw std::runtime_error("Failed to send search request.");
    debug(isDebug, "Response: " + response);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    return json::parse(response);
}

void deleteMessage(bool isVerbose,
                   bool isDebug,
                   unsigned int& delay,
                   unsigned int defaultDelay,
                   const std::string& discordToken,
                   const Message& message)
{
    debug(isDebug, std::string("[Delete Message] Parameters: isVerbose = ")
        + (isVerbose ? "true" : "false") + ", isDebug = "
        + (isDebug ? "true" : "false")  + ", delay = "
        + std::to_string(delay) + "s, defaultDelay = "
        + std::to_string(defaultDelay) + ", message (ID) = " + message.ID);

    using json = nlohmann::json;

    if ((message.type < 6 || message.type > 21) && message.type != 0) {
        log(isVerbose, "Delete Message: System message. Skipping...");
        return; // Cannot remove system message
    }

    log(isVerbose, "Delete Message: Making API URL...");
    std::string deleteAPIURL = "https://discord.com/api/v9/channels/" + message.channelID + "/messages/" + message.ID;
    debug(isDebug, "Full URL: " + deleteAPIURL);

    log(isVerbose, "Delete Message: Sending request...");
    CURL* curl = curl_easy_init();
    if (!curl)
        throw std::runtime_error("Failed to send delete message request.");

    std::string response;
    struct curl_slist* headers = nullptr;
    std::string authHeader = "Authorization: " + discordToken;
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

    debug(isDebug, "Response: " + response + ", Code: " + std::to_string(httpCode));

    if (result != CURLE_OK)
        throw std::runtime_error("Failed to send delete message request.");

    if (httpCode == 429) { // Rate limited by Discord API
        if (!response.empty()) {
            try {
                json j = json::parse(response);
                log(isVerbose, "Delete Message: Rate limited by Discord API! Trying again later...");
                delay = static_cast<unsigned int>(j["retry_after"].get<double>());
                std::this_thread::sleep_for(std::chrono::seconds(delay * 2)); // Wait twice as long to ensure not hit rate limit again
                delay = defaultDelay; // Reset back
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
                log(isVerbose, "Delete Message: Cannot remove archived thread. Skipping...");
                return;
            }
        } catch (...) {
            throw std::runtime_error("Failed to parse error JSON.");
        }
    } else if (httpCode < 200 || httpCode >= 300) {
        log(isVerbose, "Delete Message: Failed to delete message.");
        throw std::runtime_error("Failed to delete message request.");
    }

    log(isVerbose, "Delete Message: Message deleted successfully!");
}

REMOVER_STATUS discordRM(bool isVerbose,
                         bool isDebug,
                         unsigned int delay,
                         unsigned int defaultDelay,
                         const std::string& discordToken,
                         const std::string& guildID,
                         const std::string& channelID,
                         const std::string& senderID) {
    debug(isDebug, std::string("[Remover] Parameters: isVerbose = ")
        + (isVerbose ? "true" : "false") + ", isDebug = "
        + (isDebug ? "true" : "false") + ", delay = "
        + std::to_string(delay) + "s, defaultDelay = "
        + std::to_string(defaultDelay) + "s, guildID = "
        + guildID + ", channelID = "
        + channelID + ", senderID = " + senderID);
    log(isVerbose, "Remover: Searching for messages to delete...");
    json messages;
    do {
        try {
            messages = search(isVerbose, isDebug, discordToken, guildID, channelID, senderID);
            debug(isDebug, std::string("Messages [JSON]:\n") + messages.dump());
        } catch (...) {
            log(isVerbose, "Remover: Search failed! Try again later.");
            return REMOVER_STATUS::SEARCH_FAILED;
        }

        if (messages["messages"].empty()) { // fix infinite loop
            log(isVerbose, "Remover: All messages have been removed.");
            break;
        }

        // Parse Messages
        log(isVerbose, "Remover: Parsing the messages...");
        std::vector<Message> msgs;
        for (const auto& group : messages["messages"]) {
            for (const auto& msg : group) {
                if (msg.contains("id")) {
                    Message m;
                    m.ID = msg["id"].get<std::string>();
                    m.channelID = channelID;
                    m.type = msg["type"].get<int>();
                    msgs.push_back(m);
                }
            }
        }

        try {
            for (const auto& msg: msgs) {
                std::this_thread::sleep_for(std::chrono::seconds(delay)); // Delay to not hit rate limit
                deleteMessage(isVerbose, isDebug, delay, defaultDelay, discordToken, msg);
            }
        } catch (...) {
            log(isVerbose, "Delete Message failed! Please try again later.");
            return REMOVER_STATUS::DELETE_MESSAGE_FAILED;
        }
    } while (!messages["messages"].empty()); // While loop to remove all pages

    return REMOVER_STATUS::OK;
}
