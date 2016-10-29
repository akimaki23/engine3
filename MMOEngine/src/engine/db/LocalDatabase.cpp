/*
 * LocalDatabase.cpp
 *
 *  Created on: 11/11/2010
 *      Author: victor
 */

#include <zlib.h>

#include "LocalDatabase.h"
#include "DatabaseManager.h"
#include "ObjectDatabaseManager.h"

using namespace engine::db;
using namespace engine::db::berkley;

LocalDatabase::LocalDatabase(DatabaseManager* dbEnv, const String& dbFileName, bool compression) {
	environment = dbEnv->getBerkeleyEnvironment();

	setLoggingName("LocalDatabase " + dbFileName);
	setGlobalLogging(true);
	setInfoLogLevel();

	databaseFileName = dbFileName;

	objectsDatabase = NULL;

	this->compression = compression;

	//setFileLogger("log/berkeley.log");

	openDatabase();
}

LocalDatabase::~LocalDatabase() {
	closeDatabase();

	delete objectsDatabase;
	objectsDatabase = NULL;
}

void LocalDatabase::openDatabase() {
	DatabaseConfig config;
	config.setThreaded(true);
	//config.setTransactional(true);
	config.setAllowCreate(true);
	config.setType(DatabaseType::HASH);
	config.setReadUncommited(true);

	LocalDatabase::openDatabase(config);
}

int LocalDatabase::sync() {
	objectsDatabase->sync();

	return 0;
}

int LocalDatabase::truncate() {
	try {
		objectsDatabase->truncate(NULL, false);

		info("database truncated");

	} catch (Exception &e) {
		error("Error truncating database (" + databaseFileName + "):");
		error(e.getMessage());
	} catch (...) {
		error("unreported exception caught while trying to truncate berkeley DB ");
	}

	return 0;
}

void LocalDatabase::openDatabase(const DatabaseConfig& config) {
	try {

		Transaction* transaction = environment->beginTransaction(NULL);

		objectsDatabase = environment->openDatabase(transaction, databaseFileName, "", config);

		int ret = 0;

		if ((ret = transaction->commit()) != 0) {
			error(String::valueOf(db_strerror(ret)));
		}


	} catch (Exception& e) {
		error(e.getMessage());
		exit(1);
	}
}

void LocalDatabase::closeDatabase() {
	try {

		objectsDatabase->close(true);

		info("database closed");

	} catch (Exception &e) {
		error("Error closing database (" + databaseFileName + "):");
		error(e.getMessage());
	} catch (...) {
		error("unreported exception caught while trying to open berkeley DB ");
	}
}


Stream* LocalDatabase::compress(Stream* data) {
	char outputData[CHUNK_SIZE];// = (char*) malloc(CHUNK_SIZE);
	Stream* outputStream  = new Stream();

	try {
		z_stream packet;
		packet.zalloc = Z_NULL;
		packet.zfree = Z_NULL;
		packet.opaque = Z_NULL;
		packet.avail_in = 0;
		packet.next_in = Z_NULL;
		deflateInit(&packet, Z_DEFAULT_COMPRESSION);
		packet.next_in = (Bytef* )(data->getBuffer());
		packet.avail_in = data->size();

		do {
			packet.next_out = (Bytef* )outputData;
			packet.avail_out = CHUNK_SIZE;

			int ret = deflate(&packet,  Z_FINISH);

			assert(ret == Z_OK || ret == Z_STREAM_END);

			int wrote = CHUNK_SIZE - packet.avail_out;

			outputStream->writeStream(outputData, wrote);

		} while (packet.avail_out == 0);

		deflateEnd(&packet);
	} catch (...) {
		assert(0 && "LocalDatabase::compress");
	}

	//printf("%d uncompressed -> %d compressed\n", data->size(), outputStream->size());

	//free(outputData);

	return outputStream;
}

void LocalDatabase::uncompress(void* data, uint64 size, ObjectInputStream* decompressedData) {
	char outputData[CHUNK_SIZE];
	//char* outputData = (char*) malloc(CHUNK_SIZE);

	try {
		z_stream packet;
		packet.zalloc = Z_NULL;
		packet.zfree = Z_NULL;
		packet.opaque = Z_NULL;
		packet.avail_in = 0;
		packet.next_in = Z_NULL;
		inflateInit(&packet);
		packet.next_in = (Bytef* )(data);
		packet.avail_in = (size);

		do {
			packet.next_out = (Bytef* )outputData;
			packet.avail_out = CHUNK_SIZE;

			int ret = inflate(&packet, Z_NO_FLUSH);

			if (ret == Z_DATA_ERROR) {
				Logger::console.error("could not decompress stream from database returning uncompressed");

				decompressedData->writeStream((char*)data, size);

				inflateEnd(&packet);

				return;
			}

			assert(ret == Z_OK || ret == Z_STREAM_END);

			int wrote = CHUNK_SIZE - packet.avail_out;

			decompressedData->writeStream(outputData, wrote);
			//avail_out
		} while (packet.avail_out == 0);

		inflateEnd(&packet); //close buffer*/
	} catch (...) {
		assert(0 && "LocalDatabase::uncompress");
	}

	//free(outputData);
}

