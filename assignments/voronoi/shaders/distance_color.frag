#version 330 core
// FRAGMENT SHADER

// TODO voronoi 1.5

// Create an 'in float' variable to receive the depth value from the vertex shader,
// the variable must have the same name as the 'out variable' in the vertex shader.
out vec4 fragColor;
in float zCoordinate;
// Create a 'uniform vec3' to receive the cone color from your application.
uniform vec3 coneColor;

void main()
{
    // Modulate the color of the fragColor using the z-coordinate of the fragment.
    // Make sure that the z-coordinate is in the [0, 1] range (if it is not, place it in that range),
    // you can use non-linear transformations of the z-coordinate, such as the 'pow' or 'sqrt' functions,
    // to make the colors brighter close to the center of the cone.
    float color_ratio = pow(sqrt(abs(gl_FragCoord.z - zCoordinate)), 3.0);

    vec3 color = coneColor * color_ratio;
    fragColor = vec4(vec3(color), 1.0); // CODE HERE
}