#define _USE_MATH_DEFINES
#include <cmath>
#include "utilities.h"
#include "SimpleModel.h"
#include <string>


// global variables
// settings
unsigned int gWindowWidth = 1200;
unsigned int gWindowHeight = 900;

// frame stats
float gFrameRate = 60.0f;
float gFrameTime = 1 / gFrameRate;
float gAspectRatio = static_cast<float>(gWindowHeight) / gWindowWidth; // controls whether circle or elipse if window not square

// scene content
ShaderProgram gShader;	// shader program object
ShaderProgram shader2D;	// shader program object
GLuint gVBO = 0;		// vertex buffer object identifier
GLuint gVAO = 0;		// vertex array object identifier

std::map<std::string, glm::mat4> gModelMatrix;	// object model matrices
std::vector<GLfloat> gVertices;			// vertex positions of circles
glm::mat4 gViewMatrix;			// view matrix
glm::mat4 gProjectionMatrix;	// projection matrix

Light gLight;			// light properties in frag shader
std::map<std::string, Material> gMaterials;		// material properties in frag shader
std::map<std::string, SimpleModel> gModels;		// scene object model

// controls
bool gWireframe = false;	// wireframe control
float orbitIncrementor;
static float orbitAngle = 30.0 * gFrameTime; //*by frame time to keep constant


// generate vertices for a circle, pass aspect ratio, vertices array pointer, colour for vertices, radius, and the origin point for the circle
void generate_circle(std::vector<GLfloat>& gVertices, float radius)
{
	//default 25 slices is enough for a cirle at this scale
	float slices = 35;
	float slice_angle = M_PI * 2.0f / (slices);	// angle of each slice
	float angle = 0.0;							// angle used to generate x and y coordinates
	float x = 0.0;
	float y = 0.0;
	float z = 0.0;	// (x, y, z) coordinates

	// generate vertex coordinates for a circle
	for (int i = 0; i < slices; i++)
	{
		x = radius * cos(angle); //*gAspectRatio;
		y = radius * sin(angle);

		//position
		gVertices.push_back(x);
		gVertices.push_back(y);
		gVertices.push_back(z);

		// update to next angle
		angle += slice_angle;
	}
}


