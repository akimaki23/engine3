/*
Copyright (C) 2007 <SWGEmu>. All rights reserved.
Distribution of this file for usage outside of Core3 is prohibited.
*/

#include "BaseFragmentedPacket.h"

#include "../../log/Logger.h"

BaseFragmentedPacket::BaseFragmentedPacket() : BasePacket() {
	singlePacket = NULL;

	offset = 0;
}

BaseFragmentedPacket::BaseFragmentedPacket(BasePacket* pack) : BasePacket() {
	singlePacket = pack;

	offset = 0;
}

BaseFragmentedPacket::~BaseFragmentedPacket() {
	if (singlePacket != NULL)
		delete singlePacket;
}

void BaseFragmentedPacket::addFragment(Packet* pack) {
	uint32 seq = pack->parseNetShort(2);

	if (offset == 0) {
		offset = pack->parseNetInt(4);

		insertStream(pack->getBuffer() + 8, 496 - 8 - 3);

		/*Logger::console.info("received first segment of fragmented packet ("
				+ String::valueOf(seq) + ") - size = " + String::valueOf(offset));*/
	} else {
		int fragsize = MIN(496, pack->size()) - 4 - 3;

		offset -= 3;

		insertStream(pack->getBuffer() + 4, fragsize);

		/*Logger::console.info("received next segment of fragmented packet ("
				+ String::valueOf(seq) + ") - size = " + String::valueOf(fragsize));*/
	}
}

BasePacket* BaseFragmentedPacket::getFragment() {
	BasePacket* frag = new BasePacket();
	frag->setCompression(true);

	frag->insertShort(0x0D00);
	frag->insertShort(0x0000);

	int fragsize;

	if (offset == 0) {
		frag->insertIntNet(singlePacket->size() - 4);

		offset = 4;
		fragsize = 496 - 8 - 3;
	} else {
		fragsize = MIN(496 - 4 - 3, singlePacket->size() - offset);
	}

	frag->insertStream(singlePacket->getBuffer() + offset, fragsize);
	offset += fragsize;

	//System::out << "fragged packet: " << frag->toString() << "\n";

	return frag;
}

bool BaseFragmentedPacket::isComplete() {
	int currentSize = size();

	/*Logger::console.info("checking fragmented packet completeness: " + String::valueOf(currentSize)
			+ " = " + String::valueOf(offset));*/

	if (currentSize < offset)
		return false;
	else if (currentSize == offset)
		return true;
	else
		throw Exception("fragmented packet exceeded size (" + String::valueOf(offset)
				+ ") - size = " + String::valueOf(currentSize));
}

bool BaseFragmentedPacket::hasFragments() {
	return offset < singlePacket->size();
}
