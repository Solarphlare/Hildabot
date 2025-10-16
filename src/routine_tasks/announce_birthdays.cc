#include "announce_birthdays.h"
#include <dpp/dpp.h>
#include <vector>
#include <bsoncxx/builder/basic/document.hpp>
#include <mongocxx/client.hpp>
#include <chrono>
#include <format>
#include <algorithm>
#include <exception>
#include <string>
#include <optional>
#include "util/helpers.h"
#include "logging/logging.h"
#include "constants.h"
#include "config.h"

using bsoncxx::builder::basic::make_document;
using bsoncxx::builder::basic::kvp;

namespace routine_tasks {
    dpp::task<void> announce_birthdays(dpp::cluster& bot) {
        logging::event(&bot, "Birthdays", "Starting birthdays task.");

        mongocxx::client client(mongocxx::uri(MONGO_URI));
        #ifdef DEBUG
        mongocxx::database db = client["hildabot_test"];
        #else
        mongocxx::database db = client["hildabot"];
        #endif

        std::chrono::zoned_time zt{"America/Chicago", std::chrono::system_clock::now()};
        auto callback = co_await bot.co_roles_get(BASE_GUILD_ID);

        if (!callback.is_error()) [[likely]] {
            auto roles = callback.get<dpp::role_map>();

            auto it = roles.find(BIRTHDAY_ROLE_ID);

            if (it != roles.end()) [[likely]] {
                dpp::members_container members = it->second.get_members();

                for (auto& [user_id, member] : members) {
                    member.remove_role(it->second.id);
                    auto edit_callback = co_await bot.co_guild_edit_member(member);

                    if (edit_callback.is_error()) [[unlikely]] {
                        logging::error(&bot, "Birthdays", "Failed to remove birthday role from {} ({}): {}", member.get_user()->username, user_id.str(), edit_callback.get_error().human_readable);
                    }
                }
            }
        }

        auto local_time = zt.get_local_time();
        std::chrono::year_month_day ymd = std::chrono::year_month_day(std::chrono::floor<std::chrono::days>(local_time));
        uint32_t month = static_cast<uint32_t>(ymd.month());
        uint32_t day = static_cast<uint32_t>(ymd.day());

        // get all users with a birthday in the current month and day
        auto cursor = db["users"].find(
            make_document(kvp("birthday.month", static_cast<int>(month)), kvp("birthday.day", static_cast<int>(day)))
        );

        std::vector<dpp::guild_member> birthday_members;

        for (auto& doc : cursor) {
            std::string_view user_id = doc["_id"].get_string();

            auto member_callback = co_await bot.co_guild_get_member(BASE_GUILD_ID, user_id);

            if (member_callback.is_error()) continue; // skip users not in the guild

            auto member = member_callback.get<dpp::guild_member>();
            birthday_members.push_back(member);
        }

        if (birthday_members.size() == 1) {
            dpp::message message(std::format("Hey <@{}>! I just wanted to wish you the happiest of birthdays! Can I have a slice of cake too? :birthday: :heart:", birthday_members[0].user_id.str()));
            message.set_channel_id(BIRTHDAY_CHANNEL_ID);

            auto message_callback = co_await bot.co_message_create(message);

            if (message_callback.is_error()) [[unlikely]] {
                logging::error(&bot, "Birthdays", "Failed to send birthday message: {}", message_callback.get_error().human_readable);
            }
        }
        else if (birthday_members.size() == 2) {
            dpp::message message(std::format("Hey <@{}> and <@{}>! I just wanted to wish you both the happiest of birthdays! Can I have a slice of cake too? :birthday: :heart:", birthday_members[0].user_id.str(), birthday_members[1].user_id.str()));
            message.set_channel_id(BIRTHDAY_CHANNEL_ID);

            auto message_callback = co_await bot.co_message_create(message);

            if (message_callback.is_error()) [[unlikely]] {
                logging::error(&bot, "Birthdays", "Failed to send birthday message: {}", message_callback.get_error().human_readable);
            }
        }
        else if (birthday_members.size() > 2) {
            std::ostringstream stream;
            stream << "Hey";

            for (auto it = birthday_members.begin(); it != birthday_members.end(); ++it) {
                if (it + 1 != birthday_members.end()) {
                    stream << " <@" << it->user_id.str() << ">,";
                }
                else {
                    stream << " and <@" << it->user_id.str() << ">! ";
                }
            }

            stream << "I just wanted to wish you all the happiest of birthdays! Can I have a slice of cake too? :birthday: :heart:";
            dpp::message message(stream.str());
            message.set_channel_id(BIRTHDAY_CHANNEL_ID);
            auto message_callback = co_await bot.co_message_create(message);
            if (message_callback.is_error()) [[unlikely]] {
                logging::error(&bot, "Birthdays", "Failed to send birthday message: {}", message_callback.get_error().human_readable);
            }
        }

        for (auto& it : birthday_members) {
            dpp::guild_member member = it;
            member.add_role(BIRTHDAY_ROLE_ID);
            co_await bot.co_guild_edit_member(member);
        }

        logging::event(&bot, "Birthdays", "Completed birthdays task.");
    }
}
