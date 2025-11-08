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
#include <utility>
#include <vector>
#include <stdexcept>
#include <chrono>
#include <thread>
#include <unordered_set>

using nlohmann::json;
using Query = std::pair<std::string, std::string>;

const std::string DISCORD_API_URL_BASE = "https://discord.com/api/";
const std::string DISCORD_API_VERSION = "v10";
const std::string DISCORD_API_AUTHORIZATION_KEY = "Authorization: ";
const std::string CURL_GET_METHOD = "GET";
const std::string CURL_DELETE_METHOD = "DELETE";

struct Message {
    std::string id;
    int type{};

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Message, id, type);

    Message(std::string id, int type) : id(std::move(id)), type(type) {}
    bool operator==(const Message& other) const { // Required for unordered_set
        return id == other.id &&
        type == other.type;
    }
};

template<> struct std::hash<Message> // Required for unordered_set
{
    std::size_t operator()(const Message& m) const noexcept {
        std::size_t h1 = std::hash<std::string>{}(m.id);
        std::size_t h2 = std::hash<int>{}(m.type);
        return h1 ^ (h2 << 1);
    }
};

json search(const unsigned short offset) {
    debug(IS_DEBUG, std::string("[Search] Parameters: offset = " + std::to_string(offset)));
    const std::string api_url = is_dm_guild(GUILD_ID)
                            ? DISCORD_API_URL_BASE + DISCORD_API_VERSION + "/channels/" + CHANNEL_ID + "/messages/"
                            : DISCORD_API_URL_BASE + DISCORD_API_VERSION + "/guilds/" + GUILD_ID + "/messages/";

    std::vector<Query> params = {
        {"author_id", SENDER_ID},
        {"channel_id", CHANNEL_ID},
        {"offset", std::to_string(offset)},
        {"limit", std::to_string(PAGE_LIMIT)}
    };

    const std::string query = build_query_string(params);
    std::string response, auth_header = DISCORD_API_AUTHORIZATION_KEY + DISCORD_TOKEN;;
    debug(IS_DEBUG, "Query Parameters: " + query);

    const std::string url = api_url + "search?" + query;
    debug(IS_DEBUG, "Full URL: " + url);

    log(IS_VERBOSE, "Search: Sending request...");
    auto [curl, result] = send_request(response, auth_header, url, CURL_GET_METHOD);

    if (result != CURLE_OK)
        throw std::runtime_error("Failed to send search request.");
    debug(IS_DEBUG, "Response: " + response);

    curl_easy_cleanup(curl);

    json json_response = json::parse(response);

    if (json_response.contains("retry_after")) { // Rate limited by discord
        log(IS_VERBOSE, "Search: Rate limited by Discord API! Trying again later...");
        handle_rate_limit(json_response);
        log(IS_VERBOSE, "Search: Retrying...");
        json_response = search(offset); // Retry
    }

    return json_response;
}

void delete_message(const Message& message) {
    constexpr unsigned short RATE_LIMITED_HTTP_CODE = 429;
    constexpr unsigned short ARCHIVED_THREAD_CODE = 50083;

    debug(IS_DEBUG, std::string("[Delete Message] Parameters: Message (ID) = " + message.id));

    if (is_system_message(message.type)) { // Redundant, but left for safety
        log(IS_VERBOSE, "Delete Message: System message. Skipping...");
        return; // Cannot remove system message
    }

    const std::string delete_api_url = DISCORD_API_URL_BASE + DISCORD_API_VERSION + "/channels/" + CHANNEL_ID + "/messages/" + message.id;
    const std::string auth_header = DISCORD_API_AUTHORIZATION_KEY + DISCORD_TOKEN;
    std::string response;
    debug(IS_DEBUG, "Full URL: " + delete_api_url);

    log(IS_VERBOSE, "Delete Message: Sending request...");
    auto [curl, result] = send_request(response, auth_header, delete_api_url, CURL_DELETE_METHOD);
    long http_code = 0;

    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    curl_easy_cleanup(curl);

    debug(IS_DEBUG, "Response: " + response + ", Code: " + std::to_string(http_code));

    if (result != CURLE_OK)
        throw std::runtime_error("Failed to send delete message request.");

    if (http_code == RATE_LIMITED_HTTP_CODE) { // Rate limited by Discord API
        try {
            const json j = json::parse(response);
            log(IS_VERBOSE, "Delete Message: Rate limited by Discord API! Trying again later...");
            handle_rate_limit(j);
            log(IS_VERBOSE, "Delete Message: Retrying...");
            delete_message(message); // Retry
        } catch (...) {
            throw std::runtime_error("Failed to parse rate limit JSON.");
        }
    }

    if (http_code == 400 && !response.empty()) {
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

    if (http_code < 200 || http_code >= 300) { // HTTP error codes are usually less than 200 or greater than or equal to 300
        throw std::runtime_error("Failed to delete message request.");
    }

    log(IS_VERBOSE, "Delete Message: Message deleted successfully!");
}

void discord_rm() {
    log(IS_VERBOSE, "Remover: Searching for messages to delete...");

    unsigned short offset = 0, deleted_messages = 0;
    std::unordered_set<Message> skipped_messages_set; // all-time skipped messages
    json messages;

    while (true) {
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

        const unsigned int total_results = messages["total_results"].get<int>();

        // Parse Messages
        log(IS_VERBOSE, "Remover: Parsing the messages...");
        std::vector<Message> msgs;
        unsigned char skipped_messages = 0; // skipped messages in current page

        const auto msg_group = messages["messages"];
        for (const auto& msg : msg_group) {
            if (!msg[0].contains("id")) continue; // We want to parse only user messages

            Message m(msg[0]["id"].get<std::string>(), msg[0]["type"].get<int>());
            if (is_system_message(m.type)) {
                ++skipped_messages;
                skipped_messages_set.insert(m);
                continue;
            }

            msgs.emplace_back(m);
        }

        const size_t skipped_messages_set_size = skipped_messages_set.size();
        if (msgs.empty() && total_results <= skipped_messages_set_size) break; // If total_results are system messages, then no messages remain to delete

        if (msgs.empty() && total_results > skipped_messages_set_size) {
            if (deleted_messages >= total_results - skipped_messages_set_size) break; // Deleted all messages
            deleted_messages = 0;

            // This means Discord hasn't updated the data yet, so we wait 10 times longer to minimize requests
            log(IS_VERBOSE, "Remover: Discord hasn't update the data yet, waiting 10 times longer to minimize requests...");
            std::this_thread::sleep_for(std::chrono::milliseconds(std::max(DELAY_IN_MS_DEFAULT * 10, DELAY_IN_MS * 10)));
            continue;
        }

        if (msgs.empty() && skipped_messages > 0) {
            // If at least one system message was skipped, skip them; otherwise, reset to the first page
            offset = skipped_messages > 0 ? offset + skipped_messages : 0;
            continue;
        }

        for (const auto& msg: msgs) {
            try {
                std::this_thread::sleep_for(std::chrono::milliseconds(DELAY_IN_MS_DEFAULT)); // Delay to not hit rate limit
                delete_message(msg);
                ++deleted_messages;
            } catch (...) {
                if (IS_SKIP_IF_FAIL) {
                    log(IS_VERBOSE, "Delete Message failed! Skipping...");
                    continue;
                }

                throw std::runtime_error("Delete Message failed! Please try again later.");
            }
        }
    }
}
