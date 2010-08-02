/*
Copyright (C) 2007 <SWGEmu>. All rights reserved.
Distribution of this file for usage outside of Core3 is prohibited.
*/

#ifndef DATAGRAMSERVICETHREAD_H_
#define DATAGRAMSERVICETHREAD_H_

#include "ServiceMessageHandlerThread.h"

namespace engine {
  namespace service {
  	
	class DatagramAcceptor : public AbstractServiceAcceptor {
	protected:
		DatagramServerSocket* socket;
		int port;

	public:
		DatagramAcceptor();
		DatagramAcceptor(const String& s);
		
		virtual ~DatagramAcceptor();
		
		void start(int p, int mconn = 10);
	
		virtual void stop();

		virtual bool removeConnection(ServiceSession* client);

		void removeConnections();
		
		// message methods
		void receiveMessages();

		virtual void handleMessage(ServiceSession* client, Packet* message) = 0;

		virtual bool handleError(ServiceSession* client, Exception& e);
	};

  } // namespace service
} // namespace engine

using namespace engine::service;

#endif /*DATAGRAMSERVICETHREAD_H_*/
