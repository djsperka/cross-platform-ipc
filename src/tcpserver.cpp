
//
// server.cpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2021 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//


#include <iostream>
#include <string>
#include "DelimitedCommandsTCPServer.h"

#if 0

#include <boost/bind/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

class tcp_connection
    : public boost::enable_shared_from_this<tcp_connection>
{
public:
    typedef boost::shared_ptr<tcp_connection> pointer;

    static pointer create(boost::asio::io_context& io_context)
    {
        return pointer(new tcp_connection(io_context));
    }

    tcp::socket& socket()
    {
        return socket_;
    }

    void start()
    {
        std::cout << "Got connection, wait for command...." << std::endl;

        // Rather than write here, we start an async_read -- that's for waiting on client to send a command
        boost::asio::async_read_until(socket_, buf_, ';',
            boost::bind(&tcp_connection::handle_read, shared_from_this(), 
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
    }

private:
    tcp_connection(boost::asio::io_context& io_context)
    : context_(io_context)
    , socket_(io_context)
    {
    }

    void handle_write(const boost::system::error_code& /*error*/,
        size_t /*bytes_transferred*/)
    {
        // Rather than write here, we start an async_read -- that's for waiting on client to send a command
        boost::asio::async_read_until(socket_, buf_, ';',
            boost::bind(&tcp_connection::handle_read, shared_from_this(),
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
    }

    void handle_read(const boost::system::error_code& /*error*/,
        size_t nbytes/*bytes_transferred*/)
    {
        std::cout << "Got command, length " << nbytes << std::endl;
        if (nbytes > 0)
        {
            std::ostringstream oss;
            oss << &buf_;
            std::string s = oss.str();

            std::cout << "oss method: " << s << std::endl;
			if (s.find("quit") == 0)
			{
				std::cout << "Quitting." << std::endl;
				socket_.close();
				context_.stop();
			}
			else
			{
				std::cout << "Executing command." << std::endl;
				message_ = "got command.";


				// send response, and wait for another command.
				boost::asio::async_write(socket_, boost::asio::buffer(message_),
					boost::bind(&tcp_connection::handle_write, shared_from_this(),
						boost::asio::placeholders::error,
						boost::asio::placeholders::bytes_transferred));
			}
        }
    }

    boost::asio::io_context& context_;
    tcp::socket socket_;
    std::string message_;
    boost::asio::streambuf buf_;
};

class tcp_server
{
public:
    tcp_server(boost::asio::io_context& io_context)
        : io_context_(io_context),
        acceptor_(io_context, tcp::endpoint(tcp::v4(), 7002))
    {
        start_accept();
    }

private:
    void start_accept()
    {
        tcp_connection::pointer new_connection =
            tcp_connection::create(io_context_);

        acceptor_.async_accept(new_connection->socket(),
            boost::bind(&tcp_server::handle_accept, this, new_connection,
                boost::asio::placeholders::error));
    }

    void handle_accept(tcp_connection::pointer new_connection,
        const boost::system::error_code& error)
    {
        if (!error)
        {
            new_connection->start();
        }

        start_accept();
    }

    boost::asio::io_context& io_context_;
    tcp::acceptor acceptor_;
};

#endif

int main()
{
	DelimitedCommandsTCPServer server("127.0.0.1", 7001, ';', "quit");

#if 0
    try
    {
        boost::asio::io_context io_context;
        tcp_server server(io_context);
        io_context.run();
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
#endif
    return 0;
}
