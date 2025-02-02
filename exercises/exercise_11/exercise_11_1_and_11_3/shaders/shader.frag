#version 330 core
out vec4 FragColor;

in VS_OUT {
   vec3 normal;
   vec3 position;
} fin;

uniform vec3 cameraPos;
uniform samplerCube skybox;

uniform float reflectionFactor;
uniform float n2;
const float n1 = 1.0;

void main()
{
    vec3 I = normalize(fin.position - cameraPos); // incidence vector
    vec3 V = -I; // view vector (surface to camera)
    vec3 N = normalize(fin.normal); // surface normal


    // TODO exercise 10.1 - reflect camera to fragment vector and sample the skybox with the reflected direction
    vec3 refl = reflect(I, normalize(N));
    vec4 reflectColor = vec4(texture(skybox, refl).rgb, 1.0);

    // TODO exercise 10.2 - refract the camera to fragment vector and sample the skybox with the reffracted direction
    vec3 refrac = refract(I, normalize(N), n1/n2);
    vec4 refractColor = vec4(texture(skybox, refrac).rgb, 1.0);

    // TODO exercise 10.3 - implement the Schlick approximation of the Fresnel factor and set "reflectionProportion" accordingly
    float R0 = pow((n1 - n2) / (n1 + n2), 2);
    float angle = max(0, dot(V, N));
    float reflectionProportion = R0 + (1- R0)*pow(1 - angle, 5);


    // we combine reflected and refracted color here
    FragColor = reflectionProportion * reflectColor + (1.0 - reflectionProportion) * refractColor;
}