int LocalDatabase::getData(Stream* inputKey, ObjectInputStream* objectData) {
	int ret = 0;

	DatabaseEntry key, data;

	key.setData(inputKey->getBuffer(), inputKey->size());

	int i = 0;

	Transaction* transaction = NULL;

	do {
		ret  = -1;
		transaction = environment->beginTransaction(NULL);

		ret = objectsDatabase->get(transaction, &key, &data, LockMode::READ_UNCOMMITED);

		if (ret == DB_LOCK_DEADLOCK) {
			info("deadlock detected in LocalDatabase::get.. retrying iteration " + String::valueOf(i));

			transaction->abort();
			transaction = NULL;
		}

		++i;
	} while (ret == DB_LOCK_DEADLOCK && i < DEADLOCK_MAX_RETRIES);

	if (ret == 0) {
		if (!compression) {
			objectData->writeStream((const char*) data.getData(), data.getSize());
		} else {
			uncompress(data.getData(), data.getSize(), objectData);
		}

		objectData->reset();
	} else if (ret != DB_NOTFOUND) {
		error("error in LocalDatabase::getData ret " + String::valueOf(db_strerror(ret)));

		if (transaction != NULL)
			transaction->abort();

		throw DatabaseException("error in LocalDatabase::getData ret " + String(db_strerror(ret)));
	}

	if (transaction != NULL) {
		transaction->commit();
	}

	return ret;
}

//stream will be deleted
int LocalDatabase::putData(Stream* inputKey, Stream* stream, engine::db::berkley::Transaction* masterTransaction) {
	ObjectDatabaseManager* databaseManager = ObjectDatabaseManager::instance();

	CurrentTransaction* transaction = databaseManager->getCurrentTransaction();

	uint64 currentSize = transaction->addUpdateObject(inputKey, stream, this, NULL);

	if (currentSize > DatabaseManager::MAX_CACHE_SIZE) {
		ObjectDatabaseManager::instance()->commitLocalTransaction(masterTransaction);
	}

	return 0;
}

int LocalDatabase::deleteData(Stream* inputKey, engine::db::berkley::Transaction* masterTransaction) {
	ObjectDatabaseManager* databaseManager = ObjectDatabaseManager::instance();

	CurrentTransaction* transaction = databaseManager->getCurrentTransaction();

	uint64 currentSize = transaction->addDeleteObject(inputKey, this);

	/*StringBuffer msg;
		msg << "added to deleteData objid" << hex << objKey;
		info(msg);*/

	if (currentSize > DatabaseManager::MAX_CACHE_SIZE) {
		ObjectDatabaseManager::instance()->commitLocalTransaction(masterTransaction);
	}

	return 0;
}

void LocalDatabase::compressDatabaseEntries(engine::db::berkley::Transaction* transaction) {
	if (compression)
		return;

	HashSet<uint64> compressed;

	LocalDatabaseIterator iterator(transaction, this);

	ObjectInputStream keyStream;
	ObjectInputStream data;

	while (iterator.getNextKeyAndValue(&keyStream, &data)) {
		compression = true;

		ObjectOutputStream* key = new ObjectOutputStream();
		keyStream.copy(key);
		key->reset();

		ObjectOutputStream* dataNew = new ObjectOutputStream();
		data.copy(dataNew);
		dataNew->reset();

		putData(key, dataNew, transaction);
		//assert(iterator.putCurrent(dataNew) == 0);

		compression = false;

		keyStream.clear();
		data.clear();
	}

	compression = true;
}

int LocalDatabase::tryPutData(Stream* inputKey, Stream* stream, engine::db::berkley::Transaction* transaction) {
	int ret = -1;

	DatabaseEntry key, data;
	key.setData(inputKey->getBuffer(), inputKey->size());

	if (!compression) {
		data.setData(stream->getBuffer(), stream->size());

		ret = objectsDatabase->put(transaction, &key, &data);
	} else {
		Stream* compressed = compress(stream);

		data.setData(compressed->getBuffer(), compressed->size());

		ret = objectsDatabase->put(transaction, &key, &data);

		delete compressed;
	}

	return ret;
}

int LocalDatabase::tryDeleteData(Stream* inputKey, engine::db::berkley::Transaction* transaction) {
	int ret = -1;

	DatabaseEntry key;
	key.setData(inputKey->getBuffer(), inputKey->size());

	ret = objectsDatabase->del(transaction, &key);

	return ret;
}

