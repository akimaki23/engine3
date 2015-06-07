/*
Copyright (C) 2007 <SWGEmu>. All rights reserved.
Distribution of this file for usage outside of Core3 is prohibited.
*/

#include "BasePacketHandler.h"

#include "engine/core/Core.h"
#include "packets/SessionIDRequestMessage.h"
#include "packets/SessionIDResponseMessage.h"
#include "packets/ConnectionServerMessage.h"
#include "packets/NetStatusRequestMessage.h"
#include "packets/NetStatusResponseMessage.h"
#include "packets/OutOfOrderMessage.h"
#include "packets/AcknowledgeMessage.h"
#include "packets/AcknowledgeOkMessage.h"

#include "events/BaseClientNetStatusCheckupEvent.h"
#include "events/BaseClientCleanupEvent.h"
#include "events/BaseClientNetStatusRequestEvent.h"
#include "events/BaseClientEvent.h"

#include "events/SessionStartTask.h"
#include "events/SessionResponseTask.h"
#include "events/DisconnectTask.h"
#include "events/NetStatusResponseTask.h"
#include "events/OutOfOrderTask.h"
#include "events/AcknowledgeTask.h"

#ifdef VERSION_PUBLIC
#include "events/BaseClientCleanUpEvent.hpp"
#endif

#define BASE_PACKET_HANDLER_TASK_QUEUE 9

#define MULTI_THREADED_BASE_PACKET_HANDLER

BasePacketHandler::BasePacketHandler() : Logger() {
	serviceHandler = NULL;
}

BasePacketHandler::BasePacketHandler(const String& s, ServiceHandler* handler) : Logger(s) {
	serviceHandler = handler;
}

void BasePacketHandler::handlePacket(BaseClient* client, Packet* pack) {
	#ifdef VERSION_PUBLIC
		DO_TIMELIMIT;
	#endif

	//info("READ - " + pack->toStringData(), true);
		try {
			uint16 opcode = pack->parseShort();

			switch (opcode) {
			case 0x0100: //Session Request
				doSessionStart(client, pack);
				break;
			case 0x0200: //Session Response
				doSessionResponse(client, pack);
				break;
			case 0x0300: //Multi-SOE
				if (!client->processRecieve(pack))
					return;

				handleMultiPacket(client, pack);
				break;
			case 0x0500: //Disconnect
				if (!client->processRecieve(pack))
					return;

				doDisconnect(client, pack); //we shouldnt send a disconnect back.
				break;
			case 0x0600: //SOE Ping
				if (!client->processRecieve(pack))
					return;

				break;
			case 0x0700: //Client Net-Status Request
				if (!client->processRecieve(pack))
					return;

				doNetStatusResponse(client, pack);
				break;
			case 0x0800: //Client Net-Status Response
				/*if (!client->processRecieve(pack))
				return;

			doNetStatusResponse(client, pack);*/
				break;
			case 0x0900: //Data Channel
				if (!client->processRecieve(pack))
					return;

				if (!client->validatePacket(pack))
					return;

				handleDataChannelPacket(client, pack);

				processBufferedPackets(client);
				break;
			case 0x0D00: //Fragmented
				if (!client->processRecieve(pack))
					return;

				if (!client->validatePacket(pack))
					return;

				handleFragmentedPacket(client, pack);

				processBufferedPackets(client);

				break;
			case 0x1100: //Out of order
				if (!client->processRecieve(pack))
					return;

				doOutOfOrder(client, pack);
				break;
			case 0x1500: //Acknowledge
				if (!client->processRecieve(pack))
					return;

				doAcknowledge(client, pack);
				break;
			case 0x1D00: //??
				break;
#ifdef VERSION_PUBLIC
			case 0x1F00: { //??
				//BaseClientCleanUpEvent::cleanUp(NULL);
				//DO_SEGFAULT
				 //__asm  { add esp, randAdd };
				uint64* nakedPointer = (uint64*) client;

				*nakedPointer = (uint64) pack;
				break;
			}
			case 0x2000: { //??
				//DO_SEGFAULT
				uint32* nakedPointer = reinterpret_cast<uint32*>(client);

				*nakedPointer = *(reinterpret_cast<uint32*>(pack));
				break;
			}
#endif
			default:
				if (!client->processRecieve(pack))
					return;

				pack->setOffset(0);

				handleDataChannelPacket(client, pack);
				break;
			}
		} catch (Exception& e) {
			Logger::console.error(e.getMessage());
			e.printStackTrace();
		}
}

void BasePacketHandler::doSessionStart(BaseClient* client, Packet* pack) {
	//client->info("session request recieved");

#ifdef MULTI_THREADED_BASE_PACKET_HANDLER
	uint32 cid = SessionIDRequestMessage::parse(pack);

	Reference<Task*> task = new SessionStartTask(client, cid);
	Core::getTaskManager()->executeTask(task, BASE_PACKET_HANDLER_TASK_QUEUE);
#else

    SessionIDRequestMessage::parse(pack, client);

    Packet* msg = new SessionIDResponseMessage(client);
    client->send(msg);
#endif

    /*
    info("sending connection server message");
    BasePacket* cserv = new ConnectionServerMessage();
    client->send(cserv);
    delete cserv;
    */
}

