#include <dpp/dpp.h>
#include <string>
#include <unordered_map>
#include <curl/curl.h>

namespace requests {
    dpp::task<std::string> get(const std::string url, const std::unordered_map<std::string, std::string>& headers = {});
}
