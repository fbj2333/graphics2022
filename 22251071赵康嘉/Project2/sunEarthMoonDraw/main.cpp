
#include "glew/glew.h"
#include "glfw/glfw3.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "Utils.h"
#include "Sphere.h"
#include "camera.h"
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <stack>

using namespace std;

static const int screen_width = 1920;
static const int screen_height = 1080;
int width = 0;
int height = 0;

GLuint renderingProgram = 0;

static const int numberVAOs = 1;
static const int numberVBOs = 3;
GLuint vao[numberVAOs] = { 0 };
GLuint vbo[numberVBOs] = { 0 };

float cameraLocX = 0.f, cameraLocY = 0.f, cameraLocZ = 0.f;
float sunLocX = 0.f, sunLocY = 0.f, sunLocZ = 0.f;
float earthLocX = 0.f, earthLocY = 0.f, earthLocZ = 0.f;
float moonLocX = 0.f, moonLocY = 0.f, moonLocZ = 0.f;
float aspect = 0.f;
int mvLoc = 0;
int projLoc = 0;
float rotAmt = 0.f;

glm::mat4 mMat(1.f), vMat(1.f), pMat(1.f), mvMat(1.f);

//vector<glm::mat4> mvStack;

GLuint sunTextureId = 0, earthTextureId = 0, moonTextureId = 0;

Sphere sun = Sphere(48);
Sphere earth = Sphere(48);
Sphere moon = Sphere(48);

GLboolean keys[1024] = { GL_FALSE };

Camera camera(glm::vec3(0.f, 0.f, 5.f));
GLfloat lastFrame = 0.0f;
GLfloat deltaTime = 0.0f;

GLboolean firstMouse = GL_TRUE;
GLfloat lastX = 0.f;
GLfloat lastY = 0.f;

std::stack<glm::mat4> mvStack;  //注意不要和vsctor<glm::mat4>相混淆


