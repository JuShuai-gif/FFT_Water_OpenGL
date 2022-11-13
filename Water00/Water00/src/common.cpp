#include"common.h"

std::string readFile(const std::string filename) 
{
	std::ifstream in;
	in.open(filename);
	std::stringstream ss;
	ss << in.rdbuf();
	std::string sOut = ss.str();
	in.close();
	return sOut;
}

//创建Shader
GLuint buildShader(string vsDir, string fsDir, string tcsDir, string tesDir, string geoDir)
{
    GLuint vs, fs, tcs = 0, tes = 0, geo = 0;
    GLint linkOK;
    GLuint exeShader;

    //编译
    vs = complieShader(vsDir, GL_VERTEX_SHADER);
    fs = complieShader(fsDir, GL_FRAGMENT_SHADER);

    //TCS,TES
    if (tcsDir!=""&&tesDir!="")
    {
        tcs = complieShader(tcsDir, GL_TESS_CONTROL_SHADER);
        tes = complieShader(tesDir, GL_TESS_EVALUATION_SHADER);
    }
    if (geoDir!="")
    {
        geo = complieShader(geoDir, GL_GEOMETRY_SHADER);
    }

    exeShader = linkShader(vs, fs, tcs, tes, geo);

    return exeShader;
}

GLuint complieShader(string filename, GLenum type) 
{
    //读取文件
    string sTemp = readFile(filename);
    string info;
    const GLchar* source = sTemp.c_str();

    //判断类型
    switch (type) {
    case GL_VERTEX_SHADER:
        info = "Vertex";
        break;
    case GL_FRAGMENT_SHADER:
        info = "Fragment";
        break;
    }

    if (source == NULL) {
        std::cout << info << " Shader : Can't read shader source file."
            << std::endl;
        return 0;
    }

    const GLchar* sources[] = { source };
    GLuint objShader = glCreateShader(type);
    glShaderSource(objShader, 1, sources, NULL);
    glCompileShader(objShader);

    GLint compile_ok;
    glGetShaderiv(objShader, GL_COMPILE_STATUS, &compile_ok);
    if (compile_ok == GL_FALSE) {
        std::cout << info << " Shader : Fail to compile." << std::endl;
        printLog(objShader);
        glDeleteShader(objShader);
        return 0;
    }

    return objShader;
}
//链接Shader
GLuint linkShader(GLuint vsObj, GLuint fsObj, GLuint tcsObj, GLuint tesObj, GLuint geoObj)
{
    GLuint exe;
    GLint linkOK;

    exe = glCreateProgram();
    glAttachShader(exe, vsObj);
    glAttachShader(exe, fsObj);

    if (tcsObj!=0&&tesObj!=0){
        glAttachShader(exe, tcsObj);
        glAttachShader(exe, tesObj);
    }

    if (geoObj!=0){
        glAttachShader(exe, geoObj);
    }

    glLinkProgram(exe);

    glGetProgramiv(exe, GL_LINK_STATUS, &linkOK);

    if (linkOK == GL_FALSE)
    {
        std::cout << "Failed to link shader program." << std::endl;
        printLog(exe);
        glDeleteProgram(exe);

        return 0;
    }
    return exe;
}
//打印出错信息
void printLog(GLuint& object) 
{
    GLint log_length = 0;
    if (glIsShader(object)) {
        glGetShaderiv(object, GL_INFO_LOG_LENGTH, &log_length);
    }
    else if (glIsProgram(object)) {
        glGetProgramiv(object, GL_INFO_LOG_LENGTH, &log_length);
    }
    else {
        cerr << "printlog: Not a shader or a program" << endl;
        return;
    }

    char* log = (char*)malloc(log_length);

    if (glIsShader(object))
        glGetShaderInfoLog(object, log_length, NULL, log);
    else if (glIsProgram(object))
        glGetProgramInfoLog(object, log_length, NULL, log);

    cerr << log << endl;
    free(log);
}
//获取Shader uniform位置
GLint myGetUniformLocation(GLuint& prog, string name) {
    GLint location;
    location = glGetUniformLocation(prog, name.c_str());
    if (location == -1) {
        cerr << "Could not bind uniform : " << name << ". "
            << "Did you set the right name? "
            << "Or is " << name << " not used?" << endl;
    }
    return location;
}

Mesh::Mesh(const string fileName,bool reflect) {
    isReflect = reflect;

    // 一个scene代表一个物体，这点后期需要改正，应该是Object
    scene = importer.ReadFile(fileName, aiProcess_CalcTangentSpace);

    initBuffers();
    initShader();
    initUniform();
}

Mesh::~Mesh()
{
    for (size_t i = 0; i < scene->mNumMeshes; ++i){
        glDeleteBuffers(1, &vboVtxs[i]);
        glDeleteBuffers(1, &vboUvs[i]);
        glDeleteBuffers(1, &vboNormals[i]);
        glDeleteVertexArrays(1, &vaos[i]);
    }
}

void Mesh::initShader()
{
    string dir = "shaders/";
    string vs, fs;

    if (isReflect){
        vs = dir + "vsReflect.glsl";
        fs = dir + "fsReflect.glsl";
    }
    else {
        vs = dir + "vsPhong.glsl";
        fs = dir + "fsPhong.glsl";
    }
    // Mesh的shader
    shader = buildShader(vs, fs);
}

