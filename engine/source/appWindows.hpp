#pragma once

#include <vector>
#include <string>
#include "imgui.h"
#include "imgui_stdlib.h"
#include "implot.h"

#include "resourceManager.hpp"
#include "applicationData.hpp"

class SettingsWindow
{
public:
	explicit SettingsWindow(ApplicationData &coreAppData);
	void Show(bool &isOpen);

private:
	void SetGuiStyle() const;

	ApplicationData &appData;
};

class NomogramWindow
{
public:
	explicit NomogramWindow(ApplicationData &coreAppData);
	void Show(bool &isOpen);

private:
	struct CurvesRef
	{
		const std::vector<float> x;
		const std::vector<float> y;
		ImVec4 color{1.0f, 1.0f, 1.0f, 1.0f};
		std::string label;
	};

	std::vector<NDT::XrayDevice> ExposureRecalculation(const std::vector<NDT::XrayDevice> &deviceVector,
													   float distance, float current) const;

	void DrawMarkers(const std::vector<CurvesRef> &curves, float thickness) const;

	enum class Measure
	{
		mAxmin,
		min,
		sec
	};
	std::string nameAxisY = "Экспозиция, мА х мин";
	int deviceIndex{0};
	Measure measureIndex = Measure::mAxmin;
	float focusDistance{700.f};
	float steelThickness{10.f};
	float deviceCurrent{1.f};
	float steelThicknessMin{1.f};
	float steelThicknessMax{10.f};
	std::vector<NDT::XrayDevice> devices;
	std::vector<NDT::XrayDevice> calculatedDevices;
	ApplicationData &appData;
};