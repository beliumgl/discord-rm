/*
 * DISCORD-RM
 * --------------------------------
 * CLI removal tool for Discord chats,
 * using an authorization token and
 * the Discord API.
 */

#include <include/arguments.hpp>
#include <include/config.hpp>
#include <include/helpers.hpp>
#include <stdexcept>

argparse::ArgumentParser& create_arguments() {
    using namespace argparse;

    static ArgumentParser program("discord-rm", "1.3");
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
    program.add_argument("-m", "--mentions")
        .help("Specify the mentioned user ID")
        .nargs(argparse::nargs_pattern::any)
        .default_value(std::vector<std::string>{});
    program.add_argument("-b", "--before-date")
        .help("Will only delete messages before this date (ISO 8601 e.g. 2015-01-01T00:00:00)")
        .default_value(std::string());
    program.add_argument("-dd", "--during-date")
        .help("Will only delete messages during this date (ISO 8601 e.g. 2015-01-01T00:00:00)")
        .default_value(std::string());
    program.add_argument("-a", "--after-date")
        .help("Will only delete messages after this date (ISO 8601 e.g. 2015-01-01T00:00:00)")
        .default_value(std::string());
    program.add_argument("-nl", "--no-link")
        .help("Do not remove links")
        .default_value(false)
        .implicit_value(true);
    program.add_argument("-ne", "--no-embed")
        .help("Do not remove embedded messages")
        .default_value(false)
        .implicit_value(true);
    program.add_argument("-npl", "--no-poll")
        .help("Do not remove polls")
        .default_value(false)
        .implicit_value(true);
    program.add_argument("-nf", "--no-file")
        .help("Do not remove files")
        .default_value(false)
        .implicit_value(true);
    program.add_argument("-nv", "--no-video")
        .help("Do not remove videos")
        .default_value(false)
        .implicit_value(true);
    program.add_argument("-ni", "--no-image")
        .help("Do not remove images")
        .default_value(false)
        .implicit_value(true);
    program.add_argument("-na", "--no-audio")
        .help("Do not remove audio")
        .default_value(false)
        .implicit_value(true);
    program.add_argument("-ns", "--no-sticker")
        .help("Do not remove stickers")
        .default_value(false)
        .implicit_value(true);
    program.add_argument("-nfw", "--no-forward")
        .help("Do not remove forwarded messages")
        .default_value(false)
        .implicit_value(true);
    program.add_argument("-np", "--no-pinned")
        .help("Do not remove pinned messages")
        .default_value(false)
        .implicit_value(true);

    return program;
}

void process_arguments(ArgumentParser& program, int argc, char** argv) {
    program.parse_args(argc, argv);

    const bool is_interactive = program.get<bool>("--interactive");
    const auto sender     = program.get<std::string>("--sender-id");
    const auto guild      = program.get<std::string>("--guild-id");
    const auto channel    = program.get<std::string>("--channel-id");

    if (!is_interactive) {
        if (sender.empty())
            throw std::invalid_argument("`--sender-id` is required unless `--interactive` is set.");
        if (guild.empty())
            throw std::invalid_argument("`--guild-id` is required unless `--interactive` is set.");
        if (channel.empty())
            throw std::invalid_argument("`--channel-id` is required unless `--interactive` is set.");
    }

    auto before_date         = program.get<std::string>("--before-date");
    auto during_date          = program.get<std::string>("--during-date");
    auto after_date          = program.get<std::string>("--after-date");

    IS_INTERACTIVE      = is_interactive;
    SENDER_ID           = sender;
    GUILD_ID            = guild;
    CHANNEL_ID          = channel;
    DELAY_IN_MS         = program.get<unsigned int>("--delay");
    IS_VERBOSE          = program.get<bool>("--verbose");
    IS_DEBUG            = program.get<bool>("--debug");
    IS_NOCONFIRM        = program.get<bool>("--no-confirm");
    IS_SKIP_IF_FAIL     = program.get<bool>("--skip-if-fail");
    MENTIONS            = program.get<std::vector<std::string>>("--mentions");
    BEFORE_DATE         = !before_date.empty() ? convert_to_snowflake_id(before_date) : "";
    DURING_DATE         = !during_date.empty() ? convert_to_snowflake_id(during_date) : "";
    AFTER_DATE          = !after_date.empty() ? convert_to_snowflake_id(after_date) : "";
    REMOVE_PINNED       = !program.get<bool>("--no-pinned");
    NO_LINK             = program.get<bool>("--no-link");
    NO_EMBED            = program.get<bool>("--no-embed");
    NO_POLL             = program.get<bool>("--no-poll");
    NO_FILE             = program.get<bool>("--no-file");
    NO_VIDEO            = program.get<bool>("--no-video");
    NO_IMAGE            = program.get<bool>("--no-image");
    NO_SOUND            = program.get<bool>("--no-audio");
    NO_STICKER          = program.get<bool>("--no-sticker");
    NO_FORWARD          = program.get<bool>("--no-forward");
}
