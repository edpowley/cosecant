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
	PaError err = AudioIO::get().open();
	if (err == paNoError)
	{
		AudioIO::get().start();
	}

	initBuiltinMachineFactories();
	DllMachineFactory::scan(QCoreApplication::applicationDirPath() + "/gear");
	Song::initSingleton();

	Theme::initSingleton();

	CosecantMainWindow w;
	w.showMaximized();

	return a.exec();
}
