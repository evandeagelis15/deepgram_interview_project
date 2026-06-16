
#include "Connection_worker.h"
#include "Data_cache.h"

#include <iostream>
#include <boost/beast/core/ostream.hpp>
#include <boost/beast/core/buffers_to_string.hpp>

Connection_worker::Connection_worker(tcp::socket &&socket,
                                     std::shared_ptr<Data_cache> data_cache) : socket_(std::move(socket)),
                                                                               data_cache_(data_cache)
{
    deadline_ = std::make_unique<boost::asio::basic_waitable_timer<
        std::chrono::steady_clock> >(socket_.get_executor());
}

auto Connection_worker::start() -> void
{
    read_request();
    check_deadline();
}

auto Connection_worker::read_request() -> void
{
    auto self = shared_from_this();

    http::async_read(
        socket_,
        buffer_,
        request_,
        [self](const boost::beast::error_code &ec,
               std::size_t bytes_transferred) {
            boost::ignore_unused(bytes_transferred);
            if (not ec)
                self->process_request();
        });
}

auto Connection_worker::process_request() -> void
{
    response_.version(request_.version());
    response_.keep_alive(false);

    switch (request_.method())
    {
        case http::verb::get:
        {
            handle_get_request();
            break;
        }
        case http::verb::post:
        {
            handle_post_request();
            break;
        }
        default:
        {
            response_.result(http::status::bad_request);
            response_.set(http::field::content_type, "text/plain");
            boost::beast::ostream(response_.body())
                    << "Invalid request-method '"
                    << request_.method_string()
                    << "'";
            break;
        }
    }

    write_response();
}

auto Connection_worker::split_parameters(std::string &target) -> std::vector<std::pair<std::string, std::string> >
{
    auto results = std::vector<std::string>{};
    auto start = std::size_t{0};
    auto end = target.find('?');

    while (end != std::string::npos)
    {
        results.push_back(target.substr(start, end - start));
        start = end + 1;
        end = target.find('?', start);
    }
    // Add the remaining part of the string
    results.push_back(target.substr(start));

    // pull target out
    target = results.front();
    target.erase(0, 1);
    //remove it from the rest of the params
    results.erase(results.begin());

    auto out_params = std::vector<std::pair<std::string, std::string> >{};
    // pull parameters into a list of paris
    for (const auto &str: results)
    {
        const auto delimiter_pos = str.find('=');
        auto left = str.substr(0, delimiter_pos);
        // Extract the right half (from right after the delimiter to the end)
        auto right = str.substr(delimiter_pos + 1);
        out_params.emplace_back(left, right);
    }
    return out_params;
}

auto Connection_worker::pull_download(const std::vector<std::pair<std::string, std::string> > &query_params) -> void
{
    // TODO: Update to not assume only name is passed in
    auto wav_data = data_cache_->get_wav_data(query_params[0].second);

    if (wav_data.empty())
    {
        response_.result(http::status::bad_request);
        response_.set(http::field::content_type, "text/plain");
        boost::beast::ostream(response_.body()) << "No file found.";
    }
    auto mutable_buffers = response_.body().prepare(wav_data.size());

    auto bytes_copied = boost::asio::buffer_copy(
        mutable_buffers,
        boost::asio::buffer(wav_data.data(), wav_data.size())
    );

    response_.body().commit(bytes_copied);
    response_.result(http::status::ok);
    response_.set(http::field::content_type, "audio/wav");
    response_.prepare_payload();
}

auto Connection_worker::pull_info(const std::vector<std::pair<std::string, std::string> > &query_params) -> void
{
    // TODO: Update to not assume only name is passed in
    auto file_info = data_cache_->get_info(query_params[0].second);
    if (file_info.empty())
    {
        response_.result(http::status::bad_request);
        response_.set(http::field::content_type, "text/plain");
        boost::beast::ostream(response_.body()) << "No file found.";
    }
    response_.result(http::status::ok);
    response_.set(http::field::content_type, "application/json");
    boost::beast::ostream(response_.body()) << file_info;
}

auto Connection_worker::pull_list(const std::vector<std::pair<std::string, std::string> > &query_params) -> void
{
    auto file_list = data_cache_->get_list(query_params);
    if (file_list.empty())
    {
        response_.result(http::status::not_found);
        boost::beast::ostream(response_.body()) << "No files found for query.";
        return;
    }
    response_.result(http::status::ok);
    response_.set(http::field::content_type, "application/json");
    std::ranges::for_each(file_list, [&](const std::string &in_name) {
        boost::beast::ostream(response_.body()) << in_name << " ";
    });
}

auto Connection_worker::handle_get_request() -> void
{
    response_.result(http::status::ok);
    response_.set(http::field::server, "Deepgram Server");
    std::cout << "Received GET request to target: " << request_.target() << std::endl;
    auto parsed_target = std::string{request_.target()};
    auto out_params = split_parameters(parsed_target);

    if (parsed_target == "download")
    {
        pull_download(out_params);
    }
    else if (parsed_target == "info")
    {
        pull_info(out_params);
    }
    else if (parsed_target == "list")
    {
        pull_list(out_params);
    }
    else
    {
        response_.result(http::status::not_found);
        boost::beast::ostream(response_.body()) << "Unsupported service endpoint.";
    }
}

auto Connection_worker::handle_post_request() -> void
{
    const auto content_type = request_[http::field::content_type];
    const auto file_name = request_["X-File-Name"];
    const auto body_payload = request_.body();

    // Check correct target
    if (request_.target() != "/files")
    {
        response_.result(http::status::not_found);
        response_.set(http::field::content_type, "text/plain");
        boost::beast::ostream(response_.body()) << "Wrong post path";
        return;
    }

    // Check that content is only in this form
    if (content_type != "audio/wav")
    {
        response_.result(http::status::bad_request);
        response_.set(http::field::content_type, "text/plain");
        boost::beast::ostream(response_.body()) << "Only audio/wav content accepted";
        return;
    }

    std::cout << "Received POST request to target: " << request_.target() << std::endl;
    std::cout << "Content-Type: " << content_type << std::endl;
    std::cout << "File: "<< file_name << std::endl;

    // Create byte array
    auto byte_array = std::vector<uint8_t>(body_payload.size());
    boost::asio::buffer_copy(
        boost::asio::buffer(byte_array.data(), byte_array.size()), body_payload.data());

    // Store data to cache
    data_cache_->store_file(file_name, byte_array);

    // Build result
    response_.result(http::status::ok);
    response_.set(http::field::content_type, "text/plain");
    boost::beast::ostream(response_.body()) << R"({"status":"success"})";
}

auto Connection_worker::write_response() -> void
{
    auto self = shared_from_this();

    response_.content_length(response_.body().size());

    http::async_write(
        socket_,
        response_,
        [self](boost::beast::error_code ec, std::size_t) {
            self->socket_.shutdown(tcp::socket::shutdown_send, ec);
            self->deadline_->cancel();
        });
}

auto Connection_worker::check_deadline() -> void
{
    auto self = shared_from_this();

    deadline_->expires_after(std::chrono::seconds(30));
    deadline_->async_wait(
        [self](boost::beast::error_code ec) {
            if (not ec)
            {
                // Close socket to cancel any outstanding operation.
                self->socket_.close(ec);
            }
        });
}
