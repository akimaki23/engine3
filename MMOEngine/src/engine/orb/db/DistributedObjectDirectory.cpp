/*
Copyright (C) 2007 <SWGEmu>. All rights reserved.
Distribution of this file for usage outside of Core3 is prohibited.
*/

#include "DistributedObjectDirectory.h"

#include "engine/db/ObjectDatabaseManager.h"

#include "engine/core/ManagedObject.h"

class DirectoryKeyHandler : public KeyHandler<uint64> {
	AtomicLong id;

public:
	uint64 generateKey() {
		return id.increment();
	}
};

DistributedObjectDirectory::DistributedObjectDirectory() : objectMap(300000){
}

DistributedObjectDirectory::~DistributedObjectDirectory() {
	HashTableIterator<uint64, DistributedObjectAdapter*> iterator = objectMap.iterator();

	while (iterator.hasNext()) {
		delete iterator.getNextValue();
	}
}

DistributedObjectAdapter* DistributedObjectDirectory::add(uint64 objid, DistributedObjectAdapter* adapter) {
	helperObjectMap.add(objid, adapter->getStub());

	return objectMap.put(objid, adapter);
}

DistributedObject* DistributedObjectDirectory::get(uint64 objid) {
	DistributedObjectAdapter* adapter = NULL;

	try {
		adapter = objectMap.get(objid);
	} catch (...) {

	}

	if (adapter != NULL) {
		helperObjectMap.add(objid, adapter->getStub());
		return adapter->getStub();
	} else
		return NULL;
}

DistributedObjectAdapter* DistributedObjectDirectory::remove(uint64 objid) {
	DistributedObjectAdapter* adapter = objectMap.get(objid);

	if (adapter != NULL)
		objectMap.remove(objid);
	
	helperObjectMap.remove(objid);

	return adapter;
}

void DistributedObjectDirectory::removeHelper(uint64 objid) {
	helperObjectMap.remove(objid);
}

DistributedObjectAdapter* DistributedObjectDirectory::getAdapter(uint64 objid) {
	return objectMap.get(objid);
}

void DistributedObjectDirectory::getObjectsMarkedForUpdate(Vector<DistributedObject*>& objectsToUpdate, Vector<DistributedObject*>& objectsToDelete,
		Vector<DistributedObject* >& objectsToDeleteFromRAM, VectorMap<String, int>* inRamClassCount) {
		
	Logger::console.info("starting getObjectsMarkedForUpdate", true);

	objectsToUpdate.removeAll(objectMap.size(), 1);
	objectsToDelete.removeAll(100000, 0);
	
	Logger::console.info("allocated objectsToUpdate size", true);

	HashTableIterator<uint64, DistributedObjectAdapter*> iterator(&objectMap);

	while (iterator.hasNext()) {
		DistributedObjectAdapter* adapter = iterator.getNextValue();

		DistributedObject* dobObject = adapter->getStub();
		//DistributedObjectServant* dobServant = adapter->getImplementation();

		if (dobObject->getReferenceCount() == 2) // 2 is the lowest count now
			objectsToDeleteFromRAM.add(dobObject);
		else if (inRamClassCount != NULL) {
			String className = TypeInfo<DistributedObject>::getClassName(dobObject, false);

			inRamClassCount->put(className, inRamClassCount->get(className) + 1);
		}

		//ManagedObject* managedObject = dynamic_cast<ManagedObject*>(dobObject);
		

		//if (managedObject == NULL/* || !managedObject->isPersistent()*/)
		//	continue;
		
		ManagedObject* managedObject = static_cast<ManagedObject*>(dobObject);

		if (dobObject->_isMarkedForDeletion()) {
			objectsToDelete.add(dobObject);
		} else if (dobObject->_isUpdated() && managedObject->isPersistent()) {
			objectsToUpdate.add(dobObject);
		}
	}

	StringBuffer msg;
	msg << "[DistributedObjectDirectory] marked " << objectsToUpdate.size() << " objects to update and "
			<< objectsToDelete.size() << " for deletion";

	Logger::console.info(msg.toString(), true);
}
