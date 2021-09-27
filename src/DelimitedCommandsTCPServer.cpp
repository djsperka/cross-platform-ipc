/*
 * DelimitedCommandsTCPServer.cpp
 *
 *  Created on: Sep 23, 2021
 *      Author: dan
 */

#include "DelimitedCommandsTCPServer.h"

#include <iostream>
#include <string>
#include <boost/bind/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

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


void threadFunc(boost::asio::io_context* context)
{
	context->run();
}

DelimitedCommandsTCPServer::DelimitedCommandsTCPServer(const std::string& addr, int port, char delimiter, const std::string& quitPrefix)
: m_addr(addr)
, m_port(port)
, m_delim(delimiter)
, m_quitPrefix(quitPrefix)
{
	tcp_server server(m_io_context);
	m_thread = std::thread(threadFunc, &m_io_context);
}

DelimitedCommandsTCPServer::~DelimitedCommandsTCPServer() {
	// TODO Auto-generated destructor stub
}

void DelimitedCommandsTCPServer::stop()
{
	m_io_context.stop();
	m_thread.join();
}
