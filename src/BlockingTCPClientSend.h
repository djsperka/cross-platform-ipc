/*
 * BlockingTCPClientSend.h
 *
 *  Created on: Oct 11, 2021
 *      Author: dan
 */

#ifndef BLOCKINGTCPCLIENTSEND_H_
#define BLOCKINGTCPCLIENTSEND_H_

#include <string>
#include <fstream>
#include <iostream>
#include <boost/asio.hpp>
namespace boostip = boost::asio::ip;

std::string BlockingTCPClientSend(const std::string& addr, const std::string& port, const std::string& filename)
{
	std::string reply;
	std::cout << "BlockingTCPClientSend: " << addr << " " << port << " " << filename << std::endl;
	try
	{
		std::string contents;
		std::string response_from_server;
		std::string tmp;
		size_t n;
		char buf[512];
		std::ifstream is(filename.c_str(), std::ios::in | std::ios::binary);

		// read file into 'contents' string, append EOF.
		while (is.read(buf, sizeof(buf)).gcount() > 0)
			contents.append(buf, is.gcount());
		contents.push_back((char)26);

		// send
		boost::asio::io_context io_context;
		boostip::tcp::socket s(io_context);
		boostip::tcp::resolver resolver(io_context);
		boost::asio::connect(s, resolver.resolve(addr, port));
		n = boost::asio::read_until(s, boost::asio::dynamic_buffer(response_from_server), ';');
		tmp = response_from_server.substr(0, n);
		//response_from_server.erase(0, n);
		response_from_server.clear();
		std::cout << "connected to server, server message is: " << tmp << std::endl;

		boost::asio::write(s, boost::asio::buffer(contents.c_str(), contents.size()));

		std::cout << "read reply from server" << std::endl;

		n = boost::asio::read_until(s, boost::asio::dynamic_buffer(response_from_server), ';');
		reply = response_from_server.substr(0, n);
		response_from_server.clear();

		s.close();

		//size_t reply_length = boost::asio::read(s, boost::asio::buffer(buf, 512));
		//std::cout << "Reply is: ";
		//std::cout.write(buf, reply_length);
		//std::cout << "\n";
		//reply = std::string(buf, reply_length);
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << "\n";
	}

	return reply;
}




#endif /* BLOCKINGTCPCLIENTSEND_H_ */
