/*
Copyright (C) 2007 <SWGEmu>. All rights reserved.
Distribution of this file for usage outside of Core3 is prohibited.
*/

//#define TRACE_CLIENTS

#include "engine/core/Core.h"

#include "BaseClient.h"

#include "events/BaseClientNetStatusCheckupEvent.h"
#include "events/BaseClientCleanupEvent.h"
#include "events/BaseClientNetStatusRequestEvent.h"
#include "events/BaseClientEvent.h"

#include "packets/SessionIDRequestMessage.h"
#include "packets/AcknowledgeMessage.h"
#include "packets/OutOfOrderMessage.h"
#include "packets/DisconnectMessage.h"
#include "packets/NetStatusRequestMessage.h"

#include "engine/stm/TransactionalMemoryManager.h"

class AcknowledgeClientPackets : public Task {
        Reference<BaseClient*> client;
        uint16 seq;
        
public:
        AcknowledgeClientPackets(BaseClient* cl, uint16 s) {
                client = cl;
                seq = s;
        }       
        
        void run() {
                client->acknowledgeClientPackets(seq);
        } 
};

BaseClient::BaseClient() : DatagramServiceClient(),
		BaseProtocol(), Mutex(true) {
	bufferedPacket = NULL;
	receiveBuffer.setInsertPlan(SortedVector<BasePacket*>::NO_DUPLICATE);

	fragmentedPacket = NULL;

	checkupEvent = NULL;
	netcheckupEvent = NULL;
	netRequestEvent = NULL;

	reentrantTask = new BaseClientEvent(this);

	keepSocket = false;

	setDebugLogLevel();
   	setGlobalLogging(true);

#ifdef LOCKFREE_BCLIENT_BUFFERS
	sendUnreliableBuffer = new packet_buffer_t();
	sendReliableBuffer = new packet_buffer_t();
#endif

   	//reentrantTask->schedulePeriodic(10, 10);
}

BaseClient::BaseClient(const String& addr, int port) : DatagramServiceClient(addr, port),
		BaseProtocol(), Mutex(true) {
	bufferedPacket = NULL;
	receiveBuffer.setInsertPlan(SortedVector<BasePacket*>::NO_DUPLICATE);

	fragmentedPacket = NULL;

	checkupEvent = NULL;
	netcheckupEvent = NULL;
	netRequestEvent = NULL;

	reentrantTask = new BaseClientEvent(this);

	keepSocket = false;

	setInfoLogLevel();
   	setGlobalLogging(true);

#ifdef LOCKFREE_BCLIENT_BUFFERS
	sendUnreliableBuffer = new packet_buffer_t();
	sendReliableBuffer = new packet_buffer_t();
#endif

   	//reentrantTask->schedulePeriodic(10, 10);
}

BaseClient::BaseClient(Socket* sock, SocketAddress& addr) : DatagramServiceClient(sock, addr),
		BaseProtocol(), Mutex(true) {
	bufferedPacket = NULL;

	fragmentedPacket = NULL;

	checkupEvent = NULL;
	netcheckupEvent = NULL;
	netRequestEvent = NULL;

	reentrantTask = new BaseClientEvent(this);

  	ip = addr.getFullIPAddress();
   	setLockName("Client " + ip);
   	//setMutexLogging(false);

   	/*String prip = addr.getFullPrintableIPAddress();
   	setFileLogger("log/" + prip);*/

	keepSocket = true;

	setInfoLogLevel();
   	setGlobalLogging(true);

#ifdef LOCKFREE_BCLIENT_BUFFERS
	sendUnreliableBuffer = new packet_buffer_t();
	sendReliableBuffer = new packet_buffer_t();
#endif

   	//reentrantTask->schedulePeriodic(10, 10);
}

BaseClient::~BaseClient() {
	if (checkupEvent != NULL) {
		checkupEvent->cancel();

		checkupEvent = NULL;
	}

	if (netcheckupEvent != NULL) {
		netcheckupEvent->cancel();

		netcheckupEvent = NULL;
	}

	if (netRequestEvent != NULL) {
		netRequestEvent->cancel();

		netRequestEvent = NULL;
	}

	if (!keepSocket)
		ServiceClient::close();

#ifdef LOCKFREE_BCLIENT_BUFFERS
	delete sendUnreliableBuffer;
	sendUnreliableBuffer = NULL;

	delete sendReliableBuffer;
	sendReliableBuffer = NULL;
#endif

	debug("deleted");
}

