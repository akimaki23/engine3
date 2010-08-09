/*
Copyright (C) 2007 <SWGEmu>. All rights reserved.
Distribution of this file for usage outside of Core3 is prohibited.
*/

#ifndef VARIABLE_H_
#define VARIABLE_H_

#include "../platform.h"

namespace sys {
	namespace io {
		class ObjectOutputStream;
		class ObjectInputStream;
	}
}

namespace sys {
	namespace lang {

	class String;

	using namespace sys::io;

	class Variable {
	public:
		inline Variable() {

		}

		virtual ~Variable() {

		}

		virtual bool toString(String& str) = 0;
		virtual bool parseFromString(const String& str, int version = 0) = 0;

		virtual bool toBinaryStream(ObjectOutputStream* stream) = 0;
		virtual bool parseFromBinaryStream(ObjectInputStream* stream) = 0;

	};

	} // namespace lang
} // namespace sys

using namespace sys::lang;

#endif /* VARIABLE_H_ */
