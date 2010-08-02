/*
Copyright (C) 2007 <SWGEmu>. All rights reserved.
Distribution of this file for usage outside of Core3 is prohibited.
*/

#ifndef FILTERINGSERVICEHANDLER_H_
#define FILTERINGSERVICEHANDLER_H_

#include "system/lang.h"

#include "ServiceHandler.h"

#include "ServiceFilterChain.h"

namespace engine {
  namespace service {

	class FilteringServiceHandler : public ServiceHandler {
		ServiceHandler* adaptedHandler;

		ServiceFilterChain* filterChain;

	public:
		FilteringServiceHandler() {
			adaptedHandler = NULL;

			filterChain = NULL;
		}

		void sessionCreated(ServiceSession* session) {
			filterChain->fireSessionCreated(session);

			adaptedHandler->sessionCreated(session);
		}

		void sessionOpened(ServiceSession* session) {
			filterChain->fireSessionOpened(session);

			adaptedHandler->sessionOpened(session);
		}

		void sessionClosed(ServiceSession* session) {
			filterChain->fireSessionClosed(session);

			adaptedHandler->sessionClosed(session);
		}

		void messageReceived(ServiceSession* session, Packet* message) {
			filterChain->fireMessageReceived(session, message);

			adaptedHandler->messageReceived(session, message);
		}

		void exceptionCaught(ServiceSession* session, Exception& cause) {
			filterChain->fireExceptionCaught(session, cause);

			adaptedHandler->exceptionCaught(session, cause);
		}

		void setHandler(ServiceHandler* handler) {
			adaptedHandler = handler;
		}

		void setFilterChain(ServiceFilterChain* chain) {
			filterChain = chain;
		}
	};

  } // namespace service
} // namespace engine

using namespace engine::service;

#endif /* FILTERINGSERVICEHANDLER_H_ */
