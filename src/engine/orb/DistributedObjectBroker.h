/*
Copyright (C) 2007 <SWGEmu>. All rights reserved.
Distribution of this file for usage outside of Core3 is prohibited.
*/

#ifndef DISTRIBUTEDOBJECTBROKER_H_
#define DISTRIBUTEDOBJECTBROKER_H_

#include "system/lang.h"

#include "../util/Singleton.h"

#include "../service/StreamServiceThread.h"

#include "DistributedObjectDirectory.h"

#include "NamingDirectoryService.h"

#include "DistributedObjectBrokerClient.h"

#include "DOBPacketHandler.h"

#include "object/DistributedObject.h"
#include "object/DistributedObjectStub.h"
#include "object/DistributedObjectServant.h"
#include "object/DistributedObjectAdapter.h"

#include "object/DistributedObjectClassHelper.h"
#include "object/DistributedObjectClassHelperMap.h"

#include "object/DistributedMethod.h"

#include "packets/DeployObjectMessage.h"
#include "packets/MethodReturnMessage.h"

#include "object/ObjectNotDeployedException.h"
#include "object/ObjectNotLocalException.h"

namespace engine {
  namespace ORB {

	class DOBObjectManager;

	class DistributedObjectBrokerClient;

	class DistributedObjectBroker : public StreamServiceThread, public Singleton<DistributedObjectBroker> {
		static DistributedObjectBroker* impl;

		String address;
		NamingDirectoryService* namingDirectoryInterface;

		DistributedObjectClassHelperMap classMap;

		DOBPacketHandler* phandler;
		DOBObjectManager* objectManager;

		DistributedObjectBrokerClient* orbClient;

	private:
		DistributedObjectBroker();

		virtual ~DistributedObjectBroker();

	public:
		static DistributedObjectBroker* initialize(const String& addr, int port = 44433);

		void initialize();

		void run();

		void shutdown();

		DistributedObjectBrokerClient* createConnection(Socket* sock, SocketAddress& addr);

		void registerClass(const String& name, DistributedObjectClassHelper* helper);

		// deployment methods
		void deploy(DistributedObjectStub* obj);
		void deploy(const String& name, DistributedObjectStub* obj);

		bool destroyObject(DistributedObjectStub* obj);

		DistributedObject* lookUp(const String& name);
		DistributedObject* lookUp(uint64 objid);

		DistributedObjectStub* undeploy(const String& name);
		void undeploy(DistributedObjectStub* obj, bool doLock = true);

		DistributedObjectAdapter* getObjectAdapter(const String& name);
		DistributedObjectAdapter* getObjectAdapter(uint64 oid);

		// getters
		/*inline bool hasRootDirectory() {
			return namingDirectoryInterface->isRootDirectory();
		}*/

		inline DistributedObjectClassHelperMap* getClassMap() {
			return &classMap;
		}

		inline DOBPacketHandler* getPacketHandler() {
			return phandler;
		}

		inline DistributedObjectBrokerClient* getOrbClient() {
			return orbClient;
		}

		inline DOBObjectManager* getObjectManager() {
			return objectManager;
		}

		// setters
		void setCustomObjectManager(DOBObjectManager* manager);

		inline void setOrbClient(DistributedObjectBrokerClient* client) {
			orbClient = client;
		}

		friend class NamingDirectoryService;

		friend class SingletonWrapper<DistributedObjectBroker>;
	};

  } // namespace ORB
} // namespace engine

using namespace engine::ORB;

#endif /*DISTRIBUTEDOBJECTBROKER_H_*/
