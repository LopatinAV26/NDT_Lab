#pragma once

class ApplicationData;

class SettingsWindow
{
public:
	explicit SettingsWindow(ApplicationData &coreAppData);
	void Show(bool &isOpen);

private:
	void GetAppInformation() const;
	void SetVsyncMode();
	void SetGuiStyle();

	ApplicationData &appData;
};