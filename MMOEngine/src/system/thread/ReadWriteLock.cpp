/*
Copyright (C) 2007 <SWGEmu>. All rights reserved.
Distribution of this file for usage outside of Core3 is prohibited.
*/

#include <errno.h>

#include "Mutex.h"

#include "../lang/Time.h"

#include "../lang/System.h"
#include "../lang/StackTrace.h"

#include "ReadWriteLock.h"

void ReadWriteLock::rlock(bool doLock) {
	if (!doLock)
		return;

	lockAcquiring("r");

	#if !defined(TRACE_LOCKS) || defined(PLATFORM_CYGWIN)
		int res = pthread_rwlock_rdlock(&rwlock);
		if (res != 0) {
#ifdef TRACE_LOCKS
			System::out << "(" << Time::currentNanoTime() << " nsec) rlock() failed on RWLock \'" << lockName << "\' (" << strerror(res) << ")\n";
#else
			System::out << "(" << Time::currentNanoTime() << " nsec) rlock() failed on RWLock (" << strerror(res) << ")\n";
#endif
			StackTrace::printStackTrace();
			
			assert(0 && "rlock failed");
		}
	#else
		#ifndef LOG_LOCKS
			int cnt = lockCount.increment();
		#endif

		Time start;
		start.addMiliTime(10000);

		while (pthread_rwlock_timedrdlock(&rwlock, start.getTimeSpec())) {
			if (!doTrace)
				continue;

			traceDeadlock("r");

			start.addMiliTime(1000);
		}

		lockTime->updateToCurrentTime();
	#endif

	readLockCount.increment();

	lockAcquired("r");
}

void ReadWriteLock::wlock(bool doLock) {
	if (!doLock)
		return;

	lockAcquiring("w");

	#if !defined(TRACE_LOCKS) || defined(PLATFORM_CYGWIN)
		int res = pthread_rwlock_wrlock(&rwlock);
		if (res != 0){
#ifdef TRACE_LOCKS
			System::out << "(" << Time::currentNanoTime() << " nsec) wlock() failed on RWLock \'" << lockName << "\' (" << strerror(res) << ")\n";
#else
		System::out << "(" << Time::currentNanoTime() << " nsec) wlock() failed on RWLock (" << strerror(res) << ")\n";
#endif
		assert(0 && "wlock failed");
		}
	#else
		Time start;
		start.addMiliTime(10000);

		int result = 0;

		while ((result = pthread_rwlock_timedwrlock(&rwlock, start.getTimeSpec())) != 0) {
			if (result != ETIMEDOUT) {
				System::out << "(" << Time::currentNanoTime() << " nsec) wlock() error on RWLock \'" << lockName << "\' (" << strerror(result) << ")\n";

				raise(SIGSEGV);
			}

			if (!doTrace)
				continue;

			traceDeadlock("w");

			start.addMiliTime(1000);
		}

		lockTime->updateToCurrentTime();
	#endif

	lockAcquired("w");
}

void ReadWriteLock::wlock(Mutex* lock) {
	lockAcquiring(lock, "w");

    while (pthread_rwlock_trywrlock(&rwlock)) {
    	#ifndef TRACE_LOCKS
			//pthread_mutex_unlock(&(lock->mutex));

    		lock->unlock();

			Thread::yield();

      		//pthread_mutex_lock(&(lock->mutex));

			lock->lock();
		#else
      		lock->unlock();

      		Thread::yield();

	      	lock->lock();
	 	#endif
	}

	lockAcquired(lock, "w");
}

void ReadWriteLock::rlock(Lockable* lock) {
	lockAcquiring(lock, "r");

	while (pthread_rwlock_tryrdlock(&rwlock)) {
		lock->unlock();

		Thread::yield();

		lock->lock();
	}

	readLockCount.increment();

	lockAcquired(lock, "r");
}

void ReadWriteLock::rlock(ReadWriteLock* lock) {
	lockAcquiring(lock, "r");

	while (pthread_rwlock_tryrdlock(&rwlock)) {
		lock->unlock();

		Thread::yield();

		lock->lock();
	}

	readLockCount.increment();

	lockAcquired(lock, "r");
}

