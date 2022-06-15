
#include <iostream>         // cout, cerr
#include <cstdlib>          // EXIT_FAILURE
#include <GL/glew.h>        // GLEW library
#include <GLFW/glfw3.h>     // GLFW library

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"      // Image loading Utility functions

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "learnOpengl/camera.h"
#include "Cylinder.h"         // Files from www.songho.ca for the algorithms for creating a cylinder
#include "Sphere.h"           // Files from www.songho.ca for the algorithms for creating a sphere

/*
    Author:      Tiffany Gomez
    Course:      CS-330-T5625
    Instructor:  Dr. Diesch
    Assignment:  Final Project
    Date:        06/13/2022
    Description:  A 3D representation of a scene called "Mother's Tea Time" based on a photograph. This program generates accurate 
    three-dimensional objects using C++ and OpenGL libraries. The scene focuses on the positioning, scaling, and rotation of a camera,
    two lighting sources, a tea mug, a napkin, a plate, a tomato, and a placemat. Five primitive shapes are used to built these objects
    (with exception of camera and light sources) including two cubes for mug handles, three cylinders for the mug, the tea within the mug,
    and the plate, two planes for the placemat and the napkin, and a sphere for the tomato. Textures are used to bind images to the objects
    for a more realistic rending supported by two light sources that use the phong model calculations and additional light component settings
    per object, to give the objects a depth similar to that of the scene's photograph and give the objects a more polished visualization. 


*/

using namespace std; // Standard namespace

/*Shader program Macro*/
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version " core \n" #Source
#endif

// Unnamed namespace
namespace
{
    // Settings for window width and height
    const char* const WINDOW_TITLE = "Tiffany Gomez::Mother's Tea Time"; // Macro for window title
    const int WINDOW_WIDTH = 800;
    const int WINDOW_HEIGHT = 600;

    // camera
    // Add position, direction, yaw and pitch to capture scene
    Camera camera(glm::vec3(-5.0f, 6.0f, 3.0f), glm::vec3(0.0f, 1.0f, 0.0f), -40.0, -60.0);
    float lastX = WINDOW_WIDTH / 2.0f;
    float lastY = WINDOW_HEIGHT / 2.0f;
    bool firstMouse = true;


    // timing
    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    // Stores the GL data relative to a given mesh
    struct GLMesh
    {
        GLuint vao[6];         // There are 5 (0-6) vertex array objects
        GLuint vbos[10];        // There are 10 (0-9) vertex buffer objects
        GLuint nIndices;       // Number of indices of the mesh
    };

    // Structure for plane
    struct plane {
        vector<float> verts;
        vector<int> indices;
    };

    // Structure for cube object
    struct cube {
        vector<float> verts;
        vector<int> indices;
    };

    // Main GLFW window
    GLFWwindow* gWindow = nullptr;
    // Triangle mesh data
    GLMesh gMesh;
    // Texture
    GLuint textPlaceMat, textMug, textTea, textLemon, textHandle, textPlate, textNapkin, textTomato;
    glm::vec2 gUVScale(1.0f, 1.0f);

    // Shader program
    GLuint gProgramId;


    // Cylinders: (float baseRadius, float topRadius, float height, int sectors, int stacks, bool smooth)
    Cylinder cylinder1(1.0f, 1.5f, 2.0f, 25, 8, true);      // Mug
    Cylinder cylinder2(1.35f, 1.35f, 0.1f, 25, 8, true);    // Tea
    Cylinder cylinder3(1.4f, 2.3, 0.5f, 25, 8, true);      // Plate

    //Sphere: (float radius, int sectors, int stacks, bool smooth)
    Sphere sphere1(0.66, 36, 18, true);                     // Tomato

    // Plane
    plane plane1 = {};                                     // Place Mat and Napkin

    // Cube
    cube cube1 = {};                                       // Mug Handles

    // Perspective and Orthrographic global variable
    glm::mat4 projection;
    bool orthoView = false;
    
    // Lighting
    // light 1
    glm::vec3 gLightPosition1(-8.0f, 5.0f, -3.0f);
    glm::vec3 gLightColor1(1.0f, 1.0f, 1.0f);
    GLfloat lightStrength1 = 1.0f;
    glm::vec3 gAmbientStrength(glm::vec3(0.1f));
    glm::vec3 gDiffuseStrength(glm::vec3(1.0f));
    float gSpecularIntensity = 0.6f;
    // light 2
    glm::vec3 gLightPosition2(3.0f, 5.0f, 3.0f);
    glm::vec3 gLightColor2(0.5f, 0.5f, 1.0f);
    GLfloat lightStrength2 = 1.0f;
 

    // Color
    glm::vec3 objectColor(1.0f, 1.0f, 1.0f);

}

/* User-defined Function prototypes to:
 * initialize the program, set the window size,
 * redraw graphics on the window when resized,
 * and render graphics on the screen
 */
bool UInitialize(int, char* [], GLFWwindow** window);
void UResizeWindow(GLFWwindow* window, int width, int height);
void UProcessInput(GLFWwindow* window);
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos);
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void switchKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mugConfig(GLMesh& mesh);
void planeConfig(GLMesh& mesh);
void handleConfig(GLMesh& mesh);
void teaConfig(GLMesh& mesh);
void sphereConfig(GLMesh& mesh);
void plateConfig(GLMesh& mesh);
void planeMesh();
void cubeMesh();
void UDestroyMesh(GLMesh& mesh);
void URender();
bool loadTexture(const char* filename, GLuint& textureId, int textureUnit);
void UDestroyTexture(GLuint textureId);
void flipImageVertically(unsigned char* image, int width, int height, int channels);
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId);
void UDestroyShaderProgram(GLuint programId);


