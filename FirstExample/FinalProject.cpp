
//***************************************************************************
// GAME2012_A5_LastFirst.cpp by Zihan Xu 101288760 Fnu Yiliqi 101289355(C) 2020All Rights Reserved.
//
// Final Project submission.
// 
// Description:
// Click run to see the results.
// Press WASDRF to control camera.
//
//*****************************************************************************

using namespace std;

#include <cstdlib>
#include <ctime>
#include "vgl.h"
#include "LoadShaders.h"
#include "Light.h"
#include "Shape.h"
#include "glm\glm.hpp"
#include "glm\gtc\matrix_transform.hpp"
#include <iostream>
#include<vector>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define FPS 60
#define MOVESPEED 0.1f
#define TURNSPEED 0.05f
#define X_AXIS glm::vec3(1,0,0)
#define Y_AXIS glm::vec3(0,1,0)
#define Z_AXIS glm::vec3(0,0,1)
#define XY_AXIS glm::vec3(0.5,1,0)
#define YZ_AXIS glm::vec3(0,1,1)
#define XZ_AXIS glm::vec3(1,0,1)


enum keyMasks {
	KEY_FORWARD = 0b00000001,		// 0x01 or 1 or 01
	KEY_BACKWARD = 0b00000010,		// 0x02 or 2 or 02
	KEY_LEFT = 0b00000100,
	KEY_RIGHT = 0b00001000,
	KEY_UP = 0b00010000,
	KEY_DOWN = 0b00100000,
	KEY_MOUSECLICKED = 0b01000000
	// Any other keys you want to add.
};

bool hasKey, isKeyOnPos,hasEntered,hasExited;
glm::vec3 crystal_move = glm::vec3(0, 0, 0);
float CMove = 0.02f;
bool CMoveDir = true;

glm::vec3 gatePos = glm::vec3(28.0f, 5.0f, 0.5f);
glm::vec3 hoverPos = glm::vec3(23.0, 3.0f, -23.0f);
glm::vec3 crystalPos = glm::vec3(11.0f, 3.0f, -11.0f);
glm::vec3 exitPos = glm::vec3(51.0, 5.0, -32.0);

// IDs.
GLuint vao, ibo, points_vbo, colors_vbo, uv_vbo, normals_vbo, modelID, viewID, projID;
GLuint program;

// Matrices.
glm::mat4 View, Projection;

// Our bitflags. 1 byte for up to 8 keys.
unsigned char keys = 0; // Initialized to 0 or 0b00000000.

// Camera and transform variables.
float scale = 1.0f, angle = 0.0f;
glm::vec3 position, frontVec, worldUp, upVec, rightVec; // Set by function
GLfloat pitch, yaw;
int lastX, lastY;

// Texture variables.
GLuint dirtTx, stoneTx, blankTx, woodTx, grassTx,wallTx,glassTx;
GLint width, height, bitDepth;

// Light variables.
AmbientLight aLight(glm::vec3(1.0f, 1.0f, 1.0f),	// Ambient colour.
	0.1f);							// Ambient strength.

DirectionalLight dLight(glm::vec3(-1.0f, 0.0f, -0.5f), // Direction.
	glm::vec3(1.0f, 1.0f, 0.25f),  // Diffuse colour.
	0.1f);						  // Diffuse strength.

PointLight pLights[4] = { { glm::vec3(2.0f, 15.0f, -2.0f), 5.0f, glm::vec3(1.0f, 0.0f, 0.0f), 1000.0f },
						  { glm::vec3(52.0f, 15.0f, -2.0f), 2.5f, glm::vec3(0.0f, 1.0f, 0.0f), 1000.0f },
						  { glm::vec3(2.0f, 15.0f, -53.0f), 2.5f, glm::vec3(1.0f, 0.0f, 1.0f), 1000.0f },
						  { glm::vec3(52.0f, 15.0f, -53.0f), 2.5f, glm::vec3(0.0f, 0.0f, 1.0f), 1000.0f } };

SpotLight sLight(glm::vec3(51.0f, 15.0f, -32.0f),	// Position.
	glm::vec3(0.0f, 1.0f, 0.0f),	// Diffuse colour.
	1000.0f,							// Diffuse strength.
	glm::vec3(0.0f, -1.0f, 0.0f),  // Direction.
	10.0f);



Material mat = { 0.1f, 32 }; // Alternate way to construct an object.

void timer(int);

float disBetPoints(glm::vec3 point1,glm::vec3 point2)
{
	float DisX = point1.x - point2.x;
	float DisY = point1.y - point2.y;
	float DisZ = point1.z - point2.z;
	return sqrt(DisX * DisX + DisY * DisY + DisZ * DisZ);
}

void stateTest()
{
	if(!hasEntered)
	{
		if(disBetPoints(position,gatePos)<=5.0)
		{
			hasEntered = true;
			std::cout << "Welcome to my castle!" << std::endl;
		}
	}
	else if(!hasKey)
	{
		if(disBetPoints(position,crystalPos)<=3.0)
		{
			hasKey = true;
			std::cout << "get the key" << std::endl;
		}
	}
	else if(!isKeyOnPos)
	{
		if(disBetPoints(position,hoverPos)<=3.0)
		{
			isKeyOnPos = true;
			std::cout << "key is on position" << std::endl;
		}
	}
	else if(!hasExited)
	{
		if (disBetPoints(position, exitPos) <= 3.0)
		{
			hasExited = true;
			std::cout << "exit the maze" << std::endl;
			exit(0);
		}
		
	}
}

void resetView()
{
	position = glm::vec3(25.0f, 10.0f, 20.0f);
	frontVec = glm::vec3(0.0f, 0.0f, -1.0f);
	worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
	pitch = 0.0f;
	yaw = -90.0f;
	// View will now get set only in transformObject
}

// Shapes. Recommend putting in a map
Plane g_ground;
Cube g_gate;
Prism g_hover(3);
std::vector<Cone*> g_crystal;
std::vector<Cube*> g_wall;
std::vector<Cube*> g_gateWall;
std::vector<Cube*> g_merlon;
std::vector<Cube*> g_gateTowerMerlon;
std::vector<Cube*> g_step;
std::vector<Cube*> g_grass;
std::vector<Cube*> g_roomWall;
std::vector<Cube*> g_roomDoor;
std::vector<Cube*> g_crenel;
std::vector <Prism*> g_towerBase;
std::vector<Prism*> g_gateTowerBase;
std::vector<Cone*> g_towerTop;
std::vector<Cone*> g_gateTowerTop;
std::vector<Trapezoid*> g_banister;

//component nubers
const int WallNumber = 5;
const int GateWallNumber = 3;
const int MerlonNumber = 38;
const int GateMerlonNumber = 4;
const int StepNumber = 5;
const int TowerNumber = 4;
const int GateTowerNumber = 2;
const int BanisterNumber = 2;
const int GrassNumber = 20;
const int RoomWallNumber = 5;
const int RoomDoorNumber = 2;
const int CrenelNumber = 26;

