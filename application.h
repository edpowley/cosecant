#pragma once

#include "prefs.h"
#include "cosecantmainwindow.h"

class Application : public QApplication
{
	Q_OBJECT

public:
	Application(int& argc, char** argv);

	QScriptEngine* getScriptEngine() { return m_scriptEngine; }
	QScriptEngineDebugger* getScriptDebugger() { return m_scriptDebugger; }
	void attachScriptDebugger() { m_scriptDebugger->attachTo(m_scriptEngine); }
	void detachScriptDebugger() { m_scriptDebugger->detach(); }

	static PrefsVar_String s_prefLanguage;

protected slots:
	void onAboutToQuit();

protected:
	QScriptEngine* m_scriptEngine;
	QScriptEngineDebugger* m_scriptDebugger;
	CosecantMainWindow* m_mainWindow;

	void setupI18n();
	void setupAudio();
	void setupScriptEngine();
};
