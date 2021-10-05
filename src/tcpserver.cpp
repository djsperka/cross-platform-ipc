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
	session(tcp::socket socket, std::function<bool(const std::string&)> f)
	: socket_(std::move(socket))
	, f_(f)
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
		socket_.async_read_some(boost::asio::buffer(data_, max_length),
			[this, self](boost::system::error_code ec, std::size_t length)
			{
				// handle command
				f_(std::string(data_, length));
				// send reponse
				if (!ec)
				{
					do_write(length);
				}
			});
	}

	void do_write(std::size_t length)
	{
		auto self(shared_from_this());
		boost::asio::async_write(socket_, boost::asio::buffer(data_, length),
			[this, self](boost::system::error_code ec, std::size_t /*length*/)
			{
				if (!ec)
				{
					do_read();
				}
			});
	}

	tcp::socket socket_;
	enum { max_length = 1024 };
	char data_[max_length];
	std::function<bool(const std::string&)> f_;
};

class server
{
public:
	server(boost::asio::io_context& io_context, short port, std::function<bool(const std::string&)> f)
	: acceptor_(io_context, tcp::endpoint(tcp::v4(), port))
	{
		do_accept(f);
	}

private:
	void do_accept(std::function<bool(const std::string&)> f)
	{
		acceptor_.async_accept(
			[this, f](boost::system::error_code ec, tcp::socket socket)
			{
			  if (!ec)
			  {
				  std::cout << "accepted connection" << std::endl;
				  std::make_shared<session>(std::move(socket), f)->start();
			  }

			  do_accept(f);
			});
	}

	tcp::acceptor acceptor_;
};


class TCPServerWrapper
{
	boost::asio::io_context io_context_;
	std::function<bool(const std::string&)> f_;
	int port_;
	std::thread t_;
public:
	TCPServerWrapper(std::function<bool(const std::string&)> f, int port)
	: f_(f)
	,  port_(port)
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
		server s(io_context_, port_, f_);
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



//boost::asio::io_context io_context;

std::unique_ptr<TCPServerWrapper> pWrapper;

bool callback(const std::string& s)
{
	std::cout << "callback: " << s << std::endl;
	if (s.find("quit") != std::string::npos)
	{
		std::cout << "quit" << std::endl;
		pWrapper->stop();
	}
	return true;
}

void run_stim()
{
	boost::asio::io_context io_context;
	pWrapper = std::unique_ptr<TCPServerWrapper>(new TCPServerWrapper(boost::bind(callback, _1), 7001));

	// blocking
	pWrapper->start();
}

//void threadFunc()
//{
//	server s(io_context, 7001, boost::bind(callback, _1));
//	io_context.run();
//}

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