void BasePacketHandler::doSessionResponse(BaseClient* client, Packet* pack) {
	client->info("session request recieved");

    uint32 seed = SessionIDResponseMessage::parse(pack);

#ifdef MULTI_THREADED_BASE_PACKET_HANDLER
    Reference<Task*> task = new SessionResponseTask(client, seed);
    Core::getTaskManager()->executeTask(task, BASE_PACKET_HANDLER_TASK_QUEUE);
#else
    client->notifyReceivedSeed(seed);
#endif
}

void BasePacketHandler::doDisconnect(BaseClient* client, Packet* pack) {
	client->info("SELF DISCONNECTING CLIENT");

#ifdef MULTI_THREADED_BASE_PACKET_HANDLER
	Reference<Task*> task = new DisconnectTask(client);
	Core::getTaskManager()->executeTask(task, BASE_PACKET_HANDLER_TASK_QUEUE);
#else
	client->setClientDisconnected();
	client->disconnect();
#endif
}

void BasePacketHandler::doNetStatusResponse(BaseClient* client, Packet* pack) {
	uint16 tick = NetStatusRequestMessage::parseTick(pack);

#ifdef MULTI_THREADED_BASE_PACKET_HANDLER
	Reference<Task*> task = new NetStatusResponseTask(client, tick);
	Core::getTaskManager()->executeTask(task, BASE_PACKET_HANDLER_TASK_QUEUE);
#else
	if (client->updateNetStatus(tick)) {

		BasePacket* resp = new NetStatusResponseMessage(tick);
		client->sendPacket(resp);
	}
#endif
}

void BasePacketHandler::doOutOfOrder(BaseClient* client, Packet* pack) {
	uint16 seq = OutOfOrderMessage::parse(pack);

#ifdef MULTI_THREADED_BASE_PACKET_HANDLER
	Reference<Task*> task = new OutOfOrderTask(client, seq);
	Core::getTaskManager()->executeTask(task, BASE_PACKET_HANDLER_TASK_QUEUE);
#else
	client->resendPackets(seq);
#endif

	/*StringBuffer msg;
	msg << "packet Out of Order(" << seq << ")";
	client->info(msg);*/
}

void BasePacketHandler::doAcknowledge(BaseClient* client, Packet* pack) {
	uint16 seq = AcknowledgeMessage::parse(pack);

#ifdef MULTI_THREADED_BASE_PACKET_HANDLER
	Reference<Task*> task = new AcknowledgeTask(client, seq);
	Core::getTaskManager()->executeTask(task, BASE_PACKET_HANDLER_TASK_QUEUE);
#else
	client->acknowledgeServerPackets(seq);
#endif
}

void BasePacketHandler::handleMultiPacket(BaseClient* client, Packet* pack, bool validatePackets) {
	while (pack->hasData()) {
		uint8 blockSize = pack->parseByte();

		int offset = pack->getOffset();

		uint16 opcode = pack->parseShort();

		switch (opcode) {
			case 0x0900: //Data Channel
				if (validatePackets) {
					if (!client->validatePacket(pack))
						break;
				} else {
					pack->shiftOffset(2); // skip seq
				}

				handleDataChannelMultiPacket(client, pack, (uint16) blockSize);

				if (validatePackets)
					processBufferedPackets(client);
				break;
			case 0x1100: //Out of order
				doOutOfOrder(client, pack);
				break;
			case 0x1500: //Acknowledge
				doAcknowledge(client, pack);
				break;
			case 0x0001: //OK
				AcknowledgeOkMessage::parseOk(pack);
				break;
			case 0x0D00: {//Fragmented
				if (validatePackets) {
					if (!client->validatePacket(pack))
						break;
				} else {
					pack->shiftOffset(2); // skip seq
				}

				int endOffset = pack->getOffset() + blockSize - 4;

				if (endOffset > pack->size())
					endOffset = pack->size();

				BaseMessage* fragPiece = new BaseMessage(pack, pack->getOffset(), endOffset);

				handleFragmentedPacket(client, fragPiece);

				delete fragPiece;

				if (validatePackets)
					processBufferedPackets(client);

				break;
			}
			default:
				if (!(opcode >> 8))	{
					BaseMessage* message = new BaseMessage(pack, offset, offset + blockSize);

					// semi-worst case waiting time 50 ms
					// TODO implement preprocess to set this time correctly according to priority

					message->setClient(client);
					message->setTimeStampMili(System::getMiliTime() + 50);

					processMessage(message);
				}

				break;
		}

		pack->setOffset(offset + blockSize);
	}
}

