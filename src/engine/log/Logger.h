/*
Copyright (C) 2007 <SWGEmu>. All rights reserved.
Distribution of this file for usage outside of Core3 is prohibited.
*/

#ifndef LOGGER_H_
#define LOGGER_H_

#include "system/lang.h"

namespace engine {
  namespace log {

	class Logger {
		String name;

		static AtomicReference<FileWriter> globalLogFile;

		FileWriter* logFile;

		static Time starttime;

		bool doLog, doGlobalLog;

	public:
		static Logger console;

	public:
		Logger();
		Logger(const char *s);
		Logger(const String& s);

		~Logger();

		static void setGlobalFileLogger(const char* file);
		static void setGlobalFileLogger(const String& file);

		void setFileLogger(const char* file);
		void setFileLogger(const String& file);

		static void closeGlobalFileLogger();
		void closeFileLogger();

		void info(const char *msg, bool forcedLog = false);
		void info(const String& msg, bool forcedLog = false);
		void info(const StringBuffer& msg, bool forcedLog = false);

		void log(const char *msg);
		void log(const String& msg);
		void log(const StringBuffer& msg);

		void error(const char* msg);
		void error(const String& msg);
		void error(const StringBuffer& msg);

		static void getTime(String& time, bool getFull = true);
		static void printTime(bool getFull = true);

		static uint64 getElapsedTime();

		// setters
		inline void setLogging(bool doLog) {
			Logger::doLog = doLog;
		}

		inline void setGlobalLogging(bool doLog) {
			doGlobalLog = doLog;
		}

		inline void setLoggingName(const String& s) {
			name = s;
		}

		// getters
		inline String& getLoggingName() {
			return name;
		}

		inline FileWriter* getFileLogger() {
			return logFile;
		}

	};

  } // namespace log
} // namespace engine

using namespace engine::log;

#endif /*LOGGER_H_*/
