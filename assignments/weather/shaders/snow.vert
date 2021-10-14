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
   float maxPointSize = 5.0;
   float maxDistance = 20.0;
   float elapsedTime = currentTime - timeOfBirth;

   vec3 offsets = (velocity * elapsedTime) + randomOffset;
   offsets -= camPosition + forwardOffset + boxSize/2;

   offsets = mod(offsets, boxSize);

   vec3 position = mod(pos + offsets, boxSize);
   position += camPosition + forwardOffset - boxSize/2.0;
   gl_Position = model * vec4(position, 1.0);

   float distance = distance(position, camPosition); // To avoid division by zerp

   gl_PointSize = 5.0/distance;

   particleColor = color;
}