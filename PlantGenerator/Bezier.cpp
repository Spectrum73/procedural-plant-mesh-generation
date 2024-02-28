#include "Bezier.h"

// Changes the number of edges used for the curve, detail must be > 0
void Bezier::SetEdges(int aEdges) 
{
	if (aEdges < 0) return;
	edges = aEdges;

	VAO.Bind();
	std::vector <glm::vec3> vertices = { A };
	std::vector <GLuint> indices = { };
	GLuint lastIndex = 0;

	// This is the loop where we add lines to represent the curve (except the start and end points
	for (GLuint i = 1; i < (edges); i++)
	{
		// Push an edge onto the index list
		indices.push_back(lastIndex);
		indices.push_back(i);

		// Add the vertex for the line at the point we want
		vertices.push_back(Evaluate((float)i/(float)edges));

		lastIndex = i;
	}

	// Push the last edge (outside of loop as we don't want to add any more vertices)
	indices.push_back(lastIndex);
	indices.push_back(lastIndex+1);
	vertices.push_back(B);
	EBO EBO(indices);

	// Generates Vertex Buffer Object and links it to vertices
	VBO VBO(vertices);
	VAO.LinkAttrib(VBO, 0, 3, GL_FLOAT, sizeof(glm::vec3), (void*)0);					 // Positions
	// Unbind all to prevent accidentally modifying them
	VAO.Unbind();
	VBO.Unbind();
	EBO.Unbind();
}

Bezier::Bezier(int aEdges, glm::vec3 a, glm::vec3 b, glm::vec3 c1, glm::vec3 c2) {
	A = a;
	B = b;
	C1 = c1;
	C2 = c2;

	// Generate the curve's vertices
	SetEdges(aEdges);
}

/// <summary>
/// Gets the point on the curve at t (where t ranges from 0 to 1)
/// </summary>
/// <param name="t"></param>
/// <returns></returns>
/// https://stackoverflow.com/questions/25453159/getting-consistent-normals-from-a-3d-cubic-bezier-path
glm::vec3 Bezier::Evaluate(float t) {
	if (t < 0 || t > 1) return glm::vec3(0.0f);

	return A * powf((1 - t), 3) +
		(float)3 * (C1+A) * powf((1 - t), 2) * t +
		(float)3 * (C2+B) * (1 - t) * powf(t, 2) +
		B * powf(t, 3);
}

/// <summary>
/// Displays the edges of the Bezier Curve.
/// </summary>
/// <param name="shader"></param>
/// <param name="camera"></param>
void Bezier::DrawBezier(Shader& shader, Camera& camera)
{
	// Bind shader to be able to access uniforms
	shader.Activate();
	VAO.Bind();

	// Take care of the camera Matrix
	glUniform3f(glGetUniformLocation(shader.ID, "camPos"), camera.Position.x, camera.Position.y, camera.Position.z);
	camera.Matrix(shader, "camMatrix");

	// Draw the actual mesh
	glDrawElements(GL_LINES, edges*2, GL_UNSIGNED_INT, 0);
	//glDrawArrays(GL_LINES, 0, detail*2);
}