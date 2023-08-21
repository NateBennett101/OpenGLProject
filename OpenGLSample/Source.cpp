
// Nate Bennett

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cstring>

#include "shader.h"
#include "camera.h"

#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
unsigned int loadTexture(const char* path);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
float cameraSpeed = 1.5f; // Initial camera speed

Camera birdEyeCamera(glm::vec3(0.0f, 0.0f, 3.0f));
bool birdEyeView = false; // Indicates whether bird's eye view is active or not
bool birdEyeKeyPressed = false;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// lighting
glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

struct GLMesh
{
	GLuint vao;  // Vertex Array Object
	GLuint vbo;  // Vertex Buffer Object
	GLuint cbo;  // Color Buffer Object
	GLuint ebo;  // Element Buffer Object
	GLuint tbo;  // Texture Buffer Object
	GLsizei nIndices;  // Number of indices to be rendered
};

void UCreateCupMesh(GLMesh& mesh);
void UCreateHandleMesh(GLMesh& mesh);
void UCreatePlaneMesh(GLMesh& mesh);
void UCreatePaper1Mesh(GLMesh& mesh);
void UCreatePaper2Mesh(GLMesh& mesh);
void UCreatePaper3Mesh(GLMesh& mesh);
void UCreatePenMesh(GLMesh& mesh);

