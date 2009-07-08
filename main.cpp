#include "stdafx.h"
#include "common.h"
#include "cosecantmainwindow.h"

#include "song.h"
#include "audioio.h"
#include "builtinmachines.h"
#include "dllmachine.h"
#include "theme.h"
#include "htmlentity.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	initHtmlEntityMap();

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

	initBuiltinMachineFactories();
	DllMachineFactory::scan(QCoreApplication::applicationDirPath() + "/gear");
	Song::initSingleton();

	Theme::initSingleton();

	CosecantMainWindow w;
	w.showMaximized();

	return a.exec();
}