void init(void)
{
	srand((unsigned)time(NULL));
	//Specifying the name of vertex and fragment shaders.
	ShaderInfo shaders[] = {
		{ GL_VERTEX_SHADER, "triangles2.vert" },
		{ GL_FRAGMENT_SHADER, "triangles2.frag" },
		{ GL_NONE, NULL }
	};

	//Loading and compiling shaders
	program = LoadShaders(shaders);
	glUseProgram(program);	//My Pipeline is set up

	modelID = glGetUniformLocation(program, "model");
	projID = glGetUniformLocation(program, "projection");
	viewID = glGetUniformLocation(program, "view");

	// Projection matrix : 45∞ Field of View, aspect ratio, display range : 0.1 unit <-> 100 units
	Projection = glm::perspective(glm::radians(45.0f), 1.0f / 1.0f, 0.1f, 100.0f);
	// Or, for an ortho camera :
	// Projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 100.0f); // In world coordinates

	// Camera matrix
	resetView();

	// Image loading.
	stbi_set_flip_vertically_on_load(true);

	unsigned char* image = stbi_load("textures/dirt.png", &width, &height, &bitDepth, 0);
	if (!image) cout << "Unable to load dirt file!" << endl;

	glGenTextures(1, &dirtTx);
	glBindTexture(GL_TEXTURE_2D, dirtTx);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	// Note: image types with native transparency will need to be GL_RGBA instead of GL_RGB.
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);
	//glBindTexture(GL_TEXTURE_2D, 0);
	stbi_image_free(image);

	// Second texture. 
	unsigned char* image2 = stbi_load("textures/grass.png", &width, &height, &bitDepth, 0);
	if (!image) cout << "Unable to load grass file!" << endl;

	glGenTextures(1, &grassTx);
	glBindTexture(GL_TEXTURE_2D, grassTx);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image2);
	// Note: image types with native transparency will need to be GL_RGBA instead of GL_RGB.
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);
	//glBindTexture(GL_TEXTURE_2D, 0);
	stbi_image_free(image2);

	// Third texture. Blank one.
	unsigned char* image3 = stbi_load("textures/blank.jpg", &width, &height, &bitDepth, 0);
	if (!image3) cout << "Unable to load blank file!" << endl;

	glGenTextures(1, &blankTx);
	glBindTexture(GL_TEXTURE_2D, blankTx);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image3);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);
	//glBindTexture(GL_TEXTURE_2D, 0);
	stbi_image_free(image3);

	// Fourth texture. Wood one.
	unsigned char* image4 = stbi_load("textures/woodgate.png", &width, &height, &bitDepth, 0);
	if (!image4) cout << "Unable to load wood file!" << endl;

	glGenTextures(1, &woodTx);
	glBindTexture(GL_TEXTURE_2D, woodTx);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image4);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);
	//glBindTexture(GL_TEXTURE_2D, 0);
	stbi_image_free(image4);

	//Fifth texture. Glass one.
	unsigned char* image5 = stbi_load("textures/stone.png", &width, &height, &bitDepth, 0);
	if (!image5) cout << "Unable to load stone file !" << endl;

	glGenTextures(1, &stoneTx);
	glBindTexture(GL_TEXTURE_2D, stoneTx);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image5);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);
	//glBindTexture(GL_TEXTURE_2D, 0);
	stbi_image_free(image5);

	//sixth texture. Wall Texture
	unsigned char* image6 = stbi_load("textures/whitewall.png", &width, &height, &bitDepth, 0);
	if (!image6) cout << "Unable to load wall file !" << endl;

	glGenTextures(1, &wallTx);
	glBindTexture(GL_TEXTURE_2D, wallTx);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image6);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);
	//glBindTexture(GL_TEXTURE_2D, 0);
	stbi_image_free(image6);

	//seventh texture. Glass Texture
	unsigned char* image7 = stbi_load("textures/glass.png", &width, &height, &bitDepth, 0);
	if (!image7) cout << "Unable to load glass file !" << endl;

	glGenTextures(1, &glassTx);
	glBindTexture(GL_TEXTURE_2D, glassTx);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image7);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);
	//glBindTexture(GL_TEXTURE_2D, 0);
	stbi_image_free(image7);

	glUniform1i(glGetUniformLocation(program, "texture0"), 0);

	// Setting ambient Light.
	glUniform3f(glGetUniformLocation(program, "aLight.ambientColour"), aLight.ambientColour.x, aLight.ambientColour.y, aLight.ambientColour.z);
	glUniform1f(glGetUniformLocation(program, "aLight.ambientStrength"), aLight.ambientStrength);

	// Setting directional light.
	glUniform3f(glGetUniformLocation(program, "dLight.base.diffuseColour"), dLight.diffuseColour.x, dLight.diffuseColour.y, dLight.diffuseColour.z);
	glUniform1f(glGetUniformLocation(program, "dLight.base.diffuseStrength"), dLight.diffuseStrength);

	glUniform3f(glGetUniformLocation(program, "dLight.direction"), dLight.direction.x, dLight.direction.y, dLight.direction.z);

	// Setting point lights.
	glUniform3f(glGetUniformLocation(program, "pLights[0].base.diffuseColour"), pLights[0].diffuseColour.x, pLights[0].diffuseColour.y, pLights[0].diffuseColour.z);
	glUniform1f(glGetUniformLocation(program, "pLights[0].base.diffuseStrength"), pLights[0].diffuseStrength);
	glUniform3f(glGetUniformLocation(program, "pLights[0].position"), pLights[0].position.x, pLights[0].position.y, pLights[0].position.z);
	glUniform1f(glGetUniformLocation(program, "pLights[0].constant"), pLights[0].constant);
	glUniform1f(glGetUniformLocation(program, "pLights[0].linear"), pLights[0].linear);
	glUniform1f(glGetUniformLocation(program, "pLights[0].exponent"), pLights[0].exponent);

	glUniform3f(glGetUniformLocation(program, "pLights[1].base.diffuseColour"), pLights[1].diffuseColour.x, pLights[1].diffuseColour.y, pLights[1].diffuseColour.z);
	glUniform1f(glGetUniformLocation(program, "pLights[1].base.diffuseStrength"), pLights[1].diffuseStrength);
	glUniform3f(glGetUniformLocation(program, "pLights[1].position"), pLights[1].position.x, pLights[1].position.y, pLights[1].position.z);
	glUniform1f(glGetUniformLocation(program, "pLights[1].constant"), pLights[1].constant);
	glUniform1f(glGetUniformLocation(program, "pLights[1].linear"), pLights[1].linear);
	glUniform1f(glGetUniformLocation(program, "pLights[1].exponent"), pLights[1].exponent);

	glUniform3f(glGetUniformLocation(program, "pLights[2].base.diffuseColour"), pLights[2].diffuseColour.x, pLights[2].diffuseColour.y, pLights[2].diffuseColour.z);
	glUniform1f(glGetUniformLocation(program, "pLights[2].base.diffuseStrength"), pLights[2].diffuseStrength);
	glUniform3f(glGetUniformLocation(program, "pLights[2].position"), pLights[2].position.x, pLights[2].position.y, pLights[2].position.z);
	glUniform1f(glGetUniformLocation(program, "pLights[2].constant"), pLights[2].constant);
	glUniform1f(glGetUniformLocation(program, "pLights[2].linear"), pLights[2].linear);
	glUniform1f(glGetUniformLocation(program, "pLights[2].exponent"), pLights[2].exponent);

	glUniform3f(glGetUniformLocation(program, "pLights[3].base.diffuseColour"), pLights[3].diffuseColour.x, pLights[3].diffuseColour.y, pLights[3].diffuseColour.z);
	glUniform1f(glGetUniformLocation(program, "pLights[3].base.diffuseStrength"), pLights[3].diffuseStrength);
	glUniform3f(glGetUniformLocation(program, "pLights[3].position"), pLights[3].position.x, pLights[3].position.y, pLights[3].position.z);
	glUniform1f(glGetUniformLocation(program, "pLights[3].constant"), pLights[3].constant);
	glUniform1f(glGetUniformLocation(program, "pLights[3].linear"), pLights[3].linear);
	glUniform1f(glGetUniformLocation(program, "pLights[3].exponent"), pLights[3].exponent);

	// Setting spot light.
	glUniform3f(glGetUniformLocation(program, "sLight.base.diffuseColour"), sLight.diffuseColour.x, sLight.diffuseColour.y, sLight.diffuseColour.z);
	glUniform1f(glGetUniformLocation(program, "sLight.base.diffuseStrength"), sLight.diffuseStrength);

	glUniform3f(glGetUniformLocation(program, "sLight.position"), sLight.position.x, sLight.position.y, sLight.position.z);

	glUniform3f(glGetUniformLocation(program, "sLight.direction"), sLight.direction.x, sLight.direction.y, sLight.direction.z);
	glUniform1f(glGetUniformLocation(program, "sLight.edge"), sLight.edgeRad);

	vao = 0;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	ibo = 0;
	glGenBuffers(1, &ibo);

	points_vbo = 0;
	glGenBuffers(1, &points_vbo);

	colors_vbo = 0;
	glGenBuffers(1, &colors_vbo);

	uv_vbo = 0;
	glGenBuffers(1, &uv_vbo);

	normals_vbo = 0;
	glGenBuffers(1, &normals_vbo);

	glBindVertexArray(0); // Can optionally unbind the vertex array to avoid modification.

	//Init components
	for(int i=0;i<WallNumber;i++)
	{
		g_wall.push_back(new Cube);
		g_wall[i]->SetMat(0.1, 16);
	}
	for (int i = 0; i < MerlonNumber; i++)
	{
		g_merlon.push_back(new Cube);
		g_merlon[i]->SetMat(0.1, 16);
	}
	for (int i = 0; i < GateWallNumber; i++)
	{
		g_gateWall.push_back(new Cube);
		g_gateWall[i]->SetMat(0.1, 16);
	}
	for (int i = 0; i < GateMerlonNumber; i++)
	{
		g_gateTowerMerlon.push_back(new Cube);
		g_gateTowerMerlon[i]->SetMat(0.1, 16);
	}
	for (int i = 0; i < StepNumber; i++)
	{
		g_step.push_back(new Cube);
		g_step[i]->SetMat(0.1, 16);
	}
	for(int i=0;i<TowerNumber;i++)
	{
		g_towerBase.push_back(new Prism(12));
		g_towerBase[i]->SetMat(0.1, 16);
		g_towerTop.push_back(new Cone(12));
		g_towerTop[i]->SetMat(0.1, 16);
	}
	for(int i=0;i<GateTowerNumber;i++)
	{
		g_gateTowerBase.push_back(new Prism(12));
		g_gateTowerBase[i]->SetMat(0.1, 16);
		g_gateTowerTop.push_back(new Cone(12));
		g_gateTowerTop[i]->SetMat(0.1, 16);
	}
	for(int i=0;i<BanisterNumber;i++)
	{
		g_banister.push_back(new Trapezoid);
		g_banister[i]->SetMat(0.1, 16);
	}
	for (int i = 0; i < GrassNumber; i++)
	{
		g_grass.push_back(new Cube);
		g_grass[i]->SetMat(0.1, 16);
	}
	for (int i = 0; i < RoomWallNumber; i++)
	{
		g_roomWall.push_back(new Cube);
		g_roomWall[i]->SetMat(0.1, 16);
	}
	for (int i = 0; i < RoomDoorNumber; i++)
	{
		g_roomDoor.push_back(new Cube);
		g_roomDoor[i]->SetMat(0.1, 16);
	}
	for (int i = 0; i < CrenelNumber; i++)
	{
		g_crenel.push_back(new Cube);
		g_crenel[i]->SetMat(0.1, 16);
	}
	g_ground.SetMat(0.1, 16);
	
	g_hover.SetMat(0.1, 16);
	g_crystal.push_back(new Cone(4));
	g_crystal[0]->SetMat(0.1, 16);
	g_crystal.push_back(new Cone(4));
	g_crystal[1]->SetMat(0.1, 16);

	hasKey = false;
	isKeyOnPos = false;
	hasEntered = false;
	hasExited = false;

	// Enable depth test and blend.
	glEnable(GL_DEPTH_TEST);
	//glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Enable smoothing.
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_POLYGON_SMOOTH);
	// Enable face culling.
	//glEnable(GL_CULL_FACE);
	//glFrontFace(GL_CCW);
	//glCullFace(GL_BACK);

	timer(0);
}

