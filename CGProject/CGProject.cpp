#include <GL\glew.h>

#include <SDL.h>
#include <SDL_opengl.h>
#include <stdio.h>
#include <gl\GLU.h>

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

#include <iostream>

#include "Shader.h"
#include "Camera.h"
#include "Model.h"

#include "GeometryNode.h"
#include "GroupNode.h"
#include "TransformNode.h"
#include "Skybox.h"



bool init();
bool initGL();
void render();
void close();
bool loadDepthcubemap(GLuint& depthID, GLuint& FBO);
bool KelvintoRGB(glm::vec3& lightdiff, float temp);
void CreateScene();

//The window we'll be rendering to
SDL_Window* gWindow = NULL;

//OpenGL context
SDL_GLContext gContext;

Shader gShader, gSkyBoxShader, gDeapthShader;

GroupNode* gRoot;

SkyBox* skybox;

//shadows
GLuint depthMapFBO1, depthMapFBO2, texIDDeapth1, texIDDeapth2;
vector<GLuint> texIDDeapth;
vector<GLuint> depthMapFBO;
const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

//lightSwitch
bool shadow1 = false;
bool shadow2 = false;


//lamp color
float kelvin1 = 2000.0f;
float kelvin2 = 12000.0f;

//skybox
vector<std::string> faces1
{
	"./textures/skybox/right.jpg",
	"./textures/skybox/left.jpg",
	"./textures/skybox/top.jpg",
	"./textures/skybox/bottom.jpg",
	"./textures/skybox/front.jpg",
	"./textures/skybox/back.jpg",
};
vector<std::string> faces2
{
	"./textures/clouds1/posx.jpg",
	"./textures/clouds1/negx.jpg",
	"./textures/clouds1/posy.jpg",
	"./textures/clouds1/negy.jpg",
	"./textures/clouds1/posz.jpg",
	"./textures/clouds1/negz.jpg",
};
vector<std::string> faces3
{
	"./textures/night/xpos.png",
	"./textures/night/xneg.png",
	"./textures/night/ypos.png",
	"./textures/night/yneg.png",
	"./textures/night/zpos.png",
	"./textures/night/zneg.png",
};

// camera
Camera camera(glm::vec3(0.0f, 2.5f, 3.0f));
float lastX = -1;
float lastY = -1;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// lighting
vector<glm::vec3> pointLightPositions;
vector<glm::vec3> lightdiff;
float ambientLight = 0.9f;

//statics
unsigned int Node::genID;
glm::mat4 TransformNode::transformMatrix = glm::mat4(1.0f);

TransformNode* selectedTransform;


//event handlers
void HandleKeyDown(const SDL_KeyboardEvent& key);
void HandleMouseMotion(const SDL_MouseMotionEvent& motion);
void HandleMouseWheel(const SDL_MouseWheelEvent& wheel);
void HandleMouseButtonUp(const SDL_MouseButtonEvent& button);


int main(int argc, char* args[])
{
	init();

	CreateScene();

	SDL_Event e;
	//While application is running
	bool quit = false;
	while (!quit)
	{
		// per-frame time logic
		// --------------------
		float currentFrame = SDL_GetTicks() / 1000.0f;
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		//Handle events on queue
		while (SDL_PollEvent(&e) != 0)
		{
			//User requests quit
			if (e.type == SDL_QUIT)
			{
				quit = true;
			}
			switch (e.type)
			{
			case SDL_QUIT:
				quit = true;
				break;
			case SDL_KEYDOWN:
				if (e.key.keysym.sym == SDLK_ESCAPE)
				{
					quit = true;
				}
				else
				{
					HandleKeyDown(e.key);
				}
				break;
			case SDL_MOUSEMOTION:
				HandleMouseMotion(e.motion);
				break;
			case SDL_MOUSEWHEEL:
				HandleMouseWheel(e.wheel);
				break;
			case SDL_MOUSEBUTTONUP:
				HandleMouseButtonUp(e.button);
				break;
			}
		}

		//Render
		render();

		//Update screen
		SDL_GL_SwapWindow(gWindow);
	}

	close();

	return 0;
}

