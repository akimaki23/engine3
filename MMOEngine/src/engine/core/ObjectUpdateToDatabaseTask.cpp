/*
 * ObjectUpdateToDatabaseTask.cpp
 *
 *  Created on: 05/10/2009
 *      Author: victor
 */

#include "ObjectUpdateToDatabaseTask.h"

ObjectUpdateToDatabaseTask::ObjectUpdateToDatabaseTask(ManagedObject* object) : Task(300000) {
	ObjectUpdateToDatabaseTask::objectRef = object;
}

void ObjectUpdateToDatabaseTask::run() {
	ManagedReference<ManagedObject*> strongRef = objectRef.get();

	ManagedObject* object = objectRef.getForUpdate();

	if (object == NULL)
		return;

	try {
		object->wlock();

		object->clearUpdateToDatabaseTask();
		//object->updateToDatabase();

		object->unlock();
	} catch (Exception& e) {
		//object->error(e.getMessage());
		object->unlock();
	} catch (...) {
		//object->error("unreported exception caught in ObjectSaveToDatabaseTask::run");
		object->unlock();

		throw;
	}
}
