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

//���ڶ��̵߳�
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
	//��������
	Assimp::Importer importer;
	const aiScene* scene;

	//OpenGL����
	vector<GLuint> vboVtxs, vboUvs, vboNormals;
	vector<GLuint> vaos;

	GLuint shader;//��ȡ��shader�ģɣ�
	GLuint tboBase, tboNormal;
	GLint uniModel, uniView, uniProjection;
	GLint uniEyePoint, uniLightColor, uniLightPosition;
	GLint uniTexBase, uniTexNormal;
	GLint uniClipPlane0, uniClipPlane1;

	//AABB
	vec3 min, max;

	mat4 model, view, projection;

	bool isReflect;

	//���캯��
	Mesh(const string, bool = false);
	~Mesh();

	//��Ա����
	void initBuffers();
	void initShader();
	void initUniform();
	void draw(mat4, mat4, mat4, vec3, vec3, vec3, int, int);
	void setTexture(GLuint&, int, const string, FREE_IMAGE_FORMAT);
};

//��ȡ�ļ�
std::string readFile(const std::string);

//�����Ϣ
void printLog(GLuint&);
//��ȡShader��uniform����λ��
GLint myGetUniformLocation(GLuint& prog, string name);
//����Shader
GLuint buildShader(string, string,string ="",string ="",string ="");
//����Shader
GLuint complieShader(string, GLenum);
//����Shader
GLuint linkShader(GLuint, GLuint, GLuint, GLuint, GLuint);
//Ѱ��AABB
void findAABB(Mesh&);
//����Χ��
void drawBox(vec3, vec3);

