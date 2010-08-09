/*
Copyright (C) 2007 <SWGEmu>. All rights reserved.
Distribution of this file for usage outside of Core3 is prohibited.
*/

#ifndef STREAMPROXYSERVICE_H_
#define STREAMPROXYSERVICE_H_

#include "engine/engine.h"

class StreamProxyService : public StreamServiceThread
{
	class StreamProxyServiceClient* proxyServiceClient;

	String forwardAddress;
	int forwardPort;

	String fullAddress;

public:
	StreamProxyService();
	
	void init();

	void run();

	void stop();

	// client methods
	ServiceClient* createConnection(Socket* sock, SocketAddress& addr);

	bool handleError(Exception& e);

	void setForwarding(const String& address, int port);

	// setters and getters
	void setName(const String& name, int port);

	String& getAddress();

	const String& getForwardingAddress() {
		return forwardAddress;
	}
	
	int getForwardingPort() {
		return forwardPort;
	}
	
};

#endif /*STREAMPROXYSERVICE_H_*/
