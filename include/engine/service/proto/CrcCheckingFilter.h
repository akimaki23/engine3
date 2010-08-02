/*
Copyright (C) 2007 <SWGEmu>. All rights reserved.
Distribution of this file for usage outside of Core3 is prohibited.
*/

#ifndef CRCCHECKINGFILTER_H_
#define CRCCHECKINGFILTER_H_

#include "system/lang.h"

#include "engine/service/ServiceFilter.h"

#include "RUDPPacket.h"

namespace engine {
  namespace service {
    namespace proto {

	class CrcCheckingFilter : public ServiceFilter {
		unsigned int crcSeed;

	public:
		CrcCheckingFilter() {
		}

		void messageSent(ServiceSession* session, Packet* message) {
			BasePacket* pack = (BasePacket*) message;

			if (pack->doCRCChecking()) {
				appendCRC(pack);
			}
		}

		void messageReceived(ServiceSession* session, Packet* message) {
			if (!testCRC(message)) {
				throw Exception("CRC error");
			}
		}

		void setSeed(uint32 seed) {
			crcSeed = seed;
		}

	protected:
		void appendCRC(Packet* pack, sys::uint16 crcLength = 2);

		bool testCRC(Packet* pack, sys::uint16 crcLength = 2);

		unsigned int generateCrc(Packet* pack, int len);

	};

    } // namespace proto
  } // namespace service
} // namespace engine

using namespace engine::service::proto;

#endif /* CRCCHECKINGFILTER_H_ */
