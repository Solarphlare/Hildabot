#pragma once
#include <dpp/dpp.h>

namespace commands {
    namespace set_streak {
        dpp::task<void> execute(const dpp::slashcommand_t& event);
    }
}
