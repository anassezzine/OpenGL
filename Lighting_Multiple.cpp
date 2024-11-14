//-----------------------------------------------------------------------------
// This code demonstrates how to implement the Blinn-Phong lighting model for
// multiple light sources using OpenGL 3.3 or a forward compatible
// OpenGL 3.0 rendering context. 
//-----------------------------------------------------------------------------
#include <iostream>
#include <sstream>
#include <string>
#define GLEW_STATIC
#include "GL/glew.h"	// Important - this header must come before glfw3 header
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "Texture2D.h"
#include "Camera.h"
#include "Mesh.h"


// Global Variables
const char* APP_TITLE = "Introduction to Modern OpenGL - Multiple Lights";
int gWindowWidth = 1024;
int gWindowHeight = 768;
GLFWwindow* gWindow = NULL;
bool gWireframe = false;
bool gFlashlightOn = true;
bool gLightOn = true;
glm::vec4 gClearColor(0.06f, 0.06f, 0.07f, 1.0f);

FPSCamera fpsCamera(glm::vec3(0.0f, 13.5f, 40.0f));
const double ZOOM_SENSITIVITY = -3.0;
const float MOVE_SPEED = 10.0; // units per second
const float MOUSE_SENSITIVITY = 0.1f;


// Function prototypes
void glfw_onKey(GLFWwindow* window, int key, int scancode, int action, int mode);
void glfw_onFramebufferSize(GLFWwindow* window, int width, int height);
void glfw_onMouseScroll(GLFWwindow* window, double deltaX, double deltaY);
void update(double elapsedTime, glm::vec3&);
void showFPS(GLFWwindow* window);
bool initOpenGL();

