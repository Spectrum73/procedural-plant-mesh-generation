#include "Mesh.h"

void Mesh::SetupVAO() {
	VAO.Bind();
	// Generates Vertex Buffer Object and links it to vertices
	VBO VBO(vertices);
	// Generates Element Buffer Object and links it to indices
	EBO EBO(indices);
	// Links VBO attributes such as coordinates and colors to VAO
	VAO.LinkAttrib(VBO, 0, 3, GL_FLOAT, sizeof(Vertex), (void*)0);					 // Positions
	VAO.LinkAttrib(VBO, 1, 3, GL_FLOAT, sizeof(Vertex), (void*)(3 * sizeof(float))); // Normals
	VAO.LinkAttrib(VBO, 2, 3, GL_FLOAT, sizeof(Vertex), (void*)(6 * sizeof(float))); // Colours
	VAO.LinkAttrib(VBO, 3, 2, GL_FLOAT, sizeof(Vertex), (void*)(9 * sizeof(float))); // TexCoords
	// Unbind all to prevent accidentally modifying them
	VAO.Unbind();
	VBO.Unbind();
	EBO.Unbind();
}

Mesh::Mesh(std::vector <Vertex>& vertices, std::vector <GLuint>& indices, std::vector <Texture>& textures)
{
	Mesh::vertices = vertices;
	Mesh::indices = indices;
	Mesh::textures = textures;

	SetupVAO();
}

void Mesh::Concatenate(Mesh& aMesh) 
{
	// Append vertices
	int indexOffset = vertices.size();
	vertices.reserve(vertices.size() + aMesh.vertices.size());
	vertices.insert(vertices.end(), aMesh.vertices.begin(), aMesh.vertices.end());

	// Append indices
	indices.reserve(indices.size() + aMesh.indices.size());
	for (GLuint ind : aMesh.indices) 
	{
		indices.push_back(ind + indexOffset);
	}

	// Append textures
	bool exists = false;
	for (Texture &tex : aMesh.textures) {
		exists = false;
		// Only append if this texture doesn't exist
		for (Texture &tex2 : textures)
			if (&tex == &tex2)
				exists = true;
		if (!exists)
			textures.push_back(tex);
	}

	SetupVAO();
}

void Mesh::Concatenate(std::vector<Mesh*> aMeshes) 
{
	for (Mesh* mesh : aMeshes) {
		// Append vertices
		int indexOffset = vertices.size();
		vertices.reserve(vertices.size() + mesh->vertices.size());
		vertices.insert(vertices.end(), mesh->vertices.begin(), mesh->vertices.end());

		// Append indices
		indices.reserve(indices.size() + mesh->indices.size());
		indices.reserve(indices.size() + mesh->indices.size());
		for (GLuint ind : mesh->indices)
		{
			indices.push_back(ind + indexOffset);
		}

		// Append textures
		bool exists = false;
		for (Texture &tex : mesh->textures) {
			exists = false;
			// Only append if this texture doesn't exist
			for (Texture &tex2 : textures)
				if (&tex == &tex2)
					exists = true;
			if (!exists)
				textures.push_back(tex);
		}
	}

	SetupVAO();
}

void Mesh::Draw(Shader& shader, Camera& camera)
{
	// Bind shader to be able to access uniforms
	shader.Activate();
	VAO.Bind();

	// Keep track of how many of each type of textures we have
	unsigned int numDiffuse = 0;
	unsigned int numSpecular = 0;

	for (unsigned int i = 0; i < textures.size(); i++)
	{
		std::string num;
		std::string type = textures[i].type;
		if (type == "diffuse")
		{
			num = std::to_string(numDiffuse++);
		}
		else if (type == "specular")
		{
			num = std::to_string(numSpecular++);
		}
		textures[i].texUnit(shader, (type + num).c_str(), i);
		textures[i].Bind();
	}
	// Take care of the camera Matrix
	glUniform3f(glGetUniformLocation(shader.ID, "camPos"), camera.Position.x, camera.Position.y, camera.Position.z);
	camera.Matrix(shader, "camMatrix");

	// Draw the actual mesh
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
}

void Mesh::Delete() 
{
	vertices.clear();
	indices.clear();
	textures.clear();

	VAO.Delete();
}