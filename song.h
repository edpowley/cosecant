#pragma once

#include "stdafx.h"
#include <QUndoStack>
#include "routing.h"
#include "sequence.h"

ERROR_CLASS(SongLoadError);
ERROR_CLASS(SongSaveError);

class SongLoadContext
{
public:
	QDomDocument m_doc;

	template<typename T> T* getObject(const Uuid& uuid)
	{
		return dynamic_cast<T*>(m_uuidMap.value(uuid, NULL));
	}

	template<typename T> T* getObjectOrThrow(const Uuid& uuid)
	{
		T* ret = getObject<T>(uuid);
		if (ret)
			return ret;
		else
			throw SongLoadError(QString("Object '%1' is missing or of the wrong type").arg(uuid.str()));
	}

	void setObject(const Uuid& uuid, ObjectWithUuid* object)
	{
		if (m_uuidMap.contains(uuid))
			throw SongLoadError(QString("Object '%1' already exists").arg(uuid.str()));
		m_uuidMap.insert(uuid, object);
	}

	void warn(const QString& msg) { m_warnings << msg; }

protected:
	// We're playing with these things in objects' ctors, so smart ptrs would be a dumb idea
	QHash<Uuid, ObjectWithUuid*> m_uuidMap;

	QStringList m_warnings;
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
	Ptr<Seq::Sequence> m_sequence;
	QUndoStack m_undo;

	void load(const QString& filepath);
	void save(const QString& filepath);
	const QString& getSavePath() { return m_savePath; }

	void doOpen();
	void doSave();
	void doSaveAs();

signals:
	void signalSavePathChanged(const QString& path);

public slots:
	void updateWorkQueue();

protected:
	QString m_savePath;

	void clear();
};

inline QUndoStack& theUndo() { return Song::get().m_undo; }