//-----------------------------------------------------------------------------
// Main Application Entry Point
//-----------------------------------------------------------------------------
int main()
{
	if (!initOpenGL())
	{
		// An error occured
		std::cerr << "GLFW initialization failed" << std::endl;
		return -1;
	}

	ShaderProgram lightingShader;
	lightingShader.loadShaders("shaders/lighting_dir_point_spot.vert", "shaders/lighting_dir_point_spot.frag");

	// Load meshes and textures
	const int numModels = 11;
	Mesh mesh[numModels];	Texture2D texture[numModels];

	mesh[0].loadOBJ("models/floor.obj");
	mesh[1].loadOBJ("models/road.obj");
	mesh[2].loadOBJ("models/house.obj");
	mesh[3].loadOBJ("models/car.obj");
	mesh[4].loadOBJ("models/lampPost.obj");
	mesh[5].loadOBJ("models/lampPost.obj");
	mesh[6].loadOBJ("models/lampPost.obj");
	mesh[7].loadOBJ("models/car.obj");
	mesh[8].loadOBJ("models/garden.obj");
	mesh[9].loadOBJ("models/lampPost.obj");
	mesh[10].loadOBJ("models/pool.obj");



	
	texture[0].loadTexture("textures/terrain.jpg", true);
	texture[1].loadTexture("textures/road.png", true);
	texture[2].loadTexture("textures/house.png", true);
	texture[3].loadTexture("textures/car.jpeg", true);
	texture[4].loadTexture("textures/lamp_post_diffuse.png", true);
	texture[5].loadTexture("textures/lamp_post_diffuse.png", true);
	texture[6].loadTexture("textures/lamp_post_diffuse.png", true);
	texture[7].loadTexture("textures/secondcar.jpeg", true);
	texture[8].loadTexture("textures/garden.png", true);
	texture[9].loadTexture("textures/lamp_post_diffuse.png", true);
	texture[10].loadTexture("textures/pool.png", true);


	
	// Model positions
	glm::vec3 modelPos[] = {
		glm::vec3(0.0f, 0.0f, 0.0f),	// floor (x,y,z) -z recule, +z avance
		glm::vec3(0.0f,-3.65f,0.0f),	// road
		glm::vec3(0.0f,0.0f,-27.2f),	// house
		glm::vec3(150.0f,0.5f,-3.0f),	// car
		glm::vec3(-25.0f,0.0f,-25.0f),	// lamp
		glm::vec3(-25.0f,0.0f,-35.0f),	// lamp
		glm::vec3(-25.0f,0.0f,-45.0f),	// lamp
		glm::vec3(-19.0f,0.0f,-25.0f),	// second car
		glm::vec3(22.3f,0.0f,-27.9f),	// garden
		glm::vec3(15.0f,0.0f,-14.9f),	// lamp
		glm::vec3(40.3f,-2.5f,-27.9f),	// pool

	};

	// Model scale
	glm::vec3 modelScale[] = {
		glm::vec3(10.0f, 1.0f, 10.0f),	// floor
		glm::vec3(0.5f, 0.5f, 0.3f),	// road
		glm::vec3(1.3f, 1.3f, 1.3f),	// house
		glm::vec3(0.02f, 0.02f, 0.02f),	// car
		glm::vec3(2.0f, 2.0f, 2.0f),	// lamp
		glm::vec3(2.0f, 2.0f, 2.0f),	// lamp
		glm::vec3(2.0f, 2.0f, 2.0f),	// lamp
		glm::vec3(0.025f, 0.025f, 0.025f),	// second car
		glm::vec3(0.5f, 0.5f, 1.0f),	// garden
		glm::vec3(2.0f, 4.0f, 2.0f),	// lamp
		glm::vec3(0.5f, 0.5f, 0.5f),	// pool
	};

	// Point Light positions
	glm::vec3 pointLightPos[5] = {
		glm::vec3(-25.0f,10.0f,-45.0f), //third lamp
		glm::vec3(-25.0f,10.0f,-35.0f), //second lamp
		glm::vec3(-25.0f,10.0f,-25.0f), //first lamp
		glm::vec3(15.0f,17.0f,-10.0f),  //forth lamp
		glm::vec3(0.0f,  5.0f,  -20.0f),//door
	};


	double lastTime = glfwGetTime();

	// Rendering loop
	while (!glfwWindowShouldClose(gWindow))
	{
		showFPS(gWindow);

		double currentTime = glfwGetTime();
		double deltaTime = currentTime - lastTime;

		// Poll for and process events
		glfwPollEvents();
		update(deltaTime, modelPos[3]);

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 model(1.0), view(1.0), projection(1.0);

		// Create the View matrix
		view = fpsCamera.getViewMatrix();

		// Create the projection matrix
		projection = glm::perspective(glm::radians(fpsCamera.getFOV()), (float)gWindowWidth / (float)gWindowHeight, 0.1f, 200.0f);

		// update the view (camera) position
		glm::vec3 viewPos;
		viewPos.x = fpsCamera.getPosition().x;
		viewPos.y = fpsCamera.getPosition().y;
		viewPos.z = fpsCamera.getPosition().z;


		// Must be called BEFORE setting uniforms because setting uniforms is done
		// on the currently active shader program.
		lightingShader.use();
		lightingShader.setUniform("model", glm::mat4(1.0));  // do not need to translate the models so just send the identity matrix
		lightingShader.setUniform("view", view);
		lightingShader.setUniform("projection", projection);
		lightingShader.setUniform("viewPos", viewPos);

		// Directional light
		if(gLightOn)
		{
			lightingShader.setUniform("sunLight.direction", glm::vec3(0.0f, -0.9f, -0.17f));
			lightingShader.setUniform("sunLight.ambient", glm::vec3(0.1f, 0.1f, 0.1f));
			lightingShader.setUniform("sunLight.diffuse", glm::vec3(0.1f, 0.1f, 0.1f));		// dark
			lightingShader.setUniform("sunLight.specular", glm::vec3(0.1f, 0.1f, 0.1f));
		}
		else
		{
			lightingShader.setUniform("sunLight.direction", glm::vec3(0.0f, -0.9f, -0.17f));
			lightingShader.setUniform("sunLight.ambient", glm::vec3(0.1f, 0.1f, 0.1f));
			lightingShader.setUniform("sunLight.diffuse", glm::vec3(1.0f, 1.0f, 1.0f));		// dark
			lightingShader.setUniform("sunLight.specular", glm::vec3(0.1f, 0.1f, 0.1f));
		}
		

		// Point Light 1
		lightingShader.setUniform("pointLights[0].ambient", glm::vec3(3.0f, 3.0f, 3.0f));
		lightingShader.setUniform("pointLights[0].diffuse", glm::vec3(1.0f, 1.0f, 1.0f));
		lightingShader.setUniform("pointLights[0].specular", glm::vec3(2.0f, 2.0f, 2.0f));
		lightingShader.setUniform("pointLights[0].position", pointLightPos[0]);
		lightingShader.setUniform("pointLights[0].constant", 1.0f);
		lightingShader.setUniform("pointLights[0].linear", 0.11f);
		lightingShader.setUniform("pointLights[0].exponent", 0.10f);

		// Point Light 2
		lightingShader.setUniform("pointLights[1].ambient", glm::vec3(3.0f, 3.0f, 3.0f));
		lightingShader.setUniform("pointLights[1].diffuse", glm::vec3(1.0f, 1.0f, 1.0f));	
		lightingShader.setUniform("pointLights[1].specular", glm::vec3(2.0f, 2.0f, 2.0f));
		lightingShader.setUniform("pointLights[1].position", pointLightPos[1]);
		lightingShader.setUniform("pointLights[1].constant", 1.0f);
		lightingShader.setUniform("pointLights[1].linear", 0.11f);
		lightingShader.setUniform("pointLights[1].exponent", 0.10f);

		lightingShader.setUniform("pointLights[2].ambient", glm::vec3(3.0f, 3.0f, 3.0f));
		lightingShader.setUniform("pointLights[2].diffuse", glm::vec3(1.0f, 1.0f, 1.0f));	
		lightingShader.setUniform("pointLights[2].specular", glm::vec3(2.0f, 2.0f, 2.0f));
		lightingShader.setUniform("pointLights[2].position", pointLightPos[2]);
		lightingShader.setUniform("pointLights[2].constant", 1.0f);
		lightingShader.setUniform("pointLights[2].linear", 0.11f);
		lightingShader.setUniform("pointLights[2].exponent", 0.10f);

		// Point Light 3
		lightingShader.setUniform("pointLights[3].ambient", glm::vec3(3.0f, 3.0f, 3.0f));  
		lightingShader.setUniform("pointLights[3].diffuse", glm::vec3(1.0f, 1.0f, 1.0f));  
		lightingShader.setUniform("pointLights[3].specular", glm::vec3(2.0f, 2.0f, 2.0f));  //surface
		lightingShader.setUniform("pointLights[3].position", pointLightPos[3]);
		lightingShader.setUniform("pointLights[3].constant", 1.0f);
		lightingShader.setUniform("pointLights[3].linear", 0.11f);
		lightingShader.setUniform("pointLights[3].exponent", 0.10f);

		//Point Light 4
		lightingShader.setUniform("pointLights[4].ambient", glm::vec3(0.2f, 0.2f, 0.2f));
		lightingShader.setUniform("pointLights[4].diffuse", glm::vec3(1.0f, 1.0f, 1.0f));	
		lightingShader.setUniform("pointLights[4].specular", glm::vec3(1.0f, 1.0f, 1.0f));
		lightingShader.setUniform("pointLights[4].position", pointLightPos[4]);
		lightingShader.setUniform("pointLights[4].constant", 1.0f);
		lightingShader.setUniform("pointLights[4].linear", 0.22f);
		lightingShader.setUniform("pointLights[4].exponent", 0.20f);

		// Spot light
		glm::vec3 spotlightPos = fpsCamera.getPosition();

		// offset the flash light down a little
		spotlightPos.y -= 0.5f;

		lightingShader.setUniform("spotLight.ambient", glm::vec3(0.1f, 0.1f, 0.1f));
		lightingShader.setUniform("spotLight.diffuse", glm::vec3(0.8f, 0.8f, 0.8f));
		lightingShader.setUniform("spotLight.specular", glm::vec3(1.0f, 1.0f, 1.0f));
		lightingShader.setUniform("spotLight.position", spotlightPos);
		lightingShader.setUniform("spotLight.direction", fpsCamera.getLook());
		lightingShader.setUniform("spotLight.cosInnerCone", glm::cos(glm::radians(15.0f)));
		lightingShader.setUniform("spotLight.cosOuterCone", glm::cos(glm::radians(20.0f)));
		lightingShader.setUniform("spotLight.constant", 1.0f);
		lightingShader.setUniform("spotLight.linear", 0.07f);
		lightingShader.setUniform("spotLight.exponent", 0.017f);
		lightingShader.setUniform("spotLight.on", gFlashlightOn);

		// Render the scene
		for (int i = 0; i < numModels; ++i) {
            model = glm::mat4(1.0f); // Réinitialiser la matrice modèle pour chaque mesh

            // Appliquer des transformations spécifiques pour mesh[3]
            if (i == 3) {
                model = glm::translate(model, modelPos[i]);   // Translation
                model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // Rotation
                model = glm::scale(model, modelScale[i]);      // Échelle
            }else {
                model = glm::translate(model, modelPos[i]);    // Translation
                model = glm::scale(model, modelScale[i]);      // Échelle
            }

            // Charger les transformations et les propriétés du matériau dans le shader
            lightingShader.setUniform("model", model); 
            lightingShader.setUniform("material.ambient", glm::vec3(0.1f, 0.1f, 0.1f));
            lightingShader.setUniformSampler("material.diffuseMap", 0);
            lightingShader.setUniform("material.specular", glm::vec3(0.8f, 0.8f, 0.8f));
            lightingShader.setUniform("material.shininess", 32.0f);

            // Activer la texture et dessiner le mesh
            texture[i].bind(0); 
            mesh[i].draw();     
            texture[i].unbind(0);
        }   

				// Swap front and back buffers
		glfwSwapBuffers(gWindow);

		lastTime = currentTime;
	}

	glfwTerminate();

	return 0;
}