void HandleKeyDown(const SDL_KeyboardEvent& key)
{


	switch (key.keysym.sym)
	{
	case SDLK_w:
		camera.ProcessKeyboard(FORWARD, deltaTime, gRoot);
		break;
	case SDLK_s:
		camera.ProcessKeyboard(BACKWARD, deltaTime, gRoot);
		break;
	case SDLK_a:
		camera.ProcessKeyboard(LEFT, deltaTime, gRoot);
		break;
	case SDLK_d:
		camera.ProcessKeyboard(RIGHT, deltaTime, gRoot);
		break;
	case SDLK_RIGHT://for the small lamp
		if(kelvin1 < 3000.0f)
		{
			kelvin1 += 200.0f;
			KelvintoRGB(lightdiff[0], kelvin1);
		}
		break;
	case SDLK_LEFT://for the small lamp
		if (kelvin1 > 1000.0f)
		{
			kelvin1 -= 200.0f;
			KelvintoRGB(lightdiff[0], kelvin1);
		}
		break;
	case SDLK_UP://increase kelvins for the big lamp
		if (kelvin2 < 20000.0f)
		{
			kelvin2 += 200.0f;
			KelvintoRGB(lightdiff[1], kelvin2);
		}
		break;
	case SDLK_DOWN://decrease kelvins the big lamp
		if (kelvin2 > 1000.0f)
		{
			kelvin2 -= 200.0f;
			KelvintoRGB(lightdiff[1], kelvin2);
		}
		break;
	case SDLK_1://puts on and off the small lamp
		shadow1 = shadow1 ? false : true;
		break;
	case SDLK_2://puts on and off the big lamp
		shadow2 = shadow2 ? false : true;
		break;
	case SDLK_r://daytime
		skybox->ReLoadTextures(faces1);
		ambientLight = 0.9f;
		break;
	case SDLK_t://cloudy Piazza del popolo, Rome, Italy.
		skybox->ReLoadTextures(faces2);
		ambientLight = 0.5f;
		break;
	case SDLK_y://night
		skybox->ReLoadTextures(faces3);
		ambientLight = 0.1f;
		break;
	}
}

void HandleMouseMotion(const SDL_MouseMotionEvent& motion)
{
	if (firstMouse)
	{
		lastX = motion.x;
		lastY = motion.y;
		firstMouse = false;
	}
	else
	{
		camera.ProcessMouseMovement(motion.x - lastX, lastY - motion.y);
		lastX = motion.x;
		lastY = motion.y;
	}
}

void HandleMouseWheel(const SDL_MouseWheelEvent& wheel)
{
	camera.ProcessMouseScroll(wheel.y);
}

void HandleMouseButtonUp(const SDL_MouseButtonEvent& button)
{
	
}



bool init()
{
	//Initialization flag
	bool success = true;

	//Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
		success = false;
	}
	else
	{
		//Use OpenGL 3.3
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);


		//Create window
		gWindow = SDL_CreateWindow("Small Room", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1200, 900,
			SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN /*| SDL_WINDOW_FULLSCREEN*/);
		if (gWindow == NULL)
		{
			printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
			success = false;
		}
		else
		{
			//Create context
			gContext = SDL_GL_CreateContext(gWindow);
			if (gContext == NULL)
			{
				printf("OpenGL context could not be created! SDL Error: %s\n", SDL_GetError());
				success = false;
			}
			else
			{
				//Use Vsync
				if (SDL_GL_SetSwapInterval(1) < 0)
				{
					printf("Warning: Unable to set VSync! SDL Error: %s\n", SDL_GetError());
				}

				//Initialize OpenGL
				if (!initGL())
				{
					printf("Unable to initialize OpenGL!\n");
					success = false;
				}
			}
		}
	}

	return success;
}

bool initGL()
{
	bool success = true;
	GLenum error = GL_NO_ERROR;

	glewInit();

	error = glGetError();
	if (error != GL_NO_ERROR)
	{
		success = false;
		printf("Error initializing OpenGL! %s\n", gluErrorString(error));
	}

	glClearColor(0.0f, 0.5f, 0.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	gShader.Load("./shaders/vertex.vert", "./shaders/fragment.frag");
	gSkyBoxShader.Load("./shaders/skybox.vert", "./shaders/skybox.frag");
	gDeapthShader.Load("./shaders/shadowdepth.vert", "./shaders/shadowdepth.frag", "./shaders/shadowdepth.geo");

	pointLightPositions.push_back(glm::vec3(2.0f, 2.3f, -1.2f));
	pointLightPositions.push_back(glm::vec3(-1.0f, 5.0f, 2.0f));


	skybox = new SkyBox();
	skybox->SetShader(&gSkyBoxShader);
	skybox->LoadTextures(faces1);

	loadDepthcubemap(texIDDeapth1, depthMapFBO1);
	loadDepthcubemap(texIDDeapth2, depthMapFBO2);


	depthMapFBO.push_back(depthMapFBO1);
	depthMapFBO.push_back(depthMapFBO2);

	texIDDeapth.push_back(texIDDeapth1);
	texIDDeapth.push_back(texIDDeapth2);

	glUseProgram(gShader.ID);

	gShader.setInt("depthMap[0]", 3);
	gShader.setInt("depthMap[1]", 4);

	//setup lightning color
	glm::vec3 light1 = glm::vec3(0.0f);
	glm::vec3 light2 = glm::vec3(0.0f);

	KelvintoRGB(light1, kelvin1);
	KelvintoRGB(light2, kelvin2);

	lightdiff.push_back(light1);
	lightdiff.push_back(light2);


	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); //other modes GL_FILL, GL_POINT

	return success;
}

