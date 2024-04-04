#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "Mesh.h"
#include "Curve.h"
#include "PlantGeneration.h"
#include "FrameBuffer.h"

#include "Helpers.h"

#include <iostream>
#include <fstream>

// Some variables for tweaking the program
#define WINDOW_NAME "Mesh Viewport"
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 800
#define FOV_RADIANS 45.0f
#define BACKFACE_CULLING false

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
FBO frameBuffer;
bool MainWindowFocused = false;
char filename[30] = "mesh";
int shaderIndex = 0;
bool wireframe = false;
PlantParameters plantParams;
int LOD_edgeReduction = 3; int LOD_segmentReduction = 2;

// Defined globally so it can be accessed via key_callback
int plantIndex = 0;
std::vector<Plant*> Plants;

void RegeneratePlants() {
	for (int i = 0; i < Plants.size(); i++) {
		if (i == 0) {
			Plants[i]->setParameters(plantParams);
			Plants[i]->GenerateGraph();
			Plants[i]->GenerateMesh();
		}
		else {
			Plants[i]->CopyGraph(Plants[0]);
			Plants[i]->GenerateMesh(LOD_edgeReduction * i, LOD_segmentReduction * i);
		}
	}
}

void SaveCurrentPlant() {
	// Generate the file name
	// Name format: {name}_{ID}_{LOD}.obj
	std::string directory = "generations/";
	std::string baseName = std::string(filename) + '_';

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

		std::cout << "File saved as: " << directory + baseName + std::to_string(i) + ".obj" << std::endl;

		// Close the file
		MyFile.close();
	}
}

// Helper for ImGui alignment
// https://github.com/ocornut/imgui/discussions/3862
void AlignForWidth(float width, float alignment = 0.5f)
{
	ImGuiStyle& style = ImGui::GetStyle();
	float avail = ImGui::GetContentRegionAvail().x;
	float off = (avail - width) * alignment;
	if (off > 0.0f)
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);
}

// This function is executed whenever the window is resized by any means
void framebuffer_size_callback(GLFWwindow* window, int aWidth, int aHeight) {
	if (aWidth < 1 || aHeight < 1) return;

	glViewport(0, 0, aWidth, aHeight);
	//camera.setWidthAndHeight(aWidth, aHeight);
	frameBuffer.RescaleFrameBuffer(aWidth, aHeight);
}

