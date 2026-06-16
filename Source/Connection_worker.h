
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
    ///@param socket connection socket
    ///@param data_cache ptr to shared data cache
    Connection_worker(tcp::socket&& socket, std::shared_ptr<Data_cache> data_cache);

    ///@brief start processing connection
    auto start() -> void;

private:
    ///@brief read off socket
    auto read_request() -> void;

    ///@brief determine if it is get or post request and handle it
    auto process_request() -> void;

    ///@brief trim passed in target to be only the endpoint and return parsed query params
    ///@param target trimmed to be only the endpoint
    ///@return list of query params
    static auto split_parameters(std::string &target) -> std::vector<std::pair<std::string, std::string>>;

    ///@brief pull file to transmit
    auto pull_download(const std::vector<std::pair<std::string, std::string>> &query_params) -> void;

    ///@brief pull file info from data cache according to query params
    auto pull_info(const std::vector<std::pair<std::string, std::string>> &query_params) -> void;

    ///@brief pull list from data cache according to query parameters
    auto pull_list(const std::vector<std::pair<std::string, std::string>> &query_params) -> void;

    ///@brief process get request
    auto handle_get_request() -> void;

    ///@brief process post request
    auto handle_post_request() -> void;

    ///@brief publish response
    auto write_response() -> void;

    ///@brief check for timeout on connection
    auto check_deadline() -> void;

    ///@brief current socket connection
    tcp::socket socket_;

    ///@brief ptr to shared data cache for storing records
    std::shared_ptr<Data_cache> data_cache_{};

    ///@brief buffer for performing reads
    boost::beast::flat_buffer buffer_{8192};

    ///@brief request message
    http::request<http::dynamic_body> request_;

    ///@brief response message
    http::response<http::dynamic_body> response_;

     ///@brief timer for putting a deadline on connection processing
    std::unique_ptr<boost::asio::basic_waitable_timer<std::chrono::steady_clock>> deadline_{};
};
#endif //DEEPGRAM_PROJECT_CONNECTION_H