void CreateScene()
{
	gRoot = new GroupNode("root");

	TransformNode* tr = new TransformNode("wallTransform1");
	tr->SetTranslation(glm::vec3(-0.6f, 0.0f, -3.0f));
	tr->SetRotation(glm::vec3(0.0f, 90.0f, 0.0f));
	tr->SetScale(glm::vec3(0.3f, 0.45f, 0.3f));

	TransformNode* tr2 = new TransformNode("wallTransform2");
	tr2->SetTranslation(glm::vec3(4.6f, 0.0f, 1.9f));
	tr2->SetScale(glm::vec3(0.31f, 0.45f, 0.3f));


	TransformNode* tr3 = new TransformNode("wallTransform3");
	tr3->SetTranslation(glm::vec3(-5.0f, 0.0f, 1.9f));
	tr3->SetScale(glm::vec3(0.3f, 0.45f, 0.3f));

	TransformNode* tr4 = new TransformNode("wallTransform4");
	tr4->SetTranslation(glm::vec3(0.0f, 0.0f, 7.2f));
	tr4->SetRotation(glm::vec3(0.0f, -90.0f, 0.0f));
	tr4->SetScale(glm::vec3(0.32f, 0.45f, 0.3f));

	TransformNode* fl = new TransformNode("floorTransform");
	fl->SetTranslation(glm::vec3(0.0f, 0.0f, 3.0f));
	//fl->SetScale(glm::vec3(0.5f, 0.5f, 0.5f));

	TransformNode* ro = new TransformNode("roofTransform");
	ro->SetTranslation(glm::vec3(0.0f, 5.0f, 3.0f));

	TransformNode* la = new TransformNode("lamp");
	la->SetTranslation(glm::vec3(2.0f, 2.0f, -1.2f));
	la->SetScale(glm::vec3(0.02f, 0.02f, 0.02f));
	la->SetRotation(glm::vec3(-90.0f, 0.0f, 0.0f));

	TransformNode* la2 = new TransformNode("lamp2");
	la2->SetTranslation(glm::vec3(-1.0f, 5.5f, 2.0f));
	la2->SetScale(glm::vec3(0.013f, 0.013f, 0.013f));
	la2->SetRotation(glm::vec3(-90.0f, 0.0f, 0.0f));

	TransformNode* stand = new TransformNode("forlamp1");
	stand->SetTranslation(glm::vec3(2.0f, 0.0f, -0.8f));
	stand->SetScale(glm::vec3(0.6f, 0.6f, 0.6f));

	TransformNode* chair = new TransformNode("chair");
	chair->SetTranslation(glm::vec3(-1.5f, 0.0f, -1.2f));
	chair->SetScale(glm::vec3(2.5f, 2.5f, 2.5f));

	TransformNode* t = new TransformNode("tv");
	t->SetTranslation(glm::vec3(-1.0f, 0.0f, 5.0f));
	t->SetScale(glm::vec3(0.03f, 0.03f, 0.03f));
	t->SetRotation(glm::vec3(-90.0f, 0.0f, 0.0f));

	TransformNode* w = new TransformNode("window1");
	w->SetTranslation(glm::vec3(4.4f, 2.75f, 0.0f));
	w->SetRotation(glm::vec3(0.0f, 180.0f, 0.0f));
	w->SetScale(glm::vec3(0.7f, 0.67f, 0.8f));

	TransformNode* w2 = new TransformNode("window1");
	w2->SetTranslation(glm::vec3(1.92f, 2.75f, 6.97f));
	w2->SetRotation(glm::vec3(0.0f, 90.0f, 0.0f));
	w2->SetScale(glm::vec3(0.7f, 0.67f, 0.8f));

	GeometryNode* wall1 = new GeometryNode("normal_wall");

	GeometryNode* wall2 = new GeometryNode("wall_with_a_window");

	GeometryNode* floor = new GeometryNode("floor and roof");

	GeometryNode* lamp1 = new GeometryNode("NightstandLamp");

	GeometryNode* lamp2 = new GeometryNode("MaindLamp");

	GeometryNode* Nightst = new GeometryNode("forLamp1");

	GeometryNode* comfychair = new GeometryNode("chair");

	GeometryNode* tv = new GeometryNode("tv");

	GeometryNode* window = new GeometryNode("windowm");

	wall1->LoadFromFile("models/wall1/wall_1.obj");
	wall1->SetShader(&gShader);
	wall1->SetShadowShader(&gDeapthShader);

	wall2->LoadFromFile("models/wall2/wall_2.obj");
	wall2->SetShader(&gShader);
	wall2->SetShadowShader(&gDeapthShader);

	floor->LoadFromFile("models/floor/floor.obj");
	floor->SetShader(&gShader);
	floor->SetShadowShader(&gDeapthShader);

	lamp1->LoadFromFile("models/lamp/Bertfrank_Masina_Table_Lamp.obj");
	lamp1->SetShader(&gShader);
	lamp1->SetShadowShader(&gDeapthShader);

	lamp2->LoadFromFile("models/lamp2/Astep_Model_2065_mat(1).obj");
	lamp2->SetShader(&gShader);
	lamp2->SetShadowShader(&gDeapthShader);

	Nightst->LoadFromFile("models/Obj_format/Free model Drawer(Final) .obj");
	Nightst->SetShader(&gShader);
	Nightst->SetShadowShader(&gDeapthShader);

	comfychair->LoadFromFile("models/chair/uploads_files_4048722_Chair_wooden.obj");
	comfychair->SetShader(&gShader);
	comfychair->SetShadowShader(&gDeapthShader);

	tv->LoadFromFile("models/tv/Samsung_Serif_TV_Medium_32_mat(1).obj");
	tv->SetShader(&gShader);
	tv->SetShadowShader(&gDeapthShader);

	window->LoadFromFile("models/window/window.obj");
	window->SetShader(&gShader);
	window->SetShadowShader(&gDeapthShader);

	tr->AddChild(wall1);
	gRoot->AddChild(tr);

	tr2->AddChild(wall2);
	gRoot->AddChild(tr2);

	tr3->AddChild(wall1);
	gRoot->AddChild(tr3);

	tr4->AddChild(wall2);
	gRoot->AddChild(tr4);

	fl->AddChild(floor);
	gRoot->AddChild(fl);

	ro->AddChild(floor);
	gRoot->AddChild(ro);

	la->AddChild(lamp1);
	gRoot->AddChild(la);

	la2->AddChild(lamp2);
	gRoot->AddChild(la2);

	stand->AddChild(Nightst);
	gRoot->AddChild(stand);

	chair->AddChild(comfychair);
	gRoot->AddChild(chair);

	t->AddChild(tv);
	gRoot->AddChild(t);

	w->AddChild(window);
	gRoot->AddChild(w);

	w2->AddChild(window);
	gRoot->AddChild(w2);
}

