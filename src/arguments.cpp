/*
 * DISCORD-RM
 * --------------------------------
 * CLI removal tool for Discord chats,
 * using an authorization token and
 * the Discord API.
 */

#include <include/arguments.hpp>
#include <include/config.hpp>
#include <stdexcept>

argparse::ArgumentParser& create_arguments() {
    using namespace argparse;

    static ArgumentParser program("discord-rm", "1.1");
    program.add_argument("-v", "--verbose")
        .help("Verbose output")
        .default_value(false)
        .implicit_value(true);
    program.add_argument("-d", "--debug")
        .help("Debug output (use only for developing)")
        .default_value(false)
        .implicit_value(true);
    program.add_argument("-nc", "--no-confirm")
        .help("Delete messages without confirmation")
        .default_value(false)
        .implicit_value(true);
    program.add_argument("-dl", "--delay")
        .help("Delay time in milliseconds")
        .scan<'u', unsigned int>()
        .default_value(DELAY_IN_MS_DEFAULT);
    program.add_argument("-i", "--interactive")
        .help("Interactive mode")
        .default_value(false)
        .implicit_value(true);
    program.add_argument("-sif", "--skip-if-fail")
        .help("Won't exit if message deletion fails")
        .default_value(false)
        .implicit_value(true);
    program.add_argument("-s", "--sender-id")
        .default_value(std::string(""))
        .help("Specify the sender's user ID");
    program.add_argument("-g", "--guild-id")
        .default_value(std::string(""))
        .help("Specify the guild ID (server ID)");
    program.add_argument("-c", "--channel-id")
        .default_value(std::string(""))
        .help("Specify the channel ID");

    return program;
}

void process_arguments(ArgumentParser& program, int argc, char** argv) {
    program.parse_args(argc, argv);

    bool is_interactive = program.get<bool>("--interactive");
    std::string sender = program.get<std::string>("--sender-id");
    std::string guild  = program.get<std::string>("--guild-id");
    std::string channel= program.get<std::string>("--channel-id");

    if (!is_interactive) {
        if (sender.empty())
            throw std::invalid_argument("`--sender-id` is required unless `--interactive` is set.");
        if (guild.empty())
            throw std::invalid_argument("`--guild-id` is required unless `--interactive` is set.");
        if (channel.empty())
            throw std::invalid_argument("`--channel-id` is required unless `--interactive` is set.");
    }

    IS_INTERACTIVE  = is_interactive;
    SENDER_ID       = sender;
    GUILD_ID        = guild;
    CHANNEL_ID      = channel;
    DELAY_IN_MS     = program.get<unsigned int>("--delay");
    IS_VERBOSE      = program.get<bool>("--verbose");
    IS_DEBUG        = program.get<bool>("--debug");
    IS_NOCONFIRM    = program.get<bool>("--no-confirm");
    IS_SKIP_IF_FAIL = program.get<bool>("--skip-if-fail");
}
