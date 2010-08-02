/*
Copyright (C) 2007 <SWGEmu>. All rights reserved.
Distribution of this file for usage outside of Core3 is prohibited.
*/

#ifndef SERVICEHANDLER_H_
#define SERVICEHANDLER_H_

#include "system/lang.h"

namespace engine {
  namespace service {

	class ServiceSession;

	class ServiceHandler {
	public:
		virtual void sessionCreated(ServiceSession* session) {
		}

		virtual void sessionOpened(ServiceSession* session) {
		}

		virtual void sessionClosed(ServiceSession* session) {
		}

		virtual void messageReceived(ServiceSession* session, Packet* message) {
		}

		virtual void exceptionCaught(ServiceSession* session, Exception& e) {
		}
	};

  } // namespace service
} // namespace engine

using namespace engine::service;

#endif /* SERVICESHANDLER_H_ */
