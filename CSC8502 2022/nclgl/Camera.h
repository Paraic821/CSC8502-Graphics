#pragma once
#include "Matrix4.h"
#include "Vector3.h"
#include <vector>

class Camera {
public:
	Camera(void) {
		yaw = 0.0f;
		pitch = 0.0f;
	};

	Camera(float pitch, float yaw, Vector3 position) {
		this->pitch = pitch;
		this->yaw = yaw;
		this->position = position;

		InitialisePositions();
	}

	~Camera(void) {};

	void UpdateCamera(float dt = 1.0f);

	void UpdateCamera(float dt = 1.0f, double totalTime = 1.0);

	Matrix4 BuildViewMatrix();

	Vector3 GetPosition() const { return position; }
	void SetPosition(Vector3 val) { position = val; }

	float GetYaw() const { return yaw; }
	void SetYaw(float y) { yaw = y; }

	float GetPitch() const { return pitch; }
	void SetPitch(float p) { pitch = p; }

protected:
	float yaw;
	float pitch;
	Vector3 position; // Set to 0 ,0 ,0 by Vector3 constructor ;)
	bool hasControl = false;

	std::vector<Vector3> cameraPositions;
	std::vector<float> cameraPitches;
	std::vector<float> cameraYaws;

	Vector3 previousPosition;
	Vector3 nextPosition;
	float previousYaw;
	float previousPitch;
	float nextYaw;
	float nextPitch;

	bool started = false;
	bool cooldown = false;
	double cooldownTimer = 0.0f;

	double startTime;
	double timeSinceStart;

	void InitialisePositions();
	void ControlCamera(float dt);
	void Interpolate(float dt, double totalTime);
};