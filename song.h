#pragma once

#include "routing.h"

namespace Sequence { class Seq; };

ERROR_CLASS(SongLoadError);

class SongLoadContext
{
public:
	template<typename T> T* getObject(const Uuid& uuid)
	{
		std::map<Uuid, ObjectWithUuid*>::const_iterator iter = m_uuidMap.find(uuid);
		if (iter != m_uuidMap.end())
			return dynamic_cast<T*>(iter->second);
		else
			return NULL;
	}

	template<typename T> T* getObjectOrThrow(const Uuid& uuid)
	{
		T* ret = getObject<T>(uuid);
		if (ret)
			return ret;
		else
			throw SongLoadError(QString("Object '%1' not found").arg(uuid.str()));
	}

	void setObject(const Uuid& uuid, ObjectWithUuid* object)
	{
		std::map<Uuid, ObjectWithUuid*>::const_iterator iter = m_uuidMap.find(uuid);
		if (iter == m_uuidMap.end())
			m_uuidMap.insert(std::make_pair(uuid, object));
		else
			THROW_ERROR(SongLoadError, QString("Object with id %1 already exists").arg(uuid.str()));
	}

	std::list<QString> m_nonFatalErrors;

protected:
	// We're playing with these things in objects' ctors, so smart ptrs would be a dumb idea
	std::map<Uuid, ObjectWithUuid*> m_uuidMap;
};

class Song : public QObject
{
	Q_OBJECT

protected:
	Song();
	static Song* s_singleton;

public:
	static void initSingleton();
	static Song& get() { return *s_singleton; }

	Ptr<Routing> m_routing;
	Ptr<Sequence::Seq> m_sequence;
	QUndoStack m_undo;

	bool load(const bpath& filepath);
	void save(const bpath& filepath);

	void doOpen();
	void doSave();
	void doSaveAs();

public slots:
	void updateWorkQueue();

protected:
	bpath m_savePath;

	void clear();
};
