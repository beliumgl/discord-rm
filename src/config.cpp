/*
 * DISCORD-RM
 * --------------------------------
 * CLI removal tool for Discord chats,
 * using an authorization token and
 * the Discord API.
 */

#include <include/config.hpp>

std::string  DISCORD_TOKEN;
bool         IS_VERBOSE      = false;
bool         IS_DEBUG        = false;
bool         IS_NOCONFIRM    = false;
bool         IS_INTERACTIVE  = false;
bool         IS_SKIP_IF_FAIL = false;
std::string  SENDER_ID;
std::string  GUILD_ID;
std::string  CHANNEL_ID;
unsigned int DELETE_DELAY_IN_SECONDS = DELETE_DELAY_IN_SECONDS_DEFAULT;
