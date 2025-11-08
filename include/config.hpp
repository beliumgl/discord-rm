/*
 * DISCORD-RM
 * --------------------------------
 * CLI removal tool for Discord chats,
 * using an authorization token and
 * the Discord API.
 */

#pragma once
#include <string>
#include <vector>

extern unsigned int                       DELAY_IN_MS;
extern std::string                        DISCORD_TOKEN;
extern bool                               IS_VERBOSE;
extern bool                               IS_DEBUG;
extern bool                               IS_NOCONFIRM;
extern bool                               IS_INTERACTIVE;
extern bool                               IS_SKIP_IF_FAIL;
extern std::string                        SENDER_ID;
extern std::string                        GUILD_ID;
extern std::string                        CHANNEL_ID;
extern unsigned int                       DELETE_DELAY_IN_SECONDS;
extern std::vector<std::string>           MENTIONS;
extern std::string                        BEFORE_DATE;
extern std::string                        DURING_DATE;
extern std::string                        AFTER_DATE;
extern bool                               REMOVE_PINNED;
extern bool                               NO_LINK;
extern bool                               NO_EMBED;
extern bool                               NO_POLL;
extern bool                               NO_FILE;
extern bool                               NO_VIDEO;
extern bool                               NO_IMAGE;
extern bool                               NO_SOUND;
extern bool                               NO_STICKER;
extern bool                               NO_FORWARD;
constexpr unsigned int                    DELAY_IN_MS_DEFAULT     = 1000;
constexpr unsigned short                  PAGE_LIMIT              = 25;
