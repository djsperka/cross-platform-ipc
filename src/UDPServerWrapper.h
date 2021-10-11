/*
 * UDPServerWrapper.h
 *
 *  Created on: Oct 7, 2021
 *      Author: dan
 */

#ifndef UDPSERVERWRAPPER_H_
#define UDPSERVERWRAPPER_H_

#include <cstdlib>
#include <iostream>
#include <thread>
#include <boost/asio.hpp>
#include <boost/function.hpp>

using boost::asio::ip::udp;

namespace udpsw
{

	class server
	{
	public:
		server(boost::asio::io_context& io_context, short port)
		: socket_(io_context, udp::endpoint(udp::v4(), port))
		{
			do_receive();
		}

		void do_receive()
		{
			socket_.async_receive(
					boost::asio::buffer(data_, max_length),
					[this](boost::system::error_code ec, std::size_t bytes_recvd)
					{
						if (!ec && bytes_recvd > 0)
						{
							std::cout << "async_received " << bytes_recvd << std::endl;

							// call the callback - it is responsible for acting on this message.
							// No facility for replying to these messages. For 2-way communication,
							// use AsyncTCPServerWrapper
							//f_(data_, bytes_recvd);
							do_receive();
						}
					});
		}

	private:
		udp::socket socket_;
		//std::function<void(const char *, std::size_t)> f_;
		enum { max_length = 16384 };
		char data_[max_length];
	};
}

class UDPServerWrapper {
public:
	UDPServerWrapper(short port)
	: port_(port)
	{
	}
	virtual ~UDPServerWrapper()
	{
		io_context_.stop();
	}

	void start()
	{
		t_ = std::thread(&UDPServerWrapper::threadFunc, this);
		t_.join();
	}
	void threadFunc()
	{
		udpsw::server s(io_context_, port_);

		// this call blocks until the context is stopped.
		io_context_.run();
	}
	void stop()
	{
		io_context_.stop();
	}
private:
	void callback(const char *data, std::size_t bytes);
	short port_;
	std::vector<std::string> list_;
	std::thread t_;
	boost::asio::io_context io_context_;
};

#endif /* UDPSERVERWRAPPER_H_ */

#if 0




void start()
{
	t_ = std::thread(&AsyncTCPServerWrapper::threadFunc, this);
	t_.join();
}
virtual ~AsyncTCPServerWrapper()
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



//
// async_udp_echo_server.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2019 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//


#endif
