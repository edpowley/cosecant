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
#include "version.h"
#include "keyjazz.h"

PrefsVar_String Application::s_prefLanguage("app/language", "system_locale");

SingletonPtr<Application> Application::s_singleton;

Application& Application::initSingleton(int& argc, char** argv)
{
	s_singleton.set(new Application(argc, argv));
	get().init();
	return get();
}

Application::Application(int& argc, char** argv)
:	QApplication(argc, argv),
	m_mainWindow(NULL), m_splashScreen(NULL)
{
}

bool Application::notify(QObject* receiver, QEvent* ev)
{
	if (ev->type() >= QEvent::User)
	{
		// Propagate custom events to parents
		for (; receiver; receiver = receiver->parent())
		{
			QApplication::notify(receiver, ev);
			if (ev->isAccepted()) return true;
		}
		return false;
	}
	else
		return QApplication::notify(receiver, ev);
}

void Application::init()
{
	qInstallMsgHandler(&Application::textMessageHandler);

	QPixmap splashpic(":/CosecantMainWindow/images/splash.png");
	{
		QPainter painter(&splashpic);
		QFont font = painter.font();
		font.setPixelSize(20);
		painter.setFont(font);
		painter.drawText(splashpic.rect().adjusted(-10,-10,-10,-10), Qt::AlignRight | Qt::AlignBottom, getVersionString());
	}

#ifndef _DEBUG
	Qt::WindowFlags splashflags = Qt::WindowStaysOnTopHint;
#else
	Qt::WindowFlags splashflags = 0; // don't stay on top in debug build
#endif

	m_splashScreen = new QSplashScreen(splashpic, splashflags);
	m_splashScreen->setAttribute(Qt::WA_DeleteOnClose);
	m_splashScreen->show();
	processEvents();

	setupI18n();

	initHtmlEntityMap();
	KeyJazz::initSingleton();

	pushStatusMsg(tr("Scanning builtin machines"));
	initBuiltinMachineFactories();
	popStatusMsg();

	pushStatusMsg(tr("Scanning machine plugins"));
	{
		QStringList dirs = PrefsFile::get()
			->getDirList("builtin/native", tr("Native plugins"))
			->getDirs();
		dirs.prepend(QCoreApplication::applicationDirPath() + "/gear");

		foreach(const QString& dir, dirs)
		{
			pushStatusMsg(tr("Scanning %1").arg(dir));
			DllMachine::Factory::scan(dir);
			popStatusMsg();
		}
	}
	popStatusMsg();

	pushStatusMsg(tr("Creating song"));
	Song::initSingleton();
	SeqPlay::initSingleton(Song::get().m_sequence);
	popStatusMsg();

	pushStatusMsg(tr("Initialising theme"));
	Theme::initSingleton();
	popStatusMsg();

	pushStatusMsg(tr("Opening audio device"));
	setupAudio();
	popStatusMsg();

	m_mainWindow = new CosecantMainWindow;
	m_splashScreen->finish(m_mainWindow);
	m_splashScreen = NULL;
	m_mainWindow->showMaximized();

	connect( this, SIGNAL(aboutToQuit()), this, SLOT(onAboutToQuit()) );
}

void Application::onAboutToQuit()
{
	qDebug() << "About to quit";
	AudioIO::killSingleton();
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

void Application::textMessageHandler(QtMsgType type, const char *msg)
{
	QString str;

	switch (type)
	{
	case QtDebugMsg:	str = "Dbg: "; break;
	case QtWarningMsg:	str = "Wrn: "; break;
	case QtCriticalMsg:	str = "Crt: "; break;
	case QtFatalMsg:	str = "Ftl: "; break;
	default:			str = "???: "; break;
	}

	str.append(msg);
	str.append("\n");
	OutputDebugStringA(str.toAscii());
}

void Application::pushStatusMsg(const QString& msg)
{
	qDebug() << "Status +" << msg;
	m_statusStack.append(msg);
	if (m_splashScreen) writeStatusStackToSplashScreen();
}

void Application::popStatusMsg()
{
	if (m_statusStack.isEmpty()) return;
	qDebug() << "Status -" << m_statusStack.last();
	m_statusStack.removeLast();
	if (m_splashScreen) writeStatusStackToSplashScreen();
}

void Application::writeStatusStackToSplashScreen()
{
	if (!m_splashScreen) return;

	QString msg, indent;
	foreach(const QString& line, m_statusStack)
	{
		msg += indent + line + "\n";
		indent += "   ";
	}

	m_splashScreen->showMessage(msg);
}
