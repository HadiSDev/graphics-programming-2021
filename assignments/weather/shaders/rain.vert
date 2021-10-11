#version 330 core
layout (location = 0) in vec3 pos;   // the position variable has attribute position 0
layout (location = 1) in vec2 velocity;
layout (location = 2) in float timeOfBirth;

uniform float currentTime;
uniform mat4 model;
uniform vec3 cameraPos;

out float elapsedTimeFrag;

void main()
{
   float maxPointSize = 5.0;
   vec3 finalPos = pos;
   float elapsedTime = currentTime - timeOfBirth;

   finalPos -= vec3(velocity, 0.0) * elapsedTime;

   elapsedTimeFrag = elapsedTime;

   gl_Position = model * vec4(finalPos, 1.0);

   float distance = distance(pos, cameraPos) + 0.0001; // To avoid division by zerp

   gl_PointSize = max(maxPointSize, maxPointSize/distance);
}