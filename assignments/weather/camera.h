//
// Created by hadis on 09/10/2021.
//

#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>

static glm::vec3 camForward(.0f, .0f, -1.0f);
static glm::vec3 camPosition(.0f, 1.6f, 0.0f);
static float linearSpeed = 0.15f, rotationGain = 30.0f;

static float yaw = 0.0f;
static float pitch = 0.0f;
static bool firstMouse = true;
static float lastX = 300, lastY = 300;

#endif //CAMERA_H