// Vertex Shader Source Code 
const GLchar* vertexShaderSource = GLSL(440,
    // Location for vertex position, normals, and texture coordinate.
    layout(location = 0) in vec3 position;          
layout(location = 1) in vec3 normal;                
layout(location = 2) in vec2 textureCoordinate;     
// Outgoing to fragment shader (normals, color, texture coordinates).
out vec3 vertexNormal;                             
out vec3 vertexFragmentPos; 
out vec2 vertexTextureCoordinate;


// Variables for matrices transformation
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    // Reference: Tutorial module 5
    gl_Position = projection * view * model * vec4(position, 1.0f); // Vertices -> clip coordinates
    vertexFragmentPos = vec3(model * vec4(position, 1.0f));         // Fragment position in world space
    vertexNormal = mat3(transpose(inverse(model))) * normal;        // Normal vecs in world space 
    vertexTextureCoordinate = textureCoordinate;
}
);


// Fragment Shader Source Code
const GLchar* fragmentShaderSource = GLSL(440,
    in vec3 vertexNormal; 
in vec3 vertexFragmentPos; 
in vec2 vertexTextureCoordinate;

out vec4 fragmentColor; 

// Uniform variables for lighting (shader files)
uniform vec3 lightPos1;
uniform vec3 lightColor1;
uniform float lightStrength1;
uniform vec3 lightPos2;
uniform vec3 lightColor2;
uniform float lightStrength2;
uniform vec3 ambientStrength;
uniform float specularIntensity;
uniform vec3 viewPosition;
// For two cylinders
uniform sampler2D uTexture;         
uniform sampler2D uTextureExtra;
uniform bool multipleTextures;
uniform vec2 uvScale;
// Base
uniform vec3 objectColor;

void main()
{
    vec4 textureColor = texture(uTexture, vertexTextureCoordinate * uvScale);
    // Sample 2D (learnOpenLg) to find another texture based on same objects color
    if (multipleTextures) {
        vec4 extraTexture = texture(uTextureExtra, vertexTextureCoordinate);
        if (extraTexture.a != 0.0) {
            textureColor = extraTexture;
        }
    }

    /*Used information in the shader files (lightingShader and lightCubeshader that were included in OpenGLSample)
      from OpenGLSample and and learnOpenGL.com . Shader filers are not included.*/

    // Light configurations
    
    // FIRST LIGHT :
    //-------------
    //Calculate Ambient lighting*/
    vec3 ambient = ambientStrength * lightColor1; // Generate ambient light color

    //Calculate Diffuse lighting*/
    vec3 norm = normalize(vertexNormal); // Normalize vectors to 1 unit
    vec3 lightDirection = normalize(lightPos1 - vertexFragmentPos); // Calculate distance (light direction) between light source and fragments/pixels on cube
    float impact = max(dot(norm, lightDirection), 0.0);// Calculate diffuse impact by generating dot product of normal and light
    vec3 diffuse = impact * lightColor1; // Generate diffuse light color

    //Calculate Specular lighting*/
    float highlightSize = 16.0f; // Set specular highlight size
    vec3 viewDir = normalize(viewPosition - vertexFragmentPos); // Calculate view direction
    vec3 reflectDir = reflect(-lightDirection, norm);// Calculate reflection vector
    //Calculate specular component
    float specularComponent = pow(max(dot(viewDir, reflectDir), 0.0), highlightSize);
    vec3 specular = specularIntensity * specularComponent * lightColor1;

    // SECOND LIGHT:
    //--------------
    // ambient lighting - add first and second light ambient numbers
    ambient += lightStrength2 * (ambientStrength * lightColor2);

    // diffuse lighting
    lightDirection = normalize(lightPos2 - vertexFragmentPos);
    impact = max(dot(norm, lightDirection), 0.0);
    // add first and second light diffuses
    diffuse += lightStrength2 * (impact * lightColor2);

    // specular lighting
    reflectDir = reflect(-lightDirection, norm);
    specularComponent = pow(max(dot(viewDir, reflectDir), 0.0), highlightSize);
    // add first and second light speculars
    specular += lightStrength2 * (specularIntensity * specularComponent * lightColor2);

    // CALCULATE PHONG RESULT
    //-----------------------
    vec3 phong = (ambient + diffuse + specular) * textureColor.xyz;

    fragmentColor = vec4(phong, 1.0); // Send lighting results to GPU
}
);