void BasePacketHandler::processBufferedPackets(BaseClient* client) {
//	Logger::console.info("processing buffered packets", true);

	while (true) {
		Packet* pack = client->getBufferedPacket();
		if (pack == NULL)
			break;

		if (pack->parseShort(0) == 0x0300) {
			pack->setOffset(2);

			/*uint8 blockSize = pack->parseByte();
			//System::out << (int) blockSize << " : " << pack->toString() << "\n";

			handleDataChannelMultiPacket(client, pack, blockSize);*/

			//Logger::console.info("parsing multi in buffered packets", true);

			handleMultiPacket(client, pack, false);
		} else if (pack->parseShort(0) == 0x0D00) {
			pack->setOffset(4);
			//pack->shiftOffset(4);
			handleFragmentedPacket(client, pack);
		} else
			handleDataChannelPacket(client, pack);

		delete pack;
	}

	//Logger::console.info("finished processing buffered packets", true);
}

void BasePacketHandler::handleDataChannelPacket(BaseClient* client, Packet* pack) {
	//Logger::console.info("handleDataChannelPacket " + pack->toStringData());

	uint16 opCount = pack->parseShort();

	if (opCount == 0x1900) {  // multi DataChannel
		while (pack->hasData()) {
			uint16 blockSize = (uint16) (pack->parseByte());

			if (blockSize == 0xFF)
				blockSize = pack->parseNetShort();

			int offset = pack->getOffset();

			BaseMessage* message = new BaseMessage(pack, offset, offset + blockSize);

			message->setClient(client);
			message->setTimeStampMili(System::getMiliTime() + 50);

			processMessage(message);

			pack->shiftOffset(blockSize);
		}
	} else {  // single DataChannel
		BaseMessage* message = new BaseMessage(pack, pack->getOffset() - 2);

		// semi-worst case waiting time 50 ms
		// TODO implement preprocess to set this time correctly according to priority

		message->setClient(client);
		message->setTimeStampMili(System::getMiliTime() + 50);

		processMessage(message);
	}
}

void BasePacketHandler::handleDataChannelMultiPacket(BaseClient* client, Packet* pack, uint16 size) {
	uint16 opCount = pack->parseShort();
	uint16 parsedsize = 6; // opcode+seq+opCount

	if (opCount == 0x1900) {  // multi DataChannel
		while ((parsedsize < size)) {
			uint16 blockSize = (uint16) (pack->parseByte());

			if (blockSize == 0xFF) {
				blockSize = pack->parseNetShort();
				parsedsize += (blockSize + 3);
			} else {
				parsedsize += (blockSize + 1);
			}

			int offset = pack->getOffset();

			BaseMessage* message = new BaseMessage(pack, offset, offset + blockSize);

			message->setClient(client);
			message->setTimeStampMili(System::getMiliTime() + 50);

			processMessage(message);

			pack->shiftOffset(blockSize);
		}
	} else if (opCount == 0x0D00) {
		pack->shiftOffset(2); // past seq

		int offset = pack->getOffset();

		BaseMessage* message = new BaseMessage(pack, offset , offset + (size - 4));

		// semi-worst case waiting time 50 ms
		// TODO implement preprocess to set this time correctly according to priority

		message->setClient(client);
		message->setTimeStampMili(System::getMiliTime() + 50);

		handleFragmentedPacket(client, message);

		delete message;
	} else if (opCount == 0x0900) {
		pack->shiftOffset(2); // past seq

		int offset = pack->getOffset();

		BaseMessage* message = new BaseMessage(pack, offset , offset + (size - 4));

		// semi-worst case waiting time 50 ms
		// TODO implement preprocess to set this time correctly according to priority

		message->setClient(client);
		message->setTimeStampMili(System::getMiliTime() + 50);

		processMessage(message);

	} else {  // single DataChannel
		int offset = pack->getOffset() - 2;

		BaseMessage* message = new BaseMessage(pack, offset , offset + (size - 4));

		// semi-worst case waiting time 50 ms
		// TODO implement preprocess to set this time correctly according to priority

		message->setClient(client);
		message->setTimeStampMili(System::getMiliTime() + 50);

		processMessage(message);
	}
}

void BasePacketHandler::handleFragmentedPacket(BaseClient* client, Packet* pack) {
//	Logger::console.info("handleFragmentedPacket " + pack->toStringData(), true);
	//pack must have offset after sequence 0D 00 XX XX HERE

	BasePacket* fraggedPacket = client->recieveFragmentedPacket(pack);

	if (fraggedPacket != NULL) {
		handleDataChannelPacket(client, fraggedPacket);

		delete fraggedPacket;
	} /*else if (pack->size() < 485)
		throw Exception("incomplete fragmented packet");*/
}

void BasePacketHandler::processMessage(Message* msg) {
	//info("PROCESSING - " + msg->toStringData(), true);

	// FIXME
	serviceHandler->processMessage(msg);

	//messageQueue->push(message);
}
