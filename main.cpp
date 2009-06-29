#include "stdafx.h"
#include "common.h"
#include "cosecantmainwindow.h"

#include "song.h"
#include "audioio.h"
#include "builtinmachines.h"
#include "theme.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	AudioIO::initSingleton();
	PaError err = AudioIO::get().open();
	if (err == paNoError)
	{
//		AudioIO::get().start();
	}

	initBuiltinMachineFactories();
	Song::initSingleton();
	Song::get().load(bpath(L"test1.csc"));

	Theme::initSingleton();

	CosecantMainWindow w;
	w.show();

	return a.exec();
}