void BaseClient::initialize() {
	#ifdef VERSION_PUBLIC
	SocketAddress addr = ServiceClient::getAddress();
	uint16 port = addr.getPort();

	ServiceClient::setAddress("127.0.0.1", port);
	#endif

	crcSeed = 0;

	serverSequence = 0;
   	clientSequence = 0;

   	disconnected = false;
   	clientDisconnected = false;

   	acknowledgedServerSequence = -1;
	realServerSequence = 0;
	resentPackets = 0;

	service = NULL;

	checkupEvent = new BasePacketChekupEvent(this);
	netcheckupEvent = new BaseClientNetStatusCheckupEvent(this);

   	lastNetStatusTimeStamp.addMiliTime(NETSTATUSCHECKUP_TIMEOUT);
   	balancePacketCheckupTime();

   	//netcheckupEvent->schedule(NETSTATUSCHECKUP_TIMEOUT);

   	netRequestEvent = new BaseClientNetStatusRequestEvent(this);
}

void BaseClient::close() {
	disconnected = true;

	reentrantTask->cancel();

	checkupEvent->cancel();

	netcheckupEvent->cancel();
	netcheckupEvent.castTo<BaseClientNetStatusCheckupEvent*>()->clearClient();

	if (netRequestEvent) {
		netRequestEvent->cancel();

		netRequestEvent.castTo<BaseClientNetStatusRequestEvent*>()->clearClient();
	}

	Reference<Task*> task = new BaseClientCleanupEvent(this);
	task->scheduleInIoScheduler();

	for (int i = 0; i < sendBuffer.size(); ++i) {
		BasePacket* pack = sendBuffer.get(i);
		delete pack;
	}

	sendBuffer.removeAll();

	for (int i = 0; i < sequenceBuffer.size(); ++i) {
		BasePacket* pack = sequenceBuffer.get(i);
		delete pack;
	}

	sequenceBuffer.removeAll();

#ifdef LOCKFREE_BCLIENT_BUFFERS
	while (!sendUnreliableBuffer->empty()) {
		BasePacket* pack;

		if (sendUnreliableBuffer->pop(pack))
			delete pack;
	}

	while (!sendReliableBuffer->empty()) {
		BasePacket* pack;

		if (sendReliableBuffer->pop(pack))
			delete pack;
	}
#else
	for (int i = 0; i < sendUnreliableBuffer.size(); ++i) {
		delete sendUnreliableBuffer.get(i);
	}

	sendUnreliableBuffer.removeAll();
#endif

	if (fragmentedPacket != NULL) {
		delete fragmentedPacket;
		fragmentedPacket = NULL;
	}

	//serverSequence = 0;
	clientSequence = 0;

	acknowledgedServerSequence = -1;

	reportStats();

	closeFileLogger();

	//ServiceClient::close();

	debug("client resources closed");
}

void BaseClient::send(Packet* pack, bool doLock) {
	//setDebugLogLevel();
	lock(doLock);

	try {
		if (isAvailable()) {
			#ifdef TRACE_CLIENTS
				debug("SEND(RAW) - " + pack->toString());
			#endif

			if (!DatagramServiceClient::send(pack))
				debug("LOSING " + pack->toStringData());
		}
	} catch (SocketException& e) {
		error("on send()" + e.getMessage());

		setError();
		disconnect(false);
	}

	delete pack;

	unlock(doLock);
}

void BaseClient::sendPacket(BasePacket* pack, bool doLock) {
#ifdef WITH_STM
	TransactionalMemoryManager::instance()->getBaseClientManager()->sendPacket(pack, this);

	return;
#endif

#ifdef LOCKFREE_BCLIENT_BUFFERS
	if (!isAvailable())
		return;

	if (!pack->doSequencing()) {
		sendSequenceLess(pack);
	} else {
		if (!sendReliableBuffer->push(pack)) {
			error("losing message in BaseClient::sendPacket due to push");
		}
	}

	return;
#endif

	lock(doLock);

	if (!isAvailable()) {
		delete pack;

		unlock(doLock);
		return;
	}

	/*#ifdef TRACE_CLIENTS
		debug("preapare SEND " + pack->toString());
	#endif*/

	try {
		if (pack->doSequencing()) {
			if (pack->size() >= 490) {
				if (bufferedPacket != NULL) {
					sendSequenced(bufferedPacket->getPacket());
					bufferedPacket = NULL;
				}

				sendFragmented(pack);
			} else
				bufferMultiPacket(pack);
		} else {
			sendSequenceLess(pack);
		}
	} catch (...) {
		disconnect("unreported exception on sendPacket()", false);
	}

	unlock(doLock);
}

