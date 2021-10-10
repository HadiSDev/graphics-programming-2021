#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include <vector>
#include <chrono>

#include "shader.h"
#include "glmutils.h"

#include "primitives.h"

// structure to hold render info
// -----------------------------
struct SceneObject{
    unsigned int VAO;
    unsigned int vertexCount;
    void drawSceneObject() const{
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES,  vertexCount, GL_UNSIGNED_INT, 0);
    }
};

// function declarations
// ---------------------
unsigned int createArrayBuffer(const std::vector<float> &array);
unsigned int createElementArrayBuffer(const std::vector<unsigned int> &array);
unsigned int createVertexArray(const std::vector<float> &positions, const std::vector<float> &colors, const std::vector<unsigned int> &indices);
void setup();
void drawObjects();
void bindAttributes();
void createRainVertexBuffer();
void initRainParticles();
float RandomNumber(float Min, float Max);

// glfw and input functions
// ------------------------
void cursorInRange(float screenX, float screenY, int screenW, int screenH, float min, float max, float &x, float &y);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void cursor_input_callback(GLFWwindow* window, double posX, double posY);
void drawCube(glm::mat4 model);
void drawPlane(glm::mat4 model);
void emitParticle(float x, float y, float z, float velocityX, float velocityY, float timeOfBirth);

// screen settings
// ---------------
const unsigned int SCR_WIDTH = 600;
const unsigned int SCR_HEIGHT = 600;

// Camera Settings
float yaw = 0.0f;
float pitch = 0.0f;
bool firstMouse = true;
float lastX = 300, lastY = 300;
// global variables used for rendering
// -----------------------------------
SceneObject cube;
SceneObject floorObj;
Shader* shaderProgramWorld;
Shader* shaderProgramRain;

// global variables used for control
// ---------------------------------
float currentTime;
glm::vec3 camForward(.0f, .0f, -1.0f);
glm::vec3 camPosition(.0f, 1.6f, 0.0f);
float linearSpeed = 0.15f, rotationGain = 30.0f;

// Particle Settings
const unsigned int particleSize = 6;
const unsigned int numberOfParticles = 10000;
unsigned int ParticleVAO, ParticleVBO;
const unsigned int sizeOfFloat = 4;
unsigned int particleId = 0;


int main()
{
    srand (static_cast <unsigned> (time(0)));
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Assignment 2", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, cursor_input_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);


    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // setup mesh objects
    // ---------------------------------------
    setup();

    // set up the z-buffer
    // Notice that the depth range is now set to glDepthRange(-1,1), that is, a left handed coordinate system.
    // That is because the default openGL's NDC is in a left handed coordinate system (even though the default
    // glm and legacy openGL camera implementations expect the world to be in a right handed coordinate system);
    // so let's conform to that
    glDepthRange(-1,1); // make the NDC a LEFT handed coordinate system, with the camera pointing towards +z
    glEnable(GL_DEPTH_TEST); // turn on z-buffer depth test
    glDepthFunc(GL_LESS); // draws fragments that are closer to the screen in NDC

    createRainVertexBuffer();

    // render loop
    // -----------
    // render every loopInterval seconds
    float loopInterval = 0.02f;
    auto begin = std::chrono::high_resolution_clock::now();

    while (!glfwWindowShouldClose(window))
    {
        // update current time
        auto frameStart = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> appTime = frameStart - begin;
        currentTime = appTime.count();

        processInput(window);

        glClearColor(0.3f, 0.3f, 0.3f, 1.0f);

        // notice that we also need to clear the depth buffer (aka z-buffer) every new frame
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shaderProgramWorld->use();
        drawObjects();

        glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
        shaderProgramRain -> use();
        shaderProgramRain -> setFloat("currentTime", currentTime);

        glm::mat4 scale = glm::scale(1.f, 1.f, 1.f);

        // NEW!
        // update the camera pose and projection, and compose the two into the viewProjection with a matrix multiplication
        // projection * view = world_to_view -> view_to_perspective_projection
        // or if we want ot match the multiplication order (projection * view), we could read
        // perspective_projection_from_view <- view_from_world
        glm::mat4 projection = glm::perspectiveFov(70.0f, (float)SCR_WIDTH, (float)SCR_HEIGHT, .01f, 100.0f);
        glm::mat4 view = glm::lookAt(camPosition, camPosition + camForward, glm::vec3(0,1,0));
        glm::mat4 viewProjection = projection * view;

        shaderProgramRain->setMat4("model", viewProjection);

        glBindVertexArray(ParticleVAO);

        float x = RandomNumber(-20, 20);
        float y = 20;
        float z = RandomNumber(-20, 20);
        emitParticle(x, y, z, 0.0, 5, currentTime);
        glDrawArrays(GL_POINTS, 0, numberOfParticles);



        glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);

        glfwSwapBuffers(window);
        glfwPollEvents();

        // control render loop frequency
        std::chrono::duration<float> elapsed = std::chrono::high_resolution_clock::now()-frameStart;
        while (loopInterval > elapsed.count()) {
            elapsed = std::chrono::high_resolution_clock::now() - frameStart;
        }
    }

    delete shaderProgramWorld;
    delete shaderProgramRain;

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

