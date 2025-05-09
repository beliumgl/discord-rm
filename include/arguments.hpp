/*
 * DISCORD-RM
 * --------------------------------
 * CLI removal tool for Discord chats,
 * using an authorization token and
 * the Discord API.
 */

#pragma once

#include "argparse.hpp"
using namespace argparse;

ArgumentParser& createArguments();
void processArguments(ArgumentParser& p, int argc, char** argv);
