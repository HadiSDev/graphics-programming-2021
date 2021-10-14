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
void bindAttributesParticle();
void bindAttributesLine();
void createSnowVertexBuffer();
void createRainVertexBuffer();
void initSnowParticles();
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
void emitParticle(float posX, float posY, float posZ, float velX, float velY, float velZ, float ranX, float ranY, float ranZ, glm::vec3 color);
void emitLine(float posXStart, float posYStart, float posZStart, float posXEnd, float posYEnd, float posZEnd, float velX, float velY, float velZ, float ranX, float ranY, float ranZ, glm::vec3 color);

void renderParticles();
void renderLines();

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
Shader* shaderProgramSnow;
Shader* shaderProgramRain;


// global variables used for control
// ---------------------------------
float currentTime;
glm::vec3 camForward(.0f, .0f, -1.0f);
glm::vec3 camPosition(.0f, 1.6f, 0.0f);
float linearSpeed = 0.15f, rotationGain = 30.0f;

// Particle Settings
const unsigned int particleSize = 13;
const unsigned int lineSize = 16;
const unsigned int numberOfParticles = 10000;
unsigned int ParticleVAO, ParticleVBO, LineVAO, LineVBO;
const unsigned int sizeOfFloat = 4;
unsigned int particleId = 0;
const float boxSize = 5.0;
bool isRaining = true;

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
    shaderProgramSnow -> use();
    shaderProgramSnow->setFloat("boxSize", boxSize);

    shaderProgramRain -> use();
    shaderProgramRain->setFloat("boxSize", boxSize);
    // set up the z-buffer
    // Notice that the depth range is now set to glDepthRange(-1,1), that is, a left handed coordinate system.
    // That is because the default openGL's NDC is in a left handed coordinate system (even though the default
    // glm and legacy openGL camera implementations expect the world to be in a right handed coordinate system);
    // so let's conform to that
    glDepthRange(-1,1); // make the NDC a LEFT handed coordinate system, with the camera pointing towards +z
    glEnable(GL_DEPTH_TEST); // turn on z-buffer depth test
    glDepthFunc(GL_LESS); // draws fragments that are closer to the screen in NDC

    createSnowVertexBuffer();
    createRainVertexBuffer();
    initSnowParticles();
    initRainParticles();
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

        if(isRaining)
        {
            renderLines();
        }
        else{
            renderParticles();
        }



        glfwSwapBuffers(window);
        glfwPollEvents();

        // control render loop frequency
        std::chrono::duration<float> elapsed = std::chrono::high_resolution_clock::now()-frameStart;
        while (loopInterval > elapsed.count()) {
            elapsed = std::chrono::high_resolution_clock::now() - frameStart;
        }
    }

    delete shaderProgramWorld;
    delete shaderProgramSnow;

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

void renderParticles() {
    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
    shaderProgramSnow -> use();
    shaderProgramSnow -> setFloat("currentTime", currentTime);

    glm::mat4 scale = glm::scale(1.f, 1.f, 1.f);

    glm::mat4 projection = glm::perspectiveFov(70.0f, (float)SCR_WIDTH, (float)SCR_HEIGHT, .01f, 100.0f);
    glm::mat4 view = glm::lookAt(camPosition, camPosition + camForward, glm::vec3(0,1,0));
    glm::mat4 viewProjection = projection * view;

    shaderProgramSnow->setMat4("model", viewProjection);

    glm::vec3 forwardOffset(camForward * 2.0f);


    shaderProgramSnow->setVec3("camPosition", camPosition);
    shaderProgramSnow->setVec3("forwardOffset", forwardOffset);


    glBindVertexArray(ParticleVAO);
    glDrawArrays(GL_POINTS, 0, numberOfParticles);
    glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);
}

void renderLines() {
    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
    shaderProgramRain -> use();
    shaderProgramRain -> setFloat("currentTime", currentTime);

    glm::mat4 scale = glm::scale(1.f, 1.f, 1.f);

    glm::mat4 projection = glm::perspectiveFov(70.0f, (float)SCR_WIDTH, (float)SCR_HEIGHT, .01f, 100.0f);
    glm::mat4 view = glm::lookAt(camPosition, camPosition + camForward, glm::vec3(0,1,0));
    glm::mat4 viewProjection = projection * view;

    shaderProgramRain->setMat4("model", viewProjection);

    glm::vec3 forwardOffset(camForward * 2.0f);


    shaderProgramRain->setVec3("camPosition", camPosition);
    shaderProgramRain->setVec3("forwardOffset", forwardOffset);


    glBindVertexArray(LineVAO);
    glDrawArrays(GL_LINE, 0, numberOfParticles);
    glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);
}

