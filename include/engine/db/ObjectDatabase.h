#ifndef ENGINE_DB_OBJECTDATABASE_H_
#define ENGINE_DB_OBJECTDATABASE_H_

#include "system/lang.h"

#include "engine/log/Logger.h"

#include "berkley/BerkeleyDatabase.h"

namespace engine {
  namespace db {

	class ObjectDatabaseManager;

	class ObjectDatabase : public Logger {
		engine::db::berkley::BerkeleyDatabase* objectsDatabase;
		engine::db::berkley::Environment* environment;

		String databaseFileName;

		Mutex writeLock;

	private:
		void closeDatabase();
		void openDatabase();

		ObjectDatabase() {
		}

	public:
		ObjectDatabase(ObjectDatabaseManager* dbEnv, const String& dbFileName);
		~ObjectDatabase();

		const static int DEADLOCK_MAX_RETRIES = 1000;

		int getData(uint64 objKey, ObjectInputStream* objectData);
		int putData(uint64 objKey, ObjectOutputStream* stream, Object* obj);
		int deleteData(uint64 objKey);

		int tryPutData(uint64 objKey, Stream* stream, engine::db::berkley::Transaction* transaction);
		int tryDeleteData(uint64 objKey, engine::db::berkley::Transaction* transaction);

		int sync();

		inline engine::db::berkley::BerkeleyDatabase* getDatabaseHandle() {
			return objectsDatabase;
		}

		inline void getDatabaseName(String& name) {
			name = databaseFileName.replaceFirst(".db", "");
		}

	};

	class ObjectDatabaseIterator : public Logger {
		engine::db::berkley::Cursor* cursor;
		engine::db::berkley::BerkeleyDatabase* databaseHandle;

		engine::db::berkley::DatabaseEntry key, data;

	public:
		ObjectDatabaseIterator(ObjectDatabase* database, bool useCurrentThreadTransaction = false);
		ObjectDatabaseIterator(engine::db::berkley::BerkeleyDatabase* databaseHandle);
		~ObjectDatabaseIterator();

		void resetIterator();

		bool getNextKeyAndValue(uint64& key, ObjectInputStream* data);
		bool getNextValue(ObjectInputStream* data);
		bool getNextKey(uint64& key);

		inline void closeCursor() {
			if (cursor != NULL) {
				cursor->close();

				delete cursor;
			}

			cursor = NULL;
		}
	};

  } // namespace db
} // namespace engine

#endif /* ENGINE_DB_OBJECTDATABASE_H_ */
