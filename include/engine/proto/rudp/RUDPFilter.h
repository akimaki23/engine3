/*
Copyright (C) 2007 <SWGEmu>. All rights reserved.
Distribution of this file for usage outside of Core3 is prohibited.
*/

#ifndef RUDPFILTER_H_
#define RUDPFILTER_H_

#include "system/lang.h"

namespace engine {
  namespace proto {
    namespace rudp {

    class RUDPFilter {
/*
 * void BaseProtocol::prepareSend(BasePacket* pack) {
	if (pack->doCompression())
		pack->setCompression(false);

	pack->close();

	StringBuffer msg;
	msg << "SEND - " << pack->toString();
	info(msg);

	if (pack->doSequencing())
		pack->setSequence(serverSequence++);

	if (pack->doCompression()) {
		compress(pack);
	}

	if (pack->doEncryption()) {
		encrypt(pack, true);
	}

	if (pack->doCRCChecking()) {
		appendCRC(pack);
	}

}

bool BaseProtocol::processRecieve(Packet* pack) {
	if (!testCRC(pack)) {
		StringBuffer msg;
		msg << "incorrect CRC\n" << pack->toString() << "\n";
		error(msg);

		return false;
	}

	decrypt(pack);

	if (pack->size() < 3)
		throw PacketIndexOutOfBoundsException(pack, 3);

	if (pack->parseByte(pack->size() - 3) == 0x01) {
		decompress(pack);
	}

	pack->removeLastBytes(3);

	return true;
}

 */
    };

    } // namespace rudp
  } // namespace proto
} // namespace engine

using namespace engine::proto::rudp;

#endif /* RUDPFILTER_H_ */
