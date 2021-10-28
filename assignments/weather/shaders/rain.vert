#version 330 core
layout (location = 0) in vec3 pos;
uniform mat4 model;
uniform mat4 prevModel;

uniform vec3 offset;
uniform vec3 camPosition;
uniform vec3 forwardOffset;
uniform vec3 velocity;

uniform float boxSize;
const float heightScale = 0.05;
out float lenColorScale;
void main()
{
    vec3 position = mod(pos + offset, boxSize);
    position += camPosition + forwardOffset - boxSize / 2;


    vec4 worldPos = vec4(position, 1.0);
    vec4 worldPosPrev = vec4(position + velocity * heightScale, 1.0);

    vec4 bottom = model * worldPos;

    vec4 topPrev = prevModel * worldPosPrev;

    gl_Position = mix(topPrev, bottom, mod(gl_VertexID, 2));


    vec4 top = model * worldPosPrev;
    vec2 dirPrev = (topPrev.xy / topPrev.w) - (bottom.xy/bottom.w);
    vec2 dir = (top.xy/top.w) - (bottom.xy/bottom.w);
    float len = length(dir);
    float lenPrev = length(dirPrev);
    lenColorScale = clamp(len/lenPrev, 0.0, 1.0);
}