void setupVertices()
{
	vector<int> ind = sun.getIndices();
	vector<glm::vec3> vert = sun.getVertices();
	vector<glm::vec2> text = sun.getTexCoords();
	vector<glm::vec3> norm = sun.getNormals();
	vector<glm::vec3> tang = sun.getTangents();

	vector<float> pValues;   //顶点位置
	vector<float> tValues;   //纹理坐标
	vector<float> nValues;   //法线向量

	int numIndices = sun.getNumIndices();
	for (int i = 0; i < numIndices; i++)
	{
		//pValues.push_back(vert[ind[i]].x);
		pValues.push_back((vert[ind[i]]).x);
		pValues.push_back((vert[ind[i]]).y);
		pValues.push_back((vert[ind[i]]).z);

		tValues.push_back((text[ind[i]]).s);
		tValues.push_back((text[ind[i]]).t);

		nValues.push_back((norm[ind[i]]).x);
		nValues.push_back((norm[ind[i]]).y);
		nValues.push_back((norm[ind[i]]).z);
	}

	glGenVertexArrays(numberVAOs, vao);
	glBindVertexArray(vao[0]);

	glGenBuffers(numberVBOs, vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	size_t sizeFloat = sizeof(float);
	glBufferData(GL_ARRAY_BUFFER, pValues.size() * sizeof(float), &(pValues[0]), GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, tValues.size() * sizeof(float), &(tValues[0]), GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
	glBufferData(GL_ARRAY_BUFFER, nValues.size() * sizeof(float), &(nValues[0]), GL_STATIC_DRAW);
}

void press_key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if ((key == GLFW_KEY_ESCAPE) && (action == GLFW_PRESS))
	{
		glfwSetWindowShouldClose(window, GL_TRUE);
	}
	if (action == GLFW_PRESS)
	{
		keys[key] = GL_TRUE;
	}
	else if (action == GLFW_RELEASE)
	{
		//glfwSetWindowShouldClose(window, GL_FALSE);
		keys[key] = GL_FALSE;
	}
}

void key_movement_callback()
{
	if (keys[GLFW_KEY_W])
	{
		camera.ProcessKeyboard(FORWARD, deltaTime);
	}
	if (keys[GLFW_KEY_S])
	{
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	}
	if (keys[GLFW_KEY_A])
	{
		camera.ProcessKeyboard(LEFT, deltaTime);
	}
	if (keys[GLFW_KEY_D])
	{
		camera.ProcessKeyboard(RIGHT, deltaTime);
	}
}

void mouse_move_callback(GLFWwindow* window, double xPos, double yPos)
{
	if (firstMouse)
	{
		lastX = xPos;
		lastY = yPos;
		firstMouse = GL_FALSE;
	}

	GLfloat xOffset = xPos - lastX;
	GLfloat yOffset = lastY - yPos;

	lastX = xPos;
	lastY = yPos;

	camera.ProcessMouseMovement(xOffset, yOffset);
}

void scroll_callback(GLFWwindow* window, double xPos, double yPos)
{
	camera.ProcessMouseScroll(xPos);
}

void init(GLFWwindow* window)
{
	renderingProgram = Utils::createShaderProgram("vertShader.glsl", "fragShader.glsl");
	glfwGetFramebufferSize(window, &width, &height);
	aspect = (float)width / (float)height;
	pMat = glm::perspective(glm::radians(45.f), aspect, 0.01f, 1000.f);
	glfwGetFramebufferSize(window, &width, &height);


	setupVertices();

	sunTextureId = Utils::loadTexture("resource/shining.jpg");
	earthTextureId = Utils::loadTexture("resource/earth.jpg");
	moonTextureId = Utils::loadTexture("resource/moon.jpg");
}


void display(GLFWwindow* window, float currentTime)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.1f, 0.1f, 0.4f, 1.f);

	//必不可少！！！否则窗口是黑的，不会渲染任何东西
	glUseProgram(renderingProgram);

	GLfloat currentTimeFrame = glfwGetTime();
	deltaTime = currentTimeFrame - lastFrame;
	lastFrame = currentTimeFrame;

	mvLoc = glGetUniformLocation(renderingProgram, "mv_matrix");
	projLoc = glGetUniformLocation(renderingProgram, "proj_matrix");

	key_movement_callback();

	//移动相机矩阵:这里必须是-cameraZ，否则相机看不到球体
	//vMat = glm::translate(glm::mat4(1.f), glm::vec3(cameraX, cameraY, -cameraZ));
	vMat = camera.GetViewMatrix();
	/*sunLocZ = -6.f;*/
	mMat = glm::translate(glm::mat4(1.f), glm::vec3(sunLocX, sunLocY, sunLocZ));
	mMat = glm::rotate(glm::mat4(1.f), currentTime * 0.1f, glm::vec3(0.f, 1.f, 0.f));
	pMat = glm::perspective(camera.Zoom, (GLfloat)screen_width / (GLfloat)screen_height, 0.01f, 1000.f);

	//右乘规则
	//mvMat = mMat * vMat;   //金字塔会离开视口
	mvMat = vMat * mMat;

	//将视图矩阵压入堆栈
	mvStack.push(vMat);

	//更改一个uniform矩阵变量或数组的值。要更改的uniform变量的位置由location指定，location的值应该由glGetUniformLocation函数返回
	// 将透视矩阵和MV 矩阵复制给相应的统一变量
	/*通过一致变量（uniform修饰的变量）引用将一致变量值传入渲染管线。
	  location : uniform的位置。
	  count : 需要加载数据的数组元素的数量或者需要修改的矩阵的数量。
	  transpose : 指明矩阵是列优先(column major)矩阵（GL_FALSE）还是行优先(row major)矩阵（GL_TRUE）。
	  value : 指向由count个元素的数组的指针。
	*/
	glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat));

	// ----------------------  sun
	/*堆栈顶部的矩阵复制一份，并和其他的变换结合，然后再利用这个命令把新的矩阵副本推入堆栈中。
	*/
	mvStack.push(mvStack.top());
	//向矩阵堆栈顶部的矩阵添加平移
	mvStack.top() *= glm::translate(glm::mat4(1.f), glm::vec3(0.f, 0.f, 0.f));
	mvStack.push(mvStack.top());
	//在不移除的情况下，返回堆栈最顶部旋转矩阵（绕X轴）的引用。
	mvStack.top() *= glm::rotate(glm::mat4(1.f), (float)glfwGetTime() * 0.4f, glm::vec3(0.f, 1.f, 0.f));
	glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvStack.top()));

	//绑定到太阳顶点缓冲区
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	//指定了渲染时索引值为 index 的顶点属性数组的数据格式和位置
	/*Parameters
	index
		指定要修改的顶点属性的索引值

		size
		指定每个顶点属性的组件数量。必须为1、2、3或者4。初始值为4。（梦维：如position是由3个（x, y, z）组成，而颜色是4个（r, g, b, a））

		type
		指定数组中每个组件的数据类型。可用的符号常量有GL_BYTE, GL_UNSIGNED_BYTE, GL_SHORT, GL_UNSIGNED_SHORT, GL_FIXED, 和 GL_FLOAT，初始值为GL_FLOAT。

		normalized
		指定当被访问时，固定点数据值是否应该被归一化（GL_TRUE）或者直接转换为固定点值（GL_FALSE）。

		stride
		指定连续顶点属性之间的偏移量。如果为0，那么顶点属性会被理解为：它们是紧密排列在一起的。初始值为0。

		pointer
		指定一个指针，指向数组中第一个顶点属性的第一个组件。初始值为0。*/
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	//启用或禁用通用顶点属性数组,参数0索引和着色器中的layout(location = 0）中的0相对应,顶点位置
	glEnableVertexAttribArray(0);

	//绑定到纹理坐标缓冲区
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	//激活纹理坐标 
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, sunTextureId);

	/*glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_NEAREST_MIPMAP_NEAREST);
	glBindTexture(GL_TEXTURE_2D, sunTextureId);*/

	//背面剔除，默认情况下，背面剔除是关闭的
	//glEnable(GL_CULL_FACE);
	//glFrontFace(GL_CCW);

	glDrawArrays(GL_TRIANGLES, 0, sun.getNumIndices());
	mvStack.pop();


	/**********************************   earth           */
	//复制一份堆栈顶部矩阵压入堆栈
	/*push()：在堆栈顶部创建一个新的条目。我们通常会把目前在堆栈顶部的矩阵复制一份，
	  并和其他的变换结合，然后再利用这个命令把新的矩阵副本推入堆栈中。
	*/
	//top()：在不移除的情况下，返回堆栈最顶部矩阵的引用。
	mvStack.push(mvStack.top());
	mvStack.top() *= glm::translate(glm::mat4(1.f), glm::vec3(glm::sin((float)currentTime) * 2.5f, glm::cos((float)currentTime) * 0.4, glm::cos((float)currentTime) * 0.5f));
	mvStack.push(mvStack.top());
	//地球绕太阳旋转，即绕Y轴旋转
	mvStack.top() *= glm::rotate(glm::mat4(1.f), (float)currentTime, glm::vec3(0.f, 1.f, 0.f));
	mvStack.top() *= glm::scale(glm::mat4(1.f), glm::vec3(0.4f));
	//更改一个uniform矩阵变量或数组的值。要更改的uniform变量的位置由location指定，location的值应该由glGetUniformLocation函数返回
	// 将透视矩阵和MV 矩阵复制给相应的统一变量
	/*通过一致变量（uniform修饰的变量）引用将一致变量值传入渲染管线。
	  location : uniform的位置。
	  count : 需要加载数据的数组元素的数量或者需要修改的矩阵的数量。
	  transpose : 指明矩阵是列优先(column major)矩阵（GL_FALSE）还是行优先(row major)矩阵（GL_TRUE）。
	  value : 指向由count个元素的数组的指针。
	*/
	glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	//指定了渲染时索引值为 index 的顶点属性数组的数据格式和位置
	/*Parameters
	index
		指定要修改的顶点属性的索引值

		size
		指定每个顶点属性的组件数量。必须为1、2、3或者4。初始值为4。（梦维：如position是由3个（x, y, z）组成，而颜色是4个（r, g, b, a））

		type
		指定数组中每个组件的数据类型。可用的符号常量有GL_BYTE, GL_UNSIGNED_BYTE, GL_SHORT, GL_UNSIGNED_SHORT, GL_FIXED, 和 GL_FLOAT，初始值为GL_FLOAT。

		normalized
		指定当被访问时，固定点数据值是否应该被归一化（GL_TRUE）或者直接转换为固定点值（GL_FALSE）。

		stride
		指定连续顶点属性之间的偏移量。如果为0，那么顶点属性会被理解为：它们是紧密排列在一起的。初始值为0。

		pointer
		指定一个指针，指向数组中第一个顶点属性的第一个组件。初始值为0。*/
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	//启用或禁用通用顶点属性数组,参数0索引和着色器中的layout(location = 0）中的0相对应
	glEnableVertexAttribArray(0);

	//绑定到地球纹理坐标缓冲区
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, earthTextureId);
	//背面剔除，默认情况下，背面剔除是关闭的
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glDrawArrays(GL_TRIANGLES, 0, earth.getNumIndices());
	mvStack.pop();
	mvStack.pop();


	/****************************************   moon */
	//复制一份堆栈顶部矩阵压入堆栈
	/*push()：在堆栈顶部创建一个新的条目。我们通常会把目前在堆栈顶部的矩阵复制一份，
	  并和其他的变换结合，然后再利用这个命令把新的矩阵副本推入堆栈中。
	*/
	//top()：在不移除的情况下，返回堆栈最顶部矩阵的引用。
	mvStack.push(mvStack.top());
	mvStack.top() *= glm::translate(glm::mat4(1.f), glm::vec3(glm::sin((float)currentTime) * 3.f, glm::cos((float)currentTime) * 0.8f, glm::cos((float)currentTime) * 3.0f));
	mvStack.push(mvStack.top());
	//月球绕地球旋转，即绕Y轴旋转
	mvStack.top() *= glm::rotate(glm::mat4(1.f), (float)currentTime * 4.f, glm::vec3(0.f, 1.f, 0.f));
	mvStack.top() *= glm::scale(glm::mat4(1.f), glm::vec3(0.2f));
	//更改一个uniform矩阵变量或数组的值。要更改的uniform变量的位置由location指定，location的值应该由glGetUniformLocation函数返回
	// 将透视矩阵和MV 矩阵复制给相应的统一变量
	/*通过一致变量（uniform修饰的变量）引用将一致变量值传入渲染管线。
	  location : uniform的位置。
	  count : 需要加载数据的数组元素的数量或者需要修改的矩阵的数量。
	  transpose : 指明矩阵是列优先(column major)矩阵（GL_FALSE）还是行优先(row major)矩阵（GL_TRUE）。
	  value : 指向由count个元素的数组的指针。
	*/
	glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	//指定了渲染时索引值为 index 的顶点属性数组的数据格式和位置
	/*Parameters
	index
		指定要修改的顶点属性的索引值

		size
		指定每个顶点属性的组件数量。必须为1、2、3或者4。初始值为4。（梦维：如position是由3个（x, y, z）组成，而颜色是4个（r, g, b, a））

		type
		指定数组中每个组件的数据类型。可用的符号常量有GL_BYTE, GL_UNSIGNED_BYTE, GL_SHORT, GL_UNSIGNED_SHORT, GL_FIXED, 和 GL_FLOAT，初始值为GL_FLOAT。

		normalized
		指定当被访问时，固定点数据值是否应该被归一化（GL_TRUE）或者直接转换为固定点值（GL_FALSE）。

		stride
		指定连续顶点属性之间的偏移量。如果为0，那么顶点属性会被理解为：它们是紧密排列在一起的。初始值为0。

		pointer
		指定一个指针，指向数组中第一个顶点属性的第一个组件。初始值为0。*/
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	//启用或禁用通用顶点属性数组,参数0索引和着色器中的layout(location = 0）中的0相对应
	glEnableVertexAttribArray(0);
	//绑定到地球纹理坐标缓冲区
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, moonTextureId);

	//背面剔除，默认情况下，背面剔除是关闭的
	glEnable(GL_CULL_FACE | GL_DEPTH_TEST);
	glFrontFace(GL_CCW);
	glDepthFunc(GL_LEQUAL);
	glDrawArrays(GL_TRIANGLES, 0, moon.getNumIndices());
	mvStack.pop();
	mvStack.pop();
	mvStack.pop();


	////星空盒
	//GLfloat skyboxVertices[] = {
	//	// Positions          
	//	-1.0f,  1.0f, -1.0f,
	//	-1.0f, -1.0f, -1.0f,
	//	1.0f, -1.0f, -1.0f,
	//	1.0f, -1.0f, -1.0f,
	//	1.0f,  1.0f, -1.0f,
	//	-1.0f,  1.0f, -1.0f,

	//	-1.0f, -1.0f,  1.0f,
	//	-1.0f, -1.0f, -1.0f,
	//	-1.0f,  1.0f, -1.0f,
	//	-1.0f,  1.0f, -1.0f,
	//	-1.0f,  1.0f,  1.0f,
	//	-1.0f, -1.0f,  1.0f,

	//	1.0f, -1.0f, -1.0f,
	//	1.0f, -1.0f,  1.0f,
	//	1.0f,  1.0f,  1.0f,
	//	1.0f,  1.0f,  1.0f,
	//	1.0f,  1.0f, -1.0f,
	//	1.0f, -1.0f, -1.0f,

	//	-1.0f, -1.0f,  1.0f,
	//	-1.0f,  1.0f,  1.0f,
	//	1.0f,  1.0f,  1.0f,
	//	1.0f,  1.0f,  1.0f,
	//	1.0f, -1.0f,  1.0f,
	//	-1.0f, -1.0f,  1.0f,

	//	-1.0f,  1.0f, -1.0f,
	//	1.0f,  1.0f, -1.0f,
	//	1.0f,  1.0f,  1.0f,
	//	1.0f,  1.0f,  1.0f,
	//	-1.0f,  1.0f,  1.0f,
	//	-1.0f,  1.0f, -1.0f,

	//	-1.0f, -1.0f, -1.0f,
	//	-1.0f, -1.0f,  1.0f,
	//	1.0f, -1.0f, -1.0f,
	//	1.0f, -1.0f, -1.0f,
	//	-1.0f, -1.0f,  1.0f,
	//	1.0f, -1.0f,  1.0f
	//};

	//GLuint skyboxVAO, skyboxVBO;
	//glGenVertexArrays(1, &skyboxVAO);
	//glGenBuffers(1, &skyboxVBO);
	//glBindVertexArray(skyboxVAO);
	//glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);

	//glEnableVertexAttribArray(0);
	//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	//glBindVertexArray(0);

	//vector<const GLchar*> faces;
	//faces.push_back("resource/right.jpg");
	//faces.push_back("resource/left.jpg");
	//faces.push_back("resource/top.jpg");
	//faces.push_back("resource/bottom.jpg");
	//faces.push_back("resource/front.jpg");
	//faces.push_back("resource/back.jpg");

}

