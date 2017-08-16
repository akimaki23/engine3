//
// Created by Victor Popovici on 26/10/16.
//

#include "SendObjectDataMessage.h"

SendObjectDataMessage::SendObjectDataMessage(uint64 oid, Stream* data) : DOBMessage(SENDOBJECTDATA, 40), objectID(oid), objectData(NULL) {
	insertLong(objectID);
	insertStream(data);
}

SendObjectDataMessage::SendObjectDataMessage(Packet* message) : DOBMessage(message) {
		objectID = message->parseLong();

		objectData = NULL;
}

void SendObjectDataMessage::execute() {
	//printf("received object data for oid:%d\n", (int) objectID);

	/*DistributedObject* obj = DistributedObjectBroker::instance()->lookUp(name);

	if (obj != NULL) {
		insertBoolean(true);
		insertAscii(obj->_getClassName());
		insertLong(obj->_getObjectID());
	} else {
		insertBoolean(false);
	}

	client->sendReply(this);*/

	client->sendReply(this);
}

void SendObjectDataMessage::handleReply(Packet* message) {
	//printf("received object data for oid:%d\n", (int) objectID);
}
