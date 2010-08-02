/*
Copyright (C) 2007 <SWGEmu>. All rights reserved.
Distribution of this file for usage outside of Core3 is prohibited.
*/

#include "DistributedObjectDirectory.h"

DistributedObjectDirectory::DistributedObjectDirectory() {
}

DistributedObjectAdapter* DistributedObjectDirectory::add(uint64 objid, DistributedObjectAdapter* adapter) {
	return objectMap.put(objid, adapter);
}

DistributedObject* DistributedObjectDirectory::get(uint64 objid) {
	DistributedObjectAdapter* adapter = NULL;

	try {
		adapter = objectMap.get(objid);
	} catch (...) {

	}

	if (adapter != NULL)
		return adapter->getStub();
	else 
		return NULL;
}

DistributedObjectAdapter* DistributedObjectDirectory::remove(uint64 objid) {
	DistributedObjectAdapter* adapter = objectMap.get(objid);

	if (adapter != NULL)
		objectMap.remove(objid);
	
	return adapter;
}

DistributedObjectAdapter* DistributedObjectDirectory::getAdapter(uint64 objid) {
	return objectMap.get(objid);
}