void BaseClient::bufferMultiPacket(BasePacket* pack) {
	if (bufferedPacket != NULL) {
		if (pack->isDataChannelPacket() && !pack->isMultiPacket() && (pack->size() - 4 < 0xFF)) { // client is sending out of orders for our multi packets with size >= 0xFF
			if (!bufferedPacket->add(pack)) {
				sendSequenced(bufferedPacket->getPacket());

				bufferedPacket = new BaseMultiPacket(pack);
			}
		} else {
			sendSequenced(bufferedPacket->getPacket());
			bufferedPacket = NULL;

			sendSequenced(pack);
		}
	} else {
		if (pack->isDataChannelPacket() && !pack->isMultiPacket() && (pack->size() - 4 < 0xFF))
			bufferedPacket = new BaseMultiPacket(pack);
		else
			sendSequenced(pack);

#ifndef LOCKFREE_BCLIENT_BUFFERS
		if (!reentrantTask->isScheduled())
			reentrantTask->scheduleInIoScheduler(10);
#endif
	}
}

void BaseClient::sendSequenceLess(BasePacket* pack) {
	try {
		#ifdef TRACE_CLIENTS
			debug("SEND(NOSEQ) - " + pack->toString());
		#endif

#ifdef LOCKFREE_BCLIENT_BUFFERS
		if (!sendUnreliableBuffer->push(pack)) {
			warning("losing unreliable pack - push failed");
		}
#else
		sendUnreliableBuffer.add(pack);

		if (!reentrantTask->isScheduled())
			reentrantTask->scheduleInIoScheduler(10);
#endif
	} catch (SocketException& e) {
		delete pack;

		disconnect("on sendPacket()" + e.getMessage(), false);
	}
}

void BaseClient::sendSequenced(BasePacket* pack) {
	if (!isAvailable())
		return;

	try {
		pack->setTimeout(((BasePacketChekupEvent*)(checkupEvent.get()))->getCheckupTime());
		sendBuffer.add(pack);

#ifndef LOCKFREE_BCLIENT_BUFFERS
		if (!reentrantTask->isScheduled())
			reentrantTask->scheduleInIoScheduler(10);
#endif
	} catch (SocketException& e) {
		disconnect("sending packet", false);
	} catch (ArrayIndexOutOfBoundsException& e) {
		error("on sendQueued() - " + e.getMessage());
	}

	/*#ifdef TRACE_CLIENTS
		StringBuffer msg;
		msg << "SEND(" << pack->getSequence() << ")" + pack->toString());
		debug(msg);
	#endif*/
}

void BaseClient::sendFragmented(BasePacket* pack) {
	if (!isAvailable())
		return;

	try {
		BaseFragmentedPacket* frag = new BaseFragmentedPacket(pack);

		while (frag->hasFragments())
			sendSequenced(frag->getFragment());

		delete frag;
	} catch (SocketException& e) {
		disconnect("sending packet", false);
	} catch (ArrayIndexOutOfBoundsException& e) {
		error("on sendFragmented() - " + e.getMessage());
	}
}

void BaseClient::sendReliablePackets() {
	try {
		for (int i = 0; i < 8; ++i) {
			if (isAvailable()) {
				BasePacket* pack = getNextSequencedPacket();

				if (pack == NULL) {
#ifndef LOCKFREE_BCLIENT_BUFFERS
					if (!reentrantTask->isScheduled() && (!sendBuffer.isEmpty() || bufferedPacket != NULL)) {
						try {
							reentrantTask->scheduleInIoScheduler(10);
						} catch (Exception& e) {

						}
					}
#endif

					return;
				}

				if (sequenceBuffer.isEmpty()) {
					((BasePacketChekupEvent*)(checkupEvent.get()))->update(pack);
					pack->setTimeout(((BasePacketChekupEvent*)(checkupEvent.get()))->getCheckupTime());

					if (!checkupEvent->isScheduled())
						checkupEvent->scheduleInIoScheduler(pack->getTimeout());
				}

#ifdef TRACE_CLIENTS
				StringBuffer msg;
				msg << "SEND(" << serverSequence << ") - " << pack->toString();
				debug(msg);
#endif

				pack->setTimestamp();

				prepareSequence(pack);

				if (pack->getSequence() != (uint32) realServerSequence++) {
					StringBuffer msg;
					msg << "invalid server Packet " << pack->getSequence() << " sent (" << realServerSequence - 1 << ")";
					error(msg);
				}

				unlock();

				prepareEncryptionAndCompression(pack);

				lock();

				if (!DatagramServiceClient::send(pack)) {
					StringBuffer msg;
					msg << "LOSING (" << pack->getSequence() << ") " /*<< pack->toString()*/;
					debug(msg);
				}

				sequenceBuffer.add(pack);

			}
		}
#ifndef LOCKFREE_BCLIENT_BUFFERS
		if (!reentrantTask->isScheduled() && (!sendBuffer.isEmpty() || !sendUnreliableBuffer.isEmpty() || bufferedPacket != NULL)) {
			reentrantTask->scheduleInIoScheduler(10);
		}
#endif

	} catch (SocketException& e) {
		disconnect("on activate() - " + e.getMessage(), false);
	} catch (Exception& e) {
		error(e.getMessage());
		e.printStackTrace();

		disconnect("unreported exception on run()", false);
	} catch (...) {
		disconnect("unreported exception on run()", false);
	}
}

