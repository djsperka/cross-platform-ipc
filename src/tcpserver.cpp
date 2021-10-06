
#if 0
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
#include <boost/bind.hpp>

using boost::asio::ip::tcp;

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
		do_read();
	}

private:
	void do_read()
	{
		auto self(shared_from_this());

		//		socket_.async_read_some(boost::asio::buffer(data_, max_length),
		boost::asio::async_read_until(socket_, read_buffer_, delim_,
			[this, self](boost::system::error_code ec, std::size_t length)
			{
				std::cout << "read_until lambda(): length is " << length << std::endl;

				std::istream input(&read_buffer_);
			    std::string line;
			    getline(input, line, delim_); // Consumes from the streambuf.

			    std::cout << "getline() read: " << line << " and now read_buffer_ size is " << read_buffer_.size() << std::endl;

			    // handle command
			    std::ostream output(&write_buffer_);
				f_(line, output);

				std::cout << "after callback, write_buffer size is " << write_buffer_.size() << std::endl;

				// send reponse
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
						std::cout << "async_write lambda: length is " << length << std::endl;
						std::cout << "write_buffer_ size " << write_buffer_.size();
						write_buffer_.consume(length);
						std::cout << "after consume, write_buffer_ size is " << write_buffer_.size() << std::endl;
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
		server s(io_context_, port_, f_, delim_);
		std::cout << "threadFunc: run io_context" << std::endl;
		io_context_.run();
		std::cout << "threadFunc done." << std::endl;
	}
	void stop()
	{
		std::cout << "stop(): stop io_context" << std::endl;
		io_context_.stop();
		std::cout << "stop(): done." << std::endl;
	}

};
#endif

#include "TCPServerWrapper.h"
#include <iostream>
#include <memory>
std::unique_ptr<TCPServerWrapper> pWrapper;

bool callback(const std::string& s, std::ostream& out)
{
	std::cout << "callback: " << s << std::endl;
	if (s.find("quit") != std::string::npos)
	{
		std::cout << "quit" << std::endl;
		pWrapper->stop();
	}
	out << "OK";
	return true;
}

void run_stim()
{
	pWrapper = std::unique_ptr<TCPServerWrapper>(new TCPServerWrapper(boost::bind(callback, _1, _2), 7001, ';'));

	// blocking
	pWrapper->start();
}

int main(int argc, char* argv[])
{

	std::cout << "main(): call run_stim" << std::endl;
	run_stim();
	std::cout << "main(): run_stim done" << std::endl;

//	std::thread t(threadFunc);
//	t.join();
//	std::cout << "joined" << std::endl;
//	try
//	{
////    if (argc != 2)
////    {
////      std::cerr << "Usage: async_tcp_echo_server <port>\n";
////      return 1;
////    }
//
//		boost::asio::io_context io_context;
//
//		server s(io_context, 7001, boost::bind(callback, _1));
//
//		io_context.run();
//	}
//	catch (std::exception& e)
//	{
//	std::cerr << "Exception: " << e.what() << "\n";
//	}

	return 0;
}
