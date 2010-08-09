/*
Copyright (C) 2007 <SWGEmu>. All rights reserved.
Distribution of this file for usage outside of Core3 is prohibited.
*/

#include "TaskScheduler.h"

#ifdef VERSION_PUBLIC
#include "../service/proto/events/BaseClientCleanUpEvent.hpp"
#endif

#include "TaskManager.h"
#include "../db/ObjectDatabaseManager.h"

using namespace engine::db;
using namespace engine::db::berkley;

TaskScheduler::TaskScheduler() : Thread(), Logger("TaskScheduler") {
	doRun = false;

	tasks.setTaskScheduler(this);

	tasks.setLoggingName("TaskQueue");
	tasks.setMutexName("TaskQueueLock");
}

TaskScheduler::TaskScheduler(const String& s) : Thread(), Logger(s) {
	doRun = false;

	tasks.setTaskScheduler(this);

	tasks.setLoggingName(s + "TaskQueue");
	tasks.setMutexName(s + "TaskQueueLock");

	setLogging(false);
}

TaskScheduler::~TaskScheduler() {
}

void TaskScheduler::start() {
	StringBuffer msg;
	msg << "started";
	info(msg);

	doRun = true;
	Thread::start();
}

void TaskScheduler::run() {
	ObjectDatabaseManager::instance()->commitLocalTransaction();

	Task* task;

	while ((task = tasks.get()) != NULL) {
		try {
			#ifdef VERSION_PUBLIC
				DO_TIMELIMIT;
			#endif

			TaskManager::instance()->setTaskScheduler(task, NULL);

			task->run();



			/*if (!task->isReentrant())
				delete task;*/

			//should we set NULL taskScheduler for reentrant tasks here?

		} catch (Exception& e) {
			error(e.getMessage());
		} catch (...) {
			#ifdef VERSION_PUBLIC
				return;
			#else
				error("[TaskScheduler] unreported Exception caught");
			#endif
		}

		task->release();

		ObjectDatabaseManager::instance()->commitLocalTransaction();
	}
}

void TaskScheduler::stop() {
	if (doRun) {
		doRun = false;
		tasks.flush();

		join();
	}

	info("stopped");
}
