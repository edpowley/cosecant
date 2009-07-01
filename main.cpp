#include "stdafx.h"
#include "common.h"
#include "cosecantmainwindow.h"

#include "song.h"
#include "audioio.h"
#include "builtinmachines.h"
#include "machinedll.h"
#include "theme.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

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