//---------------------------------------------------------------------
//
// calculateVieww
//
void calculateView()
{
	frontVec.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	frontVec.y = sin(glm::radians(pitch));
	frontVec.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	frontVec = glm::normalize(frontVec);
	rightVec = glm::normalize(glm::cross(frontVec, worldUp));
	upVec = glm::normalize(glm::cross(rightVec, frontVec));

	View = glm::lookAt(
		position, // Camera position
		position + frontVec, // Look target
		upVec); // Up vector
	glUniform3f(glGetUniformLocation(program, "eyePosition"), position.x, position.y, position.z);
}

//---------------------------------------------------------------------
//
// transformModel
//
void transformObject(glm::vec3 scale, glm::vec3 rotationAxis, float rotationAngle, glm::vec3 translation) {
	glm::mat4 Model;
	Model = glm::mat4(1.0f);
	Model = glm::translate(Model, translation);
	Model = glm::rotate(Model, glm::radians(rotationAngle), rotationAxis);
	Model = glm::scale(Model, scale);

	calculateView();
	glUniformMatrix4fv(modelID, 1, GL_FALSE, &Model[0][0]);
	glUniformMatrix4fv(viewID, 1, GL_FALSE, &View[0][0]);
	glUniformMatrix4fv(projID, 1, GL_FALSE, &Projection[0][0]);
}

