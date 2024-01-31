#include "Bezier.h"

Bezier::Bezier(glm::vec3 a, glm::vec3 b, glm::vec3 c1, glm::vec3 c2) {
	A = a;
	B = b;
	C1 = c1;
	C2 = c2;

	VAO.Bind();
	std::vector <glm::vec3> points = { A, B };
	// Generates Vertex Buffer Object and links it to vertices
	VBO VBO(points);
	VAO.LinkAttrib(VBO, 0, 3, GL_FLOAT, sizeof(points), (void*)0);					 // Positions
	// Unbind all to prevent accidentally modifying them
	VAO.Unbind();
	VBO.Unbind();
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
		(float)3 * C1 * powf((1 - t), 2) * t +
		(float)3 * C2 * (1 - t) * powf(t, 2) +
		B * powf(t, 3);
}

void Bezier::Draw(Shader& shader, Camera& camera)
{
	// Bind shader to be able to access uniforms
	shader.Activate();
	VAO.Bind();

	// Take care of the camera Matrix
	glUniform3f(glGetUniformLocation(shader.ID, "camPos"), camera.Position.x, camera.Position.y, camera.Position.z);
	camera.Matrix(shader, "camMatrix");

	// Draw the actual mesh
	glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, 0);
}