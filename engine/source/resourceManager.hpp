#pragma once

#include <vector>
#include <filesystem>
#include <string>
#include <SDL3_image/SDL_image.h>

struct SDL_Renderer;

struct Curve
{
	std::string label;
	std::vector<float> xData;
	std::vector<float> yData;
	float voltage = {1.f};
};

struct XrayDevice
{
	std::string name;
	std::string information;
	std::vector<Curve> curveVector;
	float electricPower = {1.f};
	float voltageMaximum = {200.f};
	float currentMinimum = {0.1f};
	float currentMaximum = {1.f};
	float focusDistanceDefault = {700.f};
	float focalSpotSize = {1.f};
	float exposureMultiplier = {1.f};
	bool currentAdjustment = {true};
};

class ResourceManager
{
public:
	explicit ResourceManager(SDL_Renderer *ren);
	~ResourceManager();

	std::vector<XrayDevice> LoadDevices(const std::filesystem::path &pathToDevices);

private:
	SDL_Renderer *renderer = nullptr;
};