int main()
{
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// glfw window creation
	// --------------------
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Nate Bennett", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// configure global opengl state
	// -----------------------------
	glEnable(GL_DEPTH_TEST);

	// build and compile our shader zprogram
	// ------------------------------------
	Shader lightingShader("shaderfiles/6.multiple_lights.vs", "shaderfiles/6.multiple_lights.fs");
	Shader lightCubeShader("shaderfiles/6.light_cube.vs", "shaderfiles/6.light_cube.fs");

	// set up vertex data (and buffer(s)) and configure vertex attributes
	// ------------------------------------------------------------------

	// positions all containers
	glm::vec3 cubePositions[] = {
		glm::vec3(0.0f,  0.0f,  0.0f),
		glm::vec3(2.0f,  5.0f, -15.0f),
		glm::vec3(-1.5f, -2.2f, -2.5f),
		glm::vec3(-3.8f, -2.0f, -12.3f),
		glm::vec3(2.4f, -0.4f, -3.5f),
		glm::vec3(-1.7f,  3.0f, -7.5f),
		glm::vec3(1.3f, -2.0f, -2.5f),
		glm::vec3(1.5f,  2.0f, -2.5f),
		glm::vec3(1.5f,  0.2f, -1.5f),
		glm::vec3(-1.3f,  1.0f, -1.5f)
	};
	// positions of the point lights
	glm::vec3 pointLightPositions[] = {
		glm::vec3(0.7f,  0.2f,  2.0f),
		glm::vec3(2.3f, -3.3f, -4.0f),
		glm::vec3(-4.0f,  2.0f, -12.0f),
		glm::vec3(0.0f,  0.0f, -3.0f)
	};
	// first, configure the cube's VAO (and VBO)
	unsigned int VBO, cubeVAO;
	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &VBO);


	glBindVertexArray(cubeVAO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	// second, configure the light's VAO (VBO stays the same; the vertices are the same for the light object which is also a 3D cube)
	unsigned int lightCubeVAO;
	glGenVertexArrays(1, &lightCubeVAO);
	glBindVertexArray(lightCubeVAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	// note that we update the lamp's position attribute's stride to reflect the updated buffer data
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// load textures (we now use a utility function to keep the code more organized)
	// -----------------------------------------------------------------------------
	unsigned int diffuseMap = loadTexture("marbleTex.jpg");
	unsigned int specularMap = loadTexture("marbleTex.jpg");
	unsigned int woodTexture = loadTexture("woodTex.jpg");
	unsigned int paperTexture = loadTexture("paperTex.jpg");
	unsigned int penTexture = loadTexture("penTex.jpg");

	// shader configuration
	// --------------------
	lightingShader.use();
	lightingShader.setInt("material.diffuse", 0);
	lightingShader.setInt("material.specular", 1);

	lightingShader.setInt("material.diffuse", 2);  // Use the appropriate texture unit index (e.g., GL_TEXTURE2)


	GLMesh cupMesh;
	UCreateCupMesh(cupMesh);

	GLMesh handleMesh;
	UCreateHandleMesh(handleMesh);

	GLMesh planeMesh;
	UCreatePlaneMesh(planeMesh);

	GLMesh paper1Mesh;
	UCreatePaper1Mesh(paper1Mesh);

	GLMesh paper2Mesh;
	UCreatePaper2Mesh(paper2Mesh);

	GLMesh paper3Mesh;
	UCreatePaper2Mesh(paper3Mesh);

	GLMesh penMesh;
	UCreatePenMesh(penMesh);

	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		// per-frame time logic
		// --------------------
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// input
		// -----
		processInput(window);

		// render
		// ------
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// be sure to activate shader when setting uniforms/drawing objects
		lightingShader.use();
		lightingShader.setVec3("viewPos", camera.Position);
		lightingShader.setFloat("material.shininess", 32.0f);

		// directional light
		lightingShader.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
		lightingShader.setVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
		lightingShader.setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
		lightingShader.setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);
		// point light 1
		lightingShader.setVec3("pointLights[0].position", pointLightPositions[0]);
		lightingShader.setVec3("pointLights[0].ambient", 0.05f, 0.05f, 0.05f);
		lightingShader.setVec3("pointLights[0].diffuse", 0.8f, 0.8f, 0.8f);
		lightingShader.setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
		lightingShader.setFloat("pointLights[0].constant", 1.0f);
		lightingShader.setFloat("pointLights[0].linear", 0.09);
		lightingShader.setFloat("pointLights[0].quadratic", 0.032);
		// point light 2
		lightingShader.setVec3("pointLights[1].position", pointLightPositions[1]);
		lightingShader.setVec3("pointLights[1].ambient", 0.05f, 0.05f, 0.05f);
		lightingShader.setVec3("pointLights[1].diffuse", 0.8f, 0.8f, 0.8f);
		lightingShader.setVec3("pointLights[1].specular", 1.0f, 1.0f, 1.0f);
		lightingShader.setFloat("pointLights[1].constant", 1.0f);
		lightingShader.setFloat("pointLights[1].linear", 0.09);
		lightingShader.setFloat("pointLights[1].quadratic", 0.032);
		// point light 3
		lightingShader.setVec3("pointLights[2].position", pointLightPositions[2]);
		lightingShader.setVec3("pointLights[2].ambient", 0.05f, 0.05f, 0.05f);
		lightingShader.setVec3("pointLights[2].diffuse", 0.8f, 0.8f, 0.8f);
		lightingShader.setVec3("pointLights[2].specular", 1.0f, 1.0f, 1.0f);
		lightingShader.setFloat("pointLights[2].constant", 1.0f);
		lightingShader.setFloat("pointLights[2].linear", 0.09);
		lightingShader.setFloat("pointLights[2].quadratic", 0.032);
		// point light 4
		lightingShader.setVec3("pointLights[3].position", pointLightPositions[3]);
		lightingShader.setVec3("pointLights[3].ambient", 0.05f, 0.05f, 0.05f);
		lightingShader.setVec3("pointLights[3].diffuse", 0.8f, 0.8f, 0.8f);
		lightingShader.setVec3("pointLights[3].specular", 1.0f, 1.0f, 1.0f);
		lightingShader.setFloat("pointLights[3].constant", 1.0f);
		lightingShader.setFloat("pointLights[3].linear", 0.09);
		lightingShader.setFloat("pointLights[3].quadratic", 0.032);
		// spotLight
		lightingShader.setVec3("spotLight.position", camera.Position);
		lightingShader.setVec3("spotLight.direction", camera.Front);
		lightingShader.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.1f);
		lightingShader.setVec3("spotLight.diffuse", 0.0f, 0.0f, 0.5f);
		lightingShader.setVec3("spotLight.specular", 0.0f, 0.0f, 0.5f);
		lightingShader.setFloat("spotLight.constant", 1.0f);
		lightingShader.setFloat("spotLight.linear", 0.09);
		lightingShader.setFloat("spotLight.quadratic", 0.032);
		lightingShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
		lightingShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));

		// view/projection transformations
		glm::mat4 view = (birdEyeView ? birdEyeCamera.GetViewMatrix() : camera.GetViewMatrix());
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		if (birdEyeView) {
			birdEyeCamera.Position = glm::vec3(0.0f, 10.0f, 0.0f); // Adjust the camera position if needed
			birdEyeCamera.Front = glm::vec3(0.0f, -1.0f, 0.0f); // Make the camera look down
			birdEyeCamera.Up = glm::vec3(0.0f, 0.0f, -1.0f); // Set the new "Up" vector
			view = birdEyeCamera.GetViewMatrix();
		}

		lightingShader.setMat4("view", view);
		lightingShader.setMat4("projection", projection);

		// world transformation
		glm::mat4 model = glm::mat4(1.0f);
		lightingShader.setMat4("model", model);

		// bind diffuse map
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, diffuseMap);
		// bind specular map
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, specularMap);
		// Bind the plane's texture to a texture unit (e.g., GL_TEXTURE3)
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, woodTexture);  // Bind wood texture to unit 2

		// render containers
		glBindVertexArray(cubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);


		// Render cup mesh with translation
		glm::mat4 cupModel = glm::mat4(1.0f);
		cupModel = glm::translate(cupModel, glm::vec3(-1.0f, 0.0f, -1.0f));
		lightingShader.setMat4("model", cupModel);

		// render cup
		lightingShader.setInt("material.diffuse", 0);
		lightingShader.setInt("material.specular", 1);

		glBindVertexArray(cupMesh.vao);
		glDrawElements(GL_TRIANGLES, cupMesh.nIndices, GL_UNSIGNED_INT, 0);

		// render handle with translation and rotation
		glm::mat4 handleModel = glm::mat4(1.0f); // Identity matrix
		handleModel = glm::rotate(handleModel, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		handleModel = glm::translate(handleModel, glm::vec3(-0.5f, -1.0f, 0.0f));
		lightingShader.setMat4("model", handleModel);

		lightingShader.setInt("material.diffuse", 0);
		lightingShader.setInt("material.specular", 1);

		glBindVertexArray(handleMesh.vao);
		glDrawElements(GL_TRIANGLES, handleMesh.nIndices, GL_UNSIGNED_INT, 0);

		

		

		// Render plane with rotation
		glm::mat4 planeModel = glm::mat4(1.0f); // Identity matrix
		planeModel = glm::rotate(planeModel, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		lightingShader.setMat4("model", planeModel);
		// Set the plane's texture uniform in the shader
		lightingShader.setInt("material.diffuse", 2);
		glBindVertexArray(planeMesh.vao);
		glDrawElements(GL_TRIANGLES, planeMesh.nIndices, GL_UNSIGNED_INT, 0);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, paperTexture);

		// Translate the cube's position
		glm::mat4 cubeModel = glm::mat4(1.0f);
		cubeModel = glm::translate(cubeModel, glm::vec3(1.0f, -0.5f, 0.0f));
		lightingShader.setMat4("model", cubeModel);

		// Set the cube's texture uniform in the shader
		lightingShader.setInt("material.diffuse", 2);

		// Render the cube
		glBindVertexArray(paper1Mesh.vao);
		glDrawArrays(GL_TRIANGLES, 0, paper1Mesh.nIndices);

		// Translate the cube's position
		glm::mat4 paperModel1 = glm::mat4(1.0f);
		paperModel1 = glm::translate(paperModel1, glm::vec3(1.0f, -0.5f, 0.0f));
		lightingShader.setMat4("model", paperModel1);

		// Set the cube's texture uniform in the shader
		lightingShader.setInt("material.diffuse", 2);

		// Render the cube
		glBindVertexArray(paper2Mesh.vao);
		glDrawArrays(GL_TRIANGLES, 0, paper2Mesh.nIndices);

		// Translate the cube's position
		glm::mat4 paperModel2 = glm::mat4(1.0f);
		paperModel2 = glm::translate(paperModel2, glm::vec3(-0.5f, -0.5f, 1.0f));
		lightingShader.setMat4("model", paperModel2);

		// Set the cube's texture uniform in the shader
		lightingShader.setInt("material.diffuse", 2);

		// Render the cube
		glBindVertexArray(paper3Mesh.vao);
		glDrawArrays(GL_TRIANGLES, 0, paper3Mesh.nIndices);


		// Translate the cube's position
		glm::mat4 paperModel3 = glm::mat4(1.0f);
		paperModel3 = glm::translate(paperModel3, glm::vec3(-1.5f, -0.5f, 1.0f));
		lightingShader.setMat4("model", paperModel3);

		// Set the cube's texture uniform in the shader
		lightingShader.setInt("material.diffuse", 2);

		// Render the cube
		glBindVertexArray(paper3Mesh.vao);
		glDrawArrays(GL_TRIANGLES, 0, paper3Mesh.nIndices);

		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, penTexture);

		// Translate the cube's position
		glm::mat4 penModel = glm::mat4(1.0f);
		penModel = glm::rotate(penModel, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		penModel = glm::translate(penModel, glm::vec3(-2.0f, 0.7f, 0.45f));
		lightingShader.setMat4("model", penModel);

		// Set the cube's texture uniform in the shader
		lightingShader.setInt("material.diffuse", 4);


		glBindVertexArray(penMesh.vao);
		glDrawElements(GL_TRIANGLES, penMesh.nIndices, GL_UNSIGNED_INT, 0);








		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// optional: de-allocate all resources once they've outlived their purpose:
	// ------------------------------------------------------------------------
	glDeleteVertexArrays(1, &cubeVAO);
	glDeleteVertexArrays(1, &lightCubeVAO);
	glDeleteBuffers(1, &VBO);

	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();
	return 0;
}