void window_size_callback(GLFWwindow* window, int newWidth, int newHeight)
{
	glViewport(0, 0, newWidth, newHeight);
	aspect = (float)newWidth / (float)newHeight;
	pMat = glm::perspective(glm::radians(45.f), aspect, 0.01f, 1000.f);
}

// Loads a cubemap texture from 6 individual texture faces
// Order should be:
// +X (right)
// -X (left)
// +Y (top)
// -Y (bottom)
// +Z (front)
// -Z (back)
GLuint loadCubemap(vector<const GLchar*> faces)
{
	GLuint textureId;
	glGenTextures(1, &textureId);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureId);
	int width, height;
	unsigned char* image = nullptr;

	for (GLuint i = 0; i < faces.size(); i++)
	{
		image = SOIL_load_image(faces[i], &width, &height, 0, SOIL_LOAD_RGB);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		SOIL_free_image_data(image);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	return textureId;

}

// This function loads a texture from file. Note: texture loading functions like these are usually 
// managed by a 'Resource Manager' that manages all resources (like textures, models, audio). 
// For learning purposes we'll just define it as a utility function.
GLuint loadTexture(GLchar* path, GLboolean alpha /* = GL_FALSE */)
{
	GLuint textureId;
	glGenTextures(1, &textureId);
	glBindTexture(GL_TEXTURE_2D, textureId);


	int width, height;
	unsigned char* image = SOIL_load_image(path, &width, &height, 0, SOIL_LOAD_RGB);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	SOIL_free_image_data(image);

	return textureId;
}

