#include "commands/admin/get_version.h"
#include "util/version.h"
#include <dpp/dpp.h>
#include <vector>
#include <string>
#include <format>
#include "util/owner.h"

namespace commands {
    namespace get_version {
        dpp::task<void> execute(const dpp::message_create_t& event, const std::vector<std::string>& args) {
            if ((co_await util::get_owner_id(event.owner)) != event.msg.author.id) {
                co_return;
            }

            co_await event.co_send(std::format("{} ({})", VERSION, BUILD_NUMBER));
        }
    }
}
