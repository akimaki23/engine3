/*
Copyright (C) 2007 <SWGEmu>. All rights reserved.
Distribution of this file for usage outside of Core3 is prohibited.
*/

#include "DatagramServiceClientProxy.h"

DatagramServiceClientProxy::DatagramServiceClientProxy(Socket* sock, SocketAddress& addr) 
		: ServiceClient(sock, addr) {
}

DatagramServiceClientProxy::~DatagramServiceClientProxy() {
	socket = NULL;
}

void DatagramServiceClientProxy::run() {
}

bool DatagramServiceClientProxy::send(Packet* pack) {
	if (packetLossChance != 0 && System::random(100) < (uint32) packetLossChance)
		return false;
	
	socket->sendTo(pack, &addr);
	
	return true;
}

bool DatagramServiceClientProxy::read(Packet* pack) {
	if (packetLossChance != 0 && System::random(100) < (uint32) packetLossChance)
		return false;

	SocketAddress addr;
	socket->recieveFrom(pack, &addr);

	return true;
}
