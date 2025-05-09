/*
 * DISCORD-RM
 * --------------------------------
 * CLI removal tool for Discord chats,
 * using an authorization token and
 * the Discord API.
 */

#pragma once

#include <string>

enum class REMOVER_STATUS {
    OK = 0,
    SEARCH_FAILED = 1,
    DELETE_MESSAGE_FAILED = 2
};

REMOVER_STATUS discordRM();
