/*
Copyright (C) 2007 <SWGEmu>. All rights reserved.
Distribution of this file for usage outside of Core3 is prohibited.
*/

#ifndef SERVICEFILTERCHAIN_H_
#define SERVICEFILTERCHAIN_H_

#include "system/lang.h"

#include "ServiceFilter.h"

#include "ServiceSession.h"

namespace engine {
  namespace service {

	class ServiceFilterChain {
		Vector<ServiceFilter*> filters;

	public:
		ServiceFilterChain() {
		}

		void addFilter(ServiceFilter* filter) {
			filters.add(filter);
		}

		void fireSessionCreated(ServiceSession* session) {
			for (int i = 0; i < filters.size(); ++i) {
				ServiceFilter* filter = filters.get(i);

				filter->sessionCreated(session);
			}
		}

		void fireSessionOpened(ServiceSession* session) {
			for (int i = 0; i < filters.size(); ++i) {
				ServiceFilter* filter = filters.get(i);

				filter->sessionOpened(session);
			}
		}

		void fireSessionClosed(ServiceSession* session) {
			for (int i = 0; i < filters.size(); ++i) {
				ServiceFilter* filter = filters.get(i);

				filter->sessionClosed(session);
			}
		}

		void fireMessageReceived(ServiceSession* session, Packet* message) {
			for (int i = 0; i < filters.size(); ++i) {
				ServiceFilter* filter = filters.get(i);

				filter->messageReceived(session, message);
			}
		}

		void fireMessageSent(ServiceSession* session, Packet* message) {
			for (int i = filters.size() - 1 ; i >= 0; --i) {
				ServiceFilter* filter = filters.get(i);

				filter->messageSent(session, message);
			}
		}

		void fireExceptionCaught(ServiceSession* session, Exception& cause) {
			for (int i = 0; i < filters.size(); ++i) {
				ServiceFilter* filter = filters.get(i);

				filter->exceptionCaught(session, cause);
			}
		}

	};

  } // namespace service
} // namespace engine

using namespace engine::service;

#endif /* SERVICEFILTERCHAIN_H_ */
