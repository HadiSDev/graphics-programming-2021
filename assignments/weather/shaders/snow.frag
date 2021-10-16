#version 330 core

out vec4 fragColor;
in vec3 particleColor;
in float distanceFrag;

void main()
{
   // TODO 2.4 set the alpha value to 0.2 (alpha is the 4th value of the output color)


   vec2 center = vec2(0.5);
   vec2 pointCoord = gl_PointCoord;
   float distanceToCenter = 1 - (distance(pointCoord, center) * 2);
   // remember to replace the default output (vec4(1.0,1.0,1.0,1.0)) with the color and alpha values that you have computed
   fragColor = vec4(particleColor, distanceToCenter);

}