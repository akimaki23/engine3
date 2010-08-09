/*
Copyright (C) 2007 <SWGEmu>. All rights reserved.
Distribution of this file for usage outside of Core3 is prohibited.
*/

#include "BaseMultiPacket.h"

BaseMultiPacket::BaseMultiPacket(BasePacket* pack) : BasePacket() {
	singlePacket = pack;

	setSequencing(true);
}

BaseMultiPacket::~BaseMultiPacket() {
	//if (singlePacket != NULL)
	//	delete singlePacket;
}
	
bool BaseMultiPacket::add(BasePacket* pack) {
	if (singlePacket != NULL) {
		insertShort(0x0900);
		insertShort(0);
		insertShort(0x1900);
		
		insertPacket(singlePacket);
		
		setCompression(singlePacket->doCompression());
		
		delete singlePacket;
		singlePacket = NULL;
	}
	
	if (size() + pack->size() > 460)
		return false;
		
	insertPacket(pack);

	setCompression(doCompression() || pack->doCompression());

	delete pack;
	return true;
}

void BaseMultiPacket::insertPacket(BasePacket* pack) {
	int size = pack->size() - 4;
		
	if (size >= 0xFF) {
		insertByte(0xFF);
		insertShortNet(size);
	} else
		insertByte(size);
		
	insertStream(pack->getBuffer() + 4, size);
}

BasePacket* BaseMultiPacket::getPacket() {
	if (singlePacket != NULL) {
		BasePacket* pack = singlePacket;
		
		delete this;
		return pack;
	} else
		return this;
}