void initSnowParticles() {
    for(int i = 0; i < numberOfParticles; i ++)
    {
        float posX = RandomNumber(-20.0, 20.0);
        float posY = RandomNumber(0.0, 30.0);
        float posZ = RandomNumber(-20.0, 20.0);

        float velX = RandomNumber(-0.0, -1.0);
        float velY = RandomNumber(-2.0, -4.0);
        float velZ = RandomNumber(-0.0, -1.0);

        float ranX = RandomNumber(-1.0, 1.0);
        float ranY = 0.0;
        float ranZ = RandomNumber(-1.0, 1.0);

        glm::vec3 color = glm::vec3(0.0, 0.0, 0.0);

        emitParticle(posX, posY, posZ, velX, velY, velZ, ranX, ranY, ranZ, color);
    }
}

void initRainParticles() {
    for(int i = 0; i < numberOfParticles; i ++)
    {
        float posXStart = RandomNumber(boxSize, boxSize * 2);
        float posYStart = RandomNumber(boxSize, boxSize * 2);
        float posZStart = RandomNumber(boxSize, boxSize * 2);

        float posXEnd = RandomNumber(boxSize, boxSize * 2);
        float posYEnd = RandomNumber(boxSize, boxSize * 2);
        float posZEnd = RandomNumber(boxSize, boxSize * 2);

        float velX = RandomNumber(-0.0, -1.0);
        float velY = RandomNumber(-2.0, -4.0);
        float velZ = RandomNumber(-0.0, -1.0);

        float ranX = RandomNumber(-1.0, 1.0);
        float ranY = 0.0;
        float ranZ = RandomNumber(-1.0, 1.0);

        glm::vec3 color = glm::vec3(0.0, 0.0, 0.0);

        emitLine(posXStart, posYStart, posZStart, posXEnd, posYEnd, posZEnd, velX, velY, velZ, ranX, ranY, ranZ, color);
    }
}

void createSnowVertexBuffer(){
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
    bindAttributesParticle();
}

void createRainVertexBuffer(){
    glGenVertexArrays(1, &LineVAO);
    glGenBuffers(1, &LineVBO);

    glBindVertexArray(LineVAO);
    glBindBuffer(GL_ARRAY_BUFFER, LineVBO);

    // initialize particle buffer, set all values to 0
    std::vector<float> data(numberOfParticles * lineSize);
    for(float & i : data)
        i = 0.0f;

    // allocate at openGL controlled memory
    glBufferData(GL_ARRAY_BUFFER, numberOfParticles * lineSize * sizeOfFloat, &data[0], GL_DYNAMIC_DRAW);
    bindAttributesLine();
}

void bindAttributesParticle(){
    int posSize = 3; // each position has x,y & z
    GLuint vertexLocation = glGetAttribLocation(shaderProgramSnow->ID, "pos");
    glEnableVertexAttribArray(vertexLocation);
    glVertexAttribPointer(vertexLocation, posSize, GL_FLOAT, GL_FALSE, particleSize * sizeOfFloat, (void*)0);

    int velocitySize = 3; // each position has x,y & z
    GLuint velocityLocation = glGetAttribLocation(shaderProgramSnow->ID, "velocity");
    glEnableVertexAttribArray(velocityLocation);
    glVertexAttribPointer(velocityLocation, velocitySize, GL_FLOAT, GL_FALSE, particleSize * sizeOfFloat, (void*)(posSize*sizeOfFloat));

    int randomOffsetSize = 3; // each position has x,y & z
    GLuint randomOffsetLocation = glGetAttribLocation(shaderProgramSnow->ID, "randomOffset");
    glEnableVertexAttribArray(randomOffsetLocation);
    glVertexAttribPointer(randomOffsetLocation, randomOffsetSize, GL_FLOAT, GL_FALSE, particleSize * sizeOfFloat, (void*)((posSize + velocitySize)*sizeOfFloat));

    int colorSize = 3; // each position has x,y & z
    GLuint colorLocation = glGetAttribLocation(shaderProgramSnow->ID, "color");
    glEnableVertexAttribArray(colorLocation);
    glVertexAttribPointer(colorLocation, colorSize, GL_FLOAT, GL_FALSE, particleSize * sizeOfFloat, (void*)((posSize + velocitySize + randomOffsetSize)*sizeOfFloat));

    int timeOfBirthSize = 1; // each position has x,y & z
    GLuint timeOfBirthLocation = glGetAttribLocation(shaderProgramSnow->ID, "timeOfBirth");
    glEnableVertexAttribArray(timeOfBirthLocation);
    glVertexAttribPointer(timeOfBirthLocation, timeOfBirthSize, GL_FLOAT, GL_FALSE, particleSize * sizeOfFloat, (void*)((posSize + velocitySize + randomOffsetSize + colorSize)*sizeOfFloat));
}

