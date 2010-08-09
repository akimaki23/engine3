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
		ServiceFilterChain* filterChain;

	public:
		FilteringServiceHandler() {
			filterChain = NULL;
		}

		void sessionCreated(ServiceSession* session) {
			if (filterChain != NULL)
				filterChain->fireSessionCreated(session);
		}

		void sessionOpened(ServiceSession* session) {
			if (filterChain != NULL)
				filterChain->fireSessionOpened(session);
		}

		void sessionClosed(ServiceSession* session) {
			if (filterChain != NULL)
				filterChain->fireSessionClosed(session);
		}

		void messageReceived(ServiceSession* session, Packet* message) {
			if (filterChain != NULL)
				filterChain->fireMessageReceived(session, message);
		}

		void exceptionCaught(ServiceSession* session, Exception& cause) {
			if (filterChain != NULL)
				filterChain->fireExceptionCaught(session, cause);
		}

		void setFilterChain(ServiceFilterChain* chain) {
			if (filterChain != NULL)
				filterChain = chain;
		}
	};

  } // namespace service
} // namespace engine

using namespace engine::service;

#endif /* FILTERINGSERVICEHANDLER_H_ */