int main(int argc, char* argv[])
{
    if (!UInitialize(argc, argv, &gWindow))
        return EXIT_FAILURE;

    // Create the shader program
    if (!UCreateShaderProgram(vertexShaderSource, fragmentShaderSource, gProgramId))
        return EXIT_FAILURE;

    

    // Load textures for Mug, Tea, Place Matt, Lemon, Handles, Napkin, and Tomato.
    // -----------------------------
    // Texture for Plane Object Place Matt
    const char* texFilename1 = "resources/textures/place_matt.jpg";
    if (!loadTexture(texFilename1, textPlaceMat, 0))
    {
        cout << "Failed to load texture " << texFilename1 << endl;
        return EXIT_FAILURE;
    }
    else {
        cout << "Successfully loaded texture " << texFilename1 << endl;
    }

    // Texture for Cylinder 1 Object Mug
    const char* texFilename2 = "resources/textures/mug_trees.jfif";
    if (!loadTexture(texFilename2, textMug, 0))
    {
        cout << "Failed to load texture " << texFilename2 << endl;
        return EXIT_FAILURE;
    }
    else {
        cout << "Successfully loaded texture " << texFilename2 << endl;
    }

    // Texture for Cylinder 2 Object Tea
    const char* texFilename3 = "resources/textures/liquid_tea.png";
    if (!loadTexture(texFilename3, textTea, 0))
    {
        cout << "Failed to load texture " << texFilename3 << endl;
        return EXIT_FAILURE;
    }
    else {
        cout << "Successfully loaded texture " << texFilename3 << endl;
    }

    // Second Texture - Lemon Slice- for Cylinder 2 Object Tea
    const char* texFilename4 = "resources/textures/lemon_slice.png";
    // Set as GL_TEXTURE1 to include as second texture
    if (!loadTexture(texFilename4, textLemon, 1))
    {
        cout << "Failed to load texture " << texFilename4 << endl;
        return EXIT_FAILURE;
    }
    else {
        cout << "Successfully loaded texture " << texFilename4 << endl;
    }

    // Texture for Cube Object for Handles
    const char* texFilename5 = "resources/textures/handles.jfif";
    if (!loadTexture(texFilename5, textHandle, 0))
    {
        cout << "Failed to load texture " << texFilename5 << endl;
        return EXIT_FAILURE;
    }
    else {
        cout << "Successfully loaded texture " << texFilename5 << endl;
    }

    // Texture for Sphere Object for Tomato
    const char* texFilename6 = "resources/textures/orange.jpg";
    if (!loadTexture(texFilename6, textTomato, 0))
    {
        cout << "Failed to load texture " << texFilename6 << endl;
        return EXIT_FAILURE;
    }
    else {
        cout << "Successfully loaded texture " << texFilename6 << endl;
    }

    // Texture for Plane Object for Napkin
    const char* texFilename7 = "resources/textures/napkin.jpg";
    if (!loadTexture(texFilename7, textNapkin, 0))
    {
        cout << "Failed to load texture " << texFilename7 << endl;
        return EXIT_FAILURE;
    }
    else {
        cout << "Successfully loaded texture " << texFilename7 << endl;
    }

    // Texture for Cylinder Object for plate
    const char* texFilename8 = "resources/textures/redplate.png";
    if (!loadTexture(texFilename8, textPlate, 0))
    {
        cout << "Failed to load texture " << texFilename8 << endl;
        return EXIT_FAILURE;
    }
    else {
        cout << "Successfully loaded texture " << texFilename8 << endl;
    }


    
    // Set samplers per unit 
    glUseProgram(gProgramId);
    glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 0);
    glUniform1i(glGetUniformLocation(gProgramId, "uTextureExtra"), 1);


    // Sets the background color of the window to black (it will be implicitely used by glClear)
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(gWindow))
    {
        // per-frame timing
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        UProcessInput(gWindow);

        // Render this frame
        URender();

        glfwPollEvents();
    }

    // Release mesh data
    UDestroyMesh(gMesh);

    // Release textures
    UDestroyTexture(textPlaceMat);
    UDestroyTexture(textMug);
    UDestroyTexture(textTea);
    UDestroyTexture(textLemon);
    UDestroyTexture(textHandle);
    UDestroyTexture(textTomato);
    UDestroyTexture(textNapkin);
    UDestroyTexture(textPlate);


    // Release shader programs
    UDestroyShaderProgram(gProgramId);

    exit(EXIT_SUCCESS); // Terminates the program successfully
}


/* ------------------- Initialize GLFW, GLEW, window, and everything else -------------------*/
bool UInitialize(int argc, char* argv[], GLFWwindow** window)
{
    // GLFW: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
   

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // GLFW: window creation
    // ---------------------
    * window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
    if (*window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(*window);
    glfwSetFramebufferSizeCallback(*window, UResizeWindow);
    glfwSetCursorPosCallback(*window, UMousePositionCallback);
    glfwSetScrollCallback(*window, UMouseScrollCallback);
    glfwSetMouseButtonCallback(*window, UMouseButtonCallback);
    glfwSetKeyCallback(*window, switchKeyCallback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(*window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // GLEW: initialize
    // ----------------
    // Note: if using GLEW version 1.13 or earlier
    glewExperimental = GL_TRUE;
    GLenum GlewInitResult = glewInit();

    if (GLEW_OK != GlewInitResult)
    {
        std::cerr << glewGetErrorString(GlewInitResult) << std::endl;
        return false;
    }

    // Displays GPU OpenGL version
    cout << "INFO: OpenGL Version: " << glGetString(GL_VERSION) << endl;

    // create plane mesh
    planeMesh();
    // create cube mesh
    cubeMesh();

    // generate all the vertex arrays
    glGenVertexArrays(6, gMesh.vao);

    // Create all the buffers: 2 for each mesh: first one for the vertex data, second one for the indices
    glGenBuffers(10, gMesh.vbos);

    // set up all the GPU buffer objects
    planeConfig(gMesh);
    mugConfig(gMesh);
    handleConfig(gMesh);
    teaConfig(gMesh);
    sphereConfig(gMesh);
    plateConfig(gMesh);
    


    return true;
}


/* ------------------- Process key input for current frame -------------------*/
// called every render loop, making it a very fast input reader
void UProcessInput(GLFWwindow* window)

{
    static const float cameraSpeed = 3.5f;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        camera.ProcessKeyboard(UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        camera.ProcessKeyboard(DOWN, deltaTime);

}

/* ------------------- GLFW keyboard callback for outside render loop -------------------*/
// Switches to othrographic display at the press of a key
void switchKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
        orthoView = !orthoView;

    }
}



/* ------------------- glfw callback for changing window size -------------------*/
void UResizeWindow(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}


/* ------------------- glfw callback for mouse movement -------------------*/
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// GLFW callback for mouse scroll wheel
// --------------------------------------
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);

}

