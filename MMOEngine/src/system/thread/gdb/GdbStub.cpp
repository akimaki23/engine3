#include "system/lang/StringBuffer.h"

#include "GdbStub.h"

#include <vector>

GdbStub::GdbStub() : logFile(NULL) {
}

GdbStub::~GdbStub() {
	if (logFile) {
		logFile->close();

		delete logFile;
		logFile = NULL;
	}
}

void GdbStub::initialize(pid_t pid) {
	processPid = String::valueOf(pid);

	time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);

	File* file= new File("log/crash_" + processPid + "_" + buf);
	logFile = new FileWriter(file);
}

void GdbStub::run() {
	printStackTrace();
	//printRegisters();
}

void addGDBArgument(std::vector<char*>& args, const char* arg) {
	char* str = (char*) malloc(128);

	strcpy(str, arg);

	args.push_back(str);
}

void GdbStub::printStackTrace() {
	pipe.create();

	pid = fork();

	if (!pid) {
		pipe.redirectFile(fileno(stdout));

		//Vector<>
/*
		std::vector<char*> arguments;

		addGDBArgument(arguments, "gdb");
		addGDBArgument(arguments, "-pid");
		addGDBArgument(arguments, processPid.toCharArray());
		addGDBArgument(arguments, "--batch");
		addGDBArgument(arguments, "-f");
		addGDBArgument(arguments, "-n");
		addGDBArgument(arguments, "-ex");
		addGDBArgument(arguments, "set pagination off");

		for (int i = 1; i < 3; ++i) {
			char str[128];

			sprintf(str, "thread %d", i);

			addGDBArgument(arguments, "-ex");
			addGDBArgument(arguments, str);

			addGDBArgument(arguments, "-ex");
			addGDBArgument(arguments, "bt full");
		}

		arguments.push_back(NULL);*/

		char* argv[] = {"gdb",
				"--batch", "-f", "-n",
				"-ex", "set pagination off",
				"-ex", "thread apply all bt full",
				"-ex", "info registers",
/*				"-ex", "bt full",
				"-ex", "thread 2",
				"-ex", "bt full",*/
				"-pid", const_cast<char*>(processPid.toCharArray()),
				NULL};

		//execvp("gdb", &arguments[0]);
		execvp("gdb", argv);

		assert(0);
	} else {
		//wait();

		logFile->writeLine("Stack Trace:");

		writeOutput();

		logFile->writeLine("");
	}

	pipe.close();
}

void GdbStub::printRegisters() {
	pipe.create();

	pid = fork();

	if (!pid) {
		pipe.redirectFile(fileno(stdout));

		char* argv[] = {"gdb",
				"--batch", "-f", "-n",
				"-ex", "set pagination off",
				"-ex", "info registers",
				"-pid", const_cast<char*>(processPid.toCharArray()),
				NULL};

		execvp("gdb", argv);

		assert(0);
	} else {
		wait();

		logFile->writeLine("Registers:");

		writeOutput();

		logFile->writeLine("");
	}

	pipe.close();
}

void GdbStub::printDeadlock() {
	pipe.create();

	pid = fork();

	if (!pid) {
		pipe.redirectFile(fileno(stdout));

		char* argv[] = {"gdb",
				"--batch", "-f", "-n",
				"-ex", "set pagination off",
				"-ex", "info registers",
				"-pid", const_cast<char*>(processPid.toCharArray()),
				NULL};

		execvp("gdb", argv);

		assert(0);
	} else {
		wait();

		logFile->writeLine("Registers:");

		writeOutput();

		logFile->writeLine("");
	}

	pipe.close();
}

void GdbStub::printThread(String threadInfo) {
	if (threadInfo.contains("pthread_cond_timedwait") || threadInfo.contains("pthread_cond_wait"))
		return;

	StringTokenizer tokenizer(threadInfo);

	int threadID = tokenizer.getIntToken();

	String threadString;
	tokenizer.getStringToken(threadString);
	if (threadString.compareTo("Thread") != 0)
		return;

	pipe.create();

	pid = fork();

	if (pid == 0) {
		pipe.redirectFile(fileno(stdout));

		char threadCommand[100];
		sprintf(threadCommand, "thread %i", threadID);

		char* argv[] = {"gdb",
				"--batch", "-f", "-n",
				"-ex", "set pagination off",
				"-ex", threadCommand,
				//"-ex", "thread 1",
				"-ex", "bt full",
				"-pid", const_cast<char*>(processPid.toCharArray()),
				NULL};

		execvp("gdb", argv);

		assert(0);
	} else {
		wait();

		logFile->writeLine("Thread " + String::valueOf(threadID) + ":");

		writeOutput();

		logFile->writeLine("");
	}

	pipe.close();
}

void GdbStub::getThreads(Vector<String>& threads) {
	pipe.create();

	pid = fork();

	if (!pid) {
		pipe.redirectFile(fileno(stdout));

		char* argv[] = {"gdb",
				"--batch", "-f", "-n",
				"-ex", "set pagination off",
				"-ex", "info threads",
				"-pid", const_cast<char*>(processPid.toCharArray()),
				NULL};

		execvp("gdb", argv);

		assert(0);
	} else {
		wait();

		parseOutput(threads);
	}

	pipe.close();
}

void GdbStub::writeOutput() {
	char line[4096];

	while (pipe.readLine(line, 4096) > 0) {
		String str(line);
		if (str.beginsWith("[New Thread") || str.beginsWith("[Thread debugging"))
			continue;

		logFile->write(line);

		//printf("read line from hui:%s\n", line);
	}
}

void GdbStub::parseOutput(Vector<String>& lines) {
	char line[4096];

	while (pipe.readLine(line, 4096) > 0) {
		String str(line);
		if (str.beginsWith("[New Thread") || str.beginsWith("[Thread debugging"))
			continue;

		lines.add(str);
	}
}
