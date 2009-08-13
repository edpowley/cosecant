#pragma once

class PrefsVar_Base : public QObject
{
	Q_OBJECT

	friend class PrefsFile;

public:
	PrefsVar_Base(const QString& id);
	virtual ~PrefsVar_Base();

signals:
	void signalChange();

protected:
	QString m_id;
	Ptr<PrefsFile> m_prefsfile;
	void setDirty();

	// Should call sqlite3_column_*
	virtual void sqlRetrieve(sqlite3_stmt* stmt, int column) = 0;

	// Should call sqlite3_bind_* and return its return code
	virtual int sqlBind(sqlite3_stmt* stmt, int index) = 0;

	//virtual void setDefault() = 0;
};

template<typename T>
class PrefsVar_T : public PrefsVar_Base
{
public:
	PrefsVar_T(const QString& id, const T& def) 
		: m_value(def), PrefsVar_Base(id), m_default(def) {}

	T operator()() { return m_value; }
	void set(const T& newval)
	{
		if (m_value != newval)
		{
			m_value = newval;
			setDirty();
		}
	}

protected:
	T m_default;
	T m_value;
};

class PrefsVar_Double : public PrefsVar_T<double>
{
public:
	PrefsVar_Double(const QString& id, double def);

protected:
	virtual void sqlRetrieve(sqlite3_stmt* stmt, int column);
	virtual int sqlBind(sqlite3_stmt* stmt, int index);
};

class PrefsVar_Int : public PrefsVar_T<int>
{
public:
	PrefsVar_Int(const QString& id, int def);

protected:
	virtual void sqlRetrieve(sqlite3_stmt* stmt, int column);
	virtual int sqlBind(sqlite3_stmt* stmt, int index);
};

class PrefsVar_Bool : public PrefsVar_T<bool>
{
public:
	PrefsVar_Bool(const QString& id, bool def);

protected:
	virtual void sqlRetrieve(sqlite3_stmt* stmt, int column);
	virtual int sqlBind(sqlite3_stmt* stmt, int index);
};

class PrefsVar_String : public PrefsVar_T<QString>
{
public:
	PrefsVar_String(const QString& id, const QString& def);

protected:
	virtual void sqlRetrieve(sqlite3_stmt* stmt, int column);
	virtual int sqlBind(sqlite3_stmt* stmt, int index);
};

//////////////////////////////////////////////////////////////////////////////////

class PrefsDirList : public Object
{
	friend class PrefsFile;

protected:
	PrefsDirList(const Ptr<PrefsFile>& prefsFile, const QString& id, const QString& name);

public:
	QString getId() { return m_id; }
	QString getName() { return m_name; }
	QStringList getDirs() { return m_dirs; }

	void setDirs(const QStringList& dirs);

protected:
	Ptr<PrefsFile> m_prefsFile;
	QString m_id, m_name;
	QStringList m_dirs;
};

//////////////////////////////////////////////////////////////////////////////////

class PrefsFile : public Object
{
protected:
	PrefsFile();
	~PrefsFile();
	
	// Having the singleton as a Ptr should guarantee that it isn't destroyed until
	// after all the vars (each var owns a reference to it)
	static Ptr<PrefsFile> s_singleton;

public:
	static Ptr<PrefsFile> get()
	{
		if (!s_singleton)
			s_singleton = new PrefsFile;
		return s_singleton;
	}

	static QString getAppDataDir();

	void readValue(const QString& id, PrefsVar_Base* var);
	void writeValue(const QString& id, PrefsVar_Base* var);

	Ptr<PrefsDirList> getDirList(const QString& id, const QString& name);
	QHash< QString, Ptr<PrefsDirList> > getDirLists() { return m_dirLists; }
	QStringList getDirs(const QString& key);
	void setDirs(const QString& key, const QStringList& dirs);

protected:
	sqlite3* m_database;

	QHash< QString, Ptr<PrefsDirList> > m_dirLists;
};
