/*
 * TCPServerWrapper.h
 *
 *  Created on: Oct 6, 2021
 *      Author: dan
 */

#ifndef TCPSERVERWRAPPER_H_
#define TCPSERVERWRAPPER_H_



// djs - heavily modified version of example from boost::asio documentation.
//
//
// async_tcp_echo_server.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2019 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>
#include <thread>
#include <string>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>

using namespace boost::placeholders;
using boost::asio::ip::tcp;

namespace tcpsw {

	class session
	  : public std::enable_shared_from_this<session>
	{
	public:
		session(tcp::socket socket, std::function<bool(const std::string&, std::ostream&)> f, char delim)
		: socket_(std::move(socket))
		, f_(f)
		, delim_(delim)
		{
		}

		void start()
		{
			std::ostream output(&write_buffer_);
			output << "HELLO";
			do_write();
		}

	private:
		void do_read()
		{
			auto self(shared_from_this());

			//		socket_.async_read_some(boost::asio::buffer(data_, max_length),
			boost::asio::async_read_until(socket_, read_buffer_, delim_,
				[this, self](boost::system::error_code ec, std::size_t length)
				{
					// reading from the read buffer (via getline()) consumes characters from the buffer (i.e. removes char from it).
					// The delimiter is consumed by getline(), but it is not included in 'line'.
					std::istream input(&read_buffer_);
					std::string line;
					getline(input, line, delim_);

					// Ask callback to handle command.
					std::ostream output(&write_buffer_);
					f_(line, output);

					// In the event a series of commands were sent, peel them off one by one.
					// The responses will be bunched together, and written all at once.
					while (read_buffer_.size() > 0)
					{
						std::string remaining(
								boost::asio::buffers_begin(read_buffer_.data()),
								boost::asio::buffers_begin(read_buffer_.data()) + read_buffer_.size());
						if (remaining.find(delim_) == std::string::npos)
							break;
						getline(input, line, delim_);
						f_(line, output);
					}

					// send response(s)
					if (!ec)
					{
						do_write();
					}
				});
		}

		void do_write()
		{
			auto self(shared_from_this());
			if (write_buffer_.size() > 0)
			{
				std::ostream output(&write_buffer_);
				boost::asio::async_write(socket_, write_buffer_.data(),
					[this, self](boost::system::error_code ec, std::size_t length)
					{
						if (!ec)
						{
							write_buffer_.consume(length);
							do_read();
						}
					});
			}
			else
			{
				do_read();
			}
		}

		tcp::socket socket_;
		enum { max_length = 1024 };
		char data_[max_length];
		std::function<bool(const std::string&, std::ostream&)> f_;
		char delim_;
		boost::asio::streambuf read_buffer_;
		boost::asio::streambuf write_buffer_;

	};

	class server
	{
	public:
		server(boost::asio::io_context& io_context, short port, std::function<bool(const std::string&, std::ostream&)> f, char delim)
		: acceptor_(io_context, tcp::endpoint(tcp::v4(), port))
		{
			do_accept(f, delim);
		}

	private:
		void do_accept(std::function<bool(const std::string&, std::ostream&)> f, char delim)
		{
			acceptor_.async_accept(
				[this, f, delim](boost::system::error_code ec, tcp::socket socket)
				{
				  if (!ec)
				  {
					  std::cout << "accepted connection" << std::endl;
					  std::make_shared<session>(std::move(socket), f, delim)->start();
				  }

				  do_accept(f, delim);
				});
		}

		tcp::acceptor acceptor_;
	};
};

class TCPServerWrapper
{
	boost::asio::io_context io_context_;
	std::function<bool(const std::string&, std::ostream&)> f_;
	int port_;
	char delim_;
	std::thread t_;
public:
	TCPServerWrapper(std::function<bool(const std::string&, std::ostream&)> f, int port, char delim = ';')
	: f_(f)
	, port_(port)
	, delim_(delim)
	{}
	void start()
	{
		t_ = std::thread(&TCPServerWrapper::threadFunc, this);
		t_.join();
	}
	virtual ~TCPServerWrapper()
	{
		io_context_.stop();
	}
	void threadFunc()
	{
		tcpsw::server s(io_context_, port_, f_, delim_);

		// this call blocks until the context is stopped.
		io_context_.run();
	}
	void stop()
	{
		io_context_.stop();
	}
};





#endif /* TCPSERVERWRAPPER_H_ */
