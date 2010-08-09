# Copyright (C) 2007 <SWGEmu>. All rights reserved.
# Distribution of this file for usage outside of Core3 is prohibited.

if DEBUG
AM_CXXFLAGS =-ggdb -Wall -Wextra -Wno-unused
else
AM_CXXFLAGS =-g0 -Wall -Wextra -Wno-unused -DVERSION_PUBLIC
endif

INCLUDES =  $(SYSTEM_INCLUDES) \
	    	-I$(USER_INCLUDES_PATH)/mysql5/mysql -I$(USER_INCLUDES_PATH)/mysql -I$(USER_INCLUDES_PATH)/lua -I/usr/local/BerkeleyDB.4.8/include -I$src/

LIBS =  $(SYSTEM_LIBS) \
		-L/usr/local/BerkeleyDB.4.8/lib \
		-lmysqlclient -llua -lz -ldb \
		-lengine3

noinst_LIBRARIES = libengine3.a

libengine3_a_SOURCES = \
			system/lang.h \
			system/lang/System.cpp \
			system/lang/Exception.cpp \
			system/lang/Object.cpp \
			system/lang/String.cpp \
			system/lang/StringBuffer.cpp \
			system/lang/UnicodeString.cpp \
			system/lang/Math.cpp \
			system/lang/StackTrace.cpp \
			system/lang/types.cpp \
			system/lang/Integer.cpp \
			system/lang/ArrayIndexOutOfBoundsException.cpp \
			system/thread/Thread.cpp \
			system/thread/Lockable.cpp \
			system/thread/Mutex.cpp \
			system/thread/ReadWriteLock.cpp \
			system/net/SocketImplementation.cpp \
			system/net/SocketAddress.cpp \
			system/net/InetAddress.cpp \
			system/net/NetworkInterface.cpp \
			system/net/Packet.cpp \
			system/io/Stream.cpp \
			system/io/File.cpp \
			system/io/BufferedReader.cpp \
			system/io/StringTokenizer.cpp \
			system/io/UnicodeTokenizer.cpp \
			system/io/PrintStream.cpp \
			system/io/Serializable.cpp \
			system/util/PriorityQueue.cpp \
			engine/engine.h \
			engine/core/Task.cpp \
			engine/core/TaskManager.cpp \
			engine/core/TaskScheduler.cpp \
			engine/core/TaskQueue.cpp \
			engine/core/TimedTaskQueue.cpp \
			engine/core/TaskWorkerThread.cpp \
			engine/core/ObjectUpdateToDatabaseTask.cpp \
			engine/service/ServiceSession.cpp \
			engine/service/ServiceThread.cpp \
			engine/service/AbstractServiceAcceptor.cpp \
			engine/service/ServiceProcessThread.cpp \
			engine/service/DatagramConnector.cpp \
			engine/service/DatagramSession.cpp \
			engine/service/DatagramAcceptor.cpp \
			engine/service/StreamConnector.cpp \
			engine/service/StreamAcceptor.cpp \
			engine/service/MessageQueue.cpp \
			engine/proto/ZipCompressionFilter.cpp \
			engine/proto/CrcCheckingFilter.cpp \
			engine/proto/rudp/RUDPFilter.cpp \
			engine/proto/rudp/RUDProtocol.cpp \
			engine/proto/rudp/RDPPacket.cpp \
			engine/proto/rudp/BaseMultiPacket.cpp \
			engine/proto/rudp/BaseFragmentedPacket.cpp \
			engine/proto/rudp/BasePacketHandler.cpp \
			engine/proto/rudp/BaseClientProxy.cpp \
			engine/proto/rudp/events/BasePacketChekupEvent.cpp \
			engine/orb/DistributedObjectBroker.cpp \
			engine/orb/DistributedObjectDirectory.cpp \
			engine/orb/DOBObjectManager.cpp \
			engine/orb/DOBObjectManagerImplementation.cpp \
			engine/orb/NamingDirectoryService.cpp \
			engine/orb/NamingDirectoryServiceImpl.cpp \
			engine/orb/DistributedObjectBrokerClient.cpp \
			engine/orb/DOBPacketHandler.cpp \
			engine/orb/object/DistributedObject.cpp \
			engine/orb/object/DistributedObjectServant.cpp \
			engine/orb/object/DistributedObjectStub.cpp \
			engine/orb/object/DistributedMethod.cpp \
			engine/stm/Transaction.cpp \
			engine/stm/TransactionalMemoryManager.cpp \
			engine/core/ManagedObject.cpp \
			engine/core/ManagedObjectImplementation.cpp \
			engine/util/QuadTree.cpp \
			engine/util/iffstream/IffStream.cpp \
			engine/util/iffstream/chunks/Chunk.cpp \
			engine/util/iffstream/chunks/form/Form.cpp \
			engine/db/BinaryData.cpp \
			engine/db/ObjectDatabase.cpp \
			engine/db/ObjectDatabaseManager.cpp \
			engine/db/mysql/MySqlDatabase.cpp \
			engine/db/berkley/BerkeleyDatabase.cpp \
			engine/db/berkley/Transaction.cpp \
			engine/db/berkley/DatabaseEntry.cpp \
			engine/db/berkley/Environment.cpp \
			engine/db/berkley/Cursor.cpp \
			engine/log/Logger.cpp \
			engine/lua/Lua.cpp \
			engine/lua/LuaFunction.cpp \
			engine/lua/LuaObject.cpp

.h.gch:
	$(CXXCOMPILE) $<
			