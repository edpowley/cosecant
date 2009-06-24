#pragma once

#include "error.h"

class Uuid
{
public:
	Uuid();
	Uuid(const Uuid& other);
	Uuid(const QString& s);
	Uuid& operator=(const Uuid& other);

	bool operator==(const Uuid& other) const { return compare(other) == 0; }
	bool operator< (const Uuid& other) const { return compare(other) <  0; }

	QString str() const;

	ERROR_CLASS(Error);

protected:
	static const int c_dataBytes = 4+2+2+8;
	unsigned char m_data[c_dataBytes];

	static QRegExp s_stringRegex;

	int compare(const Uuid& other) const;
};

