#pragma once
#include <Objects/Components/Component.h>
#include <World/Graphics.h>

class PointLightComponent : public Component
{
public:
	void Start() override;
	void Tick() override;
	void Destroy() override;

	void SetRelativeLocation(Vector3 NewLocation);
	Vector3 GetRelativeLocation();

	void SetColor(Vector3 NewColor);
	Vector3 GetColor();

	void SetIntensity(float NewIntensity);
	float GetIntensity();

	void SetFalloff(float NewFalloff);
	float GetFalloff();

protected:
	Graphics::Light CurrentLight;
	Graphics::Light PreviousLight;

	void Update();
	size_t GetLightIndex();
};