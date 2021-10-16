#version 330 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 velocity;
layout (location = 2) in vec3 randomOffset;
layout (location = 3) in vec3 color;
layout (location = 4) in float timeOfBirth;

uniform float currentTime;
uniform mat4 model;
uniform float boxSize;
uniform vec3 camPosition;
uniform vec3 forwardOffset;

out vec3 particleColor;

void main()
{

}