void UCreateCupMesh(GLMesh& mesh) {
	float baseRadius = 0.4f;
	float topRadius = 0.5f;
	float height = 1.0f;
	int numSegments = 50;

	int numVertices = (numSegments + 1) * 2 + 2;  // Additional vertices for the top and bottom centers
	int numIndices = numSegments * 6 + numSegments * 3 * 2;  // Additional indices for the top and bottom faces

	float* vertices = new float[numVertices * 8];
	unsigned int* indices = new unsigned int[numIndices];

	// Generate vertices and indices
	for (int i = 0; i <= numSegments; ++i) {
		float angle = 2.0f * glm::pi<float>() * static_cast<float>(i) / static_cast<float>(numSegments);
		float x = cos(angle);
		float z = sin(angle);

		// Calculate vertex positions for the bottom base
		vertices[i * 8] = x * baseRadius;
		vertices[i * 8 + 1] = -height / 2.0f;
		vertices[i * 8 + 2] = z * baseRadius;
		vertices[i * 8 + 3] = x;
		vertices[i * 8 + 4] = 0.0f;
		vertices[i * 8 + 5] = z;
		vertices[i * 8 + 6] = static_cast<float>(i) / static_cast<float>(numSegments);
		vertices[i * 8 + 7] = 0.0f;

		// Calculate vertex positions for the top base
		vertices[(i + numSegments + 1) * 8] = x * topRadius;
		vertices[(i + numSegments + 1) * 8 + 1] = height / 2.0f;
		vertices[(i + numSegments + 1) * 8 + 2] = z * topRadius;
		vertices[(i + numSegments + 1) * 8 + 3] = x;
		vertices[(i + numSegments + 1) * 8 + 4] = 0.0f;
		vertices[(i + numSegments + 1) * 8 + 5] = z;
		vertices[(i + numSegments + 1) * 8 + 6] = static_cast<float>(i) / static_cast<float>(numSegments);
		vertices[(i + numSegments + 1) * 8 + 7] = 1.0f;
	}

	// Add vertices for the top and bottom centers
	int centerBottomIndex = numVertices - 2;
	int centerTopIndex = numVertices - 1;
	vertices[centerBottomIndex * 8] = 0.0f;
	vertices[centerBottomIndex * 8 + 1] = -height / 2.0f;
	vertices[centerBottomIndex * 8 + 2] = 0.0f;
	vertices[centerBottomIndex * 8 + 3] = 0.0f;
	vertices[centerBottomIndex * 8 + 4] = -1.0f;
	vertices[centerBottomIndex * 8 + 5] = 0.0f;
	vertices[centerBottomIndex * 8 + 6] = 0.5f;
	vertices[centerBottomIndex * 8 + 7] = 0.0f;

	vertices[centerTopIndex * 8] = 0.0f;
	vertices[centerTopIndex * 8 + 1] = height / 2.0f;
	vertices[centerTopIndex * 8 + 2] = 0.0f;
	vertices[centerTopIndex * 8 + 3] = 0.0f;
	vertices[centerTopIndex * 8 + 4] = 1.0f;
	vertices[centerTopIndex * 8 + 5] = 0.0f;
	vertices[centerTopIndex * 8 + 6] = 0.5f;
	vertices[centerTopIndex * 8 + 7] = 1.0f;

	// Set indices
	int index = 0;
	for (int i = 0; i < numSegments; ++i) {
		indices[index++] = i;
		indices[index++] = i + numSegments + 1;
		indices[index++] = i + numSegments + 2;

		indices[index++] = i;
		indices[index++] = i + numSegments + 2;
		indices[index++] = i + 1;

		// Indices for bottom face
		indices[index++] = centerBottomIndex;
		indices[index++] = i;
		indices[index++] = i + 1;

		// Indices for top face
		indices[index++] = centerTopIndex;
		indices[index++] = i + numSegments + 2;
		indices[index++] = i + numSegments + 1;
	}

	glGenVertexArrays(1, &mesh.vao);
	glGenBuffers(1, &mesh.vbo);
	glGenBuffers(1, &mesh.ebo);

	glBindVertexArray(mesh.vao);

	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
	glBufferData(GL_ARRAY_BUFFER, numVertices * 8 * sizeof(float), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndices * sizeof(unsigned int), indices, GL_STATIC_DRAW);

	// Set vertex attribute pointers
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glBindVertexArray(0);

	mesh.nIndices = numIndices;

	delete[] vertices;
	delete[] indices;
}