int main()
{
	int glfwState = glfwInit();
	if (glfwState == GLFW_FALSE)
	{
		cout << "GLFW initialize failed,invoke glfwInit()......Error file:" << __FILE__ << "......Error line:" << __LINE__ << endl;
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_CORE_PROFILE, GLFW_OPENGL_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

	GLFWwindow* window = glfwCreateWindow(screen_width, screen_height, "Sun earth moon", nullptr, nullptr);
	if (!window)
	{
		cout << "GLFW create window failed,invoke glfwCreateWindow()......Error file:" << __FILE__ << "......Error line:" << __LINE__ << endl;
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window);

	glfwSetWindowSizeCallback(window, window_size_callback);
	glfwSetKeyCallback(window, press_key_callback);
	glfwSetCursorPosCallback(window, mouse_move_callback);
	glfwSetScrollCallback(window, scroll_callback);

	//一定要初始化glew，否则，无法创建着色器句柄
	int glewState = glewInit();
	if (glewState != GLEW_OK)
	{
		cout << "GLEW initialize failed,invoke glewInit()......Error file:" << __FILE__ << "......Error line:" << __LINE__ << endl;
		glfwTerminate();
		exit(EXIT_FAILURE);
	}


	const GLubyte* renderer = glGetString(GL_RENDERER);
	const GLubyte* vendor = glGetString(GL_VENDOR);
	const GLubyte* version = glGetString(GL_VERSION);
	const GLubyte* glslVersion = glGetString(GL_SHADING_LANGUAGE_VERSION);
	GLint major, minor;
	glGetIntegerv(GL_MAJOR_VERSION, &major);
	glGetIntegerv(GL_MINOR_VERSION, &minor);
	printf("GL Vendor : %s\n", vendor);
	printf("GL Renderer : %s\n", renderer);
	printf("GL Version (string) : %s\n", version);
	printf("GL Version (integer) : %d.%d\n", major, minor);
	printf("GLSL Version : %s\n", glslVersion);

	glfwSwapInterval(1);

	init(window);

	while (!glfwWindowShouldClose(window))
	{
		display(window, glfwGetTime());
		glfwSwapBuffers(window);
		glfwPollEvents();
	}


	return 0;
}