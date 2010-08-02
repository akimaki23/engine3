/*
Copyright (C) 2007 <SWGEmu>. All rights reserved.
Distribution of this file for usage outside of Core3 is prohibited.
*/

#ifndef ABSTRACTPROTOCOL_H_
#define ABSTRACTPROTOCOL_H_

#include "system/lang.h"

#include "engine/service/Message.h"

namespace engine {
  namespace proto {
    namespace rudp {

	class AbstractProtocol {
	public:
		virtual void prepareSend(Packet* pack) = 0;
	
		virtual bool processRecieve(Packet* pack) = 0;
	};
	
    } // namespace rudp
  } // namespace proto
} // namespace engine

using namespace engine::proto::rudp;

#endif /*ABSTRACTPROTOCOL_H_*/
