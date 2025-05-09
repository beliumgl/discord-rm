/*
 * DISCORD-RM
 * --------------------------------
 * CLI removal tool for Discord chats,
 * using an authorization token and
 * the Discord API.
 */

#pragma once
#include <string>

extern std::string DISCORD_TOKEN;
extern bool        IS_VERBOSE;
extern bool        IS_DEBUG;
extern bool        IS_NOCONFIRM;
extern bool        IS_INTERACTIVE;
extern bool        IS_SKIP_IF_FAIL;
extern std::string SENDER_ID;
extern std::string GUILD_ID;
extern std::string CHANNEL_ID;
extern unsigned int DELETE_DELAY_IN_SECONDS;
constexpr unsigned int DELETE_DELAY_IN_SECONDS_DEFAULT = 1;
constexpr unsigned short PAGE_LIMIT                    = 25;
