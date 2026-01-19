#include "routine_tasks/routine_tasks.h"
#include <dpp/dpp.h>
#include "scheduler/instance.h"

#include "routine_tasks/announce_birthdays.h"
#include "routine_tasks/streak_warnings.h"
#include "routine_tasks/apply_streak_savers.h"

namespace routine_tasks {
    void schedule(dpp::cluster& bot) {
        scheduler.cron("0 0 * * *", [&bot]() -> dpp::task<void> {
            co_await announce_birthdays(bot);
        });

        scheduler.cron("0 0 * * *", [&bot]() -> dpp::task<void> {
            co_await streak_warnings(bot);
        });

        scheduler.cron("0 0 * * *", [&bot]() {
            apply_streak_savers(bot);
        });
    }
}
