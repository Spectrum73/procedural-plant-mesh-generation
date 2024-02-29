#include"Camera.h"



Camera::Camera(int width, int height, glm::vec3 position)
{
	Camera::width = width;
	Camera::height = height;
	Position = position;
}

void Camera::calculateDeltaTime() {
	deltaTime = 0;
	currentFrame = glfwGetTime();
	if (lastFrame != 0) deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;
}

void Camera::updateMatrix(float aFOVdeg, float aNearPlane, float aFarPlane)
{
	// Save the new settings
	FOVdeg = aFOVdeg;
	nearPlane = aNearPlane;
	farPlane = aFarPlane;

	// Initializes matrices since otherwise they will be the null matrix
	glm::mat4 view = glm::mat4(1.0f);
	glm::mat4 rotationMat(1.0f);
	glm::mat4 projection = glm::mat4(1.0f);

	// Makes camera look in the right direction from the right position
	if (!orbiting) view = glm::lookAt(Position, Position + Orientation, Up);
	else {
		// Set the centre of the orbit
		glm::vec3 orbitCentre = glm::vec3(0.0f, orbitHeight, 0.0f);

		// Adjust orbit rotation
		orbitRotation += orbitSpeed * deltaTime;
		if (orbitRotation > 360.0f) orbitRotation -= 360.0f;
		rotationMat = glm::rotate(rotationMat, orbitRotation, glm::vec3(0.0, 1.0, 0.0));

		// Combine to get the view matrix
		view = glm::translate(glm::vec3(0.0f, -orbitHeight, -orbitDistance)) * rotationMat;

		// Set our variables to line up with the current view for when we exit orbit mode.
		Position = -glm::mat3(glm::inverse(rotationMat)) * glm::vec3(view[3]);
		Orientation = glm::normalize(orbitCentre - Position);
	}

	// Adds perspective to the scene
	projection = glm::perspective(glm::radians(FOVdeg), (float)width / height, nearPlane, farPlane);

	// Sets new camera matrix
	cameraMatrix = projection * view;
}

void Camera::Matrix(Shader& shader, const char* uniform)
{
	// Exports camera matrix
	glUniformMatrix4fv(glGetUniformLocation(shader.ID, uniform), 1, GL_FALSE, glm::value_ptr(cameraMatrix));
}



void Camera::Inputs(GLFWwindow* window, int aMouseSnapPosX, int aMouseSnapPosY)
{
	// Handles key inputs
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		Position += speed * Orientation * deltaTime;
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		Position += speed * -glm::normalize(glm::cross(Orientation, Up)) * deltaTime;
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		Position += speed * -Orientation * deltaTime;
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		Position += speed * glm::normalize(glm::cross(Orientation, Up)) * deltaTime;
	}
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
	{
		Position += speed * Up * deltaTime;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
	{
		Position += speed * -Up * deltaTime;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
	{
		speed = 4.0f;
	}
	else if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE)
	{
		speed = 1.0f;
	}


	// Handles mouse inputs
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
	{
		// Hides mouse cursor
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

		float centreX = aMouseSnapPosX == -1 ? (width / 2) : aMouseSnapPosX;
		float centreY = aMouseSnapPosY == -1 ? (height / 2) : aMouseSnapPosY;

		// Prevents camera from jumping on the first click
		if (firstClick)
		{
			glfwSetCursorPos(window, centreX, centreY);
			std::cout << "WORK" << std::endl;
			firstClick = false;
		}

		// Stores the coordinates of the cursor
		double mouseX;
		double mouseY;

		// Fetches the coordinates of the cursor
		glfwGetCursorPos(window, &mouseX, &mouseY);

		// Normalizes and shifts the coordinates of the cursor such that they begin in the middle of the screen
		// and then "transforms" them into degrees 
		float rotX = sensitivity * (float)(mouseY - centreY) / height;
		float rotY = sensitivity * (float)(mouseX - centreX) / width;

		// Calculates upcoming vertical change in the Orientation
		glm::vec3 newOrientation = glm::rotate(Orientation, glm::radians(-rotX), glm::normalize(glm::cross(Orientation, Up)));

		// Decides whether or not the next vertical Orientation is legal or not
		if (abs(glm::angle(newOrientation, Up) - glm::radians(90.0f)) <= glm::radians(85.0f))
		{
			Orientation = newOrientation;
		}

		// Rotates the Orientation left and right
		Orientation = glm::rotate(Orientation, glm::radians(-rotY), Up);

		// Sets mouse cursor to the middle of the screen so that it doesn't end up roaming around
		glfwSetCursorPos(window, centreX, centreY);
	}
	else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE)
	{
		// Unhides cursor since camera is not looking around anymore
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		// Makes sure the next time the camera looks around it doesn't jump
		firstClick = true;
	}
}

void Camera::setOrbitSettings(float speed, float distance, float height) {
	orbitSpeed = speed;
	orbitDistance = distance;
	orbitHeight = height;
}