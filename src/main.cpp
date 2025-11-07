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
#include <fmt/base.h>
#include <string>
#include <stdexcept>

void InteractiveSession() {
    bool is_verbose = false, is_debug = false;
    std::string verbose_input, debug_input, guild_id, channel_id, sender_id;

    ask("Enable verbose output? [y/n]: ", verbose_input);
    ask("Enable debug output (intended for development)? [y/n]: ", debug_input);
    input("Enter guild ID (or '@me' for DMs): ", guild_id);
    input("Enter channel ID: ", channel_id);
    input("Enter sender ID: ", sender_id);

    if (parse_input(verbose_input))
        is_verbose = true;
    if (parse_input(debug_input))
        is_debug = true;
    if (guild_id.empty())
        throw std::invalid_argument("Guild ID is required.");
    if (channel_id.empty())
        throw std::invalid_argument("Channel ID is required.");
    if (sender_id.empty())
        throw std::invalid_argument("Sender ID is required.");

    IS_VERBOSE = is_verbose;
    IS_DEBUG = is_debug;
    GUILD_ID = guild_id;
    CHANNEL_ID = channel_id;
    SENDER_ID = sender_id;
}

int main(int argc, char** argv) {
    try {
        fmt::print("discord-rm\n");
        fmt::print("CLI removal tool for Discord chats, using an authorization token and the Discord API.\n");
        fmt::print("By beliumgl.\n\n");

        auto& args = create_arguments();
        process_arguments(args, argc, argv);

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

        fmt::print("\nWARNING: Using self-bots may result in account termination.\n\n");

        if (!IS_NOCONFIRM) {
            std::string in;
            ask("Do you want to continue? [y/n]: ", in);

            if (!parse_input(in)) {
                fmt::print("Aborting operation.\n");
                return 0;
            }
        }

        discord_rm();
        fmt::print("All messages have been removed.\n");
        return 0;
    } catch (const std::exception& ex) {
        fmt::print("ERROR: {}\n", ex.what());
        return 1;
    }
}