void Mesh::initUniform()
{
    uniModel = myGetUniformLocation(shader, "M");
    uniView = myGetUniformLocation(shader, "V");
    uniProjection = myGetUniformLocation(shader, "P");
    uniEyePoint = myGetUniformLocation(shader, "eyePoint");
    uniLightColor = myGetUniformLocation(shader, "lightColor");
    uniLightPosition = myGetUniformLocation(shader, "lightPosition");
    uniTexBase = myGetUniformLocation(shader, "texBase");
    uniTexNormal = myGetUniformLocation(shader, "texNormal");

    if (isReflect) {
        uniClipPlane0 = myGetUniformLocation(shader, "clipPlane0");
        uniClipPlane1 = myGetUniformLocation(shader, "clipPlane1");
    }
}

void Mesh::initBuffers()
{
    // 对于每个面
    for (size_t i = 0; i < scene->mNumMeshes; i++) {
        // 一个物体会有很多个面组成
        const aiMesh* mesh = scene->mMeshes[i];
        // 面的个数
        int numVtxs = mesh->mNumVertices;

        // 面的属性
        // 一个面三个顶点、两个UV坐标、三个法线
        GLfloat* aVtxCoords = new GLfloat[numVtxs * 3];
        GLfloat* aUvs = new GLfloat[numVtxs * 2];
        GLfloat* aNormals = new GLfloat[numVtxs * 3];


        for (size_t j = 0; j < numVtxs; j++) {
            aiVector3D& vtx = mesh->mVertices[j];
            aVtxCoords[j * 3 + 0] = vtx.x;
            aVtxCoords[j * 3 + 1] = vtx.y;
            aVtxCoords[j * 3 + 2] = vtx.z;

            aiVector3D& nml = mesh->mNormals[j];
            aNormals[j * 3 + 0] = nml.x;
            aNormals[j * 3 + 1] = nml.y;
            aNormals[j * 3 + 2] = nml.z;

            aiVector3D& uv = mesh->mTextureCoords[0][j];
            aUvs[j * 2 + 0] = uv.x;
            aUvs[j * 2 + 1] = uv.y;
        }



        // vao
        GLuint vao;
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        vaos.push_back(vao);

        // vbo for vertex
        GLuint vboVtx;
        glGenBuffers(1, &vboVtx);
        glBindBuffer(GL_ARRAY_BUFFER, vboVtx);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * numVtxs * 3, aVtxCoords, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(0);
        vboVtxs.push_back(vboVtx);

        // vbo for uv
        GLuint vboUv;
        glGenBuffers(1, &vboUv);
        glBindBuffer(GL_ARRAY_BUFFER, vboUv);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * numVtxs * 2, aUvs, GL_STATIC_DRAW);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(1);
        vboUvs.push_back(vboUv);

        // vbo for normal
        GLuint vboNml;
        glGenBuffers(1, &vboNml);
        glBindBuffer(GL_ARRAY_BUFFER, vboNml);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * numVtxs * 3, aNormals, GL_STATIC_DRAW);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(2);
        vboNormals.push_back(vboNml);

        // delete client data
        delete[] aVtxCoords;
        delete[] aUvs;
        delete[] aNormals;
    } // end for each mesh

}

void Mesh::draw(mat4 M, mat4 V, mat4 P, vec3 eye, vec3 lightColor, vec3 lightPosition, int unitBaseColor, int unitNormal)
{
    glUseProgram(shader);
    //设置M、V、Ｐ矩阵
    glUniformMatrix4fv(uniModel, 1, GL_FALSE, value_ptr(M));
    glUniformMatrix4fv(uniView, 1, GL_FALSE, value_ptr(V));
    glUniformMatrix4fv(uniProjection, 1, GL_FALSE, value_ptr(P));
    //设置一些眼睛、灯光的属性
    glUniform3fv(uniEyePoint, 1, value_ptr(eye));
    glUniform3fv(uniLightColor, 1, value_ptr(lightColor));
    glUniform3fv(uniLightPosition, 1, value_ptr(lightPosition));

    for (size_t i = 0; i < scene->mNumMeshes; ++i){
        int numVtxs = scene->mMeshes[i]->mNumVertices;

        glBindVertexArray(vaos[i]);
        glDrawArrays(GL_TRIANGLES, 0, numVtxs);
    }
}

void Mesh::setTexture(GLuint& tbo, int texUnit, const string texDir, FREE_IMAGE_FORMAT imgType)
{
    glActiveTexture(GL_TEXTURE0 + texUnit);

    FIBITMAP* texImage = FreeImage_ConvertTo24Bits(FreeImage_Load(imgType, texDir.c_str()));

    glGenTextures(1, &tbo);
    glBindTexture(GL_TEXTURE_2D, tbo);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, FreeImage_GetWidth(texImage), FreeImage_GetHeight(texImage), 0, GL_RGB, GL_UNSIGNED_BYTE,
        (void*)FreeImage_GetBits(texImage));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    FreeImage_Unload(texImage);
}

