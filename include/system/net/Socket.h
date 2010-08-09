/*
Copyright (C) 2007 <SWGEmu>. All rights reserved.
Distribution of this file for usage outside of Core3 is prohibited.
*/

#ifndef SOCKET_H_
#define SOCKET_H_

#include "../platform.h"

#include "SocketAddress.h"

namespace sys {
  namespace net {
	
	class Packet; 
	
	class Socket {
	protected:
		int sock;
		
		uint64 timeout;
		struct timeval tv;
	
	public:
		Socket();
		Socket(int handle);

		static void initialize();

		void bindTo(SocketAddress* bindpoint);
	
		void listen(int maxconn);
	
		Socket* accept();
	
		Socket* accept(SocketAddress* addr);
		
		void connect(SocketAddress* bindpoint);
	
		bool recieve(Packet* pack);

		bool recieveFrom(Packet* pack, SocketAddress* addr);
	
		bool read(Packet* pack);
	
		void readFrom(Packet* pack, SocketAddress* addr);
	
		void send(Packet* pack);
	
		void sendTo(Packet* pack, SocketAddress* addr);
		
		void close();
		
		bool hasData();

		// setters
		void setBlocking(bool b);

		void setLingering(int time);

		void disableLingering();

		void updateTimeOut();
		
		inline void setTimeOut(uint64 time) {
			timeout = time;
		}

#ifdef PLATFORM_WIN
	private:
		static bool winsockInitialized;
#endif

	};

  } // namespace net
} // namespace sys

using namespace sys::net;

#endif /*SOCKET_H_*/
