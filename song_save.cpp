#include "stdafx.h"
#include "common.h"

#include "song.h"
#include "xmlutils.h"
#include "version.h"
#include "parameter.h"

extern zlib_filefunc_def g_zipFileFuncs; // zipfilefuncs.cpp
static const int Z_STORE = 0;

void Song::save(const QString& filepath)
{
	zipFile zf = zipOpen2(filepath.toUtf8(), APPEND_STATUS_CREATE, NULL, &g_zipFileFuncs);
	if (!zf)
		throw SongSaveError(tr("Unable to open file '%1' for writing").arg(filepath));

	QDomDocument doc;
	QDomElement root = doc.createElement("song");
	setAttribute(root, "formatversion", 1);
	setAttribute(root, "cosecantversion", getVersionString());
	doc.appendChild(root);

	root.appendChild(m_routing->save(doc));

	zipOpenNewFileInZip(zf, "song.xml", NULL, NULL, 0, NULL, 0, NULL, Z_DEFLATED, Z_DEFAULT_COMPRESSION);
	QByteArray buf = doc.toByteArray();
	zipWriteInFileInZip(zf, buf.constData(), buf.size());
	zipCloseFileInZip(zf);

	zipClose(zf, NULL);
}

QDomElement Routing::save(QDomDocument& doc)
{
	QDomElement el = doc.createElement("routing");
	setAttribute(el, "uuid", m_objectUuid);

	foreach(const Ptr<Machine>& mac, m_machines)
	{
		el.appendChild(mac->save(doc));

		foreach(const Ptr<Pin>& pin, mac->m_outpins)
		{
			foreach(const Ptr<Connection>& conn, pin->m_connections)
			{
				el.appendChild(conn->save(doc));
			}
		}
	}

	return el;
}

QDomElement Machine::save(QDomDocument& doc)
{
	QDomElement el = doc.createElement("machine");
	setAttribute(el, "uuid", m_objectUuid);

	setAttribute(el, "typeid", m_id);
	setAttribute(el, "name", getName());
	setAttribute(el, "pos", m_pos);
	setAttribute(el, "hsize", m_halfsize);

	el.appendChild(m_params->save(doc));

	foreach(const Ptr<Pin>& pin, m_inpins)
	{
		el.appendChild(pin->save(doc));
	}

	foreach(const Ptr<Pin>& pin, m_outpins)
	{
		el.appendChild(pin->save(doc));
	}

	return el;
}

QDomElement Connection::save(QDomDocument& doc)
{
	QDomElement el = doc.createElement("connection");
	setAttribute(el, "uuid", m_objectUuid);

	setAttribute(el, "from", getPin1()->m_objectUuid);
	setAttribute(el, "to",   getPin2()->m_objectUuid);
	setAttribute(el, "feedback", m_feedback);

	return el;
}

QDomElement Pin::save(QDomDocument& doc)
{
	QDomElement el = doc.createElement(m_direction == in ? "inpin" : "outpin");
	setAttribute(el, "uuid", m_objectUuid);

	setAttribute(el, "name", m_name);
	setAttribute(el, "type", m_type);
	setAttribute(el, "side", getSide());
	setAttribute(el, "pos", getPos());

	return el;
}

QDomElement ParamPin::save(QDomDocument& doc)
{
	QDomElement el = Pin::save(doc);
	setAttribute(el, "param", getParam()->m_objectUuid);
	setAttribute(el, "paramtimeunit", getTimeUnit());

	return el;
}

QDomElement Parameter::Base::save(QDomDocument& doc)
{
	QDomElement el = doc.createElement("parameter");
	setAttribute(el, "uuid", m_objectUuid);

	setAttribute(el, "name", getName());
	setAttribute(el, "tag", getTag());

	return el;
}

QDomElement Parameter::Scalar::save(QDomDocument& doc)
{
	QDomElement el = Base::save(doc);
	setAttribute(el, "state", getState());
	return el;
}

QDomElement Parameter::Time::save(QDomDocument& doc)
{
	QDomElement el = Scalar::save(doc);
	setAttribute(el, "displayunit", getDisplayUnit());
	return el;
}

QDomElement Parameter::Group::save(QDomDocument& doc)
{
	QDomElement el = Base::save(doc);
	el.setTagName("parametergroup");

	foreach(const Ptr<Base>& param, m_params)
	{
		el.appendChild(param->save(doc));
	}

	return el;
}
