#include "stdafx.h"
#include "common.h"
#include "uuid.h"

Uuid::Uuid()
{
	GUID guid;
	if (CoCreateGuid(&guid) != S_OK)
		THROW_ERROR(Error, "CoCreateGuid failed");

	for (int i=0; i<4; i++)
		m_data[i] = (unsigned char)((guid.Data1 >> ((3-i)*8)) & 0xFF);

	for (int i=0; i<2; i++)
	{
		m_data[i+4] = (unsigned char)((guid.Data2 >> ((1-i)*8)) & 0xFF);
		m_data[i+6] = (unsigned char)((guid.Data3 >> ((1-i)*8)) & 0xFF);
	}

	for (int i=0; i<8; i++)
		m_data[i+8] = guid.Data4[i];
}

Uuid::Uuid(const Uuid& other)
{
	*this = other;
}

QRegExp Uuid::s_stringRegex("[0-9A-Fa-f]{8}-(?:[0-9A-Fa-f]{4}-){3}[0-9A-Fa-f]{12}");

unsigned char hexDigit(const QChar& ch)
{
	unsigned char h = ch.cell();
	if (h >= '0' && h <= '9')
		return h - '0';
	else if (h >= 'A' && h <= 'F')
		return h - 'A' + 10;
	else if (h >= 'a' && h <= 'f')
		return h - 'a' + 10;
	else
		THROW_ERROR(Error, "Invalid hex digit");
}

Uuid::Uuid(const QString& s)
{
	// Check against regex
	if (!s_stringRegex.exactMatch(s))
		THROW_ERROR(Error, "String is not a UUID in 8-4-4-4-12 format");

	// Erase dashes
	QString t = s;
	t.remove(8,1).remove(12,1).remove(16,1).remove(20,1);

	// Read off hex digits
	for (int i=0; i<16; i++)
	{
		m_data[i] = hexDigit(t[i*2]) << 4 | hexDigit(t[i*2+1]);
	}
}

Uuid& Uuid::operator=(const Uuid& other)
{
	memcpy(m_data, other.m_data, sizeof(m_data));
	return *this;
}

QString Uuid::str() const
{
	QString s;
	std::ostringstream ss;
	for (int i=0; i<c_dataBytes; i++)
	{
		if (i == 4 || i == 6 || i == 8 || i == 10) s.append("-");
		s.append(QString("%1").arg((int)m_data[i], 2, 16, QLatin1Char('0')));
	}
	return s;
}

uint Uuid::qHash() const
{
	uint h = 0;
	for (int i=0; i<c_dataBytes; i++)
	{
		uint shiftbytes = i % sizeof(uint);
		h += (uint)m_data[i] << (shiftbytes * 8);
	}

	return h;
}

int Uuid::compare(const Uuid& other) const
{
	for (int i=0; i<c_dataBytes; i++)
	{
		int c = m_data[i] - other.m_data[i];
		if (c != 0) return c;
	}
	return 0;
}