void UCreateHandleMesh(GLMesh& mesh)
{
	float torusRadius = 0.4f;
	float tubeRadius = 0.05f;
	int torusSegments = 50;
	int tubeSegments = 20;

	// Calculate the number of vertices and indices required
	int numVertices = torusSegments * tubeSegments;
	int numIndices = numVertices * 6;

	// Create arrays to hold the vertex, normal, texture coordinate, and index data
	float* vertices = new float[numVertices * 8]; // 3 for position, 3 for normal, 2 for texture coordinates
	unsigned int* indices = new unsigned int[numIndices];

	// Generate the vertex and index data
	float torusAngleStep = 2.0f * glm::pi<float>() / torusSegments;
	float tubeAngleStep = 2.0f * glm::pi<float>() / tubeSegments;
	int vertexIndex = 0;
	int indexIndex = 0;

	for (int i = 0; i < torusSegments; ++i)
	{
		float torusAngle = i * torusAngleStep;

		for (int j = 0; j < tubeSegments; ++j)
		{
			float tubeAngle = j * tubeAngleStep;

			// Calculate the position of the vertex
			float x = (torusRadius + tubeRadius * cos(tubeAngle)) * cos(torusAngle);
			float y = tubeRadius * sin(tubeAngle);
			float z = (torusRadius + tubeRadius * cos(tubeAngle)) * sin(torusAngle);

			// Calculate the normal direction of the vertex
			glm::vec3 normal = glm::normalize(glm::vec3(x, y, z));

			// Calculate texture coordinates based on i and j
			float s = static_cast<float>(i) / static_cast<float>(torusSegments);
			float t = static_cast<float>(j) / static_cast<float>(tubeSegments);

			// Store the vertex position, normal, and texture coordinates
			vertices[vertexIndex] = x;
			vertices[vertexIndex + 1] = y;
			vertices[vertexIndex + 2] = z;
			vertices[vertexIndex + 3] = normal.x;
			vertices[vertexIndex + 4] = normal.y;
			vertices[vertexIndex + 5] = normal.z;
			vertices[vertexIndex + 6] = s;
			vertices[vertexIndex + 7] = t;

			vertexIndex += 8;

			// Calculate the indices of the surrounding vertices
			int nextTube = (j + 1) % tubeSegments;
			int nextTorus = (i + 1) % torusSegments;

			int topLeft = i * tubeSegments + j;
			int topRight = i * tubeSegments + nextTube;
			int bottomLeft = nextTorus * tubeSegments + j;
			int bottomRight = nextTorus * tubeSegments + nextTube;

			// Create two triangles for each quad
			indices[indexIndex++] = topLeft;
			indices[indexIndex++] = topRight;
			indices[indexIndex++] = bottomLeft;

			indices[indexIndex++] = bottomLeft;
			indices[indexIndex++] = topRight;
			indices[indexIndex++] = bottomRight;
		}
	}

	glGenVertexArrays(1, &mesh.vao);
	glGenBuffers(1, &mesh.vbo);
	glGenBuffers(1, &mesh.ebo);

	glBindVertexArray(mesh.vao);

	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
	glBufferData(GL_ARRAY_BUFFER, numVertices * 8 * sizeof(float), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndices * sizeof(unsigned int), indices, GL_STATIC_DRAW);

	// Set vertex attribute pointers
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glBindVertexArray(0);

	mesh.nIndices = numIndices;

	delete[] vertices;
	delete[] indices;
}