void ReadWriteLock::wlock(ReadWriteLock* lock) {
	if (this == lock) {
#ifdef TRACE_LOCKS
		System::out << "(" << Time::currentNanoTime() << " nsec) ERROR: cross wlocking itself [" << lockName << "]\n";
#else
		System::out << "(" << Time::currentNanoTime() << " nsec) ERROR: cross wlocking itself \n";
#endif

		StackTrace::printStackTrace();
		return;
	}

	#ifdef TRACE_LOCKS
		if (lock->threadLockHolder == NULL) {
			System::out << "(" << Time::currentNanoTime() << " nsec) ERROR: cross wlocking to an unlocked mutex [" << lock->lockName << "]\n";
			StackTrace::printStackTrace();

			raise(SIGSEGV);
		}
	#endif

	lockAcquiring(lock, "w");

    while (pthread_rwlock_trywrlock(&rwlock)) {
    	#ifndef TRACE_LOCKS
  			//pthread_rwlock_unlock(&(lock->rwlock));
    		lock->unlock();

  			Thread::yield();

       		//pthread_rwlock_wrlock(&(lock->rwlock));
  			lock->wlock();
       	#else
       		lock->unlock();
       		lock->wlock();
       	#endif
	}

	lockAcquired(lock, "w");
}

void ReadWriteLock::lock(Lockable* lockable) {
	lockAcquiring(lockable, "w");

    while (pthread_rwlock_trywrlock(&rwlock)) {
  		lockable->unlock();

  		Thread::yield();

      	lockable->lock();
	}

	lockAcquired(lockable, "w");
}

void ReadWriteLock::unlock(bool doLock) {
	if (!doLock)
		return;

	#if defined(TRACE_LOCKS) && !defined(PLATFORM_CYGWIN)
		if (threadLockHolder == NULL) {
			System::out << "(" << Time::currentNanoTime() << " nsec) WARNING" << "[" << lockName << "]"
					<< " unlocking an unlocked mutex\n";
			StackTrace::printStackTrace();

			if (unlockTrace != NULL) {
				System::out << "previously unlocked by\n";
				unlockTrace->print();
			}

			raise(SIGSEGV);
		} else if (threadLockHolder != Thread::getCurrentThread()) {
			System::out << "(" << Time::currentNanoTime() << " nsec) WARNING" << "[" << lockName << "]" << " mutex unlocked by a different thread\n";
			StackTrace::printStackTrace();

			if (trace != NULL) {
				System::out << "previously locked at " << lockTime->getMiliTime() << " by\n";
				trace->print();
			}
		}
	#endif
	
//	assert(threadLockHolder == Thread::getCurrentThread());

	lockReleasing();

	int res = pthread_rwlock_unlock(&rwlock);
	if (res != 0) {
#ifdef TRACE_LOCKS
		System::out << "(" << Time::currentNanoTime() << " nsec) unlock() failed on RWLock \'" << lockName << "\' (" << strerror(res) << ")\n";
#else
		System::out << "(" << Time::currentNanoTime() << " nsec) ERROR: cross wlocking itself\n";
#endif

		StackTrace::printStackTrace();
		assert(0 && "unlock failed");
	}

	lockReleased();
}

void ReadWriteLock::runlock(bool doLock) {
	if (!doLock)
		return;

#ifdef TRACE_LOCKS
	/*if (threadIDLockHolder == 0) {
					System::out << "(" << Time::currentNanoTime() << " nsec) WARNING" << "[" << lockName << "]"
							<< " unlocking an unlocked mutex\n";
					StackTrace::printStackTrace();
				} else if (threadIDLockHolder != Thread::getCurrentThreadID()) {
					System::out << "(" << Time::currentNanoTime() << " nsec) WARNING" << "[" << lockName << "]" << " mutex unlocked by a different thread\n";
					StackTrace::printStackTrace();

					if (trace != NULL) {
						System::out << "previously locked at " << lockTime->getMiliTime() << " by\n";
						trace->print();
					}
				}*/

	/*delete trace;
	trace = NULL;

	threadIDLockHolder = 0;*/
#endif

	lockReleasing("r");

	readLockCount.decrement();

	int res = pthread_rwlock_unlock(&rwlock);
	if (res != 0) {
#ifdef TRACE_LOCKS
		System::out << "(" << Time::currentNanoTime() << " nsec) unlock() failed on RWLock \'" << lockName << "\' (" << res << ")\n";
#else
		System::out << "(" << Time::currentNanoTime() << " nsec) unlock() failed on RWLock (" << res << ")\n";
#endif

		StackTrace::printStackTrace();
		
		assert(0 && "runlock failed");
	}

	lockReleased();
}
