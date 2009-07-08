#include "stdafx.h"
#include "common.h"
#include "dlg_settings.h"
#include "audioio.h"
#include "cosecantmainwindow.h"
#include "cursor_raii.h"

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
}

Dlg_Settings::~Dlg_Settings()
{
}

void Dlg_Settings::on_okButton_clicked()
{
	QString error;
	if (!applyAudioDeviceSettings(error))
	{
		QMessageBox msg;
		msg.setIcon(QMessageBox::Critical);
		msg.setText(tr("Error opening audio device"));
		msg.setInformativeText(error);
		msg.exec();
		return;
	}

	// No errors, so...
	accept();
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
