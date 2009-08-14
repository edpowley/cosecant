#pragma once

#include "cosecant_api.h"

ERROR_CLASS(XmlError);
ERROR_CLASS_2(XmlOpenError, XmlError);
ERROR_CLASS_2(MissingAttributeError, XmlError);
ERROR_CLASS_2(MalformedAttributeError, XmlError);

template<typename T> T getAttribute(const QDomElement& el, const QString& name)
{
	if (!el.hasAttribute(name))
	{
		THROW_ERROR(MissingAttributeError, QString(
			"'%1' element at line %2 has no '%3' attribute").arg(el.tagName()).arg(el.lineNumber()).arg(name));
	}

	std::wistringstream ss(el.attribute(name).toStdWString());
	ss >> std::boolalpha;
	ss.exceptions(std::wistringstream::failbit | std::wistringstream::badbit);
	try
	{
		T t;
		ss >> t;
		return t;
	}
	catch (const std::wistringstream::failure& err)
	{
		THROW_ERROR(MalformedAttributeError, QString(
			"'%3' attribute of '%1' element at line %2 is not of the correct type (expected %4)\n%5")
			.arg(el.tagName()).arg(el.lineNumber()).arg(name).arg(typeid(T).name()).arg(err.what()) );
	}
}

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

template<> inline QString getAttribute<QString>(const QDomElement& el, const QString& name)
{
	if (!el.hasAttribute(name))
	{
		THROW_ERROR(MissingAttributeError, QString(
			"'%1' element at line %2 has no '%3' attribute").arg(el.tagName()).arg(el.lineNumber()).arg(name));
	}

	return el.attribute(name);
}

template<> inline Uuid getAttribute<Uuid>(const QDomElement& el, const QString& name)
{
	return Uuid(getAttribute<QString>(el, name));
}

template<> inline QColor getAttribute<QColor>(const QDomElement& el, const QString& name)
{
	return QColor(getAttribute<QString>(el, name));
}

////////////////////////////////////////////////////////////////////////

template<typename NumType> void setAttribute(QDomElement& el, const QString& name, const NumType& x)
{	el.setAttribute(name, QString::number(x));   }

template<> inline void setAttribute<QString>(QDomElement& el, const QString& name, const QString& x)
{	el.setAttribute(name, x);   }

template<> inline void setAttribute<double>(QDomElement& el, const QString& name, const double& x)
{	el.setAttribute(name, QString::number(x, 'g', 20));   }

template<> inline void setAttribute<float>(QDomElement& el, const QString& name, const float& x)
{	el.setAttribute(name, QString::number(x, 'g', 20));   }

template<> inline void setAttribute<bool>(QDomElement& el, const QString& name, const bool& x)
{	el.setAttribute(name, x ? "true" : "false");   }

template<> inline void setAttribute<QColor>(QDomElement& el, const QString& name, const QColor& x)
{	el.setAttribute(name, x.name());   }

template<> inline void setAttribute<Uuid>(QDomElement& el, const QString& name, const Uuid& x)
{	setAttribute(el, name, x.str());   }

template<> inline void setAttribute<QPointF>(QDomElement& el, const QString& name, const QPointF& x)
{	el.setAttribute(name, QString("%1;%2").arg(x.x(), 0, 'g', 20).arg(x.y(), 0, 'g', 20));   }

/////////////////////////////////////////////////////////////////////////

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