// function initialise scene and render settings
static void init(GLFWwindow* window)
{
	// set the color the color buffer should be cleared to
	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

	glEnable(GL_DEPTH_TEST);	// enable depth buffer test

	// compile and link a vertex and fragment shader pair
	gShader.compileAndLink("lighting.vert", "phong_point_source.frag");

	// compile and link a vertex and fragment shader pair for 2d circles
	shader2D.compileAndLink("2D_circle.vert", "2D_circle.frag");

	// initialise view matrix
	//where you are, what you're looking at, up
	gViewMatrix = glm::lookAt(glm::vec3(0.0f, 1.0f, 4.5f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	// initialise projection matrix
	//fov (effective zoom), aspect ratio, z near, z far
	gProjectionMatrix = glm::perspective(glm::radians(45.0f), 
		1.0f, 0.1f, 10.0f);
	//static_cast<float>(gWindowHeight) / gWindowWidth

	// initialise point light properties
	//position, ambient, diffuse, specular
	gLight.pos = glm::vec3(10.0f, 10.0f, 10.0f); //looking down from top left
	gLight.La = glm::vec3(0.8f);
	gLight.Ld = glm::vec3(0.8f);
	gLight.Ls = glm::vec3(0.8f);
	gLight.att = glm::vec3(1.0f, 0.0f, 0.0f);

	// initialise material properties
	gMaterials["PEARL"].Ka = glm::vec3(0.25f, 0.25f, 0.21f);
	gMaterials["PEARL"].Kd = glm::vec3(1.0f, 0.83f, 0.83f);
	gMaterials["PEARL"].Ks = glm::vec3(0.3f, 0.3f, 0.3f);
	gMaterials["PEARL"].shininess = 11.3f;

	gMaterials["JADE"].Ka = glm::vec3(0.14f, 0.22f, 0.16f);
	gMaterials["JADE"].Kd = glm::vec3(0.54f, 0.89f, 0.63f);
	gMaterials["JADE"].Ks = glm::vec3(0.32f, 0.32f, 0.32f);
	gMaterials["JADE"].shininess = 12.8f;

	// initialise model matrices
	gModelMatrix["NUCLEUS"] = glm::mat4(1.0f);
	gModelMatrix["ELECTRON_1"] = glm::mat4(1.0f);
	gModelMatrix["ELECTRON_2"] = glm::mat4(1.0f);
	gModelMatrix["ELECTRON_3"] = glm::mat4(1.0f);

	//create electron orbit paths
	gModelMatrix["PATH_1"] = glm::mat4(1.0f);
	
	gModelMatrix["PATH_1"] *= glm::rotate(glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	gModelMatrix["PATH_2"] = glm::rotate(glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	gModelMatrix["PATH_2"] *= glm::rotate(glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	/*gModelMatrix["PATH_2"] = gModelMatrix["PATH_1"] * glm::rotate(glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f));*/
	gModelMatrix["PATH_3"] = glm::rotate(glm::radians(-45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	gModelMatrix["PATH_3"] *= glm::rotate(glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

	// load model
	gModels["NUCLEUS"].loadModel("./models/sphere.obj");
	gModels["ELECTRON_1"].loadModel("./models/sphere.obj");
	gModels["ELECTRON_2"].loadModel("./models/sphere.obj");
	gModels["ELECTRON_3"].loadModel("./models/sphere.obj");

	//create orbit paths and load into 2d buffer
	gVertices.clear();
	generate_circle(gVertices, 1.5f);
	generate_circle(gVertices, 1.5f);
	generate_circle(gVertices, 1.5f);

	// create VBO and buffer the data
	glGenBuffers(1, &gVBO);					// generate unused VBO identifier
	glBindBuffer(GL_ARRAY_BUFFER, gVBO);	// bind the VBO
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * gVertices.size(), &gVertices[0], GL_DYNAMIC_DRAW);

	// create VAO, specify VBO data and format of the data
	glGenVertexArrays(1, &gVAO);			// generate unused VAO identifier
	glBindVertexArray(gVAO);				// create VAO
	glBindBuffer(GL_ARRAY_BUFFER, gVBO);	// bind the VBO
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);	// specify format of the data

	glEnableVertexAttribArray(0);	// enable vertex attributes
}

// function used to update the scene
static void update_scene(GLFWwindow* window)
{
	//orbit angle incremented by tweak bar 
	//* frame time for consisten speed
	orbitAngle += orbitIncrementor * gFrameTime;

	//make electrons orbit based on orbit angle
	gModelMatrix["ELECTRON_1"] = glm::translate(glm::vec3(1.5f, 0.0f, 0.0f))
		* glm::rotate(glm::radians(orbitAngle), glm::vec3(0.0f, 1.0f, 0.0f));
		//* glm::translate(glm::vec3(1.5f, 0.0f, 0.0f));

	gModelMatrix["ELECTRON_2"] = glm::rotate(glm::radians(-orbitAngle), glm::vec3(1.0f, 1.0f, 0.0f))
		* glm::translate(glm::vec3(0.0f, 0.0f, -1.5f));

	gModelMatrix["ELECTRON_3"] = glm::rotate(glm::radians(orbitAngle), glm::vec3(-1.0f, 1.0f, 0.0f))
		* glm::translate(glm::vec3(0.0f, 0.0f, 1.5f));

}

// function to render the scene
static void render_scene()
{
	// clear colour buffer and depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	shader2D.use();						// use the 2D shader for orbit paths

	glBindVertexArray(gVAO);			// make VAO active

	glViewport(400, 150, 600, 600);		//set viewport

	//create electron orbit paths using primitives
	glm:: mat4 MVP = gProjectionMatrix * gViewMatrix * gModelMatrix["PATH_1"];
	shader2D.setUniform("uModelViewProjectionMatrix", MVP);
	glDrawArrays(GL_LINE_LOOP, 0, 35);	// render the vertices based on primitive type 

	MVP = gProjectionMatrix * gViewMatrix * gModelMatrix["PATH_2"];
	shader2D.setUniform("uModelViewProjectionMatrix", MVP);
	glDrawArrays(GL_LINE_LOOP, 35, 35);	// render the vertices based on primitive type

	MVP = gProjectionMatrix * gViewMatrix * gModelMatrix["PATH_3"];
	shader2D.setUniform("uModelViewProjectionMatrix", MVP);
	glDrawArrays(GL_LINE_LOOP, 70, 35);	// render the vertices based on primitive type 


	// use 3D with lighting and colour for 3D models
	gShader.use();	
	// set light properties
	gShader.setUniform("uLight.pos", gLight.pos);
	gShader.setUniform("uLight.La", gLight.La);
	gShader.setUniform("uLight.Ld", gLight.Ld);
	gShader.setUniform("uLight.Ls", gLight.Ls);
	gShader.setUniform("uLight.att", gLight.att);


	// set material properties
	//scenes with multiple objects their material properties will have to be set for each before being rendered
	gShader.setUniform("uMaterial.Ka", gMaterials["PEARL"].Ka);
	gShader.setUniform("uMaterial.Kd", gMaterials["PEARL"].Kd);
	gShader.setUniform("uMaterial.Ks", gMaterials["PEARL"].Ks);
	gShader.setUniform("uMaterial.shininess", gMaterials["PEARL"].shininess);

	// set viewing position (pass where camera is to shader)
	gShader.setUniform("uViewpoint", glm::vec3(0.0f, 2.0f, 4.0f));


	// calculate matrices, pass MVP to shader
	gModelMatrix["NUCLEUS"] = glm::scale(glm::vec3(0.7f));
	MVP = gProjectionMatrix * gViewMatrix * gModelMatrix["NUCLEUS"];
	glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(gModelMatrix["NUCLEUS"])));

	// set uniform variables
	gShader.setUniform("uModelViewProjectionMatrix", MVP);
	gShader.setUniform("uModelMatrix", gModelMatrix["NUCLEUS"]);
	gShader.setUniform("uNormalMatrix", normalMatrix);

	// render model
	gModels["NUCLEUS"].drawModel();


	//render another object
	gShader.setUniform("uMaterial.Ka", gMaterials["JADE"].Ka);
	gShader.setUniform("uMaterial.Kd", gMaterials["JADE"].Kd);
	gShader.setUniform("uMaterial.Ks", gMaterials["JADE"].Ks);
	gShader.setUniform("uMaterial.shininess", gMaterials["JADE"].shininess);

	// calculate matrices
	gModelMatrix["ELECTRON_1"] *= glm::scale(glm::vec3(0.15f));
	MVP = gProjectionMatrix * gViewMatrix * gModelMatrix["ELECTRON_1"];
	normalMatrix = glm::mat3(glm::transpose(glm::inverse(gModelMatrix["ELECTRON_1"])));

	// set uniform variables
	gShader.setUniform("uModelViewProjectionMatrix", MVP);
	gShader.setUniform("uModelMatrix", gModelMatrix["ELECTRON_1"]);
	gShader.setUniform("uNormalMatrix", normalMatrix);


	gModels["ELECTRON_1"].drawModel();


	//render another object
	// calculate matrices
	gModelMatrix["ELECTRON_2"] *= glm::scale(glm::vec3(0.15f));

	MVP = gProjectionMatrix * gViewMatrix * gModelMatrix["ELECTRON_2"];
	normalMatrix = glm::mat3(glm::transpose(glm::inverse(gModelMatrix["ELECTRON_2"])));

	// set uniform variables
	gShader.setUniform("uModelViewProjectionMatrix", MVP);
	gShader.setUniform("uModelMatrix", gModelMatrix["ELECTRON_2"]);
	gShader.setUniform("uNormalMatrix", normalMatrix);

	// render model
	gModels["ELECTRON_2"].drawModel();
	

	//render another object
	// calculate matrices
	gModelMatrix["ELECTRON_3"] *= glm::scale(glm::vec3(0.15f));

	MVP = gProjectionMatrix * gViewMatrix * gModelMatrix["ELECTRON_3"];
	normalMatrix = glm::mat3(glm::transpose(glm::inverse(gModelMatrix["ELECTRON_3"])));

	// set uniform variables
	gShader.setUniform("uModelViewProjectionMatrix", MVP);
	gShader.setUniform("uModelMatrix", gModelMatrix["ELECTRON_3"]);
	gShader.setUniform("uNormalMatrix", normalMatrix);

	// render model
	gModels["ELECTRON_3"].drawModel();

	// flush the graphics pipeline
	glFlush();
}

// key press or release callback function
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// close the window when the ESCAPE key is pressed
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		// set flag to close the window
		glfwSetWindowShouldClose(window, GL_TRUE);
		return;
	}
}

// mouse movement callback function
static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	// pass cursor position to tweak bar
	TwEventMousePosGLFW(static_cast<int>(xpos), static_cast<int>(ypos));
}

// mouse button callback function
static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	// pass mouse button status to tweak bar
	TwEventMouseButtonGLFW(button, action);
}