// GLFW callback for mouse buttons
// ----------------------------------
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    switch (button)
    {
    case GLFW_MOUSE_BUTTON_LEFT:
    {
        if (action == GLFW_PRESS)
            cout << "Left mouse button pressed" << endl;
        else
            cout << "Left mouse button released" << endl;
    }
    break;

    case GLFW_MOUSE_BUTTON_MIDDLE:
    {
        if (action == GLFW_PRESS)
            cout << "Middle mouse button pressed" << endl;
        else
            cout << "Middle mouse button released" << endl;
    }
    break;

    case GLFW_MOUSE_BUTTON_RIGHT:
    {
        if (action == GLFW_PRESS)
            cout << "Right mouse button pressed" << endl;
        else
            cout << "Right mouse button released" << endl;
    }
    break;

    default:
        cout << "Unhandled mouse button event" << endl;
        break;
    }
}

// Render
// -----------
void URender()
{
    // Enable z-depth
    glEnable(GL_DEPTH_TEST);

    // Clear the frame and z buffers
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Transformation
    glm::mat4 view = camera.GetViewMatrix();

    // Conditional statement creating a  projection with Perspective/Orthographic matrix
    if (orthoView) {
        // Orthographic view matrix
        projection = glm::ortho(-(float)WINDOW_WIDTH * 0.01f, (float)WINDOW_WIDTH * 0.01f, -(float)WINDOW_HEIGHT * 0.01f, (float)WINDOW_HEIGHT * 0.01f, 0.001f, 1000.0f);
    }
    else {
        // Perspective projection
        projection = glm::perspective(45.0f, (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);
    }

    // Set the shader to be used
    glUseProgram(gProgramId);

    
    // Mug :: cylinder 1 out of 3 : array 0
    //--------------------------------------------------------------------
    // 1. Scales the object
    glm::mat4 scale = glm::mat4(1.0f);
    // 2. Rotates shape 
    glm::mat4 rotation = glm::mat4(1.0f);
    rotation = glm::rotate(rotation, -1.5708f, glm::vec3(1.0, 0.0f, 0.0f));
    // 3. Place object at the origin
    glm::mat4 translation = glm::mat4(1.0f);
    translation = glm::translate(translation, glm::vec3(0.0f, 1.00f, 0.0f));
    // Model matrix: transformations are applied right-to-left order
    glm::mat4 model = translation * rotation * scale;

    // Specify color
    GLfloat myColor[] = { 0.7f, 0.7f, 0.7f, 1.0f };

    // Retrieves transform matricies
    GLint modelLoc = glGetUniformLocation(gProgramId, "model");
    GLint viewLoc = glGetUniformLocation(gProgramId, "view");
    GLint projLoc = glGetUniformLocation(gProgramId, "projection");
    
    // Send transform matricies to shader 
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // Set texture uniform variable in shader 
    GLint UVScaleLoc = glGetUniformLocation(gProgramId, "uvScale");
    glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale));

    /*
        Here we set all the uniforms for the 5/6 types of lights we have. We have to set them manually and index
        the proper PointLight struct in the array to set each uniform variable. This can be done more code-friendly
        by defining light types as classes and set their values in there, or by using a more efficient uniform approach
        by using 'Uniform buffer objects', but that is something we'll discuss in the 'Advanced GLSL' tutorial.
    */
    GLint lightColor1Loc = glGetUniformLocation(gProgramId, "lightColor1");
    GLint lightColor2Loc = glGetUniformLocation(gProgramId, "lightColor2");
    GLint lightPosition1Loc = glGetUniformLocation(gProgramId, "lightPos1");
    GLint lightPosition2Loc = glGetUniformLocation(gProgramId, "lightPos2");
    GLint lightStrength1Loc = glGetUniformLocation(gProgramId, "lightStrength1");
    GLint lightStrength2Loc = glGetUniformLocation(gProgramId, "lightStrength2");
    GLint viewPositionLoc = glGetUniformLocation(gProgramId, "viewPosition");
    GLint ambientStrengthLoc = glGetUniformLocation(gProgramId, "ambientStrength");
    GLint diffuseStrengthLoc = glGetUniformLocation(gProgramId, "diffuseStrength");
    GLint specularIntensityLoc = glGetUniformLocation(gProgramId, "specularIntensity");

    glUniform3f(lightColor1Loc, gLightColor1.r, gLightColor1.g, gLightColor1.b);
    glUniform3f(lightColor2Loc, gLightColor2.r, gLightColor2.g, gLightColor2.b);
    glUniform3f(lightPosition1Loc, gLightPosition1.x, gLightPosition1.y, gLightPosition1.z);
    glUniform3f(lightPosition2Loc, gLightPosition2.x, gLightPosition2.y, gLightPosition2.z);
    glUniform1f(lightStrength1Loc, lightStrength1);
    glUniform1f(lightStrength2Loc, lightStrength2);
    glUniform3f(ambientStrengthLoc, gAmbientStrength.r, gAmbientStrength.g, gAmbientStrength.b);
    glUniform3f(diffuseStrengthLoc, gDiffuseStrength.r, gAmbientStrength.g, gAmbientStrength.b);
    glUniform1f(specularIntensityLoc, gSpecularIntensity);;
    const glm::vec3 cameraPosition = camera.Position;
    glUniform3f(viewPositionLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);

    // There are no multiple textures
    GLuint multipleTexturesLoc = glGetUniformLocation(gProgramId, "multipleTextures");
    glUniform1i(multipleTexturesLoc, false);

    // Activate the VBOs contained within the mesh's VAO
    glBindVertexArray(gMesh.vao[0]);

    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textMug);

    // Draw a mug using a cylinder 
    glDrawElements(GL_TRIANGLES, cylinder1.getIndexCount(), GL_UNSIGNED_INT, NULL);

    // Deactivate the Vertex Array Object
    glBindVertexArray(0);


    // Handle : Cube  1 out of 2 : Array 2
    // Set up cup handle buffers
    //-------------------------------------------------------------------------
    handleConfig(gMesh);

    // Modify model view 
    scale = glm::mat4(1.0f);
    scale = glm::scale(scale, glm::vec3(0.3f, 0.1f, 1.1f));
    rotation = glm::mat4(1.0f);
    rotation = glm::rotate(rotation, 0.0f, glm::vec3(1.0, 0.0f, 0.0f));
    translation = glm::mat4(1.0f);
    translation = glm::translate(translation, glm::vec3(0.5f, 1.5f, 1.55f));
    model = translation * rotation * scale;

    // Specify color
    myColor[0] = 0.7f;
    myColor[1] = 0.7f;
    myColor[2] = 0.7f;

    // Set new model matrix and color in shader's uniform variables
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);

    glUniform1i(multipleTexturesLoc, false);
    glBindVertexArray(gMesh.vao[2]);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textHandle);
    glDrawArrays(GL_TRIANGLES, 0, cube1.verts.size() / 8);
    glBindVertexArray(0);

    // Handle : Cube 2 out of 2 : Array 2
    // ______________________________________________________________________
    // Modify height
    scale = glm::mat4(1.0f);
    scale = glm::scale(scale, glm::vec3(0.3f, 0.1f, 1.7f));
    rotation = glm::mat4(1.0f);
    rotation = glm::rotate(rotation, -0.785398f, glm::vec3(1.0, 0.0f, 0.0f));
    translation = glm::mat4(1.0f);
    translation = glm::translate(translation, glm::vec3(0.5f, 0.83f, 1.5f));
    model = translation * rotation * scale;

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);
    glUniform1i(multipleTexturesLoc, false);
    glBindVertexArray(gMesh.vao[2]);
    glDrawArrays(GL_TRIANGLES, 0, cube1.verts.size() / 8);
    glBindVertexArray(0);


    // Tea : Cylinder 2 out of 3: Array 3
    //-----------------------------------------------------------------------
    scale = glm::mat4(1.0f);
    rotation = glm::mat4(1.0f);
    rotation = glm::rotate(rotation, -1.5708f, glm::vec3(1.0, 0.0f, 0.0f));
    translation = glm::mat4(1.0f);
    translation = glm::translate(translation, glm::vec3(0.0f, 1.951f, 0.0f));
    model = translation * rotation * scale;

       glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);

    // Set to true to let fragment shader know of multiple textures
    glUniform1i(multipleTexturesLoc, true);

    glBindVertexArray(gMesh.vao[3]);
    // Texture for Tea
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textTea);
    // Texture for the Lemon inside the Tea
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textLemon);

    glDrawElements(GL_TRIANGLES, cylinder2.getIndexCount(), GL_UNSIGNED_INT, NULL);
    glBindVertexArray(0);


    // Place Matt : Plane 1 out of 2: Array 1
   //-----------------------------------------------------------------------
    // Change model view before drawing plane
    scale = glm::mat4(1.0f);
    scale = glm::scale(scale, glm::vec3(12.0f, 1.0f, 10.0f));
    rotation = glm::mat4(1.0f);
    rotation = glm::rotate(rotation, -0.8f, glm::vec3(0.0f, 1.0f, 0.0f));
    translation = glm::mat4(1.0f);
    translation = glm::translate(translation, glm::vec3(-1.0f, -0.57f, -2.0f));
    model = translation * rotation * scale;


    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);
    // Adjust ambient strength and specular intensity for place matt
    glUniform3f(lightColor1Loc, 1.0f, 1.0f, 1.0f);
    glUniform3f(lightColor2Loc, 0.0f, 0.0f, 0.0f);
    glUniform3f(ambientStrengthLoc, 0.3001f, 0.3001f, 0.3001f);
    glUniform1f(specularIntensityLoc, 0.5f);

    // tell fragment shader there is not multiple textures
    glUniform1i(multipleTexturesLoc, false);

    // Activate the VBOs contained within the mesh's VAO
    glBindVertexArray(gMesh.vao[1]);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textPlaceMat);
    glDrawArrays(GL_TRIANGLES, 0, plane1.verts.size() / 8);

    // revert to original ambient strength, diffuse strength, and specular intensity
    glUniform3f(lightColor1Loc, gLightColor1.r, gLightColor1.g, gLightColor1.b);
    glUniform3f(lightColor2Loc, gLightColor2.r, gLightColor2.g, gLightColor2.b);
    glUniform3f(ambientStrengthLoc, gAmbientStrength.r, gAmbientStrength.g, gAmbientStrength.b);
    glUniform3f(diffuseStrengthLoc, gDiffuseStrength.r, gAmbientStrength.g, gAmbientStrength.b);
    glUniform1f(specularIntensityLoc, gSpecularIntensity);

    // Deactivate the Vertex Array Object
    glBindVertexArray(0);


   // Napkin : Plane 2 out of 2: Array 1
   //-----------------------------------------------------------------------
   // Change model view before drawing plane
    scale = glm::mat4(2.0f);
    scale = glm::scale(scale, glm::vec3(4.0f, 1.0f, 4.0f));
    rotation = glm::mat4(1.0f);
    rotation = glm::rotate(rotation, -0.8f, glm::vec3(0.0f, 1.0f, 0.0f));
    translation = glm::mat4(1.0f);
    translation = glm::translate(translation, glm::vec3(0.0f, -0.56f, 0.0f));
    model = translation * rotation * scale;

    // Set new model matrix and color in shader's uniform variables
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);
    // Modify lighting for object
    glUniform3f(ambientStrengthLoc, 0.00001f, 0.00001f, 0.00001f);
    glUniform1f(specularIntensityLoc, 0.001f);

    glUniform1i(multipleTexturesLoc, false);

    glBindVertexArray(gMesh.vao[1]);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textNapkin);

    // Draw the plane
    glDrawArrays(GL_TRIANGLES, 0, plane1.verts.size() / 8);

    // Revert to original ambient strength, diffuse strength, and specular intensity
    glUniform3f(ambientStrengthLoc, gAmbientStrength.r, gAmbientStrength.g, gAmbientStrength.b);
    glUniform1f(specularIntensityLoc, gSpecularIntensity);

    // Deactivate the Vertex Array Object
    glBindVertexArray(0);

    // Tomato : Sphere: Array 4
   //-----------------------------------------------------------------------
    scale = glm::mat4(1.0f);
    rotation = glm::mat4(1.0f);
    rotation = glm::rotate(rotation, 1.0f, glm::vec3(0.0, 1.0f, 0.0f));
    translation = glm::mat4(1.0f);
    translation = glm::translate(translation, glm::vec3(-3.5f, 0.90f, -1.3f));
    model = translation * rotation * scale;

    // Specify color
    myColor[0] = 1.0f;
    myColor[1] = 1.0f;
    myColor[2] = 1.0f;

    // Set new model matrix and color in shader's uniform variables
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);
    // Modify lighting for object
    glUniform3f(lightColor1Loc, 1.0f, 1.0f, 0.80f);
    glUniform3f(ambientStrengthLoc, 0.09f, 0.09f, 0.08f);
    glUniform1f(specularIntensityLoc, 0.4f);

    // tell fragment shader there is multiple textures
    glUniform1i(multipleTexturesLoc, false);

    // Activate the VBOs contained within the mesh's VAO
    glBindVertexArray(gMesh.vao[4]);

    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textTomato);

    // Draw the tomato sphere
    glDrawElements(GL_TRIANGLES, sphere1.getIndexCount(), GL_UNSIGNED_INT, NULL);

    // set lighting components back to normal
    glUniform3f(lightColor1Loc, gLightColor1.r, gLightColor1.g, gLightColor1.b);
    glUniform3f(ambientStrengthLoc, gAmbientStrength.r, gAmbientStrength.g, gAmbientStrength.b);
    glUniform1f(specularIntensityLoc, gSpecularIntensity);

    // Deactivate the Vertex Array Object
    glBindVertexArray(0);


    // Plate : Cylinder 3 out of 3: Array 5
   //-----------------------------------------------------------------------
    scale = glm::mat4(1.0f);
    rotation = glm::mat4(1.0f);
    rotation = glm::rotate(rotation, -1.5708f, glm::vec3(0.0, 1.0f, 0.0f));
    rotation = glm::rotate(rotation, -1.5708f, glm::vec3(1.0, 0.0f, 0.0f));
    translation = glm::mat4(1.0f);
    translation = glm::translate(translation, glm::vec3(-3.9f, 0.08f, -1.6f));
    model = translation * rotation * scale;

    // Set new model matrix and color in shader's uniform variables
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);
    // change lighting components for this object
    glUniform3f(lightColor1Loc, 1.0f, 0.60f, 0.20f);
    glUniform3f(ambientStrengthLoc, 0.08f, 0.08f, 0.08f);
    glUniform1f(specularIntensityLoc, 0.5f);

    // tell fragment shader there is multiple textures
    glUniform1i(multipleTexturesLoc, false);

    // Activate the VBOs contained within the mesh's VAO
    glBindVertexArray(gMesh.vao[5]);

    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textPlate);

    // Draw the tea cylinder
    glDrawElements(GL_TRIANGLES, cylinder3.getIndexCount(), GL_UNSIGNED_INT, NULL);

    // set lighting components back to normal
    glUniform3f(lightColor1Loc, gLightColor1.r, gLightColor1.g, gLightColor1.b);
    glUniform3f(ambientStrengthLoc, gAmbientStrength.r, gAmbientStrength.g, gAmbientStrength.b);
    glUniform1f(specularIntensityLoc, gSpecularIntensity);

    // Deactivate the Vertex Array Object
    glBindVertexArray(0);




    //-------------------------------------------------------------------------------------
    // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
    glfwSwapBuffers(gWindow);    // Flips the the back buffer with the front buffer every frame.
}



