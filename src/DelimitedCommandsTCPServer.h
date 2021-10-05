/*
 * DelimitedCommandsTCPServer.h
 *
 *  Created on: Sep 23, 2021
 *      Author: dan
 */

#ifndef DELIMITEDCOMMANDSTCPSERVER_H_
#define DELIMITEDCOMMANDSTCPSERVER_H_

#include <string>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <thread>


class DelimitedCommandsTCPServer {
	std::string m_addr;
	int m_port;
	char m_delim;
	std::string m_quitPrefix;
	boost::asio::io_context m_io_context;
	std::thread m_thread;


	typedef boost::function<bool(const std::string &)> CallbackFunc;
	CallbackFunc m_callback;

public:
	DelimitedCommandsTCPServer(int port, char delimiter=';', CallbackFunc f);
	virtual ~DelimitedCommandsTCPServer();
	void stop();
};

#endif /* DELIMITEDCOMMANDSTCPSERVER_H_ */