// error callback function
static void error_callback(int error, const char* description)
{
	std::cerr << description << std::endl;	// output error description
}

// create and populate tweak bar elements
TwBar* create_UI(const std::string name)
{
	// create a tweak bar
	TwBar* twBar = TwNewBar(name.c_str());

	// give tweak bar the size of graphics window
	TwWindowSize(gWindowWidth, gWindowHeight);
	TwDefine(" TW_HELP visible=false ");	// disable help menu
	TwDefine(" GLOBAL fontsize=3 ");		// set large font size

	TwDefine(" Main label='User Interface' refresh=0.02 text=light size='220 450'");

	// scene controls
	TwAddVarRW(twBar, "Wireframe", TW_TYPE_BOOLCPP, &gWireframe, " group='Controls' ");

	// create frame stat entries
	TwAddVarRO(twBar, "Frame Rate", TW_TYPE_FLOAT, &gFrameRate, " group='Frame Stats' precision=2 ");
	TwAddVarRO(twBar, "Frame Time", TW_TYPE_FLOAT, &gFrameTime, " group='Frame Stats' ");

	// light controls
	TwAddVarRW(twBar, "Position X", TW_TYPE_FLOAT, &gLight.pos.x, " group='Light' min=-10.0 max=10.0 step=1.0 ");
	TwAddVarRW(twBar, "Position Y", TW_TYPE_FLOAT, &gLight.pos.y, " group='Light' min=-10.0 max=10.0 step=1.0 ");
	TwAddVarRW(twBar, "Position Z", TW_TYPE_FLOAT, &gLight.pos.z, " group='Light' min=-10.0 max=10.0 step=1.0 ");

	//nucleus controls
	TwAddVarRW(twBar, "Shininess", TW_TYPE_FLOAT, &gMaterials["PEARL"].shininess, " group='Nucleus' min=1.0 max=50.0 step=1.0 ");
	TwAddVarRW(twBar, "Specular Colour", TW_TYPE_COLOR3F, &gMaterials["PEARL"].Ks, " group='Nucleus' opened=true ");

	//electron controls
	TwAddVarRW(twBar, "Orbit Speed", TW_TYPE_FLOAT, &orbitIncrementor, " group='Electron' min=0.0 max=300.0 step=10.0 ");
	TwAddVarRW(twBar, "Diffuse Colour", TW_TYPE_COLOR3F, &gMaterials["JADE"].Kd, " group='Electron' opened=true ");

	return twBar;
}

