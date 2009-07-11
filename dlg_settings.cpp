#include "stdafx.h"
#include "common.h"
#include "dlg_settings.h"
#include "audioio.h"
#include "cosecantmainwindow.h"
#include "cursor_raii.h"
#include "mygraphicsview.h"

void Dlg_Settings::run(QWidget *parent)
{
	Dlg_Settings dlg(parent);
	if (dlg.exec() == QDialog::Accepted)
	{
	}
}

Dlg_Settings::Dlg_Settings(QWidget *parent, Qt::WFlags flags)
: QDialog(parent, flags)
{
	ui.setupUi(this);

	populateAudioDeviceCombos();
	populateLanguageCombo();

	ui.checkGraphicsViewGL->setChecked(MyGraphicsView::s_prefOpenGL());
	ui.checkGraphicsViewAA->setChecked(MyGraphicsView::s_prefAntiAlias());
}

Dlg_Settings::~Dlg_Settings()
{
}

void Dlg_Settings::on_applyButton_clicked()
{
	apply();
}

void Dlg_Settings::on_okButton_clicked()
{
	if (apply()) accept();
}

bool Dlg_Settings::apply()
{
	QString error;
	if (!applyAudioDeviceSettings(error))
	{
		QMessageBox msg;
		msg.setIcon(QMessageBox::Critical);
		msg.setText(tr("Error opening audio device"));
		msg.setInformativeText(error);
		msg.exec();
		return false;
	}

	applyLanguageSettings();

	MyGraphicsView::s_prefOpenGL	.set(ui.checkGraphicsViewGL->isChecked());
	MyGraphicsView::s_prefAntiAlias	.set(ui.checkGraphicsViewAA->isChecked());

	// No errors, so...
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////

void Dlg_Settings::populateAudioDeviceCombos()
{
	int numDevices = Pa_GetDeviceCount();
	if (numDevices < 0)
	{
		THROW_ERROR(Error, "Pa_GetDeviceCount error");
	}

	ui.comboAudioInputDevice->addItem("None", QVariant::fromValue(paNoDevice));

	for (int i=0; i<numDevices; i++)
	{
		const PaDeviceInfo* dev = Pa_GetDeviceInfo(i);
		const PaHostApiInfo* api = Pa_GetHostApiInfo(dev->hostApi);
		qDebug() << i << dev->name << "    " << api->name << dev->maxInputChannels << dev->maxOutputChannels;
		if (dev->maxInputChannels > 0)
			addAudioDeviceToCombo(ui.comboAudioInputDevice, i, dev, api, dev->maxInputChannels);
		if (dev->maxOutputChannels > 0)
			addAudioDeviceToCombo(ui.comboAudioOutputDevice, i, dev, api, dev->maxOutputChannels);
	}

	if (!selectAudioDeviceInCombo(ui.comboAudioInputDevice,  AudioIO::get().getInDeviceIndex()))
		selectAudioDeviceInCombo (ui.comboAudioInputDevice,  paNoDevice);
	
	if (!selectAudioDeviceInCombo(ui.comboAudioOutputDevice, AudioIO::get().getOutDeviceIndex()))
		selectAudioDeviceInCombo (ui.comboAudioOutputDevice, Pa_GetDefaultOutputDevice());

	ui.comboAudioSamplerate->addItem("44100");
	ui.comboAudioSamplerate->addItem("48000");
	ui.comboAudioSamplerate->addItem("96000");
	ui.comboAudioSamplerate->setValidator(new QIntValidator(ui.comboAudioSamplerate));
}

void Dlg_Settings::addAudioDeviceToCombo(QComboBox* combo,
										 int index,
										 const PaDeviceInfo* dev,
										 const PaHostApiInfo* api,
										 int nchannels)
{
	//: %1 = API name (eg "ASIO"), %2 = soundcard name, %n = number of channels
	QString text = tr("%1 : %2 (%n channel(s))", "", nchannels)
		.arg(api->name).arg(dev->name);
	combo->addItem(text, QVariant::fromValue(index));
}

PaDeviceIndex Dlg_Settings::getAudioDeviceIndex(QComboBox* combo, int comboindex)
{
	if (comboindex == -1)
		comboindex = combo->currentIndex();

	if (comboindex == -1)
		return paNoDevice;
	else
		return combo->itemData(comboindex).value<int>();
}

int Dlg_Settings::findAudioDeviceInCombo(QComboBox* combo, PaDeviceIndex dev)
{
	return combo->findData(QVariant::fromValue(dev));
}

bool Dlg_Settings::selectAudioDeviceInCombo(QComboBox* combo, PaDeviceIndex dev)
{
	int i = findAudioDeviceInCombo(combo, dev);
	if (i != -1)
	{
		combo->setCurrentIndex(i);
		return true;
	}
	else
	{
		return false;
	}
}

void Dlg_Settings::on_comboAudioInputDevice_currentIndexChanged(int index)
{
}

void Dlg_Settings::on_comboAudioOutputDevice_currentIndexChanged(int index)
{
	PaDeviceIndex devindex = getAudioDeviceIndex(ui.comboAudioOutputDevice, index);
	bool isAsio = AudioIO::get().isOutputDeviceAsio(devindex);
	ui.buttonAsioPanel->setEnabled(isAsio);
	ui.checkAsioPanelClosesDevice->setEnabled(isAsio);
}

void Dlg_Settings::on_buttonAsioPanel_clicked()
{
	bool closeAudio = ui.checkAsioPanelClosesDevice->isChecked();

	if (closeAudio)
	{
		AudioIO::get().stop();
		AudioIO::get().close();
	}

#ifdef Q_OS_WIN32
	void* parent = static_cast<void*>(CosecantMainWindow::get()->winId());
#else
	void* parent = NULL;
#endif

	PaError result = PaAsio_ShowControlPanel(getAudioDeviceIndex(ui.comboAudioOutputDevice), parent);

	if (result == paNoError)
	{
		if (closeAudio)
		{
			QMessageBox msg;
			msg.setIcon(QMessageBox::Information);
			msg.setText(tr("When you have finished with the ASIO control panel, click OK to reopen the audio device."));
			msg.exec();
		}
	}
	else
	{
		QMessageBox msg;
		msg.setIcon(QMessageBox::Critical);
		msg.setText(tr("Failed to open ASIO control panel"));
		msg.setInformativeText(Pa_GetErrorText(result));
		msg.exec();
	}

	if (closeAudio)
	{
		AudioIO::get().open();
		AudioIO::get().start();
	}
}

bool Dlg_Settings::applyAudioDeviceSettings(QString& error)
{
	CursorRaii cursor(Qt::WaitCursor);

	AudioIO& aio = AudioIO::get();

	aio.stop();
	aio.close();

	aio.setDeviceIndexes( getAudioDeviceIndex(ui.comboAudioInputDevice), getAudioDeviceIndex(ui.comboAudioOutputDevice) );

	try
	{
		aio.open();
		aio.start();
		return true;
	}
	catch (const AudioIO::Error& err)
	{
		error = err.msg();
		return false;
	}
}

/////////////////////////////////////////////////////////////////////////////

void Dlg_Settings::populateLanguageCombo()
{
	QStringList languages = QDir().entryList(QStringList("cosecant_*.qm"), QDir::Files, QDir::Name);
	languages.replaceInStrings(QRegExp("^cosecant_(.*).qm$"), "\\1");

	QString syslocale = QLocale::system().name();
	//: System language
	ui.comboLanguage->addItem(tr("System (current: %1)").arg(syslocale), "system_locale");

	foreach(QString lang, languages)
	{
		ui.comboLanguage->addItem(lang, lang);
	}

	QString current = CosecantMainWindow::s_prefLanguage();
	int index = ui.comboLanguage->findData(current);
	if (index != -1)
	{
		ui.comboLanguage->setCurrentIndex(index);
	}
	else
	{
		//: Corresponds to a language set in the prefs file but with no corresponding .qm file installed
		ui.comboLanguage->addItem(tr("%1 (not installed)").arg(current), current);
		ui.comboLanguage->setCurrentIndex(ui.comboLanguage->count()-1);
	}
}

void Dlg_Settings::applyLanguageSettings()
{
	QString sel = ui.comboLanguage->itemData(ui.comboLanguage->currentIndex()).value<QString>();
	if (sel != CosecantMainWindow::s_prefLanguage())
	{
		CosecantMainWindow::s_prefLanguage.set(sel);

		const char* msgtext = QT_TR_NOOP(
			"Changes to language settings will take effect the next time Cosecant is started.");

		QMessageBox msg;
		msg.setIcon(QMessageBox::Information);
		msg.setText(tr(msgtext));

		QTranslator translator;
		translator.load("cosecant_" + sel);
		msg.setInformativeText(translator.translate("Dlg_Settings", msgtext));

		msg.exec();
	}
}
