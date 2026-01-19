#pragma once
#include <dpp/dpp.h>

namespace util {
    dpp::task<bool> can_run_sensitive_commands(dpp::cluster* bot, const dpp::guild_member& member);
}
