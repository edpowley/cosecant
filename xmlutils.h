#pragma once

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

/////////////////////////////////////////////////////////////////////////

template<typename T> T getAttribute(xmlpp::Element* el, const QString& name)
{
	xmlpp::Attribute* attr = el->get_attribute(q2ustring(name));
	if (!attr)
	{
		THROW_ERROR(MissingAttributeError, QString(
			"'%1' element at line %2 has no '%3' attribute").arg(el->get_name().c_str()).arg(el->get_line()).arg(name));
	}

	std::wistringstream ss(ustring_to_wstring(attr->get_value()));
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
		THROW_ERROR(MissingAttributeError, QString(
			"'%3' attribute of '%1' element at line %2 is not of the correct type (expected %4)\n%5")
			.arg(el->get_name().c_str()).arg(el->get_line()).arg(name).arg(typeid(T).name()).arg(err.what()) );
	}
}

template<> inline QString getAttribute<QString>(xmlpp::Element* el, const QString& name)
{
	xmlpp::Attribute* attr = el->get_attribute(q2ustring(name));
	if (!attr)
	{
		THROW_ERROR(MissingAttributeError, QString(
			"'%1' element at line %2 has no '%3' attribute").arg(el->get_name().c_str()).arg(el->get_line()).arg(name));
	}

	return attr->get_value().c_str();
}

template<> inline Uuid getAttribute<Uuid>(xmlpp::Element* el, const QString& name)
{
	return Uuid(getAttribute<QString>(el, name));
}

template<> inline QColor getAttribute<QColor>(xmlpp::Element* el, const QString& name)
{
	return QColor(getAttribute<QString>(el, name));
}

////////////////////////////////////////////////////////////////////////

template<typename T> void setAttribute(xmlpp::Element* el, const QString& name, const T& x)
{
	el->set_attribute(q2ustring(name), q2ustring(QString("%1").arg(x)));
}

template<> inline void setAttribute<QString>(xmlpp::Element* el, const QString& name, const QString& x)
{
	el->set_attribute(q2ustring(name), q2ustring(x));
}

template<> inline void setAttribute<QColor>(xmlpp::Element* el, const QString& name, const QColor& x)
{
	el->set_attribute(q2ustring(name), q2ustring(x.name()));
}

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
