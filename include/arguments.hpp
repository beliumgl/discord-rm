/*
 * DISCORD-RM
 * --------------------------------
 * CLI removal tool for Discord chats,
 * using an authorization token and
 * the Discord API.
 */

#pragma once

#include <argparse/argparse.hpp>
using namespace argparse;

ArgumentParser& create_arguments();
void process_arguments(ArgumentParser& p, int argc, char** argv);
