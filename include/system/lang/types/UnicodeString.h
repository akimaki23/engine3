/*
Copyright (C) 2007 <SWGEmu>. All rights reserved.
Distribution of this file for usage outside of Core3 is prohibited.
*/

#ifndef UNICODE_H_
#define UNICODE_H_

#include "../../platform.h"

#ifndef PLATFORM_WIN
#define wchar_t unsigned short
#endif

namespace sys {
  namespace lang {

	class String;

	class UnicodeString : public Variable {
		wchar_t* uString;
		int count;

	public:
		UnicodeString();
		UnicodeString(const char* ascii);
		UnicodeString(const String& ascii);
		UnicodeString(const char* ascii, int len);
		UnicodeString(const UnicodeString& str);

		~UnicodeString();

		UnicodeString& operator=(const UnicodeString& str);
		UnicodeString& operator=(const String& ascii);
		UnicodeString& operator=(const char* ascii);

		UnicodeString& operator+(const String& ascii);
		UnicodeString& operator+(const UnicodeString& str);
		UnicodeString& operator+(const char* ascii);

		char operator[](int index) const;

		void append(const String& ascii);
		void append(const UnicodeString& uni);
		void append(const char* ascii);
		void append(const char* ascii, int len);
		void append(const wchar_t* str, int len);

		void clear();

		int indexOf(wchar_t chr) const;
		int indexOf(const UnicodeString& str, int startPos = 0) const;

		UnicodeString subString(int beg, int end) const;
		UnicodeString substr(int beg, int length) const;

		const wchar_t* toWideCharArray() const;

		String toString() const;
		void toString(String& ascii) const;

		void parseFromString(String* str) {
			*this = *str;
		}

		void toString(String* ascii) {
			toString(*ascii);
		}

		void toBinaryStream(sys::io::ObjectOutputStream* stream);
		void parseFromBinaryStream(sys::io::ObjectInputStream* stream);

		inline bool isEmpty() const {
			return count == 0;
		}

	private:
		void create(const char* ascii, int len);
		void asciitowide(wchar_t* unicode, const char* ascii, int len);

		void copy(wchar_t* dest, const wchar_t* src);
		void copy(wchar_t* dest, const wchar_t* src, int len);

	public:
		inline int length() const {
			return count;
		}
	};

  } // namespace lang
} // namespace sys

using namespace sys::lang;

#endif /*UNICODE_H_*/