void scene_size_callback(int aWidth, int aHeight) {
	
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (!MainWindowFocused) return;
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

	// Generate the Framebuffer
	frameBuffer = *(new FBO(WINDOW_WIDTH, WINDOW_HEIGHT));

	// Texture
	std::string texPath = "";

	Texture textures[]
	{
		Texture((texPath + "terrible.png").c_str(), "diffuse", 0, GL_RGBA, GL_UNSIGNED_BYTE)
	};
	
	// Generates Shader object using shaders defualt.vert and default.frag
	Shader shaderProgram("default.vert", "default.frag");
	Shader normalShader("default.vert", "normals.frag");
	Shader basicLitShader("default.vert", "basic_lighting.frag");
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

	plantParams.Decay = 0.96f;
	plantParams.ApicalBudExtinction = 0.05f;
	plantParams.GrowthRate = 0.4f;
	plantParams.RootCircumferenceEdges = 8;
	plantParams.RootCurveSegments = 6;
	Plant mainPlant = new Plant(plantParams);
	Plants.push_back(&mainPlant);

	// This plant will be lower quality than the original
	Plant LOD1_Plant = new Plant(mainPlant);
	Plants.push_back(&LOD1_Plant);

	// This plant will be lower quality than the original
	Plant LOD2_Plant = new Plant(mainPlant);
	Plants.push_back(&LOD2_Plant);

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
	// Bind the uniforms for the mesh
	basicLitShader.Activate();
	glUniformMatrix4fv(glGetUniformLocation(basicLitShader.ID, "model"), 1, GL_FALSE, glm::value_ptr(objectModel));
	glUniform4f(glGetUniformLocation(basicLitShader.ID, "lightColor"), lightColor.x, lightColor.y, lightColor.z, lightColor.w);
	glUniform3f(glGetUniformLocation(basicLitShader.ID, "lightPos"), lightPos.x, lightPos.y, lightPos.z);
	normalShader.Activate();
	glUniformMatrix4fv(glGetUniformLocation(normalShader.ID, "model"), 1, GL_FALSE, glm::value_ptr(objectModel));

	float rotation = 0.0f;
	double prevTime = glfwGetTime();

	// Enables the depth buffer and wireframe view if enabled
	glEnable(GL_DEPTH_TEST);
	if (BACKFACE_CULLING) glEnable(GL_CULL_FACE);
	if (wireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// GUI Setup
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	// Enable Docking
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	// Prevent moving windows unless dragging title bar
	ImGui::GetIO().ConfigWindowsMoveFromTitleBarOnly = true;
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	while (!glfwWindowShouldClose(window)) 
	{
		// Clean back buffer and assign our colour to it
		glClearColor(0.12f, 0.07f, 0.09f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		camera.calculateDeltaTime();

		// Declare new ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		frameBuffer.Bind();

		shaderProgram.Activate();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0.12f, 0.07f, 0.09f, 1.0f);
		if (BACKFACE_CULLING) glEnable(GL_CULL_FACE);
		if (wireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

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

		//light.Draw(lightShader, camera);

		// Decide which shader to use
		Shader* shader = &shaderProgram;
		switch (shaderIndex)
		{
		case 0:
			shader = &shaderProgram;
			break;
		case 1:
			shader = &basicLitShader;
			break;
		case 2:
			shader = &normalShader;
			break;
		default:
			break;
		}

		// Draw the currently selected plant
		if (Plants[plantIndex] != NULL)
			Plants[plantIndex]->Draw(*shader, camera);

		frameBuffer.Unbind();

		ImGui::Begin("Plant Parameters");
		ImGui::TextWrapped("Adjust the generated plant.");
		// Align the button to the middle
		ImGui::SetCursorPosX((ImGui::GetWindowWidth()-150.0f)/2);
		if (ImGui::Button("Regenerate", ImVec2(150.0f, 0.0f)))
			RegeneratePlants();
		ImGui::Text("LOD Settings");
		ImGui::PushItemWidth(150);
		if (ImGui::InputInt("LOD Level", &plantIndex, 1))
			plantIndex = plantIndex < 0 ? 0 : (plantIndex > Plants.size()-1 ? Plants.size()-1 : plantIndex); // Clamp
		if (ImGui::InputInt("Edge Reduction", &LOD_edgeReduction, 1))
			LOD_edgeReduction = LOD_edgeReduction < 0 ? 0 : LOD_edgeReduction;
		if (ImGui::InputInt("Segment Reduction", &LOD_segmentReduction, 1))
			LOD_segmentReduction = LOD_segmentReduction < 0 ? 0 : LOD_segmentReduction;
		ImGui::PopItemWidth();
		ImGui::Text("Plant Settings");
		ImGui::PushItemWidth(150);
		/*plantParams.ApicalBudExtinction = 0.05f;
		plantParams.GrowthRate = 0.6f;
		plantParams.RootCircumferenceEdges = 8;
		plantParams.RootCurveSegments = 6;*/
		if (ImGui::InputFloat("Apical Bud Extinction", &plantParams.ApicalBudExtinction, 0.05f))
			plantParams.ApicalBudExtinction = clamp(plantParams.ApicalBudExtinction, 0.05f, 1.0f);
		if (ImGui::InputFloat("Growth Rate", &plantParams.GrowthRate, 0.05f))
			plantParams.GrowthRate = clamp(plantParams.GrowthRate, 0.05f, 1.0f);
		if (ImGui::InputFloat("Decay", &plantParams.Decay, 0.01f))
			plantParams.Decay = clamp(plantParams.Decay, 0.01f, 1.0f);
		if (ImGui::InputInt("Root Circumference Edges", &plantParams.RootCircumferenceEdges, 1))
			plantParams.RootCircumferenceEdges = clamp(plantParams.RootCircumferenceEdges, 3, 256);
		if (ImGui::InputInt("Root Curve Segments", &plantParams.RootCurveSegments, 1))
			plantParams.RootCurveSegments = clamp(plantParams.RootCurveSegments, 1, 256);
		if (ImGui::InputFloat("Root Width", &plantParams.RootWidth, 0.01f))
			plantParams.RootWidth = clamp(plantParams.RootWidth, 0.01f, 5.0f);

		ImGui::PopItemWidth();
		ImGui::End();

		ImGui::Begin("Export Settings");
		ImGui::TextWrapped("Change how the OBJ files are exported. Files are saved under /generations");
		ImGui::NewLine();
		ImGui::Text("Filename: ");
		ImGui::SameLine();
		ImGui::InputText("##mesh", filename, IM_ARRAYSIZE(filename));
		if (ImGui::Button("Save as Wavefront .OBJ"))
			SaveCurrentPlant();
		ImGui::End();

		ImGui::Begin("Camera Settings");
		ImGui::TextWrapped("Tweak how the camera performs.");
		ImGui::RadioButton("Textured", &shaderIndex, 0); ImGui::SameLine();
		ImGui::RadioButton("Basic Lit", &shaderIndex, 1); ImGui::SameLine();
		ImGui::RadioButton("Normals", &shaderIndex, 2);
		if (ImGui::Checkbox("Wireframe", &wireframe)) {
			if (wireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			else glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
		ImGui::Text("Orbit Settings");
		ImGui::Checkbox("Orbit", &camera.orbiting);
		ImGui::SliderFloat("Orbit Distance", &camera.orbitDistance, 1.0f, 25.0f);
		ImGui::SliderFloat("Orbit Height", &camera.orbitHeight, -2.0f, 6.0f);
		ImGui::SliderFloat("Orbit Speed", &camera.orbitSpeed, -5.0f, 5.0f);
		ImGui::End();

		ImGui::Begin("Scene");
		{
			ImGui::BeginChild("SceneRender");
			if (ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows) && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
				// Handle camera inputs and update its matrix to export to the vertex shader
				MainWindowFocused = true;
				camera.Inputs(window, ImGui::GetWindowPos().x + ImGui::GetWindowWidth() / 2, ImGui::GetWindowPos().y + ImGui::GetWindowHeight() / 2);
				ImGui::SetMouseCursor(ImGuiMouseCursor_None);
			}
			else {
				MainWindowFocused = false;
				// Reset the flag to detect the first click for the camera (to prevent snapping)
				camera.firstClick = true;
			}
			camera.updateMatrix(FOV_RADIANS, 0.1f, 100.0f);

			ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
			ImVec2 sceneSize = { viewportPanelSize.x, viewportPanelSize.y };
			// On window resize
			if (frameBuffer.size.x != sceneSize.x || frameBuffer.size.y != sceneSize.y) {
				frameBuffer.size = viewportPanelSize;
				scene_size_callback(frameBuffer.size.x, frameBuffer.size.y);
			}

			float width = ImGui::GetContentRegionAvail().x;
			float height = ImGui::GetContentRegionAvail().y;

			camera.setWidthAndHeight(width, height);
			ImGui::Image((ImTextureID)frameBuffer.GetTextureID(), ImGui::GetContentRegionAvail(), ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
		}
		ImGui::EndChild();
		ImGui::End();

		// Render UI Elements
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// Swap back buffer with front buffer
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// End ImGui Processes
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	// Clean up objects we've made
	shaderProgram.Delete();
	normalShader.Delete();
	basicLitShader.Delete();
	lightShader.Delete();
	// Delete window and terminate GLFW before ending the program
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}