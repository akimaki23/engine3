/*
Copyright (C) 2007 <SWGEmu>. All rights reserved.
Distribution of this file for usage outside of Core3 is prohibited.
*/

#ifndef STREAMSERVICECLIENTPROXY_H_
#define STREAMSERVICECLIENTPROXY_H_

#include "StreamServiceClient.h"

namespace engine {
  namespace service {

	class StreamSession : public StreamConnector {
		StreamSocket* socket;
		SocketAddress address;

	public:
		StreamSession(SocketImplementation* sock) : StreamConnector() {
			socket = new StreamSocket(sock);

			doRun = true;
		}

		StreamSession(SocketImplementation* sock, SocketAddress& addr) : StreamConnector() {
			socket = new StreamSocket(sock);
			address = addr;

			doRun = true;
		}

		virtual ~StreamSession() {
		}

	};

  } // namespace service
} // namespace engine

using namespace engine::service;

#endif /*STREAMSERVICECLIENTPROXY_H_*/
