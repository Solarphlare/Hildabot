#include "misc/w5.h"
#include <dpp/dpp.h>
#include <algorithm>
#include <string>
#include <random>

namespace misc::w5 {
    const std::string trigger_phrases[6] = {"who is hildabot", "what is hildabot", "when is hildabot", "where is hildabot", "why is hildabot", "how is hildabot"};
    std::mt19937 rng(std::random_device{}());

    dpp::task<void> run(const dpp::message_create_t& event) {
        std::string content = event.msg.content;
        std::transform(content.begin(), content.end(), content.begin(), ::tolower);

        // if message ends with a question mark, chop it off
        if (content.back() == '?') {
            content.pop_back();
        }

        const bool found = std::find(std::begin(trigger_phrases), std::end(trigger_phrases), content) != std::end(trigger_phrases);
        if (!found) co_return;

        const size_t index = std::uniform_int_distribution<size_t>(0, 4)(rng);

        if (content.starts_with("who")) {
            constexpr const char* responses[5] = {
                "solar probably?",
                "hildabot",
                "me",
                "some bot",
                "seth mcfarlane"
            };

            co_await event.co_send(responses[index]);
        }
        else if (content.starts_with("what")) {
            constexpr const char* responses[5] = {
                "a bot",
                "a scourge upon this server",
                "an incompetent thing that keeps skipping routine tasks for some god forsaken reason",
                "me",
                "a thing that solar made for some reason"
            };

            co_await event.co_send(responses[index]);
        }
        else if (content.starts_with("when")) {
            constexpr const char* responses[5] = {
                "whenever i feel like it",
                "january 27th, 1997",
                "sometime in the next decade",
                "whenever i feel like it"
                "whenever season 4 drops"
            };

            co_await event.co_send(responses[index]);
        }
        else if (content.starts_with("where")) {
            co_await event.co_send("Fairfax County, Virginia, United States (us-east-1)");
        }
        else if (content.starts_with("why")) {
            constexpr const char* responses[5] = {
                "i wanted to",
                "why not",
                "i felt like it",
                "i don't need a reason",
                "'cause i'm bored"
            };

            co_await event.co_send(responses[index]);
        }
        else if (content.starts_with("how")) {
            constexpr const char* responses[5] = {
                "could be better",
                "doing my best",
                "could be better",
                "pretty bad",
                "trying to be sus"
            };

            co_await event.co_send(responses[index]);
        }
    }
}
