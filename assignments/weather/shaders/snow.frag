#version 330 core

out vec4 fragColor;
in vec3 particleColor;

void main()
{
   // TODO 2.4 set the alpha value to 0.2 (alpha is the 4th value of the output color)


   fragColor = vec4(particleColor, 1.0);

}