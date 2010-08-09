/*
Copyright (C) 2007 <SWGEmu>. All rights reserved.
Distribution of this file for usage outside of Core3 is prohibited.
*/

#ifndef ENGINE_H_
#define ENGINE_H_

#include "system/lang.h"

#include "core/Core.h"

#include "core/Task.h"
#include "core/ReentrantTask.h"
#include "core/TaskManager.h"

#include "core/ManagedReference.h"
#include "core/ManagedWeakReference.h"
#include "core/ManagedObject.h"

//#include "core/util/ManagedVector.h"
//#include "core/util/ManagedVectorImplementation.h"
#include "core/util/ManagedSortedVector.h"
#include "core/util/ManagedVectorMap.h"

#include "log/Logger.h"

#include "service/ServiceException.h"
#include "service/ServiceProcessThread.h"
#include "service/ServiceSession.h"
#include "service/DatagramAcceptor.h"
#include "service/DatagramConnector.h"
#include "service/DatagramSession.h"
#include "service/StreamAcceptor.h"
#include "service/StreamConnector.h"
#include "service/StreamSession.h"

#include "service/Message.h"

#include "proto/XorEncryptionFilter.h"
#include "proto/ZipCompressionFilter.h"
#include "proto/rudp/RUDPProtocol.h"
#include "proto/rudp/RUDPPacket.h"
#include "proto/rudp/BaseMessage.h"
#include "proto/rudp/StandaloneBaseMessage.h"

#include "orb/DistributedObjectBroker.h"
#include "orb/DOBObjectManager.h"
#include "orb/DOBObjectManagerImplementation.h"

#include "stm/TransactionalMemoryManager.h"

#ifndef PLATFORM_WIN
#include "db/Database.h"
#include "db/ObjectDatabase.h"
#include "db/ObjectDatabaseManager.h"
#include "db/BinaryData.h"
#include "db/mysql/MySqlDatabase.h"
#include "db/berkley/BerkeleyDatabase.h"

#include "lua/Lua.h"
#endif

#include "util/QuadTree.h"
#include "util/Quaternion.h"
#include "util/Vector3.h"
#include "util/iffstream/IffStream.h"


#endif /*ENGINE_H_*/
