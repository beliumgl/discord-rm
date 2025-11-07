/*
 * DISCORD-RM
 * --------------------------------
 * CLI removal tool for Discord chats,
 * using an authorization token and
 * the Discord API.
 */

#include <include/config.hpp>

unsigned int DELAY_IN_MS     = DELAY_IN_MS_DEFAULT;
std::string  DISCORD_TOKEN;
bool         IS_VERBOSE      = false;
bool         IS_DEBUG        = false;
bool         IS_NOCONFIRM    = false;
bool         IS_INTERACTIVE  = false;
bool         IS_SKIP_IF_FAIL = false;
std::string  SENDER_ID;
std::string  GUILD_ID;
std::string  CHANNEL_ID;
