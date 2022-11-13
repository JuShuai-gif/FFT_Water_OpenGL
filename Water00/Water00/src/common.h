#pragma once
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/ext.hpp>

#include<assimp/Importer.hpp>
#include<assimp/scene.h>
#include<assimp/postprocess.h>

//用于多线程的
#include<omp.h>

#include<GLFW/glfw3.h>
#include<FreeImage.h>

#define M_PI 3.1415926
using namespace std;
using namespace glm;

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

class Mesh
{
public:
	//网格数据
	Assimp::Importer importer;
	const aiScene* scene;

	//OpenGL数据
	vector<GLuint> vboVtxs, vboUvs, vboNormals;
	vector<GLuint> vaos;

	GLuint shader;//获取的shader的ＩＤ
	GLuint tboBase, tboNormal;
	GLint uniModel, uniView, uniProjection;
	GLint uniEyePoint, uniLightColor, uniLightPosition;
	GLint uniTexBase, uniTexNormal;
	GLint uniClipPlane0, uniClipPlane1;

	//AABB
	vec3 min, max;

	mat4 model, view, projection;

	bool isReflect;

	//构造函数
	Mesh(const string, bool = false);
	~Mesh();

	//成员函数
	void initBuffers();
	void initShader();
	void initUniform();
	void draw(mat4, mat4, mat4, vec3, vec3, vec3, int, int);
	void setTexture(GLuint&, int, const string, FREE_IMAGE_FORMAT);
};

//读取文件
std::string readFile(const std::string);

//输出信息
void printLog(GLuint&);
//获取Shader中uniform变量位置
GLint myGetUniformLocation(GLuint& prog, string name);
//构建Shader
GLuint buildShader(string, string,string ="",string ="",string ="");
//编译Shader
GLuint complieShader(string, GLenum);
//链接Shader
GLuint linkShader(GLuint, GLuint, GLuint, GLuint, GLuint);
//寻找AABB
void findAABB(Mesh&);
//画包围盒
void drawBox(vec3, vec3);

