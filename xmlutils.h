#pragma once

#include "cosecant_api.h"

ERROR_CLASS(XmlError);
ERROR_CLASS_2(XmlOpenError, XmlError);
ERROR_CLASS_2(MissingAttributeError, XmlError);
ERROR_CLASS_2(MalformedAttributeError, XmlError);
ERROR_CLASS_2(ChildElementError, XmlError);

////////////////////////////////////////////////////////////////////////

inline QString getAttributeString(const QDomElement& el, const QString& name)
{
	if (el.hasAttribute(name))
		return el.attribute(name);
	else
		throw MissingAttributeError(QString("'%1' element at line %2 has no '%3' attribute")
				.arg(el.tagName()).arg(el.lineNumber()).arg(name));
}

//////////////////////////////////////////////////////////////////////////

template<typename NumType> NumType getAttribute(const QDomElement& el, const QString& name)
{
	bool ok;
	qlonglong val = getAttributeString(el, name).toLongLong(&ok);
	if (!ok)
		throw MalformedAttributeError(QString("'%3' attribute of '%1' element at line %2 is not an integer")
			.arg(el.tagName()).arg(el.lineNumber()).arg(name) );
	return static_cast<NumType>(val);
}

template<typename NumType> void setAttribute(QDomElement& el, const QString& name, const NumType& x)
{	el.setAttribute(name, QString::number(x));   }

///////////////////////////////////////////////////////////////////////////

template<> inline QString getAttribute<QString>(const QDomElement& el, const QString& name)
{	return getAttributeString(el, name);   }

template<> inline void setAttribute<QString>(QDomElement& el, const QString& name, const QString& x)
{	el.setAttribute(name, x);   }

///////////////////////////////////////////////////////////////////////////

template<> inline double getAttribute<double>(const QDomElement& el, const QString& name)
{
	bool ok;
	double val = getAttributeString(el, name).toDouble(&ok);
	if (!ok)
		throw MalformedAttributeError(QString("'%3' attribute of '%1' element at line %2 is not a double")
			.arg(el.tagName()).arg(el.lineNumber()).arg(name) );
	return val;
}

template<> inline void setAttribute<double>(QDomElement& el, const QString& name, const double& x)
{	el.setAttribute(name, QString::number(x, 'g', 20));   }

///////////////////////////////////////////////////////////////////////////

template<> inline float getAttribute<float>(const QDomElement& el, const QString& name)
{	return getAttribute<double>(el, name);   }

template<> inline void setAttribute<float>(QDomElement& el, const QString& name, const float& x)
{	el.setAttribute(name, QString::number(x, 'g', 20));   }

///////////////////////////////////////////////////////////////////////////

template<> inline bool getAttribute<bool>(const QDomElement& el, const QString& name)
{
	QString val = getAttributeString(el, name).toLower();
	if (val == "true")
		return true;
	else if (val == "false")
		return false;
	else
		throw MalformedAttributeError(QString("'%3' attribute of '%1' element at line %2 is not a bool")
			.arg(el.tagName()).arg(el.lineNumber()).arg(name) );
}

template<> inline void setAttribute<bool>(QDomElement& el, const QString& name, const bool& x)
{	el.setAttribute(name, x ? "true" : "false");   }

///////////////////////////////////////////////////////////////////////////

template<> inline QColor getAttribute<QColor>(const QDomElement& el, const QString& name)
{
	QColor val(getAttributeString(el, name));
	if (!val.isValid())
		throw MalformedAttributeError(QString("'%3' attribute of '%1' element at line %2 is not a QColor")
			.arg(el.tagName()).arg(el.lineNumber()).arg(name) );
	return val;
}

template<> inline void setAttribute<QColor>(QDomElement& el, const QString& name, const QColor& x)
{	el.setAttribute(name, x.name());   }

///////////////////////////////////////////////////////////////////////////

template<> inline Uuid getAttribute<Uuid>(const QDomElement& el, const QString& name)
{
	try
	{
		return Uuid(getAttributeString(el, name));
	}
	catch (const Uuid::Error& err)
	{
		throw MalformedAttributeError(QString("'%3' attribute of '%1' element at line %2 is not a Uuid")
			.arg(el.tagName()).arg(el.lineNumber()).arg(name) );
	}
}

template<> inline void setAttribute<Uuid>(QDomElement& el, const QString& name, const Uuid& x)
{	setAttribute(el, name, x.str());   }

///////////////////////////////////////////////////////////////////////////

template<> inline QPointF getAttribute<QPointF>(const QDomElement& el, const QString& name)
{
	QStringList split = getAttributeString(el, name).split(';');
	if (split.length() != 2)
		throw MalformedAttributeError(QString("'%3' attribute of '%1' element at line %2 is not a QPointF")
			.arg(el.tagName()).arg(el.lineNumber()).arg(name) );

	bool okx, oky;
	double x = split[0].toDouble(&okx);
	double y = split[1].toDouble(&oky);
	if (!okx || !oky)
		throw MalformedAttributeError(QString("'%3' attribute of '%1' element at line %2 is not a QPointF")
			.arg(el.tagName()).arg(el.lineNumber()).arg(name) );

	return QPointF(x,y);
}

template<> inline void setAttribute<QPointF>(QDomElement& el, const QString& name, const QPointF& x)
{	el.setAttribute(name, QString("%1;%2").arg(x.x(), 0, 'g', 20).arg(x.y(), 0, 'g', 20));   }

/////////////////////////////////////////////////////////////////////////

template<typename T> T getAttribute(const QDomElement& el, const QString& name, const T& def)
{
	try
	{
		return getAttribute<T>(el, name);
	}
	catch (const MissingAttributeError& err)
	{
		return def;
	}
}

/////////////////////////////////////////////////////////////////////////

inline QList<QDomElement> getChildElements(const QDomElement& parent, const QString& tagName)
{
	QList<QDomElement> ret;
	for (QDomElement el = parent.firstChildElement(tagName); !el.isNull(); el = el.nextSiblingElement(tagName))
	{
		ret << el;
	}
	return ret;
}

inline QDomElement getUniqueChildElement(const QDomElement& parent, const QString& tagName)
{
	QList<QDomElement> els = getChildElements(parent, tagName);
	if (els.isEmpty())
		throw ChildElementError(QString("'%1' element at line %2 has no child element '%3'")
				.arg(parent.tagName()).arg(parent.lineNumber()).arg(tagName));
	else if (els.length() > 1)
		throw ChildElementError(QString("'%1' element at line %2 has multiple child elements '%3'")
				.arg(parent.tagName()).arg(parent.lineNumber()).arg(tagName));
	else
		return els.first();
}

///////////////////////////////////////////////////////////////////////////

inline QDomDocument openXml(const QString& fname)
{
	QFile docfile(fname);
	if (!docfile.open(QIODevice::ReadOnly))
		throw XmlOpenError(QString("Failed to open file '%1': QFile error %2").arg(fname).arg(docfile.error()));

	QDomDocument doc(fname);
	QString errormsg; int errorline, errorcolumn;
	if (!doc.setContent(&docfile, &errormsg, &errorline, &errorcolumn))
	{
		throw XmlOpenError(QString("Failed to parse file '%1': '%2' at line %3 column %4")
			.arg(fname).arg(errormsg).arg(errorline).arg(errorcolumn));
	}

	return doc;
}
