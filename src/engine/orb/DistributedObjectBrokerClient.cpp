/*
Copyright (C) 2007 <SWGEmu>. All rights reserved.
Distribution of this file for usage outside of Core3 is prohibited.
*/

#include "DistributedObjectBroker.h"

#include "DistributedObjectBrokerClient.h"

DistributedObjectBrokerClient::DistributedObjectBrokerClient(DistributedObjectBroker* broker, SocketImplementation* sock) 
		: StreamConnector(/*sock*/), Logger("DistributedObjectBroker") {
	orb = broker;

	/*info("client \'" + host + "\' connected", true);*/
	info("client connected", true);
}

DistributedObjectBrokerClient::DistributedObjectBrokerClient(DistributedObjectBroker* broker, const String& host) 
		: StreamConnector(/*host, 44433*/) , Logger("DistributedObjectBroker") {
	orb = broker;

	try {
		//connect();
		info("connected to server \'" + host + "\'", true);
	} catch (SocketException& e) {
		error("unable to connect to \'" + host + "\'");
		error(e.getMessage());
	}
}

void DistributedObjectBrokerClient::run() {
	DOBPacketHandler* phandler = orb->getPacketHandler();

	Packet packet;
	
	while (read(&packet)) {
		phandler->handlePacket(this, &packet);
	}
}

bool DistributedObjectBrokerClient::send(Packet* pack) {
	//info("SEND " + pack->toString(), true);
	StreamConnector::send(pack);
	
	delete pack;

	return true;
}
