/*
 * DISCORD-RM
 * --------------------------------
 * CLI removal tool for Discord chats,
 * using an authorization token and
 * the Discord API.
 */

#include <include/config.hpp>
#include <vector>

unsigned int              DELAY_IN_MS     = DELAY_IN_MS_DEFAULT;
std::string               DISCORD_TOKEN;
bool                      IS_VERBOSE      = false;
bool                      IS_DEBUG        = false;
bool                      IS_NOCONFIRM    = false;
bool                      IS_INTERACTIVE  = false;
bool                      IS_SKIP_IF_FAIL = false;
bool                      REMOVE_PINNED   = true;
bool                      NO_LINK         = false;
bool                      NO_EMBED        = false;
bool                      NO_POLL         = false;
bool                      NO_FILE         = false;
bool                      NO_VIDEO        = false;
bool                      NO_IMAGE        = false;
bool                      NO_SOUND        = false;
bool                      NO_STICKER      = false;
bool                      NO_FORWARD      = false;
std::vector<std::string>  MENTIONS{};
std::string               BEFORE_DATE;
std::string               DURING_DATE;
std::string               AFTER_DATE;
std::string               SENDER_ID;
std::string               GUILD_ID;
std::string               CHANNEL_ID;
