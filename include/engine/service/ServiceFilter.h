/*
Copyright (C) 2007 <SWGEmu>. All rights reserved.
Distribution of this file for usage outside of Core3 is prohibited.
*/

#ifndef SERVICEFILTER_H_
#define SERVICEFILTER_H_

#include "system/lang.h"

#include "ServiceSession.h"

namespace engine {
  namespace service {

	class ServiceFilter {
	public:
		void init() {
		}

		void sessionCreated(ServiceSession* session) {
		}

		void sessionOpened(ServiceSession* session) {
		}

		void sessionClosed(ServiceSession* session) {
		}

		void messageReceived(ServiceSession* session, Packet* message) {
		}

		void messageSent(ServiceSession* session, Packet* message) {
		}

		void exceptionCaught(ServiceSession* session, Exception& cause) {
		}
	};

  } // namespace service
} // namespace engine

using namespace engine::service;

#endif /*SERVICEFILTER_H_*/

