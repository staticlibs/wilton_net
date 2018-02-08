/*
 * Copyright 2015, akashche at redhat.com
 * Copyright 2017, alex at staticlibs.net
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* 
 * File:   TCPConnectTask.cpp
 * Author: alex
 * 
 * Created on November 12, 2015, 1:19 PM
 */

#include "tcp_connect_checker.hpp"

#include <mutex>
#include <atomic>
#include <thread>

#include "asio.hpp"

#include "staticlib/config.hpp"
#include "staticlib/support.hpp"
#include "staticlib/pimpl/forward_macros.hpp"

namespace wilton {
namespace net {

namespace { // anonymous

const std::chrono::milliseconds attempt_timeout = std::chrono::milliseconds(100);

} // namespace

class tcp_connect_checker::impl : public staticlib::pimpl::object::impl {
    
public:

    impl() { }
    
    static std::string wait_for_connection(std::chrono::milliseconds timeout,
            const std::string& ip_addr, uint16_t tcp_port) {
        conn_checker checker{timeout, ip_addr, tcp_port};
        uint64_t start = current_time_millis();
        uint64_t tc = static_cast<uint64_t>(timeout.count());
        std::string err = "ERROR: Invalid timeout: [" + sl::support::to_string(tc) + "] (-1)";
        while (current_time_millis() - start < tc) {
            err = checker.check();
            if (err.empty()) break;
            std::this_thread::sleep_for(attempt_timeout);
        }
        return err;
    }
    
private:
    // http://stackoverflow.com/a/2834294/314015
    static uint64_t current_time_millis() {
        auto time = std::chrono::system_clock::now(); // get the current time
        auto since_epoch = time.time_since_epoch(); // get the duration since epoch
        auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(since_epoch);
        return static_cast<uint64_t>(millis.count()); // just like java (new Date()).getTime();
    }

    class conn_checker {
        std::chrono::milliseconds timeout;
        asio::io_service service{};
        asio::ip::tcp::endpoint endpoint;
        
    public:
        conn_checker(std::chrono::milliseconds timeout, const std::string& ip, uint16_t port) :
        timeout(timeout),
        endpoint(asio::ip::address_v4::from_string(ip), port) { }
        
        std::string check() {
            service.reset();
            asio::ip::tcp::socket socket{service, asio::ip::tcp::v4()};
            asio::steady_timer timer{service};
            std::mutex mutex{};
            std::atomic_bool connect_cancelled{false};
            std::atomic_bool timer_cancelled{false};
            std::string error_message = "";
            timer.expires_from_now(timeout);
            socket.async_connect(endpoint, [&](const std::error_code& ec) {
                std::lock_guard<std::mutex> guard{mutex};
                if (connect_cancelled.load(std::memory_order_acquire)) return;
                timer_cancelled.store(true, std::memory_order_release);
                timer.cancel();
                if(ec) {
                    error_message = "ERROR: " + ec.message() + " (" + sl::support::to_string(ec.value()) + ")";
                }
            });
            timer.async_wait([&](const std::error_code&) {
                std::lock_guard<std::mutex> guard{mutex};
                if (timer_cancelled.load(std::memory_order_acquire)) return;
                connect_cancelled.store(true, std::memory_order_release);
                socket.close();
                error_message = "ERROR: Connection timed out (-1)";
            });
            service.run();
            return error_message;
        }               
    };
    
};
PIMPL_FORWARD_METHOD_STATIC(tcp_connect_checker, std::string, wait_for_connection, 
        (std::chrono::milliseconds)(const std::string&)(uint16_t), (), support::exception)

} // namespace
}




