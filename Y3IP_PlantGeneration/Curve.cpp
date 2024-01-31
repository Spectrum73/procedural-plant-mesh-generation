#include "Curve.h"

MeshData Curve::calculateMesh(int aSubdivisions, glm::vec3 a, glm::vec3 b, glm::vec3 c1, glm::vec3 c2)
{
	Texture textures[]
	{
		Texture("terrible.png", "diffuse", 0, GL_RGBA, GL_UNSIGNED_BYTE)
	};

	// Calculate vertices
	std::vector <Vertex> vertices;
	std::vector <GLuint> indices;

	const float angleStep = 2.0f * glm::pi<float>() / float(aSubdivisions);
	float currentAngle = 0.0f;
	float prevY = std::cos(0.0f);
	float prevZ = std::sin(0.0f);
	for (int i = 0; i <= aSubdivisions; i++)
	{
		if (i != aSubdivisions)
		{
			currentAngle = angleStep * i;
			//std::cout << currentAngle << std::endl;
			float y = std::cos(currentAngle);
			float z = std::sin(currentAngle);

			// Add the vertices 
			vertices.push_back(Vertex{ glm::vec3(0.0f, y, z), glm::vec3(0.0f, y, z), glm::vec3(0.0f), glm::vec2(0.0f) });
			vertices.push_back(Vertex{ glm::vec3(1.0f, y, z), glm::vec3(0.0f, y, z), glm::vec3(0.0f), glm::vec2(0.0f) });

			// Add the indices (face representations)
			// We do not create faces on the first loop as we only have 2 vertices
			if (i > 0)
			{
				indices.push_back((GLuint)0 + (i - 1) * 2); indices.push_back((GLuint)1 + (i - 1) * 2); indices.push_back((GLuint)2 + (i - 1) * 2);
				indices.push_back((GLuint)2 + (i - 1) * 2); indices.push_back((GLuint)3 + (i - 1) * 2); indices.push_back((GLuint)1 + (i - 1) * 2);
			}
		}
		else
		{
			// Rather than adding extra unnecessary vertices, we instead attach the last faces to the first vertices,
			// hence we loop N+1 times as we cannot create these faces on the first loop
			indices.push_back((GLuint)0 + (i - 1) * 2); indices.push_back((GLuint)1 + (i - 1) * 2); indices.push_back((GLuint)0);
			indices.push_back((GLuint)0);				indices.push_back((GLuint)1);				indices.push_back((GLuint)1 + (i - 1) * 2);
		}
	}

	// Store Texture Data
	std::vector <Texture> tex(textures, textures + sizeof(textures) / sizeof(Texture));

	// return Mesh Data
	return (MeshData{ vertices, indices, tex });
}