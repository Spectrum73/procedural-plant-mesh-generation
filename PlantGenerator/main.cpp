#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "Mesh.h"
#include "Curve.h"
#include "PlantGeneration.h"

#include <iostream>
#include <fstream>

// Some variables for tweaking the program
#define WINDOW_NAME "Mesh Viewport"
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 800
#define FOV_RADIANS 45.0f
#define WIREFRAME false
#define BACKFACE_CULLING true

// TEMPORARY

// Vertices coordinates
Vertex vertices[] =
{ //               COORDINATES           /           NORMALS         /            COLORS          /       TEXTURE COORDINATES    //
	Vertex{glm::vec3(-1.0f, 0.0f,  1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 0.0f)},
	Vertex{glm::vec3(-1.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 1.0f)},
	Vertex{glm::vec3(1.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 1.0f)},
	Vertex{glm::vec3(1.0f, 0.0f,  1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 0.0f)}
};

// Indices for vertices order
GLuint indices[] =
{
	0, 1, 2,
	0, 2, 3
};

Vertex lightVertices[] =
{ //     COORDINATES     //
	Vertex{glm::vec3(-0.1f, -0.1f,  0.1f)},
	Vertex{glm::vec3(-0.1f, -0.1f, -0.1f)},
	Vertex{glm::vec3(0.1f, -0.1f, -0.1f)},
	Vertex{glm::vec3(0.1f, -0.1f,  0.1f)},
	Vertex{glm::vec3(-0.1f,  0.1f,  0.1f)},
	Vertex{glm::vec3(-0.1f,  0.1f, -0.1f)},
	Vertex{glm::vec3(0.1f,  0.1f, -0.1f)},
	Vertex{glm::vec3(0.1f,  0.1f,  0.1f)}
};

GLuint lightIndices[] =
{
	0, 1, 2,
	0, 2, 3,
	0, 4, 7,
	0, 7, 3,
	3, 7, 6,
	3, 6, 2,
	2, 6, 5,
	2, 5, 1,
	1, 5, 4,
	1, 4, 0,
	4, 5, 6,
	4, 6, 7
};

// /TEMPORARY

// Defined globally so it can be accessed by framebuffer_size_callback
Camera camera = Camera(WINDOW_WIDTH, WINDOW_HEIGHT, glm::vec3(0.0f, 1.0f, 0.0f));
// Defined globally so it can be accessed via key_callback
int plantIndex = 0;
std::vector<Plant*> Plants;

void RegeneratePlants() {
	for (int i = 0; i < Plants.size(); i++) {
		if (i == 0) {
			Plants[i]->GenerateGraph();
			Plants[i]->GenerateMesh();
		}
		else {
			Plants[i]->CopyGraph(Plants[0]);
			Plants[i]->GenerateMesh(3 * i, 2 * i);
		}
	}
}

void SaveCurrentPlant() {
	// Generate the file name
	// Name format: {name}_{ID}_{LOD}.obj
	std::string directory = "generations/";
	std::string baseName = "mesh_000_";

	for (int i = 0; i < Plants.size(); i++) {
		// Create and open a text file
		std::ofstream MyFile(directory + baseName + std::to_string(i) + ".obj");

		// Write to the file
		MyFile << "# Generated Mesh of LOD " + std::to_string(i) + '\n';

		// https://en.wikipedia.org/wiki/Wavefront_.obj_file

		// Write geometric vertices
		MyFile << "# List of geometric vertices\n";
		for (const Vertex &v : Plants[i]->vertices) {
			MyFile << "v " + std::to_string(v.position.x) + " "
				+ std::to_string(v.position.y) + " "
				+ std::to_string(v.position.z) + " 1.0\n";
		}

		// Write texture coordinates
		MyFile << "# List of texture coordinates\n";
		for (const Vertex& v : Plants[i]->vertices) {
			MyFile << "vt " + std::to_string(v.texUV.x) + " "
				+ std::to_string(v.texUV.y) + " 0\n";
		}

		// Write vertex normals
		MyFile << "# List of vertex normals\n";
		for (const Vertex& v : Plants[i]->vertices) {
			MyFile << "vn " + std::to_string(v.normal.x) + " "
				+ std::to_string(v.normal.y) + " "
				+ std::to_string(v.normal.z) + "\n";
		}

		// Write vertex indices
		MyFile << "# List of vertex indices\n";
		for (int j = 0; j < Plants[i]->indices.size(); j+=3) {
			std::string ind0 = std::to_string(Plants[i]->indices[j] + 1);
			std::string ind1 = std::to_string(Plants[i]->indices[j+1] + 1);
			std::string ind2 = std::to_string(Plants[i]->indices[j+2] + 1);
			MyFile << "f " + ind0 + " "
				+ ind1 + " "
				+ ind2 + "\n";
		}

		// Close the file
		MyFile.close();
	}
}

