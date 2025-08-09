#include <dpp/dpp.h>
#include <bsoncxx/builder/basic/document.hpp>
#include "db/mongo_database.h"

using bsoncxx::builder::basic::make_document;
using bsoncxx::builder::basic::kvp;

namespace heartbeat {
    dpp::task<void> send_heartbeat(const dpp::message_create_t& event) {
        if (event.msg.content != "hildabot_PING") co_return;
        auto result = MongoDatabase::get_database()["users"].find_one(make_document(kvp("xp", make_document(kvp("$exists", true)))));
        co_await event.co_send("ACK");
    }
}
