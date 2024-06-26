#ifndef MESH_CLASS_H
#define MESH_CLASS_H

#include <string>
#include <vector>

#include "VAO.h"
#include "EBO.h"
#include "Camera.h"
#include "Texture.h"

#define CONCATENATE_WITH_UNION false

struct MeshData {
	std::vector <Vertex> vertices;
	std::vector <GLuint> indices;
	std::vector <Texture> textures;
};

class Mesh
{
private:
	// Sets up the VAO
	void SetupVAO();
public:
	std::vector <Vertex> vertices;
	std::vector <GLuint> indices;
	std::vector <Texture> textures;
	// Store VAO in public so it can be used in the Draw function
	VAO VAO;

	// Initializes the mesh
	Mesh() = default; // create a mesh with no data
	Mesh(std::vector <Vertex>& vertices, std::vector <GLuint>& indices, std::vector <Texture>& textures);
	Mesh(MeshData data) : Mesh(data.vertices, data.indices, data.textures) {}


	std::vector<float> getVertexPositionVector();
	std::vector<int> getIndexVector();

	// Draws the mesh
	void Draw(Shader& shader, Camera& camera);

	// Concatenates the passed mesh onto this mesh.
	void Concatenate(Mesh& aMesh, bool doUnion = false);
	void Concatenate(std::vector<Mesh*> aMeshes);

	void MergeVerticesByDistance(float distance = 0.01f);
	void RecalculateNormals();

	// Empties the mesh's vectors
	void Delete();
};
#endif