void close()
{
	//delete GL programs, buffers and objects
	glDeleteProgram(gShader.ID);
	glDeleteProgram(gSkyBoxShader.ID);
	glDeleteProgram(gDeapthShader.ID);
	glDeleteFramebuffers(1, &depthMapFBO1);
	glDeleteFramebuffers(1, &depthMapFBO2);



	//Delete OGL context
	SDL_GL_DeleteContext(gContext);
	//Destroy window	
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;

	//Quit SDL subsystems
	SDL_Quit();
}


void render()
{

	float near_plane = 1.0f;
	float far_plane = 100.0f;
	//Clear color buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	

	for (int i = 0;i < pointLightPositions.size();i++)
	 {
		glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT, near_plane, far_plane);
		std::vector<glm::mat4> shadowTransforms;
		shadowTransforms.push_back(shadowProj * glm::lookAt(pointLightPositions[i], pointLightPositions[i] + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
		shadowTransforms.push_back(shadowProj * glm::lookAt(pointLightPositions[i], pointLightPositions[i] + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
		shadowTransforms.push_back(shadowProj * glm::lookAt(pointLightPositions[i], pointLightPositions[i] + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
		shadowTransforms.push_back(shadowProj * glm::lookAt(pointLightPositions[i], pointLightPositions[i] + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
		shadowTransforms.push_back(shadowProj * glm::lookAt(pointLightPositions[i], pointLightPositions[i] + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
		shadowTransforms.push_back(shadowProj * glm::lookAt(pointLightPositions[i], pointLightPositions[i] + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));

		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO[i]);
		glClear(GL_DEPTH_BUFFER_BIT);
		glUseProgram(gDeapthShader.ID);
		for (unsigned int i = 0; i < 6; ++i)
			gDeapthShader.setMat4("shadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);

		gDeapthShader.setFloat("far_plane", far_plane);
		gDeapthShader.setVec3("lightPos", pointLightPositions[i]);

		gRoot->TraverseShadows();
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glViewport(0, 0, 1200, 900);

	//Clear color buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	glm::mat4 view = camera.GetViewMatrix();
	glm::mat4 proj = glm::perspective(glm::radians(camera.Zoom), 4.0f / 3.0f, 0.1f, 100.0f);

	glUseProgram(gShader.ID);
	gShader.setMat4("view", view);
	gShader.setMat4("proj", proj);
	gShader.setVec3("viewPos", camera.Position);

	//lighting
	gShader.setVec3("lamp[0].position", pointLightPositions[0]);
	gShader.setVec3("lamp[0].ambient", 0.2f, 0.2f, 0.2f);
	gShader.setVec3("lamp[0].diffuse", lightdiff[0]);
	gShader.setFloat("lamp[0].constant", 1.0f);
	gShader.setFloat("lamp[0].linear", 0.14f);
	gShader.setFloat("lamp[0].quadratic", 0.07f);
	

	gShader.setVec3("lamp[1].position", pointLightPositions[1]);
	gShader.setVec3("lamp[1].ambient", 0.2f, 0.2f, 0.2f);
	gShader.setVec3("lamp[1].diffuse", lightdiff[1]);
	gShader.setFloat("lamp[1].constant", 1.0f);
	gShader.setFloat("lamp[1].linear", 0.07f);
	gShader.setFloat("lamp[1].quadratic", 0.017f);

	gShader.setFloat("ambientlight", ambientLight);

	gShader.setFloat("far_plane", far_plane);
	gShader.setBool("shadowenable[0]", shadow1);
	gShader.setBool("shadowenable[1]", shadow2);


	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texIDDeapth[0]);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texIDDeapth[1]);

	gRoot->Traverse();

	glUseProgram(gSkyBoxShader.ID);

	view = glm::mat4(glm::mat3(camera.GetViewMatrix()));

	gSkyBoxShader.setMat4("view", view);
	gSkyBoxShader.setMat4("projection", proj);

	skybox->Draw();

}

//creates a deptbuffer and a cubemap texture
bool loadDepthcubemap(GLuint& texID, GLuint& FBO)
{

	glGenFramebuffers(1, &FBO);

	glGenTextures(1, &texID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texID);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	for (unsigned int i = 0; i < 6; ++i)
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texID, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return true;
}

bool KelvintoRGB(glm::vec3& lightdiff, float temp) {
	float temperature = temp / 100.0f;
	float red, green, blue;

	if (temperature < 66.0f)
	{
		red = 255.0;
		green = 99.4708025861f * log(temperature) - 161.1195681661f;
		green = green < 0.0f ? 0.0f : green;
		green = green > 255.0f ? 255.0f : green;
		if (temperature <= 19.0f)
		{
			blue = 0.0f;
		}
		else
		{
			blue = 138.5177312231f * log(temperature - 10.0f) - 305.0447927307f;
			blue = blue < 0.0f ? 0.0f : blue;
			blue = blue > 255.0f ? 255.0f : blue;
		}
	}
	else if (temperature == 66.0f)
	{
		red = 255.0f;
		green = 99.4708025861f * log(temperature) - 161.1195681661f;
		green = green < 0.0f ? 0.0f : green;
		green = green > 255.0f ? 255.0f : green;
		blue = 255.0f;
	}
	else if (temperature > 66.0f)
	{
		red = 329.698727446f * pow(temperature - 60.0f, -0.1332047592f);
		red = red < 0.0f ? 0.0f : red;
		red = red > 255.0f ? 255.0f : red;
		green = 288.1221695283f * pow(temperature - 60.0f, -0.0755148492f);
		green = green < 0.0f ? 0.0f : green;
		green = green > 255.0f ? 255.0f : green;
		blue = 255.0f;
	}

	lightdiff.x = red / 255.0f;
	lightdiff.y = green / 255.0f;
	lightdiff.z = blue / 255.0f;

	return true;

}
