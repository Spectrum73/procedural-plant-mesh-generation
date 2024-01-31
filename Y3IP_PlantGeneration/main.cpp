#include "Mesh.h"
#include "Bezier.h"
#include "PlantGeneration.h"
#include <Windows.h>
#include <gl/GL.h>

// Some variables for tweaking the program
#define WINDOW_NAME "Mesh Viewport"
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 800
#define FOV_RADIANS 45.0f
#define WIREFRAME false

// TEMPORARY

// Vertices coordinates
Vertex vertices[] =
{ //               COORDINATES           /            COLORS          /           NORMALS         /       TEXTURE COORDINATES    //
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

// /TEMPORARY


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
	// Store Mesh Data
	std::vector <Vertex> verts(vertices, vertices + sizeof(vertices) / sizeof(Vertex));
	std::vector <GLuint> ind(indices, indices + sizeof(indices) / sizeof(GLuint));
	std::vector <Texture> tex(textures, textures + sizeof(textures) / sizeof(Texture));
	// Create Mesh
	Mesh mesh(verts, ind, tex);

	// Create Bezier
	Bezier bezier = Bezier(glm::vec3(-100, 100, 0),
		glm::vec3(200, -100, 100),
		glm::vec3(0, 100, -500),
		glm::vec3(-100, -100, 100));

	// Get the matrix for where we want to place the mesh
	glm::vec3 objectPos = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::mat4 objectModel = glm::mat4(1.0f);
	objectModel = glm::translate(objectModel, objectPos);

	// Bind the uniform for the mesh's position
	shaderProgram.Activate();
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram.ID, "model"), 1, GL_FALSE, glm::value_ptr(objectModel));

	float rotation = 0.0f;
	double prevTime = glfwGetTime();

	// Enables the depth buffer and wireframe view if enabled
	glEnable(GL_DEPTH_TEST);
	if (WIREFRAME) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	Camera camera(WINDOW_WIDTH, WINDOW_HEIGHT, glm::vec3(0.0f, 1.0f, 0.0f));

	/// TEMPORARY
	
	VAO lineVAO;
	lineVAO.Bind();
	// Generates Vertex Buffer Object and links it to vertices
	VBO VBO(verts);
	// Generates Element Buffer Object and links it to indices
	EBO EBO(ind);
	// Links VBO attributes such as coordinates and colors to VAO
	lineVAO.LinkAttrib(VBO, 0, 3, GL_FLOAT, sizeof(Vertex), (void*)0);					 // Positions
	lineVAO.LinkAttrib(VBO, 1, 3, GL_FLOAT, sizeof(Vertex), (void*)(3 * sizeof(float))); // Normals
	lineVAO.LinkAttrib(VBO, 2, 3, GL_FLOAT, sizeof(Vertex), (void*)(6 * sizeof(float))); // Colours
	lineVAO.LinkAttrib(VBO, 3, 2, GL_FLOAT, sizeof(Vertex), (void*)(9 * sizeof(float))); // TexCoords
	// Unbind all to prevent accidentally modifying them
	lineVAO.Unbind();
	VBO.Unbind();
	EBO.Unbind();

	/// TEMPORARY

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

		mesh.Draw(shaderProgram, camera);

		bezier.Draw(shaderProgram, camera);

		// Swap back buffer with front buffer
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// Clean up objects we've made
	shaderProgram.Delete();

	// Delete window and terminate GLFW before ending the program
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}