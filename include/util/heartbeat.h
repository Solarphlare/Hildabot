#pragma once

#ifndef DEBUG
#include <dpp/dpp.h>
namespace heartbeat {
    dpp::task<void> send_heartbeat(const dpp::message_create_t& bot);
}
#endif
