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
#include "seqplay.h"

PrefsVar_String Application::s_prefLanguage("app/language", "system_locale");

Application::Application(int& argc, char** argv)
: QApplication(argc, argv)
{
	QPixmap splashpic(":/CosecantMainWindow/images/splash.png");

#ifndef _DEBUG
	Qt::WindowFlags splashflags = Qt::WindowStaysOnTopHint;
#else
	Qt::WindowFlags splashflags = 0; // don't stay on top in debug build
#endif

	QSplashScreen* splash = new QSplashScreen(splashpic, splashflags);
	splash->setAttribute(Qt::WA_DeleteOnClose);
	splash->show();
	processEvents();

	setupI18n();

	initHtmlEntityMap();

	SeqPlay::initSingleton();

	splash->showMessage(tr("Opening audio device"));
	setupAudio();

	splash->showMessage(tr("Scanning builtin machines"));
	initBuiltinMachineFactories();

	splash->showMessage(tr("Scanning machines"));
	DllMachineFactory::scan(QCoreApplication::applicationDirPath() + "/gear");

	splash->showMessage(tr("Creating song"));
	Song::initSingleton();

	splash->showMessage(tr("Initialising theme"));
	Theme::initSingleton();

	splash->showMessage(tr("Initialising script engine"));
	setupScriptEngine();

	splash->showMessage(tr("Getting ready to rok"));
	m_mainWindow = new CosecantMainWindow;
	splash->finish(m_mainWindow);
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
}
