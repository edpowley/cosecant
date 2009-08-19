#pragma once

#include "prefs.h"
#include "cosecantmainwindow.h"

class Application : public QApplication
{
	Q_OBJECT

protected:
	Application(int& argc, char** argv);
	void init();
	static SingletonPtr<Application> s_singleton;

public:
	static Application& initSingleton(int& argc, char** argv);
	static Application& get() { return *s_singleton; }

	CosecantMainWindow* getMainWindow() { return m_mainWindow; }

	QScriptEngine* getScriptEngine() { return m_scriptEngine; }
	QScriptEngineDebugger* getScriptDebugger() { return m_scriptDebugger; }
	void attachScriptDebugger() { m_scriptDebugger->attachTo(m_scriptEngine); }
	void detachScriptDebugger() { m_scriptDebugger->detach(); }

	static PrefsVar_String s_prefLanguage;

	void pushStatusMsg(const QString& msg);
	void popStatusMsg();

	bool notify(QObject* receiver, QEvent* ev);

protected slots:
	void onAboutToQuit();

	void onScriptSuspended();
	void onScriptResumed();

protected:
	QScriptEngine* m_scriptEngine;
	QScriptEngineDebugger* m_scriptDebugger;
	CosecantMainWindow* m_mainWindow;
	QSplashScreen* m_splashScreen;

	QStringList m_statusStack;
	void writeStatusStackToSplashScreen();

	void setupI18n();
	void setupAudio();
	void setupScriptEngine();

	static void textMessageHandler(QtMsgType type, const char* msg);
};