/* Set up buffer(s) and configure vertex attributes for:
 * Mug (cylinder), Handle (cube), and Tea (cylinder)
 */

// Mug
// -----------------------------------------------
void mugConfig(GLMesh& mesh)
{
    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerNormals = 3;
    const GLuint floatsPerUV = 2;
    const GLuint floatsInEachStride = 8;

    glBindVertexArray(mesh.vao[0]); // activate vertex array object

    // Activates the first buffer
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos[0]);
    // Sends vertex data to the GPU
    glBufferData(GL_ARRAY_BUFFER, cylinder1.getInterleavedVertexSize(), cylinder1.getInterleavedVertices(), GL_STATIC_DRAW);

    // activate second buffer for index array
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbos[1]);
    // stores indices[] array on GPU
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, cylinder1.getIndexSize(), cylinder1.getIndices(), GL_STATIC_DRAW);

    // Strides between vertex coordinates
    GLint stride = cylinder1.getInterleavedStride();

    // Create Vertex Attribute Pointers for array 0
    // Position of data
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0); 
    // Normals data for light
    glVertexAttribPointer(1, floatsPerNormals, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1); 
    // Texture data for coordinates
    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2); 
}


// Handle
// ----------------------------------------------
void handleConfig(GLMesh& mesh) {

    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerNormals = 3;
    const GLuint floatsPerUV = 2;
    const GLuint floatsInEachStride = 8;

    glBindVertexArray(mesh.vao[2]); // activate vertex array object

    // Activates the first buffer
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos[3]);
    // Sends vertex data to the GPU
    glBufferData(GL_ARRAY_BUFFER, cube1.verts.size() * sizeof(float), cube1.verts.data(), GL_STATIC_DRAW);

    // Strides between vertex coordinates
    GLint stride = sizeof(float) * (floatsInEachStride);// The number of floats before each

    // Create Vertex Attribute Pointers for array 1
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0); 
    glVertexAttribPointer(1, floatsPerNormals, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1); 
    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormals)));
    glEnableVertexAttribArray(2); 
}


