#include "stdafx.h"
#include "common.h"
#include "prefs.h"

PrefsVar_Base::PrefsVar_Base(const QString& id)
: m_id(id), m_prefsfile(PrefsFile::get())
{
	// Derived classes must call
	//    m_prefsfile->readValue(m_id, this);
	// (can't do it here, the virtual functions aren't ready yet or something)
}

PrefsVar_Base::~PrefsVar_Base()
{
}

void PrefsVar_Base::setDirty()
{
	m_prefsfile->writeValue(m_id, this);
	signalChange();
}

//////////////////////////////////////////////////////////////////

PrefsVar_Double::PrefsVar_Double(const QString& id, double def)
: PrefsVar_T(id, def)
{
	m_prefsfile->readValue(m_id, this);
}

void PrefsVar_Double::sqlRetrieve(sqlite3_stmt* stmt, int column)
{
	m_value = sqlite3_column_double(stmt, column);
}

int PrefsVar_Double::sqlBind(sqlite3_stmt *stmt, int index)
{
	return sqlite3_bind_double(stmt, index, m_value);
}

//////////////////////////////////////////////////////////////////

PrefsVar_Int::PrefsVar_Int(const QString& id, int def)
: PrefsVar_T(id, def)
{
	m_prefsfile->readValue(m_id, this);
}

void PrefsVar_Int::sqlRetrieve(sqlite3_stmt* stmt, int column)
{
	m_value = sqlite3_column_int(stmt, column);
}

int PrefsVar_Int::sqlBind(sqlite3_stmt *stmt, int index)
{
	return sqlite3_bind_int(stmt, index, m_value);
}

//////////////////////////////////////////////////////////////////////

PrefsVar_Bool::PrefsVar_Bool(const QString& id, bool def)
: PrefsVar_T(id, def)
{
	m_prefsfile->readValue(m_id, this);
}

void PrefsVar_Bool::sqlRetrieve(sqlite3_stmt* stmt, int column)
{
	m_value = (sqlite3_column_int(stmt, column) != 0);
}

int PrefsVar_Bool::sqlBind(sqlite3_stmt *stmt, int index)
{
	return sqlite3_bind_int(stmt, index, m_value ? 1 : 0);
}

//////////////////////////////////////////////////////////////////////

PrefsVar_String::PrefsVar_String(const QString& id, const QString& def)
: PrefsVar_T(id, def)
{
	m_prefsfile->readValue(m_id, this);
}

void PrefsVar_String::sqlRetrieve(sqlite3_stmt* stmt, int column)
{
	m_value = QString::fromUtf8(reinterpret_cast<const char*>(sqlite3_column_text(stmt, column)));
}

int PrefsVar_String::sqlBind(sqlite3_stmt *stmt, int index)
{
	return sqlite3_bind_text(stmt, index, m_value.toUtf8().constData(), -1, SQLITE_TRANSIENT);
}

//////////////////////////////////////////////////////////////////////

Ptr<PrefsFile> PrefsFile::s_singleton(NULL);

ERROR_CLASS(SqliteError);

int sqlDo(sqlite3* database, int retcode)
{
	if (retcode != SQLITE_OK && retcode != SQLITE_ROW && retcode != SQLITE_DONE)
	{
		THROW_ERROR(SqliteError, sqlite3_errmsg(database));
	}
	return retcode;
}

QString PrefsFile::getAppDataDir()
{
	static TCHAR path[MAX_PATH];
	static bool done = false;

	if (!done)
	{
		if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, path)))
		{
			PathAppend(path, L"BTDSys Cosecant");
			if (!PathFileExists(path))
			{
				CreateDirectory(path, NULL);
			}

			done = true;
		}
		else
			THROW_ERROR(Error, "SHGetFolderPath error");
	}

	return QDir::fromNativeSeparators(QString::fromWCharArray(path));
}

PrefsFile::PrefsFile() : m_database(NULL)
{
	QString path = QDir::toNativeSeparators(getAppDataDir() + "/prefs.db");

	try
	{
		sqlDo(m_database, sqlite3_open(path.toUtf8(), &m_database));
	}
	catch (SqliteError&)
	{
		sqlite3_close(m_database);
		m_database = NULL;
		throw;
	}

	sqlDo(m_database, sqlite3_exec(m_database,
		"CREATE TABLE IF NOT EXISTS prefs (key UNIQUE ON CONFLICT REPLACE, value)",
		NULL, NULL, NULL));
}

PrefsFile::~PrefsFile()
{
	if (m_database) sqlite3_close(m_database);
}

void PrefsFile::readValue(const QString& id, PrefsVar_Base* var)
{
	sqlite3_stmt* stmt = NULL;
	const char sql[] = "SELECT value FROM prefs WHERE key == ?1 LIMIT 1";
	sqlDo(m_database, sqlite3_prepare(m_database, sql, sizeof(sql), &stmt, NULL));

	sqlDo(m_database, sqlite3_bind_text(stmt, 1, id.toUtf8().constData(), -1, SQLITE_TRANSIENT));

	if (sqlDo(m_database, sqlite3_step(stmt)) == SQLITE_ROW)
	{
		var->sqlRetrieve(stmt, 0);
	}

	sqlDo(m_database, sqlite3_finalize(stmt));
}

void PrefsFile::writeValue(const QString& id, PrefsVar_Base* var)
{
	sqlite3_stmt* stmt = NULL;
	const char sql[] = "INSERT OR REPLACE INTO prefs (key, value) VALUES (?1, ?2)";
	sqlDo(m_database, sqlite3_prepare(m_database, sql, sizeof(sql), &stmt, NULL));

	sqlDo(m_database, sqlite3_bind_text(stmt, 1, id.toUtf8().constData(), -1, SQLITE_TRANSIENT));
	sqlDo(m_database, var->sqlBind(stmt, 2));

	sqlDo(m_database, sqlite3_step(stmt));

	sqlDo(m_database, sqlite3_finalize(stmt));
}