void bindAttributesLine(){
    int posSizeStart = 3; // each position has x,y & z
    GLuint vertexStartLocation = glGetAttribLocation(shaderProgramRain->ID, "posStart");
    glEnableVertexAttribArray(vertexStartLocation);
    glVertexAttribPointer(vertexStartLocation, posSizeStart, GL_FLOAT, GL_FALSE, particleSize * sizeOfFloat, (void*)0);

    int posSizeEnd = 3; // each position has x,y & z
    GLuint vertexEndLocation = glGetAttribLocation(shaderProgramRain->ID, "posEnd");
    glEnableVertexAttribArray(vertexEndLocation);
    glVertexAttribPointer(vertexEndLocation, posSizeEnd, GL_FLOAT, GL_FALSE, particleSize * sizeOfFloat, (void*)(posSizeStart * sizeOfFloat) );

    int velocitySize = 3; // each position has x,y & z
    GLuint velocityLocation = glGetAttribLocation(shaderProgramSnow->ID, "velocity");
    glEnableVertexAttribArray(velocityLocation);
    glVertexAttribPointer(velocityLocation, velocitySize, GL_FLOAT, GL_FALSE, particleSize * sizeOfFloat, (void*)((posSizeStart+posSizeEnd)*sizeOfFloat));

    int randomOffsetSize = 3; // each position has x,y & z
    GLuint randomOffsetLocation = glGetAttribLocation(shaderProgramSnow->ID, "randomOffset");
    glEnableVertexAttribArray(randomOffsetLocation);
    glVertexAttribPointer(randomOffsetLocation, randomOffsetSize, GL_FLOAT, GL_FALSE, particleSize * sizeOfFloat, (void*)((posSizeStart+posSizeEnd + velocitySize)*sizeOfFloat));

    int colorSize = 3; // each position has x,y & z
    GLuint colorLocation = glGetAttribLocation(shaderProgramSnow->ID, "color");
    glEnableVertexAttribArray(colorLocation);
    glVertexAttribPointer(colorLocation, colorSize, GL_FLOAT, GL_FALSE, particleSize * sizeOfFloat, (void*)((posSizeStart+posSizeEnd + velocitySize + randomOffsetSize)*sizeOfFloat));

    int timeOfBirthSize = 1; // each position has x,y & z
    GLuint timeOfBirthLocation = glGetAttribLocation(shaderProgramSnow->ID, "timeOfBirth");
    glEnableVertexAttribArray(timeOfBirthLocation);
    glVertexAttribPointer(timeOfBirthLocation, timeOfBirthSize, GL_FLOAT, GL_FALSE, particleSize * sizeOfFloat, (void*)((posSizeStart+posSizeEnd + velocitySize + randomOffsetSize + colorSize)*sizeOfFloat));
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
    shaderProgramSnow = new Shader("shaders/snow.vert", "shaders/snow.frag");
    shaderProgramRain = new Shader("shaders/rain.vert", "shaders/rain.frag");
    // load floor mesh into openGL
    floorObj.VAO = createVertexArray(floorVertices, floorColors, floorIndices);
    floorObj.vertexCount = floorIndices.size();

    // load cube mesh into openGL
    cube.VAO = createVertexArray(cubeVertices, cubeColors, cubeIndices);
    cube.vertexCount = cubeIndices.size();
}

void emitParticle(float posX, float posY, float posZ, float velX, float velY, float velZ, float ranX, float ranY, float ranZ, glm::vec3 color){
    glBindVertexArray(ParticleVAO);
    glBindBuffer(GL_ARRAY_BUFFER, ParticleVBO);
    float data[particleSize];
    data[0] = posX;
    data[1] = posY;
    data[2] = posZ;
    data[3] = velX;
    data[4] = velY;
    data[5] = velZ;
    data[6] = ranX;
    data[7] = ranY;
    data[8] = ranZ;
    data[9] = color.x;
    data[10] = color.y;
    data[11] = color.z;
    data[12] = currentTime;

    // upload only parts of the buffer
    glBufferSubData(GL_ARRAY_BUFFER, particleId * particleSize * sizeOfFloat, particleSize * sizeOfFloat, data);
    particleId = (particleId + 1) % numberOfParticles;
}

void emitLine(float posXStart, float posYStart, float posZStart, float posXEnd, float posYEnd, float posZEnd, float velX, float velY, float velZ, float ranX, float ranY, float ranZ, glm::vec3 color){
    glBindVertexArray(LineVAO);
    glBindBuffer(GL_ARRAY_BUFFER, LineVBO);
    float data[lineSize];
    data[0] = posXStart;
    data[1] = posYStart;
    data[2] = posZStart;
    data[3] = posXEnd;
    data[4] = posYEnd;
    data[5] = posZEnd;
    data[6] = velX;
    data[7] = velY;
    data[8] = velZ;
    data[9] = ranX;
    data[10] = ranY;
    data[11] = ranZ;
    data[12] = color.x;
    data[13] = color.y;
    data[14] = color.z;
    data[15] = currentTime;

    // upload only parts of the buffer
    glBufferSubData(GL_ARRAY_BUFFER, particleId * particleSize * sizeOfFloat, particleSize * sizeOfFloat, data);
    particleId = (particleId + 1) % numberOfParticles;
}

float RandomNumber(float Min, float Max)
{
    return ((float(rand()) / float(RAND_MAX)) * (Max - Min)) + Min;
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

    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
    {
        isRaining = true;
    }

    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
    {
        isRaining = false;
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