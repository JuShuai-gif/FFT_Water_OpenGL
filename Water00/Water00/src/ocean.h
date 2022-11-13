#pragma once
#include <complex>
#include "timer.h"
#include "fft.h"
#include "common.h"

typedef complex<double> Complex;

float uniformRandomVariable();
Complex gaussianRandomVariable();

struct vertex_ocean {
	GLfloat x, y, z;    // vertex
	GLfloat nx, ny, nz; // normal
	GLfloat a, b, c;    // htilde0
	GLfloat _a, _b, _c; // htilde0mk conjugate
	GLfloat ox, oy, oz; // original position
};

class ocean
{
public:
	float g;
	int N, Nplus1;
	float A;
	vec2 w;
	float length;
	Complex* h_tilde, * h_tilde_slopex, * h_tilde_slopez, * h_tilde_dx, * h_tilde_dz;
	FFT* fft; // 快速傅里叶变换
	vertex_ocean* vertices;                  // 海洋顶点信息


	Assimp::Importer importer;
	const aiScene* scene;

	static const float BASELINE;
	static vec2 dudvMave;
	// 存放顶点、UV、法线
	vector<GLuint> vboVtxs, vboUvs, vboNmls;
	vector<GLuint> ebos;
	//　索引
	vector<unsigned int> indices;
	vector<GLuint> vaos;

	GLuint shader;
	GLuint tboDisp, tboNormal, tboFresnel;
	GLuint tboPerlin, tboPerlinN, tboPerlinDudv;
	GLint uniM, uniV, uniP;
	GLint uniLightColor, uniLightPos;
	GLint uniTexReflect, uniTexRefract, uniTexDisp, uniTexNormal, uniTexSkybox;
	GLint uniTexPerlin, uniTexPerlinN, uniTexPerlinDudv, uniTexFresnel;
	GLint uniEyePoint;
	GLint uniDudvMove;
	GLuint tboRefract, tboReflect;
	GLuint fboRefract, fboReflect;
	GLuint rboDepthRefract, rboDepthReflect;

	FIBITMAP* dispMap, * normalMap;

public:
	ocean(const int N, const float A, const vec2 w, const float length);
	~ocean();

	float dispersion(int n_prime, int m_prime); // 水深
	float phillips(int n_prime, int m_prime);   // phillips spectrum
	Complex hTilde_0(int n_prime, int m_prime);
	Complex hTilde(float t, int n_prime, int m_prime);
	void evaluateWavesFFT(float t);
	void render(float t,mat4 M,mat4 V,mat4 P,vec3 eyePoint,vec3 lightColor,vec3 lightPos,bool resume,int frameN);
	vec3 getVertex(int ix, int iz);

	void initBuffers();
	void initShader();
	void initTexture();
	void initUniform();
	void initReflect();
	void initRefract();
	void setTexture(GLuint&, int, const string, FREE_IMAGE_FORMAT);
	void writeHeightMap(int);
	void writeNormalMap(int);
	void writeFoldingMap(int);
	float Heaviside(float);
	const char* getFileDir(string, int);

};

