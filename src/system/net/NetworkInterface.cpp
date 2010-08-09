/*
Copyright (C) 2007 <SWGEmu>. All rights reserved.
Distribution of this file for usage outside of Core3 is prohibited.
*/

#include "../lang/Integer.h"
#include "../lang/StringBuffer.h"

#include "NetworkInterface.h"

NetworkInterface* NetworkInterface::sinterface = NULL;

NetworkInterface::NetworkInterface() {
	interfaces = new Vector<InetAddress*>();

	initialize();
}

NetworkInterface::~NetworkInterface() {
    close();

    delete interfaces;
}

void NetworkInterface::finalize() {
	if (sinterface != NULL) {
		delete sinterface;
	}
}

void NetworkInterface::initialize() {
	char hostname[128];
	if (gethostname(hostname, 128) != 0)
		return;

	struct hostent* he = gethostbyname(hostname);

	for (int i = 0; he != NULL && he->h_addr_list[i] != NULL; i++) {
		StringBuffer address;

		for (int j = 0; j < he->h_length; j++) {
			unsigned int iptag = ((unsigned int) ((unsigned char*) he->h_addr_list[i])[j]);

			if (j > 0)
				address << '.';

			address << iptag;
		}

		InetAddress* inetaddr = new InetAddress(address.toString());
		inetaddr->setHostName(hostname);

		interfaces->add(inetaddr);
	}

	sortInterfaces();
}

void NetworkInterface::sortInterfaces() {
	for (int i = 0; i < interfaces->size(); ++i) {
		InetAddress* inetaddr = interfaces->get(i);
		String address = inetaddr->getHostAddress();

		int tag = Integer::valueOf(address.subString(0, 3));

		if (tag == 192) {
			interfaces->remove(i);

			interfaces->add(inetaddr);
		}
	}

	for (int i = 0; i < interfaces->size(); ++i) {
		InetAddress* inetaddr = interfaces->get(i);
		String address = inetaddr->getHostAddress();

		int tag = Integer::valueOf(address.subString(0, 3));

		if (tag == 127) {
			interfaces->remove(i);

			interfaces->add(inetaddr);
		}
	}

	/*for (int i = 0; i < interfaces->size(); ++i) {
		InetAddress* inetaddr = interfaces->get(i);
		String address = inetaddr->getHostAddress();

		System::out << address << "\n";
	}*/
}

void NetworkInterface::close() {
	for (int i = 0; i < interfaces->size(); ++i) {
		InetAddress* inetaddr = interfaces->get(i);

		delete inetaddr;
	}

	interfaces->removeAll();
}

void NetworkInterface::setPrimaryAddress(InetAddress* address) {
	if (sinterface == NULL)
		sinterface = new NetworkInterface();

	Vector<InetAddress*>* interfaces = sinterface->getInterfaces();
	interfaces->add(0, address);
}

InetAddress* NetworkInterface::getPrimaryAddress() {
	if (sinterface == NULL)
		sinterface = new NetworkInterface();

	Vector<InetAddress*>* interfaces = sinterface->getInterfaces();

	if (!interfaces->isEmpty())
		return interfaces->get(0);
	else
		return NULL;
}
