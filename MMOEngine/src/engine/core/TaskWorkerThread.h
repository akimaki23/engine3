/*
Copyright (C) 2007 <SWGEmu>. All rights reserved.
Distribution of this file for usage outside of Core3 is prohibited.
*/

#ifndef TASKWORKERTHREAD_H_
#define TASKWORKERTHREAD_H_

#include "engine/service/ServiceThread.h"

#include "TaskStatistics.h"

namespace engine {
  namespace core {

	class TaskManager;
	class TaskQueue;

	class TaskWorkerThread : public ServiceThread {
		Mutex blockMutex;
		TaskQueue* queue;
		int cpu;

#ifdef COLLECT_TASKSTATISTICS
		HashTable<const char*, TaskStatistics> tasksStatistics;
		ReadWriteLock tasksStatsGuard;
#endif

	public:
		TaskWorkerThread(const String& s, TaskQueue* queue, int cpu = 0);

		virtual ~TaskWorkerThread();

		void start(TaskManager* manager);

		//void pushToRetryQueue(Task* task);

		virtual void run();

		void stop();

#ifdef COLLECT_TASKSTATISTICS
		HashTable<const char*, TaskStatistics> getTasksStatistics();

		void clearTaskStatistics();
#endif

		inline Mutex* getBlockMutex() {
			return &blockMutex;
		}
	};

  } // namespace service
} // namespace engine

using namespace engine::core;

#endif /* TASKWORKERTHREAD_H_ */