void BaseClient::sendUnreliablePackets() {
	try {
		for (int i = 0; i < 8; ++i) {
			if (isAvailable()) {
				BasePacket* pack = getNextUnreliablePacket();

				if (pack == NULL) {
#ifndef LOCKFREE_BCLIENT_BUFFERS
					if (!reentrantTask->isScheduled() && (!sendBuffer.isEmpty() || !sendUnreliableBuffer.isEmpty() || bufferedPacket != NULL)) {
						try {
							reentrantTask->scheduleInIoScheduler(10);
						} catch (Exception& e) {

						}
					}
#endif
					return;
				}

				unlock();

				pack->close();

				prepareEncryptionAndCompression(pack);

				lock();

				if (!DatagramServiceClient::send(pack)) {
					StringBuffer msg;
					msg << "LOSING (" << pack->getSequence() << ") " /*<< pack->toString()*/;
					debug(msg);
				}

				delete pack;
			}
		}
#ifndef LOCKFREE_BCLIENT_BUFFERS
		if (!reentrantTask->isScheduled() && (!sendBuffer.isEmpty() || !sendUnreliableBuffer.isEmpty() || bufferedPacket != NULL)) {
			reentrantTask->scheduleInIoScheduler(10);
		}
#endif
	} catch (SocketException& e) {
		error("on activate() - " + e.getMessage());
	} catch (Exception& e) {
		error("exception on sendUnreliablePackets()");
		error(e.getMessage());
		e.printStackTrace();
	} catch (...) {
		error("unreported exception on sendUnreliablePackets()");
	}
}

void BaseClient::run() {
	//info("run event", true);

	lock();

#ifdef LOCKFREE_BCLIENT_BUFFERS
	int i = 0;
	BasePacket* pack;

	while (i < 20 && sendReliableBuffer->pop(pack)) {
		try {
			if (pack->size() >= 490) {
				if (bufferedPacket != NULL) {
					sendSequenced(bufferedPacket->getPacket());
					bufferedPacket = NULL;
				}

				sendFragmented(pack);
			} else
				bufferMultiPacket(pack);
		} catch (...) {
			disconnect("unreported exception on lockfree sendPacket()", false);
		}

		++i;
	}

	if (i >= 20) {
		warning("more than 20 packets in sendReliableBuffer on BaseClient tick");
	}
#endif

	sendReliablePackets();

	sendUnreliablePackets();

#ifdef LOCKFREE_BCLIENT_BUFFERS
	if (isAvailable()) {
		try {
			auto ref = reentrantTask;

			if (ref != NULL)
				ref->scheduleInIoScheduler(10);
		} catch (...) {

		}
	}
#endif

	unlock();
}

BasePacket* BaseClient::getNextUnreliablePacket() {
#ifdef LOCKFREE_BCLIENT_BUFFERS
	BasePacket* pack;

	if (sendUnreliableBuffer->pop(pack)) {
		return pack;
	} else {
		return NULL;
	}
#else
	if (!sendUnreliableBuffer.isEmpty()) {
		BasePacket* pack = sendUnreliableBuffer.remove(0);

		return pack;
	}

	return NULL;
#endif
}

BasePacket* BaseClient::getNextSequencedPacket() {
	BasePacket* pack = NULL;

	/*#ifdef TRACE_CLIENTS
		StringBuffer msg;
		msg << "SEQ = " << serverSequence << ", ACKSEQ = " << acknowledgedServerSequence << ", BUFFSIZE = "
			<< sendBuffer.size();
		debug(msg);
	#endif*/

	if (serverSequence - acknowledgedServerSequence > 50) { //originally 25
#ifndef LOCKFREE_BCLIENT_BUFFERS
		if ((!sendBuffer.isEmpty() || bufferedPacket != NULL) && !reentrantTask->isScheduled())
			reentrantTask->scheduleInIoScheduler(10);
#endif
		try {
			if (!checkupEvent->isScheduled()) {
				checkupEvent->scheduleInIoScheduler(5);
			}
		} catch (...) {
		}

//      resendPackets();

		if (sendBuffer.size() > 6000) {
			StringBuffer msg;
			msg << "WARNING - send buffer overload [" << sendBuffer.size() << "]";
			error(msg);

			disconnect(false);
		}

		return NULL;
	} else if (!sendBuffer.isEmpty()) {
		pack = sendBuffer.remove(0);
	} else if (bufferedPacket != NULL) {
		pack = bufferedPacket->getPacket();
		pack->setTimeout(((BasePacketChekupEvent*)(checkupEvent.get()))->getCheckupTime());

		bufferedPacket = NULL;
	} else
		return NULL;

	return pack;
}

