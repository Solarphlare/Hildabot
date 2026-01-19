#include "util/requests.h"
#include <curl/curl.h>
#include <memory>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <string>
#include <format>
#include <stdexcept>

struct write_ctx {
    std::string buffer;
    std::exception_ptr eptr;
};

static size_t write_cb(char* ptr, size_t size, size_t nmemb, void* userdata) noexcept {
    const size_t total = size * nmemb;
    auto* ctx = static_cast<write_ctx*>(userdata);

    try {
        ctx->buffer.append(ptr, total);
        return total;
    } catch (...) {
        if (!ctx->eptr) ctx->eptr = std::current_exception();
        return 0;
    }
}

std::string get_sync(const std::string& url, const std::unordered_map<std::string, std::string>& headers) {
    static std::once_flag curl_initialized;
    std::call_once(curl_initialized, []{
        auto rc = curl_global_init(CURL_GLOBAL_DEFAULT);
        if (rc != CURLE_OK) {
            throw std::runtime_error("curl_global_init() failed");
        }
    });

    std::unique_ptr<CURL, decltype(&curl_easy_cleanup)> curl(curl_easy_init(), &curl_easy_cleanup);
    if (!curl) throw std::runtime_error("curl_easy_init() failed");

    write_ctx ctx;

    char errbuf[CURL_ERROR_SIZE];
    errbuf[0] = '\0';
    curl_easy_setopt(curl.get(), CURLOPT_ERRORBUFFER, errbuf);

    curl_easy_setopt(curl.get(), CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl.get(), CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(curl.get(), CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl.get(), CURLOPT_ACCEPT_ENCODING, "");
    curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, &write_cb);
    curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &ctx);

    curl_slist* headers_list_ptr = nullptr;
    for (const auto& [key, value] : headers) {
        std::string header = key + ": " + value;
        headers_list_ptr = curl_slist_append(headers_list_ptr, header.c_str());
    }
    std::unique_ptr<curl_slist, decltype(&curl_slist_free_all)> headers_ptr(headers_list_ptr, &curl_slist_free_all);
    curl_easy_setopt(curl.get(), CURLOPT_HTTPHEADER, headers_ptr.get());

    CURLcode res = curl_easy_perform(curl.get());

    if (ctx.eptr) std::rethrow_exception(ctx.eptr);

    if (res != CURLE_OK) {
        std::string extra = errbuf[0] ? std::format(" ({})", errbuf) : "";
        throw std::runtime_error(
            "curl_easy_perform() failed: " + std::string(curl_easy_strerror(res)) + extra
        );
    }

    long status_code = 0;
    curl_easy_getinfo(curl.get(), CURLINFO_RESPONSE_CODE, &status_code);

    if (status_code != 200) {
        throw std::runtime_error("HTTP request failed with status code: " + std::to_string(status_code));
    }

    return std::move(ctx.buffer);
}

namespace requests {
    dpp::task<std::string> get(const std::string url, const std::unordered_map<std::string, std::string>& headers) {
        auto promise = std::make_shared<dpp::basic_promise<std::string>>();
        dpp::awaitable<std::string> awaitable = promise->get_awaitable();

        std::thread([promise, url = std::move(url), headers = std::move(headers)]() mutable {
            try {
                promise->set_value(get_sync(url, headers));
            } catch (...) {
                promise->set_exception(std::current_exception());
            }
        }).detach();

        co_return co_await awaitable;
    }
}
