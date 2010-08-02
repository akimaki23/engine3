/*
Copyright (C) 2007 <SWGEmu>. All rights reserved.
Distribution of this file for usage outside of Core3 is prohibited.
*/

#ifndef PACKETHANDLER_H_
#define PACKETHANDLER_H_

namespace engine {
  namespace proto {
    namespace rudp {

	class PacketHandler {
	public:
		virtual void handlePacket(Packet* pack) = 0;
	};

    } // namespace rudp
  } // namespace proto
} // namespace engine

using namespace engine::proto::rudp;

#endif /*PACKETHANDLER_H_*/