//-----------------------------------------------------------------------------
// Initialize GLFW and OpenGL
//-----------------------------------------------------------------------------
bool initOpenGL()
{
	// Intialize GLFW 
	// GLFW is configured.  Must be called before calling any GLFW functions
	if (!glfwInit())
	{
		// An error occured
		std::cerr << "GLFW initialization failed" << std::endl;
		return false;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);	// forward compatible with newer versions of OpenGL as they become available but not backward compatible (it will not run on devices that do not support OpenGL 3.3


	// Create an OpenGL 3.3 core, forward compatible context window
	gWindow = glfwCreateWindow(gWindowWidth, gWindowHeight, APP_TITLE, NULL, NULL);
	if (gWindow == NULL)
	{
		std::cerr << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return false;
	}

	// Make the window's context the current one
	glfwMakeContextCurrent(gWindow);

	// Initialize GLEW
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		std::cerr << "Failed to initialize GLEW" << std::endl;
		return false;
	}

	// Set the required callback functions
	glfwSetKeyCallback(gWindow, glfw_onKey);
	glfwSetFramebufferSizeCallback(gWindow, glfw_onFramebufferSize);
	glfwSetScrollCallback(gWindow, glfw_onMouseScroll);

	// Hides and grabs cursor, unlimited movement
	glfwSetInputMode(gWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPos(gWindow, gWindowWidth / 2.0, gWindowHeight / 2.0);

	glClearColor(gClearColor.r, gClearColor.g, gClearColor.b, gClearColor.a);

    // Define the viewport dimensions
    int w, h;
    glfwGetFramebufferSize( gWindow, &w, &h); // For retina display
    glViewport(0, 0, w, h);
    
    //    glViewport(0, 0, gWindowWidth, gWindowHeight);

    glEnable(GL_DEPTH_TEST);

	return true;
}

