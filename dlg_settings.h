#pragma once

#include "ui_settings_dlg.h"
#include "prefs.h"

class Dlg_Settings : public QDialog
{
	Q_OBJECT

public:
	Dlg_Settings(QWidget *parent = 0, Qt::WFlags flags = 0);
	~Dlg_Settings();

	static void run(QWidget *parent);

protected:
	bool apply();

	void populateAudioDeviceCombos();
	void addAudioDeviceToCombo(QComboBox* combo, int index, const PaDeviceInfo* dev, const PaHostApiInfo* api,
								int nchannels);
	PaDeviceIndex getAudioDeviceIndex(QComboBox* combo, int comboindex = -1);
	int findAudioDeviceInCombo(QComboBox* combo, PaDeviceIndex dev);
	bool selectAudioDeviceInCombo(QComboBox* combo, PaDeviceIndex dev);

	bool applyAudioDeviceSettings(QString& error);

	void populateLanguageCombo();
	void applyLanguageSettings();

	enum
	{
		itemIsPath = Qt::UserRole,
		childrenHaveChanged,
	};

	void populatePaths();
	void populatePaths(const Ptr<PrefsDirList>& dirs);
	void applyPathSettings();

protected slots:
	void on_okButton_clicked();
	void on_applyButton_clicked();

	void on_comboAudioInputDevice_currentIndexChanged(int index);
	void on_comboAudioOutputDevice_currentIndexChanged(int index);
	void on_buttonAsioPanel_clicked();

	void on_treePaths_currentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);
	void on_buttonPathBrowse_clicked();
	void on_buttonPathAdd_clicked();
	void on_buttonPathEdit_clicked();
	void on_buttonPathRemove_clicked();

private:
	Ui::SettingsDlg ui;
};
