
#ifndef DEEPGRAM_PROJECT_CONNECTION_H
#define DEEPGRAM_PROJECT_CONNECTION_H

#include <memory>
#include <boost/beast/http.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/http/dynamic_body_fwd.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/message_fwd.hpp>
#include "Data_cache.h"

using tcp = boost::asio::ip::tcp; // from <boost/asio.hpp>
namespace http = boost::beast::http; // from <boost/beast/http.hpp>

///@brief process single request and publish response
class Connection_worker : public std::enable_shared_from_this<Connection_worker> {
public:

    ///@brief Constructor
    ///@param socket
    ///@param data_cache
    Connection_worker(tcp::socket&& socket, std::shared_ptr<Data_cache> data_cache);

    auto start() -> void;

private:
    auto read_request() -> void;

    auto process_request() -> void;

    static auto split_parameters(std::string &target) -> std::vector<std::pair<std::string, std::string>>;

    auto pull_download(const std::vector<std::pair<std::string, std::string>> &query_params) -> void;

    auto pull_info(const std::vector<std::pair<std::string, std::string>> &query_params) -> void;

    auto pull_list(const std::vector<std::pair<std::string, std::string>> &query_params) -> void;

    auto handle_get_request() -> void;

    auto handle_post_request() -> void;

    auto write_response() -> void;

    auto check_deadline() -> void;

    tcp::socket socket_;

    std::shared_ptr<Data_cache> data_cache_{};

    // The buffer for performing reads.
    boost::beast::flat_buffer buffer_{8192};

    // The request message.
    http::request<http::dynamic_body> request_;

    // The response message.
    http::response<http::dynamic_body> response_;

    // The timer for putting a deadline on connection processing.
    std::unique_ptr<boost::asio::basic_waitable_timer<std::chrono::steady_clock>> deadline_{};
};
#endif //DEEPGRAM_PROJECT_CONNECTION_H