// This function is executed whenever the window is resized by any means
void framebuffer_size_callback(GLFWwindow* window, int aWidth, int aHeight) {
	if (aWidth < 1 || aHeight < 1) return;

	glViewport(0, 0, aWidth, aHeight);
	camera.setWidthAndHeight(aWidth, aHeight);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
		if (plantIndex > 0)
			plantIndex--;
		std::cout << "plantIndex: " + std::to_string(plantIndex) << std::endl;
	}
	else if (key == GLFW_KEY_2 && action == GLFW_PRESS){
		plantIndex++;
		std::cout << "plantIndex: " + std::to_string(plantIndex) << std::endl;
	}
	else if (key == GLFW_KEY_R && action == GLFW_PRESS) {
		// Regenerate Plant
		std::cout << "REGENERATING PLANTS" << std::endl;
		RegeneratePlants();
	}
	else if (key == GLFW_KEY_S && action == GLFW_PRESS && mods == GLFW_MOD_CONTROL) {
		// Save the current Plant as .obj
		SaveCurrentPlant();
	}
}

int main() 
{
	glfwInit();

	// Using OpenGL version 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	// and the CORE profile
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Window parameters are defined at the top of this script
	GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_NAME, NULL, NULL);
	// Check if window creation failed
	if (window == NULL) 
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Adjusts the window if it is resized
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// Function to call when inputs are made
	glfwSetKeyCallback(window, key_callback);

	gladLoadGL();
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

	// Texture
	std::string texPath = "";

	Texture textures[]
	{
		Texture((texPath + "terrible.png").c_str(), "diffuse", 0, GL_RGBA, GL_UNSIGNED_BYTE)
	};
	
	// Generates Shader object using shaders defualt.vert and default.frag
	Shader shaderProgram("default.vert", "default.frag");
	// Shader for light (to display an object where it's coming from)
	Shader lightShader("light.vert", "light.frag");

	// Store Mesh Data
	std::vector <Vertex> verts(vertices, vertices + sizeof(vertices) / sizeof(Vertex));
	std::vector <GLuint> ind(indices, indices + sizeof(indices) / sizeof(GLuint));
	std::vector <Texture> tex(textures, textures + sizeof(textures) / sizeof(Texture));
	// Create Mesh
	Mesh mesh(verts, ind, tex);

	// Store mesh data in vectors for the mesh
	std::vector <Vertex> lightVerts(lightVertices, lightVertices + sizeof(lightVertices) / sizeof(Vertex));
	std::vector <GLuint> lightInd(lightIndices, lightIndices + sizeof(lightIndices) / sizeof(GLuint));
	// Create light mesh
	Mesh light(lightVerts, lightInd, tex);

	// Create Bezier
	Bezier bezier = Bezier(BEZIER_DEFAULT_EDGES,
		glm::vec3(-1, 1, 0),
		glm::vec3(2, -1, 1),
		glm::vec3(0, 1, -5),
		glm::vec3(-1, -1, 1));

	// Create a Curve
	Curve curve = Curve(CURVE_SUBDIVISIONS, BEZIER_DEFAULT_EDGES, 0.05f, 0.4f,
		bezier);

	// Create another curve
	Curve curve2 = Curve(5, 64, 0.1f, 0.1f,
		glm::vec3(1, -1, -1),
		glm::vec3(-1, 1, -5),
		glm::vec3(2, 1, 1),
		glm::vec3(1, 1, 0));

	Curve curve3 = Curve(5, 64, 0.3f, 0.06f,
		glm::vec3(0, 0, 0),
		glm::vec3(.2f, 2, .5f),
		glm::vec3(.2f, 1, .4f),
		glm::vec3(.4f, 1, .2f));

	Curve basicCurve = Curve(8, 6, 0.1f, 0.1f,
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 1.0f),
		glm::vec3(0.0f, 2.0f, 0.0f),
		glm::vec3(0.0f, -2.0f, 0.0f));

	PlantParameters testParams;
	testParams.ApicalBudExtinction = 0.05f;
	testParams.GrowthRate = 0.6f;
	testParams.RootCircumferenceEdges = 8;
	testParams.RootCurveSegments = 6;
	Plant testPlant = new Plant(testParams);
	Plants.push_back(&testPlant);

	// This plant will be lower quality than the original
	Plant LOD1_Plant = new Plant(testPlant);
	Plants.push_back(&LOD1_Plant);

	RegeneratePlants();

	// Get the matrix for where we want to place the meshes
	glm::vec3 objectPos = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::mat4 objectModel = glm::mat4(1.0f);
	objectModel = glm::translate(objectModel, objectPos);

	glm::vec4 lightColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	glm::vec3 lightPos = glm::vec3(0.5f, 0.5f, -1.5f);
	glm::mat4 lightModel = glm::mat4(1.0f);
	lightModel = glm::translate(lightModel, lightPos);

	// Bind uniforms for the light shader
	lightShader.Activate();
	glUniformMatrix4fv(glGetUniformLocation(lightShader.ID, "model"), 1, GL_FALSE, glm::value_ptr(lightModel));
	glUniform4f(glGetUniformLocation(lightShader.ID, "lightColor"), lightColor.x, lightColor.y, lightColor.z, lightColor.w);
	// Bind the uniforms for the mesh
	shaderProgram.Activate();
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram.ID, "model"), 1, GL_FALSE, glm::value_ptr(objectModel));
	glUniform4f(glGetUniformLocation(shaderProgram.ID, "lightColor"), lightColor.x, lightColor.y, lightColor.z, lightColor.w);
	glUniform3f(glGetUniformLocation(shaderProgram.ID, "lightPos"), lightPos.x, lightPos.y, lightPos.z);

	float rotation = 0.0f;
	double prevTime = glfwGetTime();

	// Enables the depth buffer and wireframe view if enabled
	glEnable(GL_DEPTH_TEST);
	if (BACKFACE_CULLING) glEnable(GL_CULL_FACE);
	if (WIREFRAME) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	while (!glfwWindowShouldClose(window)) 
	{
		// Clean back buffer and assign our colour to it
		glClearColor(0.12f, 0.07f, 0.09f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shaderProgram.Activate();

		// Handle camera inputs and update its matrix to export to the vertex shader
		camera.Inputs(window);
		camera.updateMatrix(FOV_RADIANS, 0.1f, 100.0f);


		float currentTime = glfwGetTime();
		if (currentTime - prevTime >= 1 / 60) 
		{
			rotation += 0.5f;
			prevTime = currentTime;
		}

		//mesh.Draw(shaderProgram, camera);

		//bezier.DrawBezier(shaderProgram, camera);

		//curve.Draw(shaderProgram, camera);
		//curve2.Draw(shaderProgram, camera);
		//curve3.Draw(shaderProgram, camera);
		//basicCurve.Draw(shaderProgram, camera);

		light.Draw(lightShader, camera);

		switch (plantIndex) {
		case 0:
			testPlant.Draw(shaderProgram, camera);
			break;
		case 1:
			LOD1_Plant.Draw(shaderProgram, camera);
			break;
		}

		// Swap back buffer with front buffer
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// Clean up objects we've made
	shaderProgram.Delete();
	lightShader.Delete();
	// Delete window and terminate GLFW before ending the program
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}