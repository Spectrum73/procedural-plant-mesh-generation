#include "Mesh.h"
#include "BooleanUnion.h"

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

std::vector<float> Mesh::getVertexPositionVector() {
	std::vector<float> posVector;
	for (int i = 0; i < vertices.size()*3; i+=3) {
		posVector.push_back(vertices[i].position.x);
		posVector.push_back(vertices[i].position.y);
		posVector.push_back(vertices[i].position.z);
	}
	return posVector;
}
std::vector<int> Mesh::getIndexVector() { 
	return std::vector<int>(indices.begin(), indices.end()); 
}

void Mesh::Concatenate(Mesh& aMesh, bool doUnion)
{
	int boolStatus = 0;
	if (doUnion)
		boolStatus = BooleanUnion(*this, aMesh) != 0;
	boolStatus = 0; // TEMPORARY
	if (!doUnion || boolStatus != 0) {
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
		for (Texture& tex : aMesh.textures) {
			exists = false;
			// Only append if this texture doesn't exist
			for (Texture& tex2 : textures)
				if (&tex == &tex2)
					exists = true;
			if (!exists)
				textures.push_back(tex);
		}
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
		for (Texture& tex : mesh->textures) {
			exists = false;
			// Only append if this texture doesn't exist
			for (Texture& tex2 : textures)
				if (&tex == &tex2)
					exists = true;
			if (!exists)
				textures.push_back(tex);
		}
	}

	SetupVAO();
}

void Mesh::MergeVerticesByDistance(float aDistance) {
	fprintf(stdout, "[Cleanup] Starting Merge by Distance %f\n", aDistance);
	int deletedCount = 0;
	// Compare each vertex against all others
	for (int i = 0; i < vertices.size()-1; i++) {
		//for (int j = (i+1); j < vertices.size(); j++) {
		std::vector<Vertex>::iterator iter;
		for (iter = vertices.begin() + i + 1; iter != vertices.end(); ) {
			int iter_pos = iter - vertices.begin();
			// Check if vertices are too close
			if (glm::distance(vertices[i].position, vertices[iter_pos].position) < aDistance) {
				// Merge
				for (int k = 0; k < indices.size(); k++) {
					if (indices[k] == iter_pos)
						indices[k] = i;
					else if (indices[k] > iter_pos)
						indices[k]--;
				}
				// Delete vertex
				iter = vertices.erase(iter);
				deletedCount++;
			}
			else
				++iter;
		}
	}

	// Cleanup faces (Remove any with repeating vertices)
	for (int i = 0; i < indices.size(); i += 3) 
	{
		if (indices[i] == indices[i + 1] || indices[i] == indices[i + 2] || indices[i + 1] == indices[i + 2]) {
			// Mark for deletion
			indices[i] = -1;
			indices[i+1] = -1;
			indices[i+2] = -1;
		}
	}
	// Remove marked faces
	indices.erase(std::remove_if(indices.begin(), indices.end(), [](int index) {
		return index == -1;
	}), indices.end());


	fprintf(stdout, "[Cleanup] Merged %i vertices.\n", deletedCount);
	SetupVAO();
}

// Recalculates the mesh's normals
void Mesh::RecalculateNormals() {
	fprintf(stdout, "[Cleanup] Recalculating vertex normals.\n");
	// Recalculate vertex normals
	for (int i = 0; i < vertices.size(); i++)
		vertices[i].normal = glm::vec3(.0f);
	
	// For each face calculate normals and append to the corresponding vertices of the face
	for (int i = 0; i < indices.size(); i += 3)
	{
		glm::vec3 A = vertices[indices[i]].position;
		glm::vec3 B = vertices[indices[i + 1]].position;
		glm::vec3 C = vertices[indices[i + 2]].position;
		glm::vec3 AB = B - A;
		glm::vec3 AC = C - A;
		glm::vec3 ABxAC = glm::normalize(glm::cross(AB, AC));
		vertices[indices[i]].normal += ABxAC;
		vertices[indices[i + 1]].normal += ABxAC;
		vertices[indices[i + 2]].normal += ABxAC;
	}

	// Normalize normals
	for (int i = 0; i < vertices.size(); i++)
		vertices[i].normal = glm::normalize(vertices[i].normal);

	SetupVAO();
}