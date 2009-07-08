#pragma once

#include "ui_settings_dlg.h"

class Dlg_Settings : public QDialog
{
	Q_OBJECT

public:
	Dlg_Settings(QWidget *parent = 0, Qt::WFlags flags = 0);
	~Dlg_Settings();

	static void run(QWidget *parent);

protected:
	void populateAudioDeviceCombos();
	void addAudioDeviceToCombo(QComboBox* combo, int index, const PaDeviceInfo* dev, const PaHostApiInfo* api,
								int nchannels);
	PaDeviceIndex getAudioDeviceIndex(QComboBox* combo, int comboindex = -1);

protected slots:
	void on_comboAudioInputDevice_currentIndexChanged(int index);
	void on_comboAudioOutputDevice_currentIndexChanged(int index);
	void on_buttonAsioPanel_clicked();

private:
	Ui::SettingsDlg ui;
};
