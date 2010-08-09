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
		virtual void init() {
		}

		virtual void sessionCreated(ServiceSession* session) {
		}

		virtual void sessionOpened(ServiceSession* session) {
		}

		virtual void sessionClosed(ServiceSession* session) {
		}

		virtual void messageReceived(ServiceSession* session, Packet* message) {
		}

		virtual void messageSent(ServiceSession* session, Packet* message) {
		}

		virtual void exceptionCaught(ServiceSession* session, Exception& cause) {
		}
	};

  } // namespace service
} // namespace engine

using namespace engine::service;

#endif /*SERVICEFILTER_H_*/