void createRainVertexBuffer(){
    glGenVertexArrays(1, &ParticleVAO);
    glGenBuffers(1, &ParticleVBO);

    glBindVertexArray(ParticleVAO);
    glBindBuffer(GL_ARRAY_BUFFER, ParticleVBO);

    // initialize particle buffer, set all values to 0
    std::vector<float> data(numberOfParticles * particleSize);
    for(float & i : data)
        i = 0.0f;

    // allocate at openGL controlled memory
    glBufferData(GL_ARRAY_BUFFER, numberOfParticles * particleSize * sizeOfFloat, &data[0], GL_DYNAMIC_DRAW);
    bindAttributes();
}

void bindAttributes(){
    int posSize = 3; // each position has x,y & z
    GLuint vertexLocation = glGetAttribLocation(shaderProgramRain->ID, "pos");
    glEnableVertexAttribArray(vertexLocation);
    glVertexAttribPointer(vertexLocation, posSize, GL_FLOAT, GL_FALSE, particleSize * sizeOfFloat, (void*)0);

    // TODO 2.2 set velocity and timeOfBirth shader attributes
    int velSize = 2;
    GLuint vertexVelocity = glGetAttribLocation(shaderProgramRain->ID, "velocity");

    glEnableVertexAttribArray(vertexVelocity);
    glVertexAttribPointer(vertexVelocity, velSize, GL_FLOAT, GL_FALSE,
                          particleSize * sizeOfFloat, (void*) (posSize * sizeOfFloat));

    int timeBirthSize = 1;
    GLuint timeBirthLocation = glGetAttribLocation(shaderProgramRain->ID, "timeOfBirth");
    glEnableVertexAttribArray(timeBirthLocation);
    glVertexAttribPointer(timeBirthLocation, timeBirthSize, GL_FLOAT, GL_FALSE,
                          particleSize * sizeOfFloat,  (void*) ( (posSize + velSize) * sizeOfFloat));

}


void drawObjects(){

    glm::mat4 scale = glm::scale(1.f, 1.f, 1.f);

    // NEW!
    // update the camera pose and projection, and compose the two into the viewProjection with a matrix multiplication
    // projection * view = world_to_view -> view_to_perspective_projection
    // or if we want ot match the multiplication order (projection * view), we could read
    // perspective_projection_from_view <- view_from_world
    glm::mat4 projection = glm::perspectiveFov(70.0f, (float)SCR_WIDTH, (float)SCR_HEIGHT, .01f, 100.0f);
    glm::mat4 view = glm::lookAt(camPosition, camPosition + camForward, glm::vec3(0,1,0));
    glm::mat4 viewProjection = projection * view;

    // draw floor (the floor was built so that it does not need to be transformed)
    shaderProgramWorld->setMat4("model", viewProjection);

    floorObj.drawSceneObject();

    // draw 2 cubes and 2 planes in different locations and with different orientations
    drawCube(viewProjection * glm::translate(2.0f, 1.f, 2.0f) * glm::rotateY(glm::half_pi<float>()) * scale);
    drawCube(viewProjection * glm::translate(-2.0f, 1.f, -2.0f) * glm::rotateY(glm::quarter_pi<float>()) * scale);


}


void drawCube(glm::mat4 model){
    // draw object
    shaderProgramWorld->setMat4("model", model);
    cube.drawSceneObject();
}

