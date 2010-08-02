/*
Copyright (C) 2007 <SWGEmu>. All rights reserved.
Distribution of this file for usage outside of Core3 is prohibited.
*/

#ifndef SERVICECLIENT_H_
#define SERVICECLIENT_H_

#include "system/lang.h"

namespace engine {
  namespace service {

	class ServiceSession : public virtual Object {
	protected:
		SocketImplementation* socket;

		SocketAddress address;

		bool hasError, disconnected;

		int packetLossChance;

	public:
		ServiceSession();

		virtual ~ServiceSession();

		void bind(SocketImplementation* sock, const SocketAddress& addr);

		virtual void finalize();

		virtual void acquire();

		virtual void release();

		bool isAvailable();

		inline bool isDisconnected() {
			return disconnected;
		}

		// getters
		SocketImplementation* getSocket() {
			return socket;
		}

		virtual uint64 getNetworkID() {
			return 0;//= 0;
		}

		const SocketAddress& getAddress() {
			return address;
		}

	    String getIPAddress() {
	    	return address.getIPAddress();
	    }

		// setters
		inline void setError() {
			hasError = true;
		}

		inline void setPacketLoss(int ratio) {
			packetLossChance = ratio;
		}
	};

  } // namespace service
} // namespace engine

using namespace engine::service;

#endif /*SERVICECLIENT_H_*/