bool BaseClient::validatePacket(Packet* pack) {
	uint16 seq = pack->parseNetShort();

	Locker locker(this);

	/*if (clientSequence % 0xFF == 0) {
		info("current sequence " + String::valueOf((uint64) clientSequence), true);
	}*/

#ifdef VERSION_PUBLIC
		if (seq < clientSequence) {
#else
		if (seq < (clientSequence & 0xFFFF)) {
#endif
		acknowledgeClientPackets(seq);
		//Core::getTaskManager()->executeTask(new AcknowledgeClientPackets(this, seq), 9);

		return false;
#ifdef VERSION_PUBLIC
		} else if (seq > clientSequence) {
#else
		} else if (seq > (clientSequence & 0xFFFF)) {
#endif
		BasePacket* packet = new BasePacket(pack, seq);
		receiveBuffer.put(packet);

		/*StringBuffer msg3;
		msg3 << "READ buffer (";

		for (int i = 0; i < receiveBuffer.size(); ++i) {
			msg3 << receiveBuffer.get(i)->getSequence() << ", ";
		}

		debug(msg3);*/

		BasePacket* oor = new OutOfOrderMessage(seq);
		sendPacket(oor, false);

		#ifdef TRACE_CLIENTS
			StringBuffer msg;
   			msg << "OUT of order READ(" << seq << ") expected " << clientSequence;
			debug(msg);
		#endif

		return false;
	} /*else
		throw Exception("received same packet sequence");*/

	acknowledgeClientPackets(clientSequence++);
	//Core::getTaskManager()->executeTask(new AcknowledgeClientPackets(this, clientSequence++), 9);
		

	#ifdef TRACE_CLIENTS
		StringBuffer msg;
		msg  << "READ(" << seq << ") - " << pack->toString();
		debug(msg);
	#endif

	return true;
}

Packet* BaseClient::getBufferedPacket() {
	if (!receiveBuffer.isEmpty()) {
		BasePacket* packet = receiveBuffer.get(0);

		uint32 packseq = packet->getSequence();
		if ((packseq & 0xFFFF) != (clientSequence & 0xFFFF))
			return NULL;

		receiveBuffer.remove(0);

		acknowledgeClientPackets(clientSequence++);

		#ifdef TRACE_CLIENTS
			StringBuffer msg;
			msg << "BUFFERED READ(" << packseq << ")";
			debug(msg);
		#endif

		return packet;
	}

	return NULL;
}

BasePacket* BaseClient::recieveFragmentedPacket(Packet* pack) {
	//Logger::console.debug("recieveFragmentedPacket " + pack->toStringData());

	BasePacket* packet = NULL;

	if (fragmentedPacket == NULL) {
		fragmentedPacket = new BaseFragmentedPacket();
		//Logger::console.info("creating new BaseFragmentedPacket", true);
	}

	if (!fragmentedPacket->addFragment(pack)) {
		delete fragmentedPacket;
		fragmentedPacket = NULL;

		return NULL;
	}

	try {

		if (fragmentedPacket->isComplete()) {
			fragmentedPacket->setOffset(0);

			//Logger::console.info("completed fragmented packet", true);

			packet = fragmentedPacket;
			fragmentedPacket = NULL;
		}
	} catch (Exception& e) {
		Logger::console.error(e.getMessage());
		Logger::console.error(pack->toStringData());

		if (fragmentedPacket != NULL) {
			StringBuffer msg;
			msg << "current fragmented packet.." << fragmentedPacket->toStringData();
			Logger::console.error(msg.toString());

			delete fragmentedPacket;
			fragmentedPacket = NULL;
			packet = NULL;
		}
	} catch (...) {
		Logger::console.error("unreproted exception caught in BasePacket* BaseClient::recieveFragmentedPacket");
		Logger::console.error(pack->toStringData());

		if (fragmentedPacket != NULL) {
			StringBuffer msg;
			msg << "current fragmented packet.." << fragmentedPacket->toString();
			Logger::console.error(msg.toString());

			delete fragmentedPacket;
			fragmentedPacket = NULL;
			packet = NULL;
		}
	}

	return packet;
}

void BaseClient::checkupServerPackets(BasePacket* pack) {
//        return;
        
	lock();

	try {
		if (!isAvailable() || checkupEvent->isScheduled() || sequenceBuffer.size() == 0) {
			unlock();
			return;
		}

		uint32 seq = pack->getSequence();

		#ifdef TRACE_CLIENTS
			StringBuffer msg;
			msg << "CHECKING UP sequence " << seq << "[" << acknowledgedServerSequence
				<< "]";
			debug(msg);
		#endif

		if (seq > (uint32) acknowledgedServerSequence) {
			resendPackets();

			((BasePacketChekupEvent*)(checkupEvent.get()))->increaseCheckupTime(250);
			((BasePacketChekupEvent*)(checkupEvent.get()))->update(pack);

			#ifdef TRACE_CLIENTS
				StringBuffer msg;
				msg << "checkup time incresed to " << checkupEvent->getCheckupTime();
				debug(msg);
			#endif

			if (!checkupEvent->isScheduled())
				checkupEvent->scheduleInIoScheduler(pack->getTimeout());
		}
	} catch (SocketException& e) {
		disconnect("on checkupServerPackets() - " + e.getMessage(), false);
	} catch (ArrayIndexOutOfBoundsException& e) {
		error("on checkupServerPackets() - " + e.getMessage());
	} catch (...) {
		disconnect("unreported exception on checkupServerPackets()", false);
	}

	unlock();
}

void BaseClient::resendPackets() {
	/*#ifdef TRACE_CLIENTS
		StringBuffer msg2;
		msg2 << "[" << seq << "] resending " << MIN(sequenceBuffer.size(), 5) << " packets to \'" << ip << "\' ["
			 << checkupEvent->getCheckupTime() << "]";
		debug(msg2, true);
	#endif*/
	
	if (sequenceBuffer.size() == 0)
		return;
	
	float checkupTime = (float) ((float)((BasePacketChekupEvent*)(checkupEvent.get()))->getCheckupTime()) / 1000.f;
	int maxPacketResent = MAX(5, (float)30000.f * checkupTime / 496.f); //30kb * second assuming 496 packet size

	/*StringBuffer msg2;
	msg2 << "resending MIN(" << sequenceBuffer.size() << " and " << maxPacketResent << ") packets to \'" << ip << "\' ["
			<< ((BasePacketChekupEvent*)(checkupEvent.get()))->getCheckupTime() << "]";
	info(msg2, true);*/
	
	for (int i = 0; i < MIN(sequenceBuffer.size(), maxPacketResent); ++i) {
//	for (int i = 0; i < sequenceBuffer.size(); ++i) {
//	for (int i = 0; i < MIN(sequenceBuffer.size(), 1); ++i) {
		BasePacket* packet = sequenceBuffer.get(i);

		/*if (packet->getTimeout().isFuture())
			break;*/

		packet->setTimeout(((BasePacketChekupEvent*)(checkupEvent.get()))->getCheckupTime());

		if (!DatagramServiceClient::send(packet)) {
			StringBuffer msg;
			msg << "LOSING on resend (" << packet->getSequence() << ") " << packet->toString();
			debug(msg);
		}

		++resentPackets;

		#ifdef TRACE_CLIENTS
			StringBuffer msg;
			msg << "RESEND(" << packet->getSequence() << ") - " << packet->toString();
			debug(msg);
		#endif
	}
}

void BaseClient::resendPackets(int seq) {
        return;
        
	lock();
	
	int maxPackets = 5;
	
	
	for (int i = 0; i < sequenceBuffer.size(); ++i) {
	        BasePacket* packet = sequenceBuffer.get(i);
	        
	        if (packet->getSequence() != (uint32)seq - 1) {
	                continue;
	        }
	        
	        if (!DatagramServiceClient::send(packet)) {
			StringBuffer msg;
			msg << "LOSING on resend (" << packet->getSequence() << ") " << packet->toString();
			debug(msg);
		}
		
		packet->setTimeout(((BasePacketChekupEvent*)(checkupEvent.get()))->getCheckupTime());

		++resentPackets;
		
		break;
	}
	
	unlock();
	
	return;

	for (int i = 0; i < sequenceBuffer.size(); ++i) {
	
	        if (i >= maxPackets) {
	                break;
	        }
	        
		BasePacket* packet = sequenceBuffer.get(i);

		if (packet->getSequence() == (uint32) seq) {
//			sequenceBuffer.remove(i);

//			delete packet;
			break;
		}

/*		if (packet->getTimeout().isFuture())
			continue;

		packet->setTimeout(((BasePacketChekupEvent*)(checkupEvent.get()))->getCheckupTime());
*/

		if (!DatagramServiceClient::send(packet)) {
			StringBuffer msg;
			msg << "LOSING on resend (" << packet->getSequence() << ") " << packet->toString();
			debug(msg);
		}

		++resentPackets;
	}

	unlock();
}

void BaseClient::balancePacketCheckupTime() {
	setPacketCheckupTime(2000);

	#ifdef TRACE_CLIENTS
		StringBuffer msg;
		msg << "checkup time set to " << 2000;
		debug(msg);
	#endif
}

void BaseClient::resetPacketCheckupTime() {
	setPacketCheckupTime(100);

	#ifdef TRACE_CLIENTS
		StringBuffer msg;
		msg << "checkup time set to " << 100;
		debug(msg);
	#endif
}

void BaseClient::setPacketCheckupTime(uint32 time) {
	lock();

	try {
		#ifdef TRACE_CLIENTS
			StringBuffer msg;
			msg << "changing packet checkup time to " << time;
			debug(msg);
		#endif

		((BasePacketChekupEvent*)(checkupEvent.get()))->setCheckupTime(time);
	} catch (...) {
		disconnect("unreported exception on setPacketCheckupTime()", false);
	}

	unlock();
}

void BaseClient::acknowledgeClientPackets(uint16 seq) {
	lock();

	try {
		if (!isAvailable()) {
			unlock();
			return;
		}

		#ifdef TRACE_CLIENTS
			StringBuffer msg;
			msg << hex << "ACKING READ(" << seq << ")";
			debug(msg);
		#endif

		BasePacket* ack = new AcknowledgeMessage(seq);
		sendPacket(ack, false);
	} catch (SocketException& e) {
		disconnect("acknowledging client packets", false);
	} catch (...) {
		disconnect("unreported exception on acknowledgeClientPackets()", false);
	}

	unlock();
}

void BaseClient::acknowledgeServerPackets(uint16 seq) {
	lock();

	try {
		if (!isAvailable()) {
			unlock();
			return;
		}

		int32 realseq = seq;
		if (seq < acknowledgedServerSequence) {
			realseq = (seq & 0xFFFF) | (serverSequence & 0xFFFF0000);
		}

		if ((uint32)realseq > serverSequence) {
			realseq -= 0x10000;
		}

		#ifdef TRACE_CLIENTS
			StringBuffer msg;
			msg << "ACKNOWLEDGED SEND(" << seq << ") [real = " << realseq << ", ackedseq = " << acknowledgedServerSequence << "]";
			debug(msg);
		#endif
		
		if (realseq < acknowledgedServerSequence) {
		        unlock();
		        return;
		}

		checkupEvent->cancel();

		flushSendBuffer(realseq);
		acknowledgedServerSequence = realseq;

		((BasePacketChekupEvent*)(checkupEvent.get()))->decreaseCheckupTime(1);

		if (!sequenceBuffer.isEmpty()) {
			#ifdef TRACE_CLIENTS
				StringBuffer msg;
				msg << "reschudeling for sequence " << realseq + 1;
				debug(msg);
			#endif

			BasePacket* pack = sequenceBuffer.get(0);

			((BasePacketChekupEvent*)(checkupEvent.get()))->update(pack);
			pack->setTimeout(((BasePacketChekupEvent*)(checkupEvent.get()))->getCheckupTime());

			if (!checkupEvent->isScheduled())
				checkupEvent->scheduleInIoScheduler(pack->getTimeout());
		}
	} catch (ArrayIndexOutOfBoundsException& e) {
		debug("on acknowledgeServerPackets() - " + e.getMessage());
	} catch (...) {
		disconnect("unreported exception on acknowledgeServerPackets()", false);
	}

	unlock();
}

void BaseClient::flushSendBuffer(int seq) {
	while (!sequenceBuffer.isEmpty()) {
		BasePacket* pack = sequenceBuffer.get(0);
		if (pack->getSequence() > (uint32) seq)
			break;

		Time& timestamp = pack->getTimestamp();

		/*#ifdef TRACE_CLIENTS
			StringBuffer msg;
			msg << "deleting packet sequence number " << pack->getSequence() << " ("
				<< timestamp.miliDifference() << " ms)";
			debug(msg);
		#endif*/

		sequenceBuffer.remove(0);
		delete pack;
	}

	acknowledgedServerSequence = seq;

	#ifdef TRACE_CLIENTS
		StringBuffer msg;
		msg << sequenceBuffer.size() << " packet remained in queue";
		debug(msg);
	#endif
}

bool BaseClient::updateNetStatus(uint16 recievedTick) {
	lock();

	try {
		if (!isAvailable()) {
			unlock();
			return false;
		}

		uint16 hostByte = htons(recievedTick);

		if (lastRecievedNetStatusTick != 0) {
			uint16 clientDelta = hostByte - lastRecievedNetStatusTick;
			uint16 serverDelta = lastNetStatusTimeStamp.miliDifference();

			/*StringBuffer msg;
			msg << "recievedTick: " << hostByte << " clientDelta:" << clientDelta << " serverDelta:" << serverDelta;
			debug(msg, true);*/
/*
			if (clientDelta > serverDelta) {
				uint16 difference = clientDelta - serverDelta;

				if ((difference > 200) && (++erroneusTicks > 10)) {
					disconnect("client clock desync", false);

					unlock();

					return false;
				}
			} else
				erroneusTicks = 0;
				
				*/
		}

		lastNetStatusTimeStamp.updateToCurrentTime();
		lastRecievedNetStatusTick = hostByte;

		#ifdef TRACE_CLIENTS
			debug("setting net status");
		#endif

		netcheckupEvent->rescheduleInIoScheduler(NETSTATUSCHECKUP_TIMEOUT);
	} catch (Exception& e) {
		e.printStackTrace();
		disconnect("Exception on updateNetStatus()", false);
	} catch (...) {
		disconnect("unreported exception on updateNetStatus()", false);
	}

	unlock();

	return true;
}

void BaseClient::requestNetStatus() {
	lock();

	try {
		if (!isAvailable()) {
			unlock();

			return;
		}

		uint16 tick = System::random(0xFFF);

		BasePacket* resp = new NetStatusRequestMessage(tick);
		sendPacket(resp, false);

		netRequestEvent->rescheduleInIoScheduler(NETSTATUSREQUEST_TIME);
	} catch (Exception& e) {
		e.printStackTrace();
		disconnect("Exception on requestNetStatus()", false);
	} catch (...) {
		disconnect("unreported exception caught in BaseClient::requestNetStatus()", true);
	}

	unlock();
}

bool BaseClient::checkNetStatus() {
#ifdef VERSION_PUBLIC
	return false;
#endif

	lock();

	try {
		#ifdef TRACE_CLIENTS
			debug("getting net status event");
		#endif

		/*if (!isAvailable()) {
			unlock();
			return false;
		}*/

		error("netStatusTimeout on client");

		setError();
		disconnect(false);
	} catch (Exception& e) {
		disconnect("Exception on checkNetStatus()", false);
		e.printStackTrace();
	} catch (...) {
		disconnect("unreported exception on checkNetStatus()", false);
	}

	unlock();
	return false;
}

class ConnectTask : public Task {
	Reference<BaseClient*> client;

public:
	ConnectTask(BaseClient* cli) {
		client = cli;
	}

	void run() {
		Packet* sreq = new SessionIDRequestMessage();
		client->send(sreq, false);
	}
};

bool BaseClient::connect() {
	try {
		lock();

		#ifdef VERSION_PUBLIC
		SocketAddress addr = ServiceClient::getAddress();
		uint16 port = addr.getPort();

		ServiceClient::setAddress("127.0.0.1", port);
		#endif

		debug("sending session request");

		Reference<Task*> task = new ConnectTask(this);
		task->execute();

		if (crcSeed == 0) {
			Time timeout;
			timeout.addMiliTime(15000);

			debug("waiting for connection established");

			if (connectionEstablishedCondition.timedWait(this, &timeout) != 0) {
				error("connection timeout");

				unlock();
				return false;
			}
		}

		debug("connection established");

		if (!netRequestEvent->isScheduled())
			netRequestEvent->scheduleInIoScheduler(NETSTATUSREQUEST_TIME);

		unlock();
	} catch (...) {
		error("unable to connect");

		unlock();
	}

	return true;
}

void BaseClient::notifyReceivedSeed(uint32 seed) {
	lock();

	crcSeed = seed;
	connectionEstablishedCondition.signal(this);

	unlock();
}

void BaseClient::disconnect(const String& msg, bool doLock) {
	error(msg);

	setError();
	disconnect(doLock);
}

void BaseClient::disconnect(bool doLock) {
	//lock(doLock);
	Locker locker(this);

	if (disconnected) {
		//unlock(doLock);
		return;
	}

	try {
		#ifdef TRACE_CLIENTS
			debug("disconnecting client");
		#endif

		if (!hasError()) {
			if (bufferedPacket != NULL)
				delete bufferedPacket;

			if (!clientDisconnected) {
				BasePacket* disc = new DisconnectMessage(this);
				prepareSend(disc);
				DatagramServiceClient::send(disc);

				delete disc;
			}
		} else {
			debug("kicking client");
		}
	} catch (SocketException& e) {
		error("disconnecting client");
		setError();
	} catch (...) {
		error("unreported exception on disconnect()");
		setError();
	}

	close();

	//unlock(doLock);
	locker.release();

	if (service != NULL) {
		service->removeConnection(this);

		service = NULL;
	}
}

void BaseClient::reportStats(bool doLog) {
	int packetloss;
	if (serverSequence == 0 || resentPackets == 0)
		packetloss = 0;
	else
	 	packetloss = (100 * resentPackets) / (serverSequence + resentPackets);

	//if (packetloss > 15)
	//	doLog = true;

	StringBuffer msg;
	msg << "STATS: sent = " << serverSequence << ", resent = " << resentPackets << " [" << packetloss << "%]";

	if (doLog)
		info(msg);
	else
		debug(msg);
}
