#pragma once

#include <math.h>
#include <cglm/cglm.h>
#include "elapsed_time.h"

#define NEAR_CLIPPING_PLANE 0.1f
#define FAR_CLIPPING_PLANE 200.f

#define CONST_CAM_SPEED 0.005f
#define CONST_MOUSE_SENSETIVITY 0.08f
#define CONST_CAM_LERP 0.005f

typedef struct
{
	mat4 view;
	mat4 projection;

	vec3 cameraPos;
	vec3 cameraFront;
	vec3 cameraUp;

	float yaw;		// horizontal rotation
	float pitch;	// vertical rotation

	float fov;

	vec3 velocityVector;	// world-relative velocity vector
}
Camera;

void cam_init(Camera* cam)
{
	glm_mat4_identity(cam->view);
	glm_mat4_identity(cam->projection);

	glm_vec3_zero(cam->cameraPos);
	cam->cameraPos[2] = 3.0f;
	glm_vec3_zero(cam->cameraFront);
	cam->cameraFront[2] = -1.0f;
	glm_vec3_zero(cam->cameraUp);
	cam->cameraUp[1] = 1.0f;

	cam->yaw = -90.f;
	cam->pitch = 0.f;

	cam->fov = 70.f;
}

void cam_updateLookAt(Camera* cam)
{
	glm_look(cam->cameraPos, cam->cameraFront, cam->cameraUp, cam->view);
}

void cam_updateDirection(Camera* cam)
{
	vec3 direction;

	direction[0] = cos(glm_rad(cam->yaw)) * cos(glm_rad(cam->pitch));
	direction[2] = sin(glm_rad(cam->yaw)) * cos(glm_rad(cam->pitch));
	direction[1] = sin(glm_rad(cam->pitch));
	glm_vec3_normalize_to(direction, cam->cameraFront);
}

void cam_updatePerspective(Camera* cam, float screenRatio)
{
	glm_ortho_default_s(screenRatio, 5.0, cam->projection);
	// glm_perspective(
	// 	glm_rad(cam->fov), screenRatio,
	// 	NEAR_CLIPPING_PLANE, FAR_CLIPPING_PLANE,
	// 	cam->projection
	// );
}

// Freecam implementation
enum movementStates {
	MOVE_FORWARD = 1,
	MOVE_BACKWARD = 2,
	MOVE_LEFT = 4,
	MOVE_RIGHT = 8,
	MOVE_UP = 16,
	MOVE_DOWN = 32,
};

uint8_t movementState;	// ? should we store it outside cam struct or not

__forceinline void cam_setInputState(int type, _Bool state)
{
	if (state == true)
		movementState |= 1U << (int)log2(type);
	else
		movementState &= ~(1U << (int)log2(type));
}

void cam_processInput(Camera* cam)
{
	vec3 velocity = GLM_VEC3_ZERO_INIT;

	if (movementState & MOVE_FORWARD) {
		glm_vec3_add(velocity, cam->cameraFront, velocity);
	}
	if (movementState & MOVE_BACKWARD) {
		glm_vec3_sub(velocity, cam->cameraFront, velocity);
	}
	if (movementState & MOVE_LEFT) {
		vec3 cross;
		glm_vec3_cross(cam->cameraFront, cam->cameraUp, cross);
		glm_vec3_sub(velocity, cross, velocity);
	}
	if (movementState & MOVE_RIGHT) {
		vec3 cross;
		glm_vec3_cross(cam->cameraFront, cam->cameraUp, cross);
		glm_vec3_add(velocity, cross, velocity);
	}
	if (movementState & MOVE_UP) {
		velocity[1] += 1;
	}
	if (movementState & MOVE_DOWN) {
		velocity[1] -= 1;
	}

	glm_vec3_norm(velocity);

	float speed = CONST_CAM_SPEED * timeDelta;
	glm_vec3_scale(velocity, speed, velocity);

	glm_vec3_add(cam->velocityVector, velocity, cam->velocityVector);
}

void cam_processMovement(Camera* cam)
{
	vec3 lerp_velocity;
	glm_vec3_lerp(GLM_VEC3_ZERO, cam->velocityVector, CONST_CAM_LERP, lerp_velocity);
	glm_vec3_scale(lerp_velocity, timeDelta, lerp_velocity);

	glm_vec3_add(cam->cameraPos, lerp_velocity, cam->cameraPos);
	glm_vec3_sub(cam->velocityVector, lerp_velocity, cam->velocityVector);
}

__forceinline void cam_updateCursor(Camera* cam, double x_change, double y_change)
{
	if (x_change) {
		cam->yaw += x_change * CONST_MOUSE_SENSETIVITY;
	}
	if (y_change) {
		cam->pitch -= y_change * CONST_MOUSE_SENSETIVITY;
		if (cam->pitch > 90)
			cam->pitch = 89.9;
		else if (cam->pitch < -90)
			cam->pitch = -89.9;
	}
}