// Tea
// -------------------------------------------
void teaConfig(GLMesh& mesh)
{
    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerNormals = 3;
    const GLuint floatsPerUV = 2;
    const GLuint floatsInEachStride = 8;

    glBindVertexArray(mesh.vao[3]); // activate vertex array object

    // Activates the first buffer
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos[4]);
    // Sends vertex data to the GPU
    glBufferData(GL_ARRAY_BUFFER, cylinder2.getInterleavedVertexSize(), cylinder2.getInterleavedVertices(), GL_STATIC_DRAW);

    // activate second buffer for index array
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbos[5]);
    // stores indices[] array on GPU
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, cylinder2.getIndexSize(), cylinder2.getIndices(), GL_STATIC_DRAW);

    // The number of floats that make up a block of vertex data. Should be 32 bytes
    GLint stride = cylinder2.getInterleavedStride();

    // Create Vertex Attribute Pointers for array 2
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, floatsPerNormals, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2); 

}

// Place Matt
// -------------------------------------------
void planeConfig(GLMesh& mesh) {
    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerNormals = 3;
    const GLuint floatsPerUV = 2;
    const GLuint floatsInEachStride = 8;

    glBindVertexArray(mesh.vao[1]); // activate vertex array object

    // Activates the first buffer
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos[2]);
    // Sends vertex data to the GPU
    glBufferData(GL_ARRAY_BUFFER, plane1.verts.size() * sizeof(float), plane1.verts.data(), GL_STATIC_DRAW);

    // Strides between vertex coordinates
    GLint stride = sizeof(float) * (floatsInEachStride);// The number of floats before each

    // Create Vertex Attribute Pointers for array 3
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, floatsPerNormals, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormals)));
    glEnableVertexAttribArray(2);

}

