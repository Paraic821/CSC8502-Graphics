#include "Camera.h"
#include "Window.h"
#include <algorithm>

void Camera::UpdateCamera(float dt) {

	ControlCamera(dt);
}

void Camera::UpdateCamera(float dt, double totalTime) {
	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_1)) {
		hasControl = !hasControl;
	}
	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_2)) {
		started = true;
		startTime = totalTime;
	}

	if (hasControl)
		ControlCamera(dt);
	else if(started)
		Interpolate(dt, totalTime);
}

void Camera::Interpolate(float dt, double totalTime) {

	timeSinceStart = totalTime - startTime;

	int index = timeSinceStart / 10000;
	if (index >= cameraPositions.size()) {
		index = cameraPositions.size() - 1;
	}

	//previousPosition = index >= 1 ? cameraPositions[index - 1] : cameraPositions[index];
	nextPosition = cameraPositions[index];
	nextYaw = cameraYaws[index];
	nextPitch = cameraPitches[index];


	float interpFactor = (float)((int)timeSinceStart % 10000) / 10000;

	if (interpFactor >= 0.9975f){
		previousPosition = nextPosition;
		previousYaw = nextYaw;
		previousPitch = nextPitch;
	}

	position = previousPosition + (nextPosition - previousPosition).operator*(interpFactor);
	yaw = previousYaw + ((nextYaw - previousYaw) * interpFactor);
	pitch = previousPitch + ((nextPitch - previousPitch) * interpFactor);

}

void Camera::ControlCamera(float dt) {
	pitch -= (Window::GetMouse()->GetRelativePosition().y);
	yaw -= (Window::GetMouse()->GetRelativePosition().x);

	pitch = std::min(pitch, 90.0f);
	pitch = std::max(pitch, -90.0f);

	if (yaw < 0) {
		yaw += 360.0f;
	}

	if (yaw > 360.0f) {
		yaw -= 360.0f;
	}

	Matrix4 rotation = Matrix4::Rotation(yaw, Vector3(0, 1, 0));

	Vector3 forward = rotation * Vector3(0, 0, -1);
	Vector3 right = rotation * Vector3(1, 0, 0);

	float speed = 100.0f * dt;

	if (Window::GetKeyboard()->KeyDown(KEYBOARD_W)) {
		position += forward * speed;
	}
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_S)) {
		position -= forward * speed;
	}
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_A)) {
		position -= right * speed;
	}
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_D)) {
		position += right * speed;
	}

	if (Window::GetKeyboard()->KeyDown(KEYBOARD_SHIFT)) {
		position.y += speed;
	}
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_SPACE)) {
		position.y -= speed;
	}
}

Matrix4 Camera::BuildViewMatrix() {
	return	Matrix4::Rotation(-pitch, Vector3(1, 0, 0)) *
		Matrix4::Rotation(-yaw, Vector3(0, 1, 0)) *
		Matrix4::Translation(-position);
};

void Camera::InitialisePositions() {
	cameraPositions = { Vector3(2239, 187, 3773), Vector3(2203, 322, 3493), Vector3(2303, 367, 3234),
						Vector3(2219, 108, 2453), Vector3(2490, 117, 2173), Vector3(2113, 32, 2363) };
	cameraYaws = { 5.17f, 5.45f, 21.0f, 27.2f, 302.0f, 11.3f };
	cameraPitches = { 19.0f, -5.0f, -12.2f, 27.2f, -9.8f, 11.27f };

	previousPosition = cameraPositions[0];
	previousYaw = cameraYaws[0];
	previousPitch = cameraPitches[0];
}