LocalDatabaseIterator::LocalDatabaseIterator(engine::db::berkley::Transaction* transaction, LocalDatabase* database)
	: Logger("LocalDatabaseIterator") {

	databaseHandle = database->getDatabaseHandle();

	cursor = databaseHandle->openCursor(transaction);

	localDatabase = database;

	data.setReuseBuffer(true);
	key.setReuseBuffer(true);
}

LocalDatabaseIterator::LocalDatabaseIterator(LocalDatabase* database, bool useCurrentThreadTransaction)
	: Logger("LocalDatabaseIterator") {

	databaseHandle = database->getDatabaseHandle();

	Transaction* txn = NULL;//ObjectDatabaseManager::instance()->getLocalTransaction();

/*	if (useCurrentThreadTransaction)
		txn = ObjectDatabaseManager::instance()->getLocalTransaction();*/

	cursor = databaseHandle->openCursor(txn);

	localDatabase = database;

	data.setReuseBuffer(true);
	key.setReuseBuffer(true);
}

LocalDatabaseIterator::LocalDatabaseIterator(BerkeleyDatabase* dbHandle)
	: Logger("LocalDatabaseIterator") {
	databaseHandle = dbHandle;
	Transaction* txn = NULL; // ObjectDatabaseManager::instance()->getLocalTransaction();
	cursor = databaseHandle->openCursor(txn);

	data.setReuseBuffer(true);
	key.setReuseBuffer(true);
}

LocalDatabaseIterator::~LocalDatabaseIterator() {
	closeCursor();
}

void LocalDatabaseIterator::resetIterator() {
	if (cursor != NULL) {
		cursor->close();
		delete cursor;
	}


	Transaction* txn = NULL;//
	//Transaction* txn = ObjectDatabaseManager::instance()->getLocalTransaction();

	cursor = databaseHandle->openCursor(txn);
}

bool LocalDatabaseIterator::getNextKeyAndValue(ObjectInputStream* key, ObjectInputStream* data) {
	try {
		if (cursor->getNext(&this->key, &this->data, LockMode::READ_COMMITED) != 0) {
			/*this->key.setData(NULL, 0);
			this->data.setData(NULL, 0);*/
			return false;
		}

		key->writeStream((char*)this->key.getData(), this->key.getSize());

		if (!localDatabase->hasCompressionEnabled())
			data->writeStream((char*)this->data.getData(), this->data.getSize());
		else
			LocalDatabase::uncompress(this->data.getData(), this->data.getSize(), data);

		data->reset();
		key->reset();

	} catch (...) {
		error("unreported exception caught in ObjectDatabaseIterator::getNextKeyAndValue(uint64& key, String& data)");
	}

	return true;
}

bool LocalDatabaseIterator::getNextValue(ObjectInputStream* data) {
	try {
		if (cursor->getNext(&this->key, &this->data, LockMode::READ_UNCOMMITED) != 0) {
			/*this->key.setData(NULL, 0);
			this->data.setData(NULL, 0);*/
			return false;
		}

		if (!localDatabase->hasCompressionEnabled()) {
			data->writeStream((char*)this->data.getData(), this->data.getSize());
		} else {
			LocalDatabase::uncompress(this->data.getData(), this->data.getSize(), data);
		}

		data->reset();

	} catch(Exception &e) {
		error("Error in ObjectDatabaseIterator::getNextValue(String& data)");
		error(e.getMessage());
	} catch (...) {
		error("unreported exception caught in ObjectDatabaseIterator::getNextValue(String& data)");
	}

	return true;
}

bool LocalDatabaseIterator::getNextKey(ObjectInputStream* key) {
	try {
		if (cursor->getNext(&this->key, &this->data, LockMode::READ_UNCOMMITED) != 0) {
			/*this->key.setData(NULL, 0);
			this->data.setData(NULL, 0);*/
			return false;
		}

		key->writeStream((char*)this->key.getData(), this->key.getSize());

		key->reset();

	}  catch(Exception &e) {
		error("Error in ObjectDatabaseIterator::getNextKey");
		error(e.getMessage());
	} catch (...) {
		error("unreported exception caught  in ObjectDatabaseIterator::getNextKey(uint64& key) ");
	}

	return true;
}

int LocalDatabaseIterator::putCurrent(ObjectOutputStream* data) {
	DatabaseEntry dataEntry;

	int ret = -1;

	if (!localDatabase->hasCompressionEnabled()) {
		dataEntry.setData(data->getBuffer(), data->size());

		ret = cursor->putCurrent(&dataEntry);

		delete data;
	} else {
		Stream* compressed = LocalDatabase::compress(data);

		delete data;

		dataEntry.setData(compressed->getBuffer(), compressed->size());

		ret = cursor->putCurrent(&dataEntry);

		delete compressed;
	}

	return ret;
}

