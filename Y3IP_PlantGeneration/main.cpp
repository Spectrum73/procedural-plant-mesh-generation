#include "Mesh.h"
#include "Curve.h"
#include "PlantGeneration.h"

// Some variables for tweaking the program
#define WINDOW_NAME "Mesh Viewport"
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 800
#define FOV_RADIANS 45.0f
#define WIREFRAME true

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
	Bezier bezier = Bezier(BEZIER_DEFAULT_EDGES,
		glm::vec3(-1, 1, 0),
		glm::vec3(2, -1, 1),
		glm::vec3(0, 1, -5),
		glm::vec3(-1, -1, 1));

	// Create a Curve
	Curve curve = Curve(CURVE_SUBDIVISIONS, BEZIER_DEFAULT_EDGES,
		glm::vec3(-1, 1, 0),
		glm::vec3(2, -1, 1),
		glm::vec3(0, 1, -5),
		glm::vec3(-1, -1, 1));

	// Create another curve
	Curve curve2 = Curve(3, 64,
		glm::vec3(1, -1, -1),
		glm::vec3(-1, 1, -5),
		glm::vec3(2, 1, 1),
		glm::vec3(1, 1, 0));

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

		bezier.DrawBezier(shaderProgram, camera);

		curve.Draw(shaderProgram, camera);
		curve2.Draw(shaderProgram, camera);

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