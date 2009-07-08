#include "stdafx.h"
#include "common.h"
#include "dlg_settings.h"
#include "audioio.h"
#include "cosecantmainwindow.h"

void Dlg_Settings::run(QWidget *parent)
{
	Dlg_Settings dlg(parent);
	dlg.exec();
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
		if (dev->maxInputChannels > 0)
			addAudioDeviceToCombo(ui.comboAudioInputDevice, i, dev, api, dev->maxInputChannels);
		if (dev->maxOutputChannels > 0)
			addAudioDeviceToCombo(ui.comboAudioOutputDevice, i, dev, api, dev->maxOutputChannels);
	}
}

void Dlg_Settings::addAudioDeviceToCombo(QComboBox* combo,
										 int index,
										 const PaDeviceInfo* dev,
										 const PaHostApiInfo* api,
										 int nchannels)
{
	//: %1 = API name (eg "ASIO"), %2 = soundcard name, %3 = number of channels
	QString text = tr("%1: %2 (%3 channels)", NULL, nchannels)
		.arg(api->name).arg(dev->name)
		.arg(nchannels);
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
