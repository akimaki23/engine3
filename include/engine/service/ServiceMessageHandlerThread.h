/*
Copyright (C) 2007 <SWGEmu>. All rights reserved.
Distribution of this file for usage outside of Core3 is prohibited.
*/

#ifndef SERVICEMESSAGEHANDLERTHREAD_H_
#define SERVICEMESSAGEHANDLERTHREAD_H_

#include "ServiceThread.h"

#include "ServiceSession.h"
#include "ServiceSessionMap.h"

#include "ServiceHandler.h"

#include "MessageQueue.h"

namespace engine {
  namespace service {
  	
	class AbstractServiceAcceptor : public ServiceThread {
	protected:
		ServiceSession* session;

		ServiceSessionMap* sessions;
		
		ServiceHandler* handler;

		MessageQueue messageQueue;
		
	public:
		AbstractServiceAcceptor(const String& s);
		
		virtual ~AbstractServiceAcceptor();
	
		virtual bool deleteConnection(ServiceSession* client);

		// message functions
		inline void addMessage(Message* msg) {
			messageQueue.push(msg);
		}
		
		inline Message* getMessage() {
			return messageQueue.pop();
		}

		inline void flushMessages() {
			messageQueue.flush();
		}
		
		// getters
		inline ServiceSession* getSession() {
			return session;
		}

		inline ServiceHandler* getHandler() {
			return handler;
		}

		inline MessageQueue* getMessageQueue() {
			return &messageQueue;
		}

		inline void setHandler(ServiceHandler* hand) {
			handler = hand;
		}
	};

  } // namespace service
} // namespace engine

using namespace engine::service;

#endif /*SERVICEMESSAGEHANDLERTHREAD_H_*/