// Tomato
// -------------------------------------------
void sphereConfig(GLMesh& mesh)
{
    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerNormals = 3;
    const GLuint floatsPerUV = 2;
    const GLuint floatsInEachStride = 8;

    glBindVertexArray(mesh.vao[4]); // activate vertex array object

    // Activates the first buffer
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos[6]);
    // Sends vertex data to the GPU
    glBufferData(GL_ARRAY_BUFFER, sphere1.getInterleavedVertexSize(), sphere1.getInterleavedVertices(), GL_STATIC_DRAW);

    // activate second buffer for index array
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbos[7]);
    // stores indices[] array on GPU
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphere1.getIndexSize(), sphere1.getIndices(), GL_STATIC_DRAW);

    // Strides between vertex coordinates
    GLint stride = sphere1.getInterleavedStride();

    // Create Vertex Attribute Pointers
    // position attribute -- instructs GPU how to handle vertex position data
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0); // enables the vertex attribute array numbered 0
    // normals attribute -- instructs GPU how to handle normals data
    glVertexAttribPointer(1, floatsPerNormals, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1); // enables the vertex attribute array numbered 0
    // texture coordinate attribute -- instructs GPU how to handle texture coordinates
    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2); // enables the vertex attribute array numbered 0
}

// Plate
// -------------------------------------------
void plateConfig(GLMesh& mesh)
{
    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerNormals = 3;
    const GLuint floatsPerUV = 2;
    const GLuint floatsInEachStride = 8;

    glBindVertexArray(mesh.vao[5]); // activate vertex array object

    // Activates the first buffer
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos[8]);
    // Sends vertex data to the GPU
    glBufferData(GL_ARRAY_BUFFER, cylinder3.getInterleavedVertexSize(), cylinder3.getInterleavedVertices(), GL_STATIC_DRAW);

    // activate second buffer for index array
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbos[9]);
    // stores indices[] array on GPU
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, cylinder3.getIndexSize(), cylinder3.getIndices(), GL_STATIC_DRAW);

    // The number of floats that make up a block of vertex data. Should be 32 bytes
    GLint stride = cylinder3.getInterleavedStride();

    // Create Vertex Attribute Pointers
    // position attribute -- instructs GPU how to handle vertex position data
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0); // enables the vertex attribute array numbered 0
    // normals attribute -- instructs GPU how to handle normals data
    glVertexAttribPointer(1, floatsPerNormals, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1); // enables the vertex attribute array numbered 0
    // texture coordinate attribute -- instructs GPU how to handle texture coordinates
    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2); // enables the vertex attribute array numbered 0

}

