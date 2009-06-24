#pragma once

class Error : public std::exception
{
protected:
	QString m_msg;
public:
	Error() {}
	Error(const QString& msg) : m_msg(msg) {}
	virtual const char* what() const { return m_msg.toUtf8(); }
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

#define ERROR_CLASS(classname) \
	class classname : public ::Error \
	{ public: classname() {} ; classname(const QString& msg) : ::Error(msg) {} };

ERROR_CLASS(AssertionFailure)

#define ASSERT(expr)									\
	{													\
		if (!(expr))									\
			THROW_ERROR(AssertionFailure, #expr);		\
	}													\
// end define ASSERT
