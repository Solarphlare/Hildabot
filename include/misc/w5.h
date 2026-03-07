#pragma once
#include <dpp/dpp.h>

namespace misc {
    // w5 = who what when where why
    namespace w5 {
        dpp::task<void> run(const dpp::message_create_t& event);
    }
}
