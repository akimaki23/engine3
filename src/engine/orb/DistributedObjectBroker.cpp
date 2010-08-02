/*
Copyright (C) 2007 <SWGEmu>. All rights reserved.
Distribution of this file for usage outside of Core3 is prohibited.
*/

#include "DistributedObjectBroker.h"

#include "DistributedObjectBrokerClient.h"

#include "NamingDirectoryServiceImpl.h"

#include "DOBObjectManager.h"
#include "DOBObjectManagerImplementation.h"

DistributedObjectBroker::DistributedObjectBroker()
		: StreamAcceptor("DistributedObjectBroker") {
	phandler = NULL;

	namingDirectoryInterface = NULL;

	objectManager = NULL;

	orbClient = NULL;

	setLogging(false);
}

DistributedObjectBroker::~DistributedObjectBroker() {
	shutdown();

	if (namingDirectoryInterface != NULL) {
		delete namingDirectoryInterface;
		namingDirectoryInterface = NULL;
	}

	if (phandler != NULL) {
		delete phandler;
		phandler = NULL;
	}

	/*if (objectManager != NULL) {
		delete objectManager;
		objectManager = NULL;
	}*/
}

DistributedObjectBroker* DistributedObjectBroker::initialize(const String& addr, int port) {
	DistributedObjectBroker* inst = DistributedObjectBroker::instance();
	inst->address = addr;

	inst->initialize();

	if (addr.isEmpty())
		inst->start(port);

	return inst;
}

void DistributedObjectBroker::initialize() {
	if (address.isEmpty()) {
		namingDirectoryInterface = new NamingDirectoryServiceImpl();

		info("root naming directory initialized", true);
	} else {
		namingDirectoryInterface = new NamingDirectoryService(address);
		orbClient = namingDirectoryInterface->getClient();
	}

	phandler = new DOBPacketHandler("ORBPacketHandler", this);
	phandler->setLogging(false);

	if (address.isEmpty()) {
		objectManager = new DOBObjectManagerImplementation();
	} else {
		objectManager = new DOBObjectManager(orbClient);
	}
}

void DistributedObjectBroker::run() {
	acceptConnections();
}

void DistributedObjectBroker::shutdown() {
	if (socket != NULL) {
		socket->close();

		ServiceThread::stop(false);

		info("stopped", true);
	}
}

DistributedObjectBrokerClient* DistributedObjectBroker::createConnection(ServiceSession* session) {
	DistributedObjectBrokerClient* client = new DistributedObjectBrokerClient(this, session->getSocket());
	client->start();

	return client;
}

void DistributedObjectBroker::registerClass(const String& name, DistributedObjectClassHelper* helper) {
	//Locker locker(this);

	classMap.put(name, helper);
}

void DistributedObjectBroker::deploy(DistributedObjectStub* obj) {
	Locker locker(this);

	DistributedObjectServant* servant = obj->_getImplementation();
	if (servant == NULL)
		throw ObjectNotLocalException(obj);

	try {
		uint64 objectid = obj->_getObjectID();

		if (objectid == 0) {
			objectid = objectManager->getNextFreeObjectID();
			obj->_setObjectID(objectid);
		}

		namingDirectoryInterface->deploy(obj);

		if (objectManager->addObject(obj) != NULL) {
			StringBuffer msg;
			msg << "obejctid 0x" << hex << objectid << " already deployed";
			error(msg.toString());
			StackTrace::printStackTrace();
		} else
			info("object \'" + obj->_getName() + "\' deployed");
	} catch (Exception& e) {
		error(e.getMessage());
	}
}

void DistributedObjectBroker::deploy(const String& name, DistributedObjectStub* obj) {
	Locker locker(this);

	DistributedObjectServant* servant = obj->_getImplementation();
	if (servant == NULL)
		throw ObjectNotLocalException(obj);

	try {
		uint64 objectid = obj->_getObjectID();

		if (objectid == 0) {
			objectid = objectManager->getNextFreeObjectID();
			obj->_setObjectID(objectid);
		}

		namingDirectoryInterface->deploy(name, obj);

		if (objectManager->addObject(obj) != NULL) {
			StringBuffer msg;
			msg << "obejctid 0x" << hex << objectid << " already deployed";
			error(msg.toString());
			StackTrace::printStackTrace();
		} else
			info("object \'" + obj->_getName() + "\' deployed");

	} catch (Exception& e) {
		error(e.getMessage());
	}
}

DistributedObject* DistributedObjectBroker::lookUp(const String& name) {
	Locker locker(this);

	return namingDirectoryInterface->lookUp(name);
}

DistributedObject* DistributedObjectBroker::lookUp(uint64 objid) {
	Locker locker(this);

	DistributedObject* obj = objectManager->getObject(objid);

	locker.release();

	if (obj == NULL)
		obj = objectManager->loadPersistentObject(objid);

	return obj;
}

DistributedObjectStub* DistributedObjectBroker::undeploy(const String& name) {
	Locker locker(this);

	DistributedObjectServant* servant = NULL;

	DistributedObjectStub* obj = (DistributedObjectStub*) namingDirectoryInterface->undeploy(name);

	if (obj != NULL) {
		DistributedObjectAdapter* adapter = objectManager->removeObject(obj->_getObjectID());

		if (adapter != NULL) {
			servant = adapter->getImplementation();

			delete adapter;
		}

		info("object \'" + obj->_getName() + "\' deployed");
	}

	locker.release();

	if (servant != NULL) {
		info("deleting servant \'" + name + "\'");

		delete servant;
	}

	return obj;
}

/*ORBObjectAdapter* ObjectRequestBroker::getObjectAdapter(const String& name) {
	lock();

	ORBObjectStub* obj = namingDirectoryInterface->lookUp(name);
	ORBObjectAdapter* adapter = objectDirectory.getAdapter(obj->getObjectID());

	unlock();
	return adapter;
}*/

void DistributedObjectBroker::setCustomObjectManager(DOBObjectManager* manager) {
	Locker locker(this);

	delete objectManager;
	objectManager = manager;
}

DistributedObjectAdapter* DistributedObjectBroker::getObjectAdapter(uint64 oid) {
	Locker locker(this);

	return objectManager->getAdapter(oid);
}