//-----------------------------------------------------------------------------
// Is called whenever a key is pressed/released via GLFW
//-----------------------------------------------------------------------------
void glfw_onKey(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
	
	if (key == GLFW_KEY_F1 && action == GLFW_PRESS)
	{
		gWireframe = !gWireframe;
		if (gWireframe)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		else
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	if (key == GLFW_KEY_F && action == GLFW_PRESS)
	{
		// toggle the flashlight
		gFlashlightOn = !gFlashlightOn;
	}

	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
	{
		gLightOn = !gLightOn;
	}

}

//-----------------------------------------------------------------------------
// Is called when the window is resized
//-----------------------------------------------------------------------------
void glfw_onFramebufferSize(GLFWwindow* window, int width, int height)
{
	gWindowWidth = width;
	gWindowHeight = height;

    // Define the viewport dimensions
    int w, h;
    glfwGetFramebufferSize( gWindow, &w, &h); // For retina display
    glViewport(0, 0, w, h);
    
    //    glViewport(0, 0, gWindowWidth, gWindowHeight);

}

//-----------------------------------------------------------------------------
// Called by GLFW when the mouse wheel is rotated
//-----------------------------------------------------------------------------
void glfw_onMouseScroll(GLFWwindow* window, double deltaX, double deltaY)
{
	double fov = fpsCamera.getFOV() + deltaY * ZOOM_SENSITIVITY;

	fov = glm::clamp(fov, 1.0, 120.0);

	fpsCamera.setFOV((float)fov);
}

//-----------------------------------------------------------------------------
// Update stuff every frame
//-----------------------------------------------------------------------------

float movementRange = 40.0f;   // Amplitude du mouvement horizontal
float movementSpeed = 10.0f;     // Vitesse de déplacement de l'objet

void update(double elapsedTime, glm::vec3& pos)
{
    // Utiliser une position qui fait un aller simple de droite à gauche puis recommence
    float totalRange = 2 * movementRange;  // Plage complète de déplacement de +40 à -40
    float posX = fmod(movementSpeed * (float)glfwGetTime(), totalRange);  // Position cyclique de 0 à totalRange

    // Transformer pour qu'elle aille de +movementRange à -movementRange
    pos.x = movementRange - posX;

	// Camera orientation
	double mouseX, mouseY;

	// Get the current mouse cursor position delta
	glfwGetCursorPos(gWindow, &mouseX, &mouseY);

	// Rotate the camera the difference in mouse distance from the center screen.  Multiply this delta by a speed scaler
	fpsCamera.rotate((float)(gWindowWidth / 2.0 - mouseX) * MOUSE_SENSITIVITY, (float)(gWindowHeight / 2.0 - mouseY) * MOUSE_SENSITIVITY);

	// Clamp mouse cursor to center of screen
	glfwSetCursorPos(gWindow, gWindowWidth / 2.0, gWindowHeight / 2.0);

	// Forward/backward
	if (glfwGetKey(gWindow, GLFW_KEY_W) == GLFW_PRESS)
		fpsCamera.move(MOVE_SPEED * (float)elapsedTime * fpsCamera.getLook());
	else if (glfwGetKey(gWindow, GLFW_KEY_S) == GLFW_PRESS)
		fpsCamera.move(MOVE_SPEED * (float)elapsedTime * -fpsCamera.getLook());

	// Strafe left/right
	if (glfwGetKey(gWindow, GLFW_KEY_A) == GLFW_PRESS)
		fpsCamera.move(MOVE_SPEED * (float)elapsedTime * -fpsCamera.getRight());
	else if (glfwGetKey(gWindow, GLFW_KEY_D) == GLFW_PRESS)
		fpsCamera.move(MOVE_SPEED * (float)elapsedTime * fpsCamera.getRight());

	// Up/down
	if (glfwGetKey(gWindow, GLFW_KEY_Z) == GLFW_PRESS)
		fpsCamera.move(MOVE_SPEED * (float)elapsedTime * glm::vec3(0.0f, 1.0f, 0.0f));
	else if (glfwGetKey(gWindow, GLFW_KEY_X) == GLFW_PRESS)
		fpsCamera.move(MOVE_SPEED * (float)elapsedTime * -glm::vec3(0.0f, 1.0f, 0.0f));
}

//-----------------------------------------------------------------------------
// Code computes the average frames per second, and also the average time it takes
// to render one frame.  These stats are appended to the window caption bar.
//-----------------------------------------------------------------------------
void showFPS(GLFWwindow* window)
{
	static double previousSeconds = 0.0;
	static int frameCount = 0;
	double elapsedSeconds;
	double currentSeconds = glfwGetTime(); // returns number of seconds since GLFW started, as double float

	elapsedSeconds = currentSeconds - previousSeconds;

	// Limit text updates to 4 times per second
	if (elapsedSeconds > 0.25)
	{
		previousSeconds = currentSeconds;
		double fps = (double)frameCount / elapsedSeconds;
		double msPerFrame = 1000.0 / fps;

		// The C++ way of setting the window title
		std::ostringstream outs;
		outs.precision(3);	// decimal places
		outs << std::fixed
			<< APP_TITLE << "    "
			<< "FPS: " << fps << "    "
			<< "Frame Time: " << msPerFrame << " (ms)";
		glfwSetWindowTitle(window, outs.str().c_str());

		// Reset for next average.
		frameCount = 0;
	}

	frameCount++;
}