int main(void)
{
	GLFWwindow* window = nullptr;	// GLFW window handle

	glfwSetErrorCallback(error_callback);	// set GLFW error callback function

	// initialise GLFW
	if (!glfwInit())
	{
		// if failed to initialise GLFW
		exit(EXIT_FAILURE);
	}

	// minimum OpenGL version 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// create a window and its OpenGL context
	window = glfwCreateWindow(gWindowWidth, gWindowHeight, "Assignment 2", nullptr, nullptr);

	// check if window created successfully
	if (window == nullptr)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window);	// set window context as the current context
	glfwSwapInterval(1);			// swap buffer interval based on the gFrame rate

	// initialise GLEW
	if (glewInit() != GLEW_OK)
	{
		// if failed to initialise GLEW
		std::cerr << "GLEW initialisation failed" << std::endl;
		exit(EXIT_FAILURE);
	}

	// set GLFW callback functions
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);

	// initialise scene and render settings
	init(window);

	// initialise AntTweakBar
	TwInit(TW_OPENGL_CORE, nullptr);
	TwBar* tweakBar = create_UI("Main");		// create and populate tweak bar elements

	// timing data
	double lastUpdateTime = glfwGetTime();	// last update time
	double elapsedTime = lastUpdateTime;	// time since last update
	int frameCount = 0;						// number of frames since last update

	// the rendering loop
	while (!glfwWindowShouldClose(window))
	{
		update_scene(window);	// update the scene

		// if wireframe set polygon render mode to wireframe
		if (gWireframe)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		render_scene();			// render the scene

		// set polygon render mode to fill
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		TwDraw();				// draw tweak bar

		glfwSwapBuffers(window);	// swap buffers
		glfwPollEvents();			// poll for events

		frameCount++;
		elapsedTime = glfwGetTime() - lastUpdateTime;	// time since last update

		// if elapsed time since last update > 1 second
		if (elapsedTime > 1.0)
		{
			gFrameTime = elapsedTime / frameCount;	// average time per frame
			gFrameRate = 1 / gFrameTime;			// frames per second
			lastUpdateTime = glfwGetTime();			// set last update time to current time
			frameCount = 0;							// reset frame counter
		}
	}

	// uninitialise tweak bar
	TwDeleteBar(tweakBar);
	TwTerminate();

	// close the window and terminate GLFW
	glfwDestroyWindow(window);
	glfwTerminate();

	exit(EXIT_SUCCESS);
}