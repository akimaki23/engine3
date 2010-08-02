/*
Copyright (C) 2007 <SWGEmu>. All rights reserved.
Distribution of this file for usage outside of Core3 is prohibited.
*/

#ifndef DISTRIBUTEDOBJECTREQUESTCLIENT_H_
#define DISTRIBUTEDOBJECTREQUESTCLIENT_H_

#include "engine/log/Logger.h"

#include "engine/service/StreamConnector.h"

namespace engine {
  namespace ORB {

	class DistributedObjectBroker;
	
	class DistributedObjectBrokerClient : public StreamConnector, public Logger {
		DistributedObjectBroker* orb;
		
	public:
		DistributedObjectBrokerClient(DistributedObjectBroker* broker, SocketImplementation* sock);
		DistributedObjectBrokerClient(DistributedObjectBroker* broker, const String& host);
		
		void run();
		
		bool send(Packet* pack);
		
		inline DistributedObjectBroker* getBroker() {
			return orb;
		}

		uint64 getNetworkID() {
			return address.getNetworkID();
		}
	};

  } // namespace ORB
} // namespace engine

using namespace engine::ORB;

#endif /*DISTRIBUTEDOBJECTREQUESTCLIENT_H_*/