// Set up vertex data and populate plane structure for configuration
// -----------------------------------------------------------------------
void planeMesh() {

    vector<float> verts = {

        // position           // normal           // tex coordinate
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f
    };

    // populate plane1 struct with this mesh data
    verts.swap(plane1.verts);
}


// Set up vertex data and populate cube structure for configuration
// -----------------------------------------------------------------------
void cubeMesh() {

    vector<float> verts = {

        // positions          // normals           // texture coordinates
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f
    };
    glm::vec3 glColor3f(1.0f, 0.0f, 0.0f);

    // populate cube1 struct with this mesh data
    verts.swap(cube1.verts);
}




void UDestroyMesh(GLMesh& mesh)
{
    // delete the VAOs
    glDeleteVertexArrays(6, mesh.vao);
    // delete the VBOs
    glDeleteBuffers(8, mesh.vbos);
}


// utility function for loading a 2D texture from file
// ---------------------------------------------------
bool loadTexture(const char* filename, GLuint& textureId, int textureUnit)
{
    int width, height, channels;
    unsigned char* image = stbi_load(filename, &width, &height, &channels, 0);
    if (image)
    {
        switch (textureUnit){
        case 0:{
            glActiveTexture(GL_TEXTURE0);
        }
        break;

        case 1:{
            glActiveTexture(GL_TEXTURE1);
        }
        break;

        default:
            glActiveTexture(GL_TEXTURE0);
            break;
        }

        glGenTextures(1, &textureId);
        glBindTexture(GL_TEXTURE_2D, textureId);

        // set the texture wrapping parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // set texture filtering parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        if (channels == 3)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
        else if (channels == 4)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
        else
        {
            cout << "Not implemented to handle image with " << channels << " channels" << endl;
            return false;
        }

        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(image);
        glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture

        return true;
    }

    // Error loading the image
    return false;
}


/* ------------------- Deletes a texture -------------------*/
void UDestroyTexture(GLuint textureId)
{
    glGenTextures(1, &textureId);
}

/* ------------------- Flip image vertically -------------------*/
// Images are loaded with Y axis going down, but OpenGL's Y axis goes up, so let's flip it
void flipImageVertically(unsigned char* image, int width, int height, int channels)
{
    for (int j = 0; j < height / 2; ++j)
    {
        int index1 = j * width * channels;
        int index2 = (height - 1 - j) * width * channels;

        for (int i = width * channels; i > 0; --i)
        {
            unsigned char tmp = image[index1];
            image[index1] = image[index2];
            image[index2] = tmp;
            ++index1;
            ++index2;
        }
    }
}




bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId)
{
    // Compilation and linkage error reporting
    int success = 0;
    char infoLog[512]; // create character string of length 512 for the error log

    // Create a Shader program object.
    programId = glCreateProgram();

    // Create the vertex and fragment shader objects
    GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

    // Retrive the shader source
    glShaderSource(vertexShaderId, 1, &vtxShaderSource, NULL);
    glShaderSource(fragmentShaderId, 1, &fragShaderSource, NULL);

    // Compile the vertex shader, and print compilation errors (if any)
    glCompileShader(vertexShaderId); // compile the vertex shader
    // check for shader compile errors
    glGetShaderiv(vertexShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShaderId, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;

        return false;
    }

    glCompileShader(fragmentShaderId); // compile the fragment shader
    // check for shader compile errors
    glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShaderId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;

        return false;
    }

    // Attached compiled shaders to the shader program
    glAttachShader(programId, vertexShaderId);
    glAttachShader(programId, fragmentShaderId);

    glLinkProgram(programId);   // links the shader program
    // check for linking errors
    glGetProgramiv(programId, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(programId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;

        return false;
    }

    glUseProgram(programId);    // Uses the shader program

    return true;
}



void UDestroyShaderProgram(GLuint programId)
{
    glDeleteProgram(programId); // delete the shader program
}



