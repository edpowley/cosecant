#include "stdafx.h"
#include "common.h"
#include "application.h"

#include "song.h"
#include "audioio.h"
#include "builtinmachines.h"
#include "dllmachine.h"
#include "theme.h"
#include "htmlentity.h"
#include "prefs.h"

PrefsVar_String Application::s_prefLanguage("app/language", "system_locale");

Application::Application(int& argc, char** argv)
: QApplication(argc, argv)
{
	setupI18n();

	initHtmlEntityMap();

	setupAudio();

	initBuiltinMachineFactories();
	DllMachineFactory::scan(QCoreApplication::applicationDirPath() + "/gear");
	Song::initSingleton();

	Theme::initSingleton();

	setupScriptEngine();

	m_mainWindow = new CosecantMainWindow;
	m_mainWindow->showMaximized();
}

void Application::setupI18n()
{
	QString language = s_prefLanguage();
	if (language == "system_locale") language = QLocale::system().name();
	qDebug() << "Language:" << language;

	QTranslator* qtTranslator = new QTranslator(this);
	qtTranslator->load("qt_" + language,
		QLibraryInfo::location(QLibraryInfo::TranslationsPath));
	installTranslator(qtTranslator);

	QTranslator* myappTranslator = new QTranslator(this);
	myappTranslator->load("cosecant_" + language);
	installTranslator(myappTranslator);
}

void Application::setupAudio()
{
	AudioIO::initSingleton();

	try
	{
		AudioIO::get().open();
		AudioIO::get().start();
	}
	catch (const AudioIO::Error& err)
	{
		QMessageBox msg;
		msg.setIcon(QMessageBox::Critical);
		msg.setText(QApplication::tr("Error opening audio device. Choose 'Settings' from the 'View' menu to change "
			"your audio device settings."));
		msg.setInformativeText(err.msg());
		msg.exec();
	}
}

void Application::setupScriptEngine()
{
	m_scriptEngine = new QScriptEngine(this);
	m_scriptDebugger = new QScriptEngineDebugger(this);

	QStringList extensions = QString("qt.core qt.gui qt.xml").split(' ');
	foreach(QString extension, extensions)
	{
		QScriptValue v = m_scriptEngine->importExtension(extension);
		if (m_scriptEngine->hasUncaughtException())
		{
			qDebug() << "Failed to import extension" << extension << ":" << v.toString();
		}
	}

	QScriptValue v = m_scriptEngine->evaluate("var v = { a: 1, b: 2, c: 3 }; v");
	qDebug() << v.toString();
}
