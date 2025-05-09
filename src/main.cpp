/*
 * DISCORD-RM
 * --------------------------------
 * CLI removal tool for Discord chats,
 * using an authorization token and
 * the Discord API.
 */

#include <include/arguments.hpp>
#include <include/config.hpp>
#include <include/remover.hpp>
#include <include/helpers.hpp>
#include <iostream>
#include <string>
#include <stdexcept>

void InteractiveSession() {
    bool isVerbose = false, isDebug = false;
    std::string verboseInput, debugInput, guildID, channelID, senderID;

    ask("Enable verbose output? [y/n]: ", verboseInput);
    ask("Enable debug output (intended for development)? [y/n]: ", debugInput);
    input("Enter guild ID (or '@me' for DMs): ", guildID);
    input("Enter channel ID: ", channelID);
    input("Enter sender ID: ", senderID);

    if (parseInput(verboseInput))
        isVerbose = true;
    if (parseInput(debugInput))
        isDebug = true;
    if (guildID.empty())
        throw std::invalid_argument("Guild ID is required.");
    if (channelID.empty())
        throw std::invalid_argument("Channel ID is required.");
    if (senderID.empty())
        throw std::invalid_argument("Sender ID is required.");

    IS_VERBOSE = isVerbose;
    IS_DEBUG = isDebug;
    GUILD_ID = guildID;
    CHANNEL_ID = channelID;
    SENDER_ID = senderID;
}

int main(int argc, char** argv) {
    std::cout << "discord-rm" << '\n';
    std::cout << "CLI removal tool for Discord chats, using an authorization token and the Discord API." << '\n';
    std::cout << "By beliumgl." << '\n';

    auto& args = createArguments();
    processArguments(args, argc, argv);

    /*
     * Ask for the Discord token, even if not in an interactive session.
     * (Passing the Discord token as an argument is dangerous.)
     *
     * Currently, with echoing, but I may fix this later.
     */
    input("Enter your Discord token: ", DISCORD_TOKEN);

    if (DISCORD_TOKEN.empty())
        throw std::invalid_argument("Discord token is required for authorization.");

    if (IS_INTERACTIVE)
        InteractiveSession();

    std::cout << "\nWARNING: Using self-bots may result in account termination.\n";

    if (!IS_NOCONFIRM) {
        std::string in;
        ask("Do you want to continue? [y/n]: ", in);

        if (!parseInput(in)) {
            log(IS_VERBOSE, "Aborting operation.");
            return 0;
        }
    }

    REMOVER_STATUS status = discordRM();

    if (status != REMOVER_STATUS::OK)
        log(IS_VERBOSE, "Failed to remove messages.");
    else
        log(IS_VERBOSE, "All messages have been removed.");

    return status == REMOVER_STATUS::OK ? 0 : 1;
}

