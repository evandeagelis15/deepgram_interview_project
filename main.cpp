#include <iostream>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio.hpp>
#include <memory>

#include "Data_cache.h"
#include "Source/Connection_worker.h"

namespace ip = boost::asio::ip; // from <boost/asio.hpp>
using tcp = ip::tcp; // from <boost/asio.hpp>
namespace http = boost::beast::http; // from <boost/beast/http.hpp>

auto http_server(tcp::acceptor &acceptor, tcp::socket& socket, std::shared_ptr<Data_cache> data_cache) -> void
{
    acceptor.async_accept(socket,
                          [&acceptor,
                              &socket,
                              shared_cache = data_cache ](const boost::beast::error_code &ec) mutable  {
                              if (not ec)
                              {
                                  const auto new_connection = std::make_shared<Connection_worker>(
                                      std::move(socket), shared_cache);
                                  new_connection->start();
                              }
                              http_server(acceptor, socket, shared_cache);
                          });
}

int main()
{
    try
    {
        auto data_cache = std::make_shared<Data_cache>();
        auto const address = ip::make_address("127.0.0.1");
        auto port = static_cast<unsigned short>(8080);

        auto ioc = boost::asio::io_context{1};

        auto acceptor = tcp::acceptor(ioc, {address, port});
        auto socket = tcp::socket(ioc);
        http_server(acceptor,socket, data_cache);

        ioc.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}
