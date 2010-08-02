/*
Copyright (C) 2007 <SWGEmu>. All rights reserved.
Distribution of this file for usage outside of Core3 is prohibited.
*/

#ifndef DISTRIBUTEDOBJECTNAMEMAP_H_
#define DISTRIBUTEDOBJECTNAMEMAP_H_

#include "system/lang.h"

#include "DistributedObjectStub.h"

namespace engine {
  namespace ORB {

	class DistributedObjectNameMap : public HashTable<String, DistributedObjectStub*> {
		int hash(const String& key) {
	        return key.hashCode();
		}

	public:
		DistributedObjectNameMap() : HashTable<String, DistributedObjectStub*>(100) {
			setNullValue(NULL);
		}

	};

  } // namespace ORB
} // namespace engine

using namespace engine::ORB;

#endif /*DISTRIBUTEDOBJECTNAMEMAP_H_*/
