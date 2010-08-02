/*
Copyright (C) 2007 <SWGEmu>. All rights reserved.
Distribution of this file for usage outside of Core3 is prohibited.
*/

#ifndef SOCKET_H_
#define SOCKET_H_

#include "../platform.h"

#include "SocketImplementation.h"

namespace sys {
  namespace net {

	class Socket {
		SocketImplementation* socketImpl;

	public:
		Socket(SocketImplementation* socket) {
			socketImpl = socket;
		}

		bool read(Packet* pack);

		void read(Packet* pack, SocketAddress* addr);

		void send(Packet* pack);

		void send(Packet* pack, const SocketAddress& addr);
	};


  } // namespace net
} // namespace sys

using namespace sys::net;

#endif /*SOCKET_H_*/
