#include "slashcommands/staff/set_streak.h"
#include <dpp/dpp.h>
#include <format>
#include <bsoncxx/builder/basic/document.hpp>
#include "util/command_permissions.h"
#include "util/helpers.h"
#include "logging/logging.h"
#include "db/user_entry.h"
#include "db/mongo_database.h"

using bsoncxx::builder::basic::make_document;
using bsoncxx::builder::basic::kvp;

namespace commands {
    namespace set_streak { // command ID: 1420061660177105059
        dpp::task<void> execute(const dpp::slashcommand_t& event) {
            if (!(co_await util::can_run_sensitive_commands(event.owner, event.command.member))) {
                dpp::message reply = dpp::message("You don't have permission to use this command.").set_flags(dpp::m_ephemeral);
                co_await event.co_reply(reply);
                co_return;
            }

            dpp::command_interaction interaction = event.command.get_command_interaction();
            auto subcommand = interaction.options[0];

            const dpp::guild_member member = event.command.get_resolved_member(subcommand.get_value<dpp::snowflake>(0));
            const int32_t streak = subcommand.get_value<long>(1);

            if (member.user_id == event.command.usr.id) {
                dpp::message reply = dpp::message("You can't modify your own streak.").set_flags(dpp::m_ephemeral);
                co_await event.co_reply(reply);
                co_return;
            }

            if (streak < 0) {
                dpp::message reply = dpp::message("Streak must be a positive number.").set_flags(dpp::m_ephemeral);
                co_await event.co_reply(reply);
                co_return;
            }

            UserEntry user_entry(member);

            if (user_entry.get_high_score() < streak) {
                dpp::message reply = dpp::message("You can't set a user's streak higher than their high score.").set_flags(dpp::m_ephemeral);
                co_await event.co_reply(reply);
                co_return;
            }

            if (user_entry.get_streak() >= streak) {
                dpp::message reply = dpp::message("You can't set a user's streak to a value lower than or equal to their current streak.").set_flags(dpp::m_ephemeral);
                co_await event.co_reply(reply);
                co_return;
            }

            const int64_t new_expiry = util::midnight_seconds_in_a_week();

            auto& db = MongoDatabase::get_database();
            db["users"].update_one(
                make_document(kvp("_id", member.user_id.str())),
                make_document(kvp("$set", make_document(
                    kvp("streak", make_document(
                        kvp("count", streak), // this MUST be an int32_t
                        kvp("expiry", new_expiry)
                    ))
                )
            )));

            co_await event.co_reply(std::format("Set {}'s streak to {}. It now expires on <t:{}>.", member.get_mention(), streak, new_expiry));
            logging::event(event.owner, "Set Streak", "{} ({}) set {} ({})'s streak to {}.", event.command.usr.username, event.command.usr.id.str(), member.get_user()->username, member.user_id.str(), streak);
        }
    }
}
