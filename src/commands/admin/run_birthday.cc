#include "commands/admin/run_birthday.h"
#include <dpp/dpp.h>
#include <vector>
#include "routine_tasks/announce_birthdays.h"
#include "util/owner.h"

dpp::task<void> commands::run_birthday::execute(const dpp::message_create_t& event, const std::vector<std::string>& args) {
    if ((co_await util::get_owner_id(event.owner)) != event.msg.author.id) {
        co_return;
    }

    co_await routine_tasks::announce_birthdays(*event.owner);
    co_await event.owner->co_message_add_reaction(event.msg, "\U0001F44D"); // thumbs up
}
