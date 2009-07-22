#pragma once

class Error : public std::exception
{
protected:
	QString m_msg;
	QByteArray m_msgUtf8;
public:
	Error() {}
	Error(const QString& msg) : m_msg(msg) { m_msgUtf8 = m_msg.toUtf8(); }
	virtual const char* what() const { return m_msgUtf8; }
	QString msg() const { return m_msg; }
};


#define STRINGISE2(x) #x
#define STRINGISE(x) STRINGISE2(x)

#define THROW_ERROR(xxxtype,xxxmsg)						\
	{													\
		QString msg(									\
			#xxxtype "\n  "								\
			__FILE__ ":" STRINGISE(__LINE__) "\n  "		\
			__FUNCSIG__ "\n  ");						\
		msg += xxxmsg;									\
		throw xxxtype(msg);								\
	}													\
// end define THROW_ERROR

#define ERROR_CLASS_2(classname, baseclassname) \
	class classname : public baseclassname \
	{ public: classname() {} ; classname(const QString& msg) : baseclassname(msg) {} };

#define ERROR_CLASS(classname) ERROR_CLASS_2(classname, ::Error)

ERROR_CLASS(AssertionFailure)

#define ASSERT(expr)									\
	{													\
		if (!(expr))									\
			THROW_ERROR(AssertionFailure, #expr);		\
	}													\
// end define ASSERT