void setup(){
    // initialize shaders
    shaderProgramWorld = new Shader("shaders/shader.vert", "shaders/shader.frag");
    shaderProgramRain = new Shader("shaders/rain.vert", "shaders/rain.frag");

    // load floor mesh into openGL
    floorObj.VAO = createVertexArray(floorVertices, floorColors, floorIndices);
    floorObj.vertexCount = floorIndices.size();

    // load cube mesh into openGL
    cube.VAO = createVertexArray(cubeVertices, cubeColors, cubeIndices);
    cube.vertexCount = cubeIndices.size();
}

void emitParticle(float x, float y, float z, float velocityX, float velocityY, float timeOfBirth){
    glBindVertexArray(ParticleVAO);
    glBindBuffer(GL_ARRAY_BUFFER, ParticleVBO);
    float data[particleSize];
    data[0] = x;
    data[1] = y,
    data[2] = z,
    data[3] = velocityX,
    data[4] = velocityY,
    data[5] = timeOfBirth;

    // upload only parts of the buffer
    glBufferSubData(GL_ARRAY_BUFFER, particleId * particleSize * sizeOfFloat, particleSize * sizeOfFloat, data);
    particleId = (particleId + 1) % numberOfParticles;
}

float RandomNumber(float Min, float Max)
{
    return ((float(rand()) / float(RAND_MAX)) * (Max - Min)) + Min;
}

void initRainParticles()
{
    for(int i = 0; i < numberOfParticles; i++)
    {


    }
}

unsigned int createVertexArray(const std::vector<float> &positions, const std::vector<float> &colors, const std::vector<unsigned int> &indices){
    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    // bind vertex array object
    glBindVertexArray(VAO);

    // set vertex shader attribute "pos"
    createArrayBuffer(positions); // creates and bind  the VBO
    int posAttributeLocation = glGetAttribLocation(shaderProgramWorld->ID, "pos");
    glEnableVertexAttribArray(posAttributeLocation);
    glVertexAttribPointer(posAttributeLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);

    // set vertex shader attribute "color"
    createArrayBuffer(colors); // creates and bind the VBO
    int colorAttributeLocation = glGetAttribLocation(shaderProgramWorld->ID, "color");
    glEnableVertexAttribArray(colorAttributeLocation);
    glVertexAttribPointer(colorAttributeLocation, 4, GL_FLOAT, GL_FALSE, 0, 0);

    // creates and bind the EBO
    createElementArrayBuffer(indices);

    return VAO;
}


unsigned int createArrayBuffer(const std::vector<float> &array){
    unsigned int VBO;
    glGenBuffers(1, &VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, array.size() * sizeof(GLfloat), &array[0], GL_STATIC_DRAW);

    return VBO;
}


unsigned int createElementArrayBuffer(const std::vector<unsigned int> &array){
    unsigned int EBO;
    glGenBuffers(1, &EBO);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, array.size() * sizeof(unsigned int), &array[0], GL_STATIC_DRAW);

    return EBO;
}

// NEW!
// instead of using the NDC to transform from screen space you can now define the range using the
// min and max parameters
void cursorInRange(float screenX, float screenY, int screenW, int screenH, float min, float max, float &x, float &y){
    float sum = max - min;
    float xInRange = (float) screenX / (float) screenW * sum - sum/2.0f;
    float yInRange = (float) screenY / (float) screenH * sum - sum/2.0f;
    x = xInRange;
    y = -yInRange; // flip screen space y axis
}

void cursor_input_callback(GLFWwindow* window, double posX, double posY){
    // TODO - rotate the camera position based on mouse movements
    //  if you decide to use the lookAt function, make sure that the up vector and the
    //  vector from the camera position to the lookAt target are not collinear


    if (firstMouse) // initially set to true
    {
        lastX = posX;
        lastY = posY;
        firstMouse = false;
    }

    float xoffset = posX - lastX;
    float yoffset = lastY - posY; // reversed since y-coordinates range from bottom to top
    lastX = posX;
    lastY = posY;

    const float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw   += xoffset;
    pitch += yoffset;

    if(pitch > 89.0f)
        pitch =  89.0f;
    if(pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    camForward = glm::normalize(direction);
}

void processInput(GLFWwindow *window) {
    glm::vec3 forwardInXZ = glm::normalize(glm::vec3(camForward.x, 0, camForward.z));
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        camPosition += forwardInXZ * linearSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        camPosition -= glm::cross(forwardInXZ, glm::vec3(0, 1, 0)) * linearSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        camPosition -= forwardInXZ * linearSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        camPosition += glm::cross(forwardInXZ, glm::vec3(0, 1, 0)) * linearSpeed;
    }
}


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}