//---------------------------------------------------------------------
//
// display
//
void display(void)
{
	glClearColor(0.0f, 0.0f, 1.0f, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBindVertexArray(vao);
	// Draw all shapes.

	//glEnable(GL_DEPTH_TEST);

	//Ground
	glBindTexture(GL_TEXTURE_2D, dirtTx);
	g_ground.BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
	transformObject(glm::vec3(55.0f, 55.0f, 1.0f), X_AXIS, -90.0f, glm::vec3(0.0f, 0.0f, 0.0f));
	glDrawElements(GL_TRIANGLES, g_ground.NumIndices(), GL_UNSIGNED_SHORT, 0);

	//Castle Wall
	glBindTexture(GL_TEXTURE_2D, wallTx);
	(*g_wall[0]).BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
	transformObject(glm::vec3(55.0f, 11.0f, 1.0f), Z_AXIS, 0.0f, glm::vec3(0.0f, 0.0f, -55.0f));
	glDrawElements(GL_TRIANGLES, (*g_wall[0]).NumIndices(), GL_UNSIGNED_SHORT, 0);

	glBindTexture(GL_TEXTURE_2D, wallTx);
	(*g_wall[1]).BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
	transformObject(glm::vec3(1.0f, 11.0f, 55.0f), Y_AXIS, 0.0f, glm::vec3(0.0f, 0.0f, -55.0f));
	glDrawElements(GL_TRIANGLES, (*g_wall[1]).NumIndices(), GL_UNSIGNED_SHORT, 0);

	glBindTexture(GL_TEXTURE_2D, wallTx);
	(*g_wall[2]).BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
	transformObject(glm::vec3(1.0f, 11.0f, 55.0f), Y_AXIS, 0.0f, glm::vec3(55.0f, 0.0f, -55.0f));
	glDrawElements(GL_TRIANGLES, (*g_wall[2]).NumIndices(), GL_UNSIGNED_SHORT, 0);

	//Tower Base
	glBindTexture(GL_TEXTURE_2D, wallTx);
	(*g_towerBase[0]).BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
	transformObject(glm::vec3(4.0f, 13.0f, 4.0f), Y_AXIS, 0.0f, glm::vec3(-1.5f, 0.0f, -2.0f));
	glDrawElements(GL_TRIANGLES, (*g_towerBase[0]).NumIndices(), GL_UNSIGNED_SHORT, 0);

	glBindTexture(GL_TEXTURE_2D, wallTx);
	(*g_towerBase[0]).BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
	transformObject(glm::vec3(4.0f, 13.0f, 4.0f), Y_AXIS, 0.0f, glm::vec3(-1.5f, 0.0f, -56.0f));
	glDrawElements(GL_TRIANGLES, (*g_towerBase[0]).NumIndices(), GL_UNSIGNED_SHORT, 0);

	glBindTexture(GL_TEXTURE_2D, wallTx);
	(*g_towerBase[0]).BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
	transformObject(glm::vec3(4.0f, 13.0f, 4.0f), Y_AXIS, 0.0f, glm::vec3(53.0f, 0.0f, -56.0f));
	glDrawElements(GL_TRIANGLES, (*g_towerBase[0]).NumIndices(), GL_UNSIGNED_SHORT, 0);

	glBindTexture(GL_TEXTURE_2D, wallTx);
	(*g_towerBase[0]).BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
	transformObject(glm::vec3(4.0f, 13.0f, 4.0f), Y_AXIS, 0.0f, glm::vec3(53.0f, 0.0f, -2.0f));
	glDrawElements(GL_TRIANGLES, (*g_towerBase[0]).NumIndices(), GL_UNSIGNED_SHORT, 0);

	//TowerTop
	glBindTexture(GL_TEXTURE_2D, wallTx);
	(*g_towerTop[0]).BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
	transformObject(glm::vec3(4.0f, 4.0f, 4.0f), Y_AXIS, 0.0f, glm::vec3(-1.5f, 13.0f, -2.0f));
	glDrawElements(GL_TRIANGLES, (*g_towerTop[0]).NumIndices(), GL_UNSIGNED_SHORT, 0);

	glBindTexture(GL_TEXTURE_2D, wallTx);
	(*g_towerTop[0]).BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
	transformObject(glm::vec3(4.0f, 4.0f, 4.0f), Y_AXIS, 0.0f, glm::vec3(-1.5f, 13.0f, -56.0f));
	glDrawElements(GL_TRIANGLES, (*g_towerTop[0]).NumIndices(), GL_UNSIGNED_SHORT, 0);
	
	glBindTexture(GL_TEXTURE_2D, wallTx);
	(*g_towerTop[0]).BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
	transformObject(glm::vec3(4.0f, 4.0f, 4.0f), Y_AXIS, 0.0f, glm::vec3(53.0f, 13.0f, -56.0f));
	glDrawElements(GL_TRIANGLES, (*g_towerTop[0]).NumIndices(), GL_UNSIGNED_SHORT, 0);

	glBindTexture(GL_TEXTURE_2D, wallTx);
	(*g_towerTop[0]).BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
	transformObject(glm::vec3(4.0f, 4.0f, 4.0f), Y_AXIS, 0.0f, glm::vec3(53.0f, 13.0f, -2.0f));
	glDrawElements(GL_TRIANGLES, (*g_towerTop[0]).NumIndices(), GL_UNSIGNED_SHORT, 0);

	//gate wall
	glBindTexture(GL_TEXTURE_2D, wallTx);
	(*g_wall[3]).BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
	transformObject(glm::vec3(11.0f, 11.0f, 1.0f), Y_AXIS, 0.0f, glm::vec3(0.0f, 0.0f, 0.0f));
	glDrawElements(GL_TRIANGLES, (*g_wall[3]).NumIndices(), GL_UNSIGNED_SHORT, 0);

	glBindTexture(GL_TEXTURE_2D, wallTx);
	(*g_wall[4]).BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
	transformObject(glm::vec3(11.0f, 11.0f, 1.0f), Y_AXIS, 0.0f, glm::vec3(45.0f, 0.0f, 0.0f));
	glDrawElements(GL_TRIANGLES, (*g_wall[4]).NumIndices(), GL_UNSIGNED_SHORT, 0);

	//Gate Tower Base
	glBindTexture(GL_TEXTURE_2D, wallTx);
	(*g_gateTowerBase[0]).BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
	transformObject(glm::vec3(4.0f, 13.0f, 4.0f), Y_AXIS, 0.0f, glm::vec3(10.0f, 0.0f, -1.5f));
	glDrawElements(GL_TRIANGLES, (*g_gateTowerBase[0]).NumIndices(), GL_UNSIGNED_SHORT, 0);

	glBindTexture(GL_TEXTURE_2D, wallTx);
	(*g_gateTowerBase[1]).BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
	transformObject(glm::vec3(4.0f, 13.0f, 4.0f), Y_AXIS, 0.0f, glm::vec3(43.0f, 0.0f, -1.5f));
	glDrawElements(GL_TRIANGLES, (*g_gateTowerBase[1]).NumIndices(), GL_UNSIGNED_SHORT, 0);

	//Gate Tower Top
	glBindTexture(GL_TEXTURE_2D, wallTx);
	(*g_gateTowerTop[0]).BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
	transformObject(glm::vec3(4.0f, 4.0f, 4.0f), Y_AXIS, 0.0f, glm::vec3(10.0f, 13.0f, -1.5f));
	glDrawElements(GL_TRIANGLES, (*g_gateTowerTop[0]).NumIndices(), GL_UNSIGNED_SHORT, 0);

	glBindTexture(GL_TEXTURE_2D, wallTx);
	(*g_gateTowerTop[1]).BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
	transformObject(glm::vec3(4.0f, 4.0f, 4.0f), Y_AXIS, 0.0f, glm::vec3(43.0f, 13.0f, -1.5f));
	glDrawElements(GL_TRIANGLES, (*g_gateTowerTop[1]).NumIndices(), GL_UNSIGNED_SHORT, 0);

	//Gate Cube
	glBindTexture(GL_TEXTURE_2D, wallTx);
	(*g_gateWall[0]).BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
	transformObject(glm::vec3(8.0f, 11.0f, 1.0f), Z_AXIS, 0.0f, glm::vec3(13.0f, 0.0f, 0.0f));
	glDrawElements(GL_TRIANGLES, (*g_gateWall[0]).NumIndices(), GL_UNSIGNED_SHORT, 0);

	glBindTexture(GL_TEXTURE_2D, wallTx);
	(*g_gateWall[1]).BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
	transformObject(glm::vec3(8.0f, 11.0f, 1.0f), Y_AXIS, 0.0f, glm::vec3(35.0f, 0.0f, 0.0f));
	glDrawElements(GL_TRIANGLES, (*g_gateWall[1]).NumIndices(), GL_UNSIGNED_SHORT, 0);

	glBindTexture(GL_TEXTURE_2D, wallTx);
	(*g_gateWall[2]).BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
	transformObject(glm::vec3(14.0f, 1.0f, 1.0f), Y_AXIS, 0.0f, glm::vec3(21.0f, 10.0f, 0.0f));
	glDrawElements(GL_TRIANGLES, (*g_gateWall[2]).NumIndices(), GL_UNSIGNED_SHORT, 0);

	//Gate
	glBindTexture(GL_TEXTURE_2D, woodTx);
	g_gate.BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
	transformObject(glm::vec3(14.0f, 10.0f, 1.0f), Y_AXIS, 0.0f, glm::vec3(21.0f, 0.0f, 0.0f));
	glDrawElements(GL_TRIANGLES, g_gate.NumIndices(), GL_UNSIGNED_SHORT, 0);

	//Step
	glBindTexture(GL_TEXTURE_2D, stoneTx);
	(*g_step[0]).BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
	transformObject(glm::vec3(14.0f, 0.5f, 2.0f), Z_AXIS, 0.0f, glm::vec3(21.0f, 2.0f, 1.0f));
	glDrawElements(GL_TRIANGLES, (*g_step[0]).NumIndices(), GL_UNSIGNED_SHORT, 0);

	glBindTexture(GL_TEXTURE_2D, stoneTx);
	(*g_step[1]).BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
	transformObject(glm::vec3(14.0f, 0.5f, 2.0f), Y_AXIS, 0.0f, glm::vec3(21.0f, 1.5f, 2.0f));
	glDrawElements(GL_TRIANGLES, (*g_step[1]).NumIndices(), GL_UNSIGNED_SHORT, 0);

	glBindTexture(GL_TEXTURE_2D, stoneTx);
	(*g_step[2]).BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
	transformObject(glm::vec3(14.0f, 0.5f, 2.0f), Y_AXIS, 0.0f, glm::vec3(21.0f, 1.0f, 3.0f));
	glDrawElements(GL_TRIANGLES, (*g_step[2]).NumIndices(), GL_UNSIGNED_SHORT, 0);

	glBindTexture(GL_TEXTURE_2D, stoneTx);
	(*g_step[3]).BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
	transformObject(glm::vec3(14.0f, 0.5f, 2.0f), Y_AXIS, 0.0f, glm::vec3(21.0f, 0.5f, 4.0f));
	glDrawElements(GL_TRIANGLES, (*g_step[3]).NumIndices(), GL_UNSIGNED_SHORT, 0);

	glBindTexture(GL_TEXTURE_2D, stoneTx);
	(*g_step[4]).BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
	transformObject(glm::vec3(14.0f, 0.5f, 2.0f), Y_AXIS, 0.0f, glm::vec3(21.0f, 0.0f, 5.0f));
	glDrawElements(GL_TRIANGLES, (*g_step[4]).NumIndices(), GL_UNSIGNED_SHORT, 0);

	//Banister
	glBindTexture(GL_TEXTURE_2D, stoneTx);
	(*g_banister[0]).BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
	transformObject(glm::vec3(1.0f, 2.0f, 6.0f), Z_AXIS, 0.0f, glm::vec3(20.0f, 0.0f, 1.0f));
	glDrawElements(GL_TRIANGLES, (*g_banister[0]).NumIndices(), GL_UNSIGNED_SHORT, 0);

	glBindTexture(GL_TEXTURE_2D, stoneTx);
	(*g_banister[1]).BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
	transformObject(glm::vec3(1.0f, 2.0f, 6.0f), Y_AXIS, 0.0f, glm::vec3(35.0f, 0.0f, 1.0f));
	glDrawElements(GL_TRIANGLES, (*g_banister[1]).NumIndices(), GL_UNSIGNED_SHORT, 0);

	//Gate Wall Merlon
	glBindTexture(GL_TEXTURE_2D, stoneTx);
	(*g_merlon[0]).BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
	transformObject(glm::vec3(3.0f, 1.0f, 0.25f), Z_AXIS, 0.0f, glm::vec3(2.0f, 11.0f, 0.0f));
	glDrawElements(GL_TRIANGLES, (*g_merlon[0]).NumIndices(), GL_UNSIGNED_SHORT, 0);

	glBindTexture(GL_TEXTURE_2D, stoneTx);
	(*g_merlon[1]).BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
	transformObject(glm::vec3(3.0f, 1.0f, 0.25f), Y_AXIS, 0.0f, glm::vec3(2.0f, 11.0f, 0.75f));
	glDrawElements(GL_TRIANGLES, (*g_merlon[1]).NumIndices(), GL_UNSIGNED_SHORT, 0);

	glBindTexture(GL_TEXTURE_2D, stoneTx);
	(*g_merlon[2]).BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
	transformObject(glm::vec3(3.0f, 1.0f, 0.25f), Z_AXIS, 0.0f, glm::vec3(8.0f, 11.0f, 0.0f));
	glDrawElements(GL_TRIANGLES, (*g_merlon[0]).NumIndices(), GL_UNSIGNED_SHORT, 0);

	glBindTexture(GL_TEXTURE_2D, stoneTx);
	(*g_merlon[3]).BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
	transformObject(glm::vec3(3.0f, 1.0f, 0.25f), Y_AXIS, 0.0f, glm::vec3(8.0f, 11.0f, 0.75f));
	glDrawElements(GL_TRIANGLES, (*g_merlon[1]).NumIndices(), GL_UNSIGNED_SHORT, 0);

	glBindTexture(GL_TEXTURE_2D, stoneTx);
	(*g_merlon[4]).BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
	transformObject(glm::vec3(3.0f, 1.0f, 0.25f), Y_AXIS, 0.0f, glm::vec3(46.0f, 11.0f, 0.0f));
	glDrawElements(GL_TRIANGLES, (*g_merlon[4]).NumIndices(), GL_UNSIGNED_SHORT, 0);

	glBindTexture(GL_TEXTURE_2D, stoneTx);
	(*g_merlon[5]).BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
	transformObject(glm::vec3(3.0f, 1.0f, 0.25f), Y_AXIS, 0.0f, glm::vec3(46.0f, 11.0f, 0.75f));
	glDrawElements(GL_TRIANGLES, (*g_merlon[5]).NumIndices(), GL_UNSIGNED_SHORT, 0);

	glBindTexture(GL_TEXTURE_2D, stoneTx);
	(*g_merlon[6]).BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
	transformObject(glm::vec3(3.0f, 1.0f, 0.25f), Y_AXIS, 0.0f, glm::vec3(51.0f, 11.0f, 0.0f));
	glDrawElements(GL_TRIANGLES, (*g_merlon[6]).NumIndices(), GL_UNSIGNED_SHORT, 0);

	glBindTexture(GL_TEXTURE_2D, stoneTx);
	(*g_merlon[7]).BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
	transformObject(glm::vec3(3.0f, 1.0f, 0.25f), Y_AXIS, 0.0f, glm::vec3(51.0f, 11.0f, 0.75f));
	glDrawElements(GL_TRIANGLES, (*g_merlon[7]).NumIndices(), GL_UNSIGNED_SHORT, 0);

	//Wall Merlon
	for(int i=0;i<5;i++)
	{
		glBindTexture(GL_TEXTURE_2D, stoneTx);
		(*g_merlon[8 + i * 4]).BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
		transformObject(glm::vec3(0.25f, 1.0f, 5.0f), Y_AXIS, 0.0f, glm::vec3(0.0f, 11.0f, -10.0f - 10.0f * i));
		glDrawElements(GL_TRIANGLES, (*g_merlon[8 + i * 4]).NumIndices(), GL_UNSIGNED_SHORT, 0);

		glBindTexture(GL_TEXTURE_2D, stoneTx);
		(*g_merlon[9 + i * 4]).BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
		transformObject(glm::vec3(0.25f, 1.0f, 5.0f), Y_AXIS, 0.0f, glm::vec3(0.75f, 11.0f, -10.0f - 10.0f * i));
		glDrawElements(GL_TRIANGLES, (*g_merlon[9 + i * 4]).NumIndices(), GL_UNSIGNED_SHORT, 0);

		glBindTexture(GL_TEXTURE_2D, stoneTx);
		(*g_merlon[10 + i * 4]).BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
		transformObject(glm::vec3(0.25f, 1.0f, 5.0f), Y_AXIS, 0.0f, glm::vec3(55.0f, 11.0f, -10.0f - 10.0f * i));
		glDrawElements(GL_TRIANGLES, (*g_merlon[10 + i * 4]).NumIndices(), GL_UNSIGNED_SHORT, 0);

		glBindTexture(GL_TEXTURE_2D, stoneTx);
		(*g_merlon[11 + i * 4]).BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
		transformObject(glm::vec3(0.25f, 1.0f, 5.0f), Y_AXIS, 0.0f, glm::vec3(55.75f, 11.0f, -10.0f - 10.0f * i));
		glDrawElements(GL_TRIANGLES, (*g_merlon[11 + i * 4]).NumIndices(), GL_UNSIGNED_SHORT, 0);
	}

	for(int i=0;i<5;i++)
	{
		glBindTexture(GL_TEXTURE_2D, stoneTx);
		(*g_merlon[28 + i * 2]).BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
		transformObject(glm::vec3(5.0f, 1.0f, 0.25f), Y_AXIS, 0.0f, glm::vec3(7.0f + i * 10.0f, 11.0f, -55.0f));
		glDrawElements(GL_TRIANGLES, (*g_merlon[28 + i * 2]).NumIndices(), GL_UNSIGNED_SHORT, 0);

		glBindTexture(GL_TEXTURE_2D, stoneTx);
		(*g_merlon[29 + i * 2]).BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
		transformObject(glm::vec3(5.0f, 1.0f, 0.25f), Y_AXIS, 0.0f, glm::vec3(7.0f + i * 10.0f, 11.0f, -55.75f));
		glDrawElements(GL_TRIANGLES, (*g_merlon[29 + i * 2]).NumIndices(), GL_UNSIGNED_SHORT, 0);
	}

	//Grass
	glBindTexture(GL_TEXTURE_2D, grassTx);
	(g_grass[0])->BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
	transformObject(glm::vec3(45.0f, 2.5f, 7.5f), X_AXIS, -90.0f, glm::vec3(5.0f, 0.0f, -5.0f));
	glDrawElements(GL_TRIANGLES, g_grass[0]->NumIndices(), GL_UNSIGNED_SHORT, 0);

	glBindTexture(GL_TEXTURE_2D, grassTx);
	(g_grass[1])->BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
	transformObject(glm::vec3(2.5f, 25.0f, 7.5f), X_AXIS, -90.0f, glm::vec3(50.0f, 0.0f, -5.0f));
	glDrawElements(GL_TRIANGLES, g_grass[0]->NumIndices(), GL_UNSIGNED_SHORT, 0);

	glBindTexture(GL_TEXTURE_2D, grassTx);
	(g_grass[2])->BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
	transformObject(glm::vec3(2.5f, 12.5f, 7.5f), X_AXIS, -90.0f, glm::vec3(50.0f, 0.0f, -35.0f));
	glDrawElements(GL_TRIANGLES, g_grass[0]->NumIndices(), GL_UNSIGNED_SHORT, 0);

	glBindTexture(GL_TEXTURE_2D, grassTx);
	(g_grass[3])->BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
	transformObject(glm::vec3(10.0f, 2.5f, 7.5f), X_AXIS, -90.0f, glm::vec3(42.5f, 0.0f, -20));
	glDrawElements(GL_TRIANGLES, g_grass[0]->NumIndices(), GL_UNSIGNED_SHORT, 0);

	glBindTexture(GL_TEXTURE_2D, grassTx);
	(g_grass[4])->BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
	transformObject(glm::vec3(45.0f, 2.5f, 7.5f), X_AXIS, -90.0f, glm::vec3(5.0f, 0.0f, -45.0f));
	glDrawElements(GL_TRIANGLES, g_grass[0]->NumIndices(), GL_UNSIGNED_SHORT, 0);

	glBindTexture(GL_TEXTURE_2D, grassTx);
	(g_grass[5])->BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
	transformObject(glm::vec3(2.5f, 35.0f, 7.5f), X_AXIS, -90.0f, glm::vec3(5.0f, 0.0f, -10.0f));
	glDrawElements(GL_TRIANGLES, g_grass[0]->NumIndices(), GL_UNSIGNED_SHORT, 0);

	glBindTexture(GL_TEXTURE_2D, grassTx);
	(g_grass[6])->BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
	transformObject(glm::vec3(5.0f, 2.5f, 7.5f), X_AXIS, -90.0f, glm::vec3(7.5, 0.0f, -17.5));
	glDrawElements(GL_TRIANGLES, g_grass[0]->NumIndices(), GL_UNSIGNED_SHORT, 0);

	glBindTexture(GL_TEXTURE_2D, grassTx);
	(g_grass[6])->BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
	transformObject(glm::vec3(10.0f, 2.5f, 7.5f), X_AXIS, -90.0f, glm::vec3(7.5, 0.0f, -35.0f));
	glDrawElements(GL_TRIANGLES, g_grass[0]->NumIndices(), GL_UNSIGNED_SHORT, 0);

	glBindTexture(GL_TEXTURE_2D, grassTx);
	(g_grass[7])->BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
	transformObject(glm::vec3(2.5f, 10.0f, 7.5f), X_AXIS, -90.0f, glm::vec3(10.0f, 0.0f, -25.0f));
	glDrawElements(GL_TRIANGLES, g_grass[0]->NumIndices(), GL_UNSIGNED_SHORT, 0);

	glBindTexture(GL_TEXTURE_2D, grassTx);
	(g_grass[8])->BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
	transformObject(glm::vec3(25.0f, 2.5f, 7.5f), X_AXIS, -90.0f, glm::vec3(12.5f, 0.0f, -10.0f));
	glDrawElements(GL_TRIANGLES, g_grass[0]->NumIndices(), GL_UNSIGNED_SHORT, 0);

	glBindTexture(GL_TEXTURE_2D, grassTx);
	(g_grass[9])->BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
	transformObject(glm::vec3(2.5f, 10.0f, 7.5f), X_AXIS, -90.0f, glm::vec3(37.5f, 0.0f, -10.0f));
	glDrawElements(GL_TRIANGLES, g_grass[0]->NumIndices(), GL_UNSIGNED_SHORT, 0);

	glBindTexture(GL_TEXTURE_2D, grassTx);
	(g_grass[10])->BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
	transformObject(glm::vec3(2.5f, 12.5f, 7.5f), X_AXIS, -90.0f, glm::vec3(42.5f, 0.0f, -35.0f));
	glDrawElements(GL_TRIANGLES, g_grass[0]->NumIndices(), GL_UNSIGNED_SHORT, 0);

	glBindTexture(GL_TEXTURE_2D, grassTx);
	(g_grass[11])->BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
	transformObject(glm::vec3(15.0f, 2.5f, 7.5f), X_AXIS, -90.0f, glm::vec3(30.0f, 0.0f, -32.5f));
	glDrawElements(GL_TRIANGLES, g_grass[0]->NumIndices(), GL_UNSIGNED_SHORT, 0);

	glBindTexture(GL_TEXTURE_2D, grassTx);
	(g_grass[12])->BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
	transformObject(glm::vec3(2.5f, 5.0f, 7.5f), X_AXIS, -90.0f, glm::vec3(32.5f, 0.0f, -27.5f));
	glDrawElements(GL_TRIANGLES, g_grass[0]->NumIndices(), GL_UNSIGNED_SHORT, 0);

	glBindTexture(GL_TEXTURE_2D, grassTx);
	(g_grass[13])->BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
	transformObject(glm::vec3(2.5f, 5.0f, 7.5f), X_AXIS, -90.0f, glm::vec3(22.5f, 0.0f, -40.0f));
	glDrawElements(GL_TRIANGLES, g_grass[0]->NumIndices(), GL_UNSIGNED_SHORT, 0);

	glBindTexture(GL_TEXTURE_2D, stoneTx);//sidewall
	(g_grass[14])->BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
	transformObject(glm::vec3(1.0f, 10.0f, 7.5f), X_AXIS, -90.0f, glm::vec3(15.0f, 0.0f, -17.5f));
	glDrawElements(GL_TRIANGLES, g_grass[0]->NumIndices(), GL_UNSIGNED_SHORT, 0);

	glBindTexture(GL_TEXTURE_2D, stoneTx);//sidewall
	(g_grass[15])->BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
	transformObject(glm::vec3(1.0f, 10.0f, 7.5f), X_AXIS, -90.0f, glm::vec3(27.5f, 0.0f, -17.5f));
	glDrawElements(GL_TRIANGLES, g_grass[0]->NumIndices(), GL_UNSIGNED_SHORT, 0);

	glBindTexture(GL_TEXTURE_2D, stoneTx);//4
	(g_grass[16])->BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
	transformObject(glm::vec3(2.5f, 1.0f, 7.5f), X_AXIS, -90.0f, glm::vec3(25.0f, 0.0f, -17.5f));
	glDrawElements(GL_TRIANGLES, g_grass[0]->NumIndices(), GL_UNSIGNED_SHORT, 0);

	glBindTexture(GL_TEXTURE_2D, stoneTx);//2
	(g_grass[17])->BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
	transformObject(glm::vec3(2.5f, 1.0f, 7.5f), X_AXIS, -90.0f, glm::vec3(25.0f, 0.0f, -26.5f));
	glDrawElements(GL_TRIANGLES, g_grass[0]->NumIndices(), GL_UNSIGNED_SHORT, 0);

	glBindTexture(GL_TEXTURE_2D, stoneTx);//3
	(g_grass[18])->BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
	transformObject(glm::vec3(2.5f, 1.0f, 7.5f), X_AXIS, -90.0f, glm::vec3(16.0f, 0.0f, -17.5f));
	glDrawElements(GL_TRIANGLES, g_grass[0]->NumIndices(), GL_UNSIGNED_SHORT, 0);

	glBindTexture(GL_TEXTURE_2D, stoneTx);//1
	(g_grass[19])->BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
	transformObject(glm::vec3(2.5f, 1.0f, 7.5f), X_AXIS, -90.0f, glm::vec3(16.0f, 0.0f, -26.5f));
	glDrawElements(GL_TRIANGLES, g_grass[0]->NumIndices(), GL_UNSIGNED_SHORT, 0);

	//Room
	glBindTexture(GL_TEXTURE_2D, woodTx);
	(g_roomDoor[0])->BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
	transformObject(glm::vec3(6.5f, 1.0f, 7.5f), X_AXIS, -90.0f, glm::vec3(18.5f, 0.0f, -26.5f));
	glDrawElements(GL_TRIANGLES, g_roomDoor[0]->NumIndices(), GL_UNSIGNED_SHORT, 0);

	glBindTexture(GL_TEXTURE_2D, woodTx);
	(g_roomDoor[1])->BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
	transformObject(glm::vec3(6.5f, 1.0f, 7.5f), X_AXIS, -90.0f, glm::vec3(18.5f, 0.0f, -17.5f));
	glDrawElements(GL_TRIANGLES, g_roomDoor[0]->NumIndices(), GL_UNSIGNED_SHORT, 0);

	glBindTexture(GL_TEXTURE_2D, stoneTx);
	(g_roomWall[2])->BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
	transformObject(glm::vec3(12.5f, 10.0f, 0.5f), X_AXIS, -90.0f, glm::vec3(15.0f, 0.2f, -17.5f));
	glDrawElements(GL_TRIANGLES, g_roomWall[0]->NumIndices(), GL_UNSIGNED_SHORT, 0);

	glBindTexture(GL_TEXTURE_2D, stoneTx);
	g_hover.BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
	transformObject(glm::vec3(2.0f, 2.0f, 2.0f), X_AXIS, 0.0f, glm::vec3(22.0f, 2.0f, -22.0f));
	glDrawElements(GL_TRIANGLES, g_hover.NumIndices(), GL_UNSIGNED_SHORT, 0);

	if(!hasKey)
	{
		glBindTexture(GL_TEXTURE_2D, glassTx);
		g_crystal[0]->BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
		transformObject(glm::vec3(2.0f, 2.0f, 2.0f), Y_AXIS, 0.0f, glm::vec3(10.0f, 3.0f, -10.0f));
		glDrawElements(GL_TRIANGLES, g_crystal[0]->NumIndices(), GL_UNSIGNED_SHORT, 0);

		glBindTexture(GL_TEXTURE_2D, glassTx);
		g_crystal[1]->BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
		transformObject(glm::vec3(2.0f, 2.0f, 2.0f), Z_AXIS, 180.0f, glm::vec3(12.0f, 3.0f, -10.0f));
		glDrawElements(GL_TRIANGLES, g_crystal[1]->NumIndices(), GL_UNSIGNED_SHORT, 0);
	}
	else if(isKeyOnPos)
	{
		if (CMoveDir)
		{
			crystal_move.y += CMove;
		}
		else if (!CMoveDir)
		{
			crystal_move.y -= CMove;
		}

		if (crystal_move.y >= 0.4f)
		{
			CMoveDir = false;
		}
		else if (crystal_move.y <= 0.0f)
		{
			CMoveDir = true;
		}
		
		glBindTexture(GL_TEXTURE_2D, glassTx);
		g_crystal[0]->BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
		transformObject(glm::vec3(2.0f, 2.0f, 2.0f), Y_AXIS, 0.0f, glm::vec3(22.0f, 7.0f+crystal_move.y, -22.0f));
		glDrawElements(GL_TRIANGLES, g_crystal[0]->NumIndices(), GL_UNSIGNED_SHORT, 0);

		glBindTexture(GL_TEXTURE_2D, glassTx);
		g_crystal[1]->BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
		transformObject(glm::vec3(2.0f, 2.0f, 2.0f), Z_AXIS, 180.0f, glm::vec3(24.0f, 7.0f + crystal_move.y, -22.0f));
		glDrawElements(GL_TRIANGLES, g_crystal[1]->NumIndices(), GL_UNSIGNED_SHORT, 0);
	}

	glUniform3f(glGetUniformLocation(program, "sLight.position"), sLight.position.x, sLight.position.y, sLight.position.z);
	
	glBindVertexArray(0); // Done writing.
	glutSwapBuffers(); // Now for a potentially smoother render.
}

void parseKeys()
{
	if (keys & KEY_FORWARD)
		position += frontVec * MOVESPEED;
	else if (keys & KEY_BACKWARD)
		position -= frontVec * MOVESPEED;
	if (keys & KEY_LEFT)
		position -= rightVec * MOVESPEED;
	else if (keys & KEY_RIGHT)
		position += rightVec * MOVESPEED;
	if (keys & KEY_UP)
		position.y += MOVESPEED;
	else if (keys & KEY_DOWN)
		position.y -= MOVESPEED;
}

void timer(int) { // essentially our update()
	parseKeys();
	stateTest();
	glutPostRedisplay();
	glutTimerFunc(1000 / FPS, timer, 0); // 60 FPS or 16.67ms.
}

//---------------------------------------------------------------------
//
// keyDown
//
void keyDown(unsigned char key, int x, int y) // x and y is mouse location upon key press.
{
	switch (key)
	{
	case 'w':
		if (!(keys & KEY_FORWARD))
			keys |= KEY_FORWARD; break;
	case 's':
		if (!(keys & KEY_BACKWARD))
			keys |= KEY_BACKWARD; break;
	case 'a':
		if (!(keys & KEY_LEFT))
			keys |= KEY_LEFT; break;
	case 'd':
		if (!(keys & KEY_RIGHT))
			keys |= KEY_RIGHT; break;
	case 'r':
		if (!(keys & KEY_UP))
			keys |= KEY_UP; break;
	case 'f':
		if (!(keys & KEY_DOWN))
			keys |= KEY_DOWN; break;
	}
}

void keyDownSpec(int key, int x, int y) // x and y is mouse location upon key press.
{
	if (key == GLUT_KEY_UP)
	{
		if (!(keys & KEY_FORWARD))
			keys |= KEY_FORWARD;
	}
	else if (key == GLUT_KEY_DOWN)
	{
		if (!(keys & KEY_BACKWARD))
			keys |= KEY_BACKWARD;
	}
}

void keyUp(unsigned char key, int x, int y) // x and y is mouse location upon key press.
{
	switch (key)
	{
	case 'w':
		keys &= ~KEY_FORWARD; break;
	case 's':
		keys &= ~KEY_BACKWARD; break;
	case 'a':
		keys &= ~KEY_LEFT; break;
	case 'd':
		keys &= ~KEY_RIGHT; break;
	case 'r':
		keys &= ~KEY_UP; break;
	case 'f':
		keys &= ~KEY_DOWN; break;
	case ' ':
		resetView();
	}
}

void keyUpSpec(int key, int x, int y) // x and y is mouse location upon key press.
{
	if (key == GLUT_KEY_UP)
	{
		keys &= ~KEY_FORWARD;
	}
	else if (key == GLUT_KEY_DOWN)
	{
		keys &= ~KEY_BACKWARD;
	}
}

void mouseMove(int x, int y)
{
	if (keys & KEY_MOUSECLICKED)
	{
		pitch += (GLfloat)((y - lastY) * TURNSPEED);
		yaw -= (GLfloat)((x - lastX) * TURNSPEED);
		lastY = y;
		lastX = x;
	}
}

void mouseClick(int btn, int state, int x, int y)
{
	if (state == 0)
	{
		lastX = x;
		lastY = y;
		keys |= KEY_MOUSECLICKED; // Flip flag to true
		glutSetCursor(GLUT_CURSOR_NONE);
		//cout << "Mouse clicked." << endl;
	}
	else
	{
		keys &= ~KEY_MOUSECLICKED; // Reset flag to false
		glutSetCursor(GLUT_CURSOR_INHERIT);
		//cout << "Mouse released." << endl;
	}
}

void clean()
{
	cout << "Cleaning up!" << endl;
	glDeleteTextures(1, &grassTx);
	glDeleteTextures(1, &stoneTx);
	glDeleteTextures(1, &blankTx);
	glDeleteTextures(1, &wallTx);
	glDeleteTextures(1, &dirtTx);
	glDeleteTextures(1, &woodTx);
}

//---------------------------------------------------------------------
//
// main
//
int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA | GLUT_MULTISAMPLE);
	glutSetOption(GLUT_MULTISAMPLE, 8);
	glutInitWindowSize(1000, 1000);
	glutCreateWindow("GAME2012_FinalProject_XuZihan & Fnu Yiliqi");

	glewInit();	//Initializes the glew and prepares the drawing pipeline.
	init();

	glutDisplayFunc(display);
	glutKeyboardFunc(keyDown);
	glutSpecialFunc(keyDownSpec);
	glutKeyboardUpFunc(keyUp); // New function for third example.
	glutSpecialUpFunc(keyUpSpec);

	glutMouseFunc(mouseClick);
	glutMotionFunc(mouseMove); // Requires click to register.

	atexit(clean); // This GLUT function calls specified function before terminating program. Useful!

	glutMainLoop();

	return 0;
}
