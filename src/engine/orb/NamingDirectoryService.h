/*
Copyright (C) 2007 <SWGEmu>. All rights reserved.
Distribution of this file for usage outside of Core3 is prohibited.
*/

#ifndef NAMINGDIRECTORYSERVICE_H_
#define NAMINGDIRECTORYSERVICE_H_

#include "DistributedObjectBrokerClient.h"

#include "object/DistributedObject.h"

#include "object/DistributedObjectClassHelperMap.h"

namespace engine {
  namespace ORB {

	class NamingDirectoryService {
		DistributedObjectBrokerClient* brokerClient;

		DistributedObjectClassHelperMap* classMap;
		
	public:
		NamingDirectoryService();
		NamingDirectoryService(const String& address);
		
		virtual ~NamingDirectoryService();
	
		virtual void deploy(DistributedObjectStub* stub);
		virtual void deploy(const String& name, DistributedObjectStub* stub);
	
		virtual DistributedObject* lookUp(const String& name);
		
		virtual DistributedObject* undeploy(const String& name);
		
		inline DistributedObjectBrokerClient* getClient() {
			return brokerClient;
		}

	};

  } // namespace ORB
} // namespace engine

using namespace engine::ORB;

#endif /*NAMINGDIRECTORYSERVICE_H_*/