void UCreatePlaneMesh(GLMesh& mesh)
{
	// Define the vertices, texture coordinates, and indices for the plane here.
	float vertices[] = {
		// Position          // TexCoords
		-3.0f, -0.5f, -3.0f,  0.0f, 0.0f, // Bottom left
		 3.0f, -0.5f, -3.0f,  1.0f, 0.0f, // Bottom right
		 3.0f, -0.5f,  3.0f,  1.0f, 1.0f, // Top right
		-3.0f, -0.5f,  3.0f,  0.0f, 1.0f  // Top left
	};

	unsigned int indices[] = {
		0, 1, 2, // First triangle
		0, 2, 3  // Second triangle
	};

	// Create and bind the vertex array object (VAO)
	glGenVertexArrays(1, &mesh.vao);
	glBindVertexArray(mesh.vao);

	// Create and bind the vertex buffer object (VBO)
	glGenBuffers(1, &mesh.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// Create and bind the element buffer object (EBO)
	glGenBuffers(1, &mesh.ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// Set the vertex attribute pointers for position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// Set the vertex attribute pointers for texture coordinates
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// Unbind the VAO
	glBindVertexArray(0);

	// Set the number of indices
	mesh.nIndices = sizeof(indices) / sizeof(indices[0]);
}

void UCreatePaper1Mesh(GLMesh& mesh)
{
	// Define the vertices, texture coordinates, normals, and indices for a cube
	float vertices[] = {
		// Front face
		-1.0f, -0.01f,  1.5f,    0.0f, 0.0f,  1.0f,    0.0f, 0.0f, // Bottom-left
		 1.0f, -0.01f,  1.5f,    0.0f, 0.0f,  1.0f,    1.0f, 0.0f, // Bottom-right
		 1.0f,  0.01f,  1.5f,    0.0f, 0.0f,  1.0f,    1.0f, 1.0f, // Top-right
		 1.0f,  0.01f,  1.5f,    0.0f, 0.0f,  1.0f,    1.0f, 1.0f, // Top-right
		-1.0f,  0.01f,  1.5f,    0.0f, 0.0f,  1.0f,    0.0f, 1.0f, // Top-left
		-1.0f, -0.01f,  1.5f,    0.0f, 0.0f,  1.0f,    0.0f, 0.0f, // Bottom-left

		// Back face
		-1.0f, -0.01f, -1.5f,    0.0f, 0.0f, -1.0f,    0.0f, 0.0f, // Bottom-left
		 1.0f, -0.01f, -1.5f,    0.0f, 0.0f, -1.0f,    1.0f, 0.0f, // Bottom-right
		 1.0f,  0.01f, -1.5f,    0.0f, 0.0f, -1.0f,    1.0f, 1.0f, // Top-right
		 1.0f,  0.01f, -1.5f,    0.0f, 0.0f, -1.0f,    1.0f, 1.0f, // Top-right
		-1.0f,  0.01f, -1.5f,    0.0f, 0.0f, -1.0f,    0.0f, 1.0f, // Top-left
		-1.0f, -0.01f, -1.5f,    0.0f, 0.0f, -1.0f,    0.0f, 0.0f, // Bottom-left

		// Left face
		-1.0f,  0.01f,  1.5f,   -1.0f, 0.0f,  0.0f,    1.0f, 0.0f, // Top-right
		-1.0f,  0.01f, -1.5f,   -1.0f, 0.0f,  0.0f,    0.0f, 0.0f, // Top-left
		-1.0f, -0.01f, -1.5f,   -1.0f, 0.0f,  0.0f,    0.0f, 1.0f, // Bottom-left
		-1.0f, -0.01f, -1.5f,   -1.0f, 0.0f,  0.0f,    0.0f, 1.0f, // Bottom-left
		-1.0f, -0.01f,  1.5f,   -1.0f, 0.0f,  0.0f,    1.0f, 1.0f, // Bottom-right
		-1.0f,  0.01f,  1.5f,   -1.0f, 0.0f,  0.0f,    1.0f, 0.0f, // Top-right

		// Right face
		 1.0f,  0.01f,  1.5f,    1.0f, 0.0f,  0.0f,    0.0f, 0.0f, // Top-left
		 1.0f,  0.01f, -1.5f,    1.0f, 0.0f,  0.0f,    1.0f, 0.0f, // Top-right
		 1.0f, -0.01f, -1.5f,    1.0f, 0.0f,  0.0f,    1.0f, 1.0f, // Bottom-right
		 1.0f, -0.01f, -1.5f,    1.0f, 0.0f,  0.0f,    1.0f, 1.0f, // Bottom-right
		 1.0f, -0.01f,  1.5f,    1.0f, 0.0f,  0.0f,    0.0f, 1.0f, // Bottom-left
		 1.0f,  0.01f,  1.5f,    1.0f, 0.0f,  0.0f,    0.0f, 0.0f, // Top-left

		// Top face
		-1.0f,  0.01f, -1.5f,    0.0f, 1.0f,  0.0f,    0.0f, 1.0f, // Bottom-left
		 1.0f,  0.01f, -1.5f,    0.0f, 1.0f,  0.0f,     1.0f, 1.0f, // Bottom-right
		 1.0f,  0.01f,  1.5f,    0.0f, 1.0f,  0.0f,     1.0f, 0.0f, // Top-right
		 1.0f,  0.01f,  1.5f,    0.0f, 1.0f,  0.0f,     1.0f, 0.0f, // Top-right
		-1.0f,  0.01f,  1.5f,    0.0f, 1.0f,  0.0f,     0.0f, 0.0f, // Top-left
		-1.0f,  0.01f, -1.5f,    0.0f, 1.0f,  0.0f,     0.0f, 1.0f, // Bottom-left

		// Bottom face
		-1.0f, -0.01f, -1.5f,    0.0f, -1.0f,  0.0f,    0.0f, 1.0f, // Top-left
		 1.0f, -0.01f, -1.5f,    0.0f, -1.0f,  0.0f,    1.0f, 1.0f, // Top-right
		 1.0f, -0.01f,  1.5f,    0.0f, -1.0f,  0.0f,    1.0f, 0.0f, // Bottom-right
		 1.0f, -0.01f,  1.5f,    0.0f, -1.0f,  0.0f,    1.0f, 0.0f, // Bottom-right
		-1.0f, -0.01f,  1.5f,    0.0f, -1.0f,  0.0f,    0.0f, 0.0f, // Bottom-left
		-1.0f, -0.01f, -1.5f,    0.0f, -1.0f,  0.0f,    0.0f, 1.0f, // Top-left
	};

	// Create and bind the vertex array object (VAO)
	glGenVertexArrays(1, &mesh.vao);
	glBindVertexArray(mesh.vao);

	// Create and bind the vertex buffer object (VBO)
	glGenBuffers(1, &mesh.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// Set the vertex attribute pointers for position, normal, and texture coordinates
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	// Unbind the VAO
	glBindVertexArray(0);

	// Set the number of indices
	mesh.nIndices = 36; // 6 sides * 2 triangles per side * 3 vertices per triangle
}

void UCreatePaper2Mesh(GLMesh& mesh)
{
	// Define the vertices, texture coordinates, normals, and indices for a paper mesh
	float vertices[] = {
		// Front face
		-0.333f, -0.01f,  0.5f,    0.0f, 0.0f,  1.0f,    0.0f, 0.0f, // Bottom-left
		 0.333f, -0.01f,  0.5f,    0.0f, 0.0f,  1.0f,    1.0f, 0.0f, // Bottom-right
		 0.333f,  0.01f,  0.5f,    0.0f, 0.0f,  1.0f,    1.0f, 1.0f, // Top-right
		 0.333f,  0.01f,  0.5f,    0.0f, 0.0f,  1.0f,    1.0f, 1.0f, // Top-right
		-0.333f,  0.01f,  0.5f,    0.0f, 0.0f,  1.0f,    0.0f, 1.0f, // Top-left
		-0.333f, -0.01f,  0.5f,    0.0f, 0.0f,  1.0f,    0.0f, 0.0f, // Bottom-left

		// Back face
		-0.333f, -0.01f, -0.5f,    0.0f, 0.0f, -1.0f,    0.0f, 0.0f, // Bottom-left
		 0.333f, -0.01f, -0.5f,    0.0f, 0.0f, -1.0f,    1.0f, 0.0f, // Bottom-right
		 0.333f,  0.01f, -0.5f,    0.0f, 0.0f, -1.0f,    1.0f, 1.0f, // Top-right
		 0.333f,  0.01f, -0.5f,    0.0f, 0.0f, -1.0f,    1.0f, 1.0f, // Top-right
		-0.333f,  0.01f, -0.5f,    0.0f, 0.0f, -1.0f,    0.0f, 1.0f, // Top-left
		-0.333f, -0.01f, -0.5f,    0.0f, 0.0f, -1.0f,    0.0f, 0.0f, // Bottom-left

		// Left face
		-0.333f,  0.01f,  0.5f,   -1.0f, 0.0f,  0.0f,    1.0f, 0.0f, // Top-right
		-0.333f,  0.01f, -0.5f,   -1.0f, 0.0f,  0.0f,    0.0f, 0.0f, // Top-left
		-0.333f, -0.01f, -0.5f,   -1.0f, 0.0f,  0.0f,    0.0f, 1.0f, // Bottom-left
		-0.333f, -0.01f, -0.5f,   -1.0f, 0.0f,  0.0f,    0.0f, 1.0f, // Bottom-left
		-0.333f, -0.01f,  0.5f,   -1.0f, 0.0f,  0.0f,    1.0f, 1.0f, // Bottom-right
		-0.333f,  0.01f,  0.5f,   -1.0f, 0.0f,  0.0f,    1.0f, 0.0f, // Top-right

		// Right face
		 0.333f,  0.01f,  0.5f,    1.0f, 0.0f,  0.0f,    0.0f, 0.0f, // Top-left
		 0.333f,  0.01f, -0.5f,    1.0f, 0.0f,  0.0f,    1.0f, 0.0f, // Top-right
		 0.333f, -0.01f, -0.5f,    1.0f, 0.0f,  0.0f,    1.0f, 1.0f, // Bottom-right
		 0.333f, -0.01f, -0.5f,    1.0f, 0.0f,  0.0f,    1.0f, 1.0f, // Bottom-right
		 0.333f, -0.01f,  0.5f,    1.0f, 0.0f,  0.0f,    0.0f, 1.0f, // Bottom-left
		 0.333f,  0.01f,  0.5f,    1.0f, 0.0f,  0.0f,    0.0f, 0.0f, // Top-left

		// Top face
		-0.333f,  0.01f, -0.5f,    0.0f, 1.0f,  0.0f,    0.0f, 1.0f, // Bottom-left
		 0.333f,  0.01f, -0.5f,    0.0f, 1.0f,  0.0f,    1.0f, 1.0f, // Bottom-right
		 0.333f,  0.01f,  0.5f,    0.0f, 1.0f,  0.0f,    1.0f, 0.0f, // Top-right
		 0.333f,  0.01f,  0.5f,    0.0f, 1.0f,  0.0f,    1.0f, 0.0f, // Top-right
		-0.333f,  0.01f,  0.5f,    0.0f, 1.0f,  0.0f,    0.0f, 0.0f, // Top-left
		-0.333f,  0.01f, -0.5f,    0.0f, 1.0f,  0.0f,    0.0f, 1.0f, // Bottom-left

		// Bottom face
		-0.333f, -0.01f, -0.5f,    0.0f, -1.0f,  0.0f,    0.0f, 1.0f, // Top-left
		 0.333f, -0.01f, -0.5f,    0.0f, -1.0f,  0.0f,    1.0f, 1.0f, // Top-right
		 0.333f, -0.01f,  0.5f,    0.0f, -1.0f,  0.0f,    1.0f, 0.0f, // Bottom-right
		 0.333f, -0.01f,  0.5f,    0.0f, -1.0f,  0.0f,    1.0f, 0.0f, // Bottom-right
		-0.333f, -0.01f,  0.5f,    0.0f, -1.0f,  0.0f,    0.0f, 0.0f, // Bottom-left
		-0.333f, -0.01f, -0.5f,    0.0f, -1.0f,  0.0f,    0.0f, 1.0f, // Top-left
	};


	// Create and bind the vertex array object (VAO)
	glGenVertexArrays(1, &mesh.vao);
	glBindVertexArray(mesh.vao);

	// Create and bind the vertex buffer object (VBO)
	glGenBuffers(1, &mesh.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// Set the vertex attribute pointers for position, normal, and texture coordinates
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	// Unbind the VAO
	glBindVertexArray(0);

	// Set the number of indices
	mesh.nIndices = 36; // 6 sides * 2 triangles per side * 3 vertices per triangle
}

void UCreatePaper3Mesh(GLMesh& mesh)
{
	// Define the vertices, texture coordinates, normals, and indices for a paper mesh
	float vertices[] = {
		// Front face
		-0.333f, -0.01f,  0.5f,    0.0f, 0.0f,  1.0f,    0.0f, 0.0f, // Bottom-left
		 0.333f, -0.01f,  0.5f,    0.0f, 0.0f,  1.0f,    1.0f, 0.0f, // Bottom-right
		 0.333f,  0.01f,  0.5f,    0.0f, 0.0f,  1.0f,    1.0f, 1.0f, // Top-right
		 0.333f,  0.01f,  0.5f,    0.0f, 0.0f,  1.0f,    1.0f, 1.0f, // Top-right
		-0.333f,  0.01f,  0.5f,    0.0f, 0.0f,  1.0f,    0.0f, 1.0f, // Top-left
		-0.333f, -0.01f,  0.5f,    0.0f, 0.0f,  1.0f,    0.0f, 0.0f, // Bottom-left

		// Back face
		-0.333f, -0.01f, -0.5f,    0.0f, 0.0f, -1.0f,    0.0f, 0.0f, // Bottom-left
		 0.333f, -0.01f, -0.5f,    0.0f, 0.0f, -1.0f,    1.0f, 0.0f, // Bottom-right
		 0.333f,  0.01f, -0.5f,    0.0f, 0.0f, -1.0f,    1.0f, 1.0f, // Top-right
		 0.333f,  0.01f, -0.5f,    0.0f, 0.0f, -1.0f,    1.0f, 1.0f, // Top-right
		-0.333f,  0.01f, -0.5f,    0.0f, 0.0f, -1.0f,    0.0f, 1.0f, // Top-left
		-0.333f, -0.01f, -0.5f,    0.0f, 0.0f, -1.0f,    0.0f, 0.0f, // Bottom-left

		// Left face
		-0.333f,  0.01f,  0.5f,   -1.0f, 0.0f,  0.0f,    1.0f, 0.0f, // Top-right
		-0.333f,  0.01f, -0.5f,   -1.0f, 0.0f,  0.0f,    0.0f, 0.0f, // Top-left
		-0.333f, -0.01f, -0.5f,   -1.0f, 0.0f,  0.0f,    0.0f, 1.0f, // Bottom-left
		-0.333f, -0.01f, -0.5f,   -1.0f, 0.0f,  0.0f,    0.0f, 1.0f, // Bottom-left
		-0.333f, -0.01f,  0.5f,   -1.0f, 0.0f,  0.0f,    1.0f, 1.0f, // Bottom-right
		-0.333f,  0.01f,  0.5f,   -1.0f, 0.0f,  0.0f,    1.0f, 0.0f, // Top-right

		// Right face
		 0.333f,  0.01f,  0.5f,    1.0f, 0.0f,  0.0f,    0.0f, 0.0f, // Top-left
		 0.333f,  0.01f, -0.5f,    1.0f, 0.0f,  0.0f,    1.0f, 0.0f, // Top-right
		 0.333f, -0.01f, -0.5f,    1.0f, 0.0f,  0.0f,    1.0f, 1.0f, // Bottom-right
		 0.333f, -0.01f, -0.5f,    1.0f, 0.0f,  0.0f,    1.0f, 1.0f, // Bottom-right
		 0.333f, -0.01f,  0.5f,    1.0f, 0.0f,  0.0f,    0.0f, 1.0f, // Bottom-left
		 0.333f,  0.01f,  0.5f,    1.0f, 0.0f,  0.0f,    0.0f, 0.0f, // Top-left

		// Top face
		-0.333f,  0.01f, -0.5f,    0.0f, 1.0f,  0.0f,    0.0f, 1.0f, // Bottom-left
		 0.333f,  0.01f, -0.5f,    0.0f, 1.0f,  0.0f,    1.0f, 1.0f, // Bottom-right
		 0.333f,  0.01f,  0.5f,    0.0f, 1.0f,  0.0f,    1.0f, 0.0f, // Top-right
		 0.333f,  0.01f,  0.5f,    0.0f, 1.0f,  0.0f,    1.0f, 0.0f, // Top-right
		-0.333f,  0.01f,  0.5f,    0.0f, 1.0f,  0.0f,    0.0f, 0.0f, // Top-left
		-0.333f,  0.01f, -0.5f,    0.0f, 1.0f,  0.0f,    0.0f, 1.0f, // Bottom-left

		// Bottom face
		-0.333f, -0.01f, -0.5f,    0.0f, -1.0f,  0.0f,    0.0f, 1.0f, // Top-left
		 0.333f, -0.01f, -0.5f,    0.0f, -1.0f,  0.0f,    1.0f, 1.0f, // Top-right
		 0.333f, -0.01f,  0.5f,    0.0f, -1.0f,  0.0f,    1.0f, 0.0f, // Bottom-right
		 0.333f, -0.01f,  0.5f,    0.0f, -1.0f,  0.0f,    1.0f, 0.0f, // Bottom-right
		-0.333f, -0.01f,  0.5f,    0.0f, -1.0f,  0.0f,    0.0f, 0.0f, // Bottom-left
		-0.333f, -0.01f, -0.5f,    0.0f, -1.0f,  0.0f,    0.0f, 1.0f, // Top-left
	};


	// Create and bind the vertex array object (VAO)
	glGenVertexArrays(1, &mesh.vao);
	glBindVertexArray(mesh.vao);

	// Create and bind the vertex buffer object (VBO)
	glGenBuffers(1, &mesh.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// Set the vertex attribute pointers for position, normal, and texture coordinates
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	// Unbind the VAO
	glBindVertexArray(0);

	// Set the number of indices
	mesh.nIndices = 36; // 6 sides * 2 triangles per side * 3 vertices per triangle
}

void UCreatePenMesh(GLMesh& mesh) {
    float baseRadius = 0.05f;
    float topRadius = 0.05f;
    float height = 1.0f;
    int numSegments = 50;

    int numVertices = (numSegments + 1) * 2 + 2;  // Additional vertices for the top and bottom centers
    int numIndices = numSegments * 6 + numSegments * 3 * 2;  // Additional indices for the top and bottom faces

    float* vertices = new float[numVertices * 8];
    unsigned int* indices = new unsigned int[numIndices];

    // Generate vertices and indices
    for (int i = 0; i <= numSegments; ++i) {
        float angle = 2.0f * glm::pi<float>() * static_cast<float>(i) / static_cast<float>(numSegments);
        float x = cos(angle);
        float z = sin(angle);

        // Calculate vertex positions for the bottom base
        vertices[i * 8] = x * baseRadius;
        vertices[i * 8 + 1] = -height / 2.0f;
        vertices[i * 8 + 2] = z * baseRadius;
        vertices[i * 8 + 3] = x;
        vertices[i * 8 + 4] = 0.0f;
        vertices[i * 8 + 5] = z;
        vertices[i * 8 + 6] = static_cast<float>(i) / static_cast<float>(numSegments);
        vertices[i * 8 + 7] = 0.0f;

        // Calculate vertex positions for the top base
        vertices[(i + numSegments + 1) * 8] = x * topRadius;
        vertices[(i + numSegments + 1) * 8 + 1] = height / 2.0f;
        vertices[(i + numSegments + 1) * 8 + 2] = z * topRadius;
        vertices[(i + numSegments + 1) * 8 + 3] = x;
        vertices[(i + numSegments + 1) * 8 + 4] = 0.0f;
        vertices[(i + numSegments + 1) * 8 + 5] = z;
        vertices[(i + numSegments + 1) * 8 + 6] = static_cast<float>(i) / static_cast<float>(numSegments);
        vertices[(i + numSegments + 1) * 8 + 7] = 1.0f;
    }

    // Add vertices for the top and bottom centers
    int centerBottomIndex = numVertices - 2;
    int centerTopIndex = numVertices - 1;
    vertices[centerBottomIndex * 8] = 0.0f;
    vertices[centerBottomIndex * 8 + 1] = -height / 2.0f;
    vertices[centerBottomIndex * 8 + 2] = 0.0f;
    vertices[centerBottomIndex * 8 + 3] = 0.0f;
    vertices[centerBottomIndex * 8 + 4] = -1.0f;
    vertices[centerBottomIndex * 8 + 5] = 0.0f;
    vertices[centerBottomIndex * 8 + 6] = 0.5f;
    vertices[centerBottomIndex * 8 + 7] = 0.0f;

    vertices[centerTopIndex * 8] = 0.0f;
    vertices[centerTopIndex * 8 + 1] = height / 2.0f;
    vertices[centerTopIndex * 8 + 2] = 0.0f;
    vertices[centerTopIndex * 8 + 3] = 0.0f;
    vertices[centerTopIndex * 8 + 4] = 1.0f;
    vertices[centerTopIndex * 8 + 5] = 0.0f;
    vertices[centerTopIndex * 8 + 6] = 0.5f;
    vertices[centerTopIndex * 8 + 7] = 1.0f;

    // Set indices
    int index = 0;
    for (int i = 0; i < numSegments; ++i) {
        indices[index++] = i;
        indices[index++] = i + numSegments + 1;
        indices[index++] = i + numSegments + 2;

        indices[index++] = i;
        indices[index++] = i + numSegments + 2;
        indices[index++] = i + 1;

        // Indices for bottom face
        indices[index++] = centerBottomIndex;
        indices[index++] = i;
        indices[index++] = i + 1;

        // Indices for top face
        indices[index++] = centerTopIndex;
        indices[index++] = i + numSegments + 2;
        indices[index++] = i + numSegments + 1;
    }

    glGenVertexArrays(1, &mesh.vao);
    glGenBuffers(1, &mesh.vbo);
    glGenBuffers(1, &mesh.ebo);

    glBindVertexArray(mesh.vao);

    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glBufferData(GL_ARRAY_BUFFER, numVertices * 8 * sizeof(float), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndices * sizeof(unsigned int), indices, GL_STATIC_DRAW);

    // Set vertex attribute pointers
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    mesh.nIndices = numIndices;

    delete[] vertices;
    delete[] indices;
}











// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		(birdEyeView ? birdEyeCamera : camera).ProcessKeyboard(FORWARD, deltaTime * cameraSpeed);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		(birdEyeView ? birdEyeCamera : camera).ProcessKeyboard(BACKWARD, deltaTime * cameraSpeed);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		(birdEyeView ? birdEyeCamera : camera).ProcessKeyboard(LEFT, deltaTime * cameraSpeed);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		(birdEyeView ? birdEyeCamera : camera).ProcessKeyboard(RIGHT, deltaTime * cameraSpeed);

	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS && !birdEyeKeyPressed) {
        birdEyeView = !birdEyeView;
        birdEyeKeyPressed = true;
    }
    
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_RELEASE) {
        birdEyeKeyPressed = false;
    }

	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		(birdEyeView ? birdEyeCamera : camera).ProcessKeyboard(UP, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		(birdEyeView ? birdEyeCamera : camera).ProcessKeyboard(DOWN, deltaTime);
}



// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{

	// Adjust the camera speed based on scroll input
	if (cameraSpeed >= 1.0f && cameraSpeed <= 10.0f)
		cameraSpeed += yoffset * 0.1f;
	if (cameraSpeed < 1.0f)
		cameraSpeed = 1.0f;
	if (cameraSpeed > 10.0f)
		cameraSpeed = 10.0f;
}


// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int loadTexture(char const* path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}
