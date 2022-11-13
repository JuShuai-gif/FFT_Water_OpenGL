#include"common.h"
#include"ocean.h"
#include"skybox.h"
#include"screenQuard.h"

using namespace glm;
using namespace std;

void initGL();
void initOther();
void initMatrix();
void releaseResource();

void computeMatricesFromInputs();
void keyCallback(GLFWwindow*, int, int, int, int);

GLFWwindow* window;
Skybox* skybox;
ocean* ocen;
ScreenQuad* screenQuad;
Mesh* mesh;
       
bool saveTrigger = false;
int frameNumber = 0;
bool resume = true;
bool saveMap = false;

float verticalAngle = -1.79557;
float horizontalAngle = 3.16513;
float initialFoV = 45.0f;
float speed = 5.0f;
float mouseSpeed = 0.005f;
float nearPlane = 0.01f, farPlane = 3000.f;

vec3 eyePoint = vec3(-36.3384, 1.621686, 1.606902);
vec3 eyeDirection = vec3(sin(verticalAngle) * cos(horizontalAngle), cos(verticalAngle), sin(verticalAngle) * sin(horizontalAngle));
vec3 up = vec3(0.f, 1.f, 0.f);

vec3 direction;

mat4 model, view, projection;
bool isRising = false, isDiving = false;

// for reflection texture
float verticalAngleReflect;
float horizontalAngleReflect;
vec3 eyePointReflect;
mat4 reflectV;

vec3 lightPos = vec3(0.f, 5.f, 0.f);
vec3 lightColor = vec3(1.f, 1.f, 1.f);
float lightPower = 12.f;

int N = 128;
float t = 0.f;


int main(int argc, char* argv[]) {
    initGL();
    initOther();
    initMatrix();

    skybox = new Skybox();

    screenQuad = new ScreenQuad();

    //cTimer timer;

    // 海洋模拟
    ocen = new ocean(N, 0.005f, vec2(16.0f, 16.0f), 16);

    mesh = new Mesh("mesh/monkey.obj",true);

    // 初始化上下文
    glfwPollEvents();
    glfwSetCursorPos(window, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);

    // 渲染循环
    while (!glfwWindowShouldClose(window)) {
        glClearColor(97 / 256.f, 175 / 256.f, 239 / 256.f, 1.0f);

        // view control
        computeMatricesFromInputs();
        // Model矩阵构建
        mat4 meshM = translate(mat4(1.f), vec3(-29.f, -0.75f, 0.f));
        meshM = rotate(meshM, -3.14f / 4.f, vec3(1.f, 0.f, 0.f));
        meshM = rotate(meshM, -3.14f / 4.f, vec3(0.f, 1.f, 0.f));
        meshM = scale(meshM, vec3(2.f, 2.f, 2.f));

        // 渲染折射贴图
        // for user-defined framebuffer,
        // must clear the depth buffer before rendering to enable depth test
        glBindFramebuffer(GL_FRAMEBUFFER, ocen->fboRefract);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // clipping
        glEnable(GL_CLIP_DISTANCE0);
        glDisable(GL_CLIP_DISTANCE1);

        vec4 clipPlane0 = vec4(0, -1, 0, ocean::BASELINE);
        glUseProgram(mesh->shader);
        glUniform4fv(mesh->uniClipPlane0, 1, value_ptr(clipPlane0));

        mesh->draw(meshM, view, projection, eyePoint, lightColor, lightPos, 0, 0);

        // skybox
        glEnable(GL_CULL_FACE);
        skybox->draw(model, view, projection, eyePoint);

        // 渲染反射贴图
        // for user-defined framebuffer,
        // must clear the depth buffer before rendering to enable depth test
        glBindFramebuffer(GL_FRAMEBUFFER, ocen->fboReflect);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // clipping
        glDisable(GL_CLIP_DISTANCE0);
        glEnable(GL_CLIP_DISTANCE1);

        vec4 clipPlane1 = vec4(0.f, 1.f, 0.f, ocean::BASELINE + 0.125f);

        // draw scene
        skybox->draw(model, reflectV, projection, eyePointReflect);

        // 渲染主场景
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glDisable(GL_CLIP_DISTANCE0);
        glDisable(GL_CLIP_DISTANCE1);

        // 天空盒
        skybox->draw(model, view, projection, eyePoint);

        mesh->draw(meshM, view, projection, eyePoint, lightColor, lightPos, 0, 0);

        // 海洋
        glDisable(GL_CULL_FACE);
        vec3 tempLightPos = eyePoint + vec3(direction.x * 4.0, 2.0, direction.z * 4.0);

        // double start = omp_get_wtime();
        ocen->render(t, model, view, projection, eyePoint, lightColor, tempLightPos, resume, frameNumber);
        // double end = omp_get_wtime();
        // std::cout << end - start << '\n';

        // for pre-computed FFT water
        // if (t < 50.0) {
        //   ocean->render(t, model, view, projection, eyePoint, lightColor,
        //                 tempLightPos, resume, frameNumber);
        // }

        if (resume) {
            t += 0.01f;
        }

        /* render to main screen */
        // glBindFramebuffer(GL_FRAMEBUFFER, 0);
        // glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        // clear framebuffer
        // glClearColor(97 / 256.f, 175 / 256.f, 239 / 256.f, 1.0f);
        // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //
        // screenQuad->draw(eyePoint);

        // 双缓冲
        glfwSwapBuffers(window);
        glfwPollEvents();

        ocean::dudvMave += vec2(0.001, 0.001);

        // 保存帧
        if (saveTrigger) {
            string dir = "result/output";
            string num = to_string(frameNumber);
            num = string(4 - num.length(), '0') + num;
            string output = dir + num + ".bmp";

            FIBITMAP* outputImage = FreeImage_AllocateT(FIT_UINT32, WINDOW_WIDTH, WINDOW_HEIGHT);
            glReadPixels(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV,(GLvoid*)FreeImage_GetBits(outputImage));
            FreeImage_Save(FIF_BMP, outputImage, output.c_str(), 0);
            FreeImage_Unload(outputImage);
            std::cout << output << " saved." << '\n';
        }

        frameNumber++;
    }

    // 释放资源
    releaseResource();

    return EXIT_SUCCESS;
}

void initGL() {
    // Initialise GLFW
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        getchar();
        exit(EXIT_FAILURE);
    }
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "FFT ocean", NULL, NULL);

    if (window == NULL) {
        std::cout << "Failed to open GLFW window." << std::endl;
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetKeyCallback(window, keyCallback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return;
    }

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST); // must enable depth test!!

    // glEnable(GL_PROGRAM_POINT_SIZE);
    // glPointSize(15);
}

void computeMatricesFromInputs() {

    static float lastTime = glfwGetTime();

    // 计算时间间隔
    float currentTime = glfwGetTime();
    float deltaTime = float(currentTime - lastTime);

    // 获取鼠标间隔
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    // 复原鼠标在屏幕中央
    glfwSetCursorPos(window, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);

    // Compute new orientation
    // As the cursor is put at the center of the screen,
    // (WINDOW_WIDTH/2.f - xpos) and (WINDOW_HEIGHT/2.f - ypos) are offsets
    horizontalAngle += mouseSpeed * float(xpos - WINDOW_WIDTH / 2.f);
    verticalAngle += mouseSpeed * float(-ypos + WINDOW_HEIGHT / 2.f);

    // restrict viewing angles
    // verticalAngle = glm::clamp(verticalAngle, -2.0f, -0.75f);

    // Direction : Spherical coordinates to Cartesian coordinates conversion
    vec3 direction =
        vec3(sin(verticalAngle) * cos(horizontalAngle), cos(verticalAngle),
            sin(verticalAngle) * sin(horizontalAngle));

    // Right vector
    vec3 right = vec3(cos(horizontalAngle - 3.14 / 2.f), 0.f,
        sin(horizontalAngle - 3.14 / 2.f));

    // new up vector
    vec3 newUp = cross(right, direction);

    // restrict movements, can only move horizontally
    // vec3 forwardDir = normalize(vec3(direction.x, 0, direction.z));
    // vec3 rightDir = normalize(vec3(right.x, 0, right.z));
    vec3 forwardDir = normalize(direction);
    vec3 rightDir = normalize(right);

    // Move forward
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        eyePoint += forwardDir * deltaTime * speed;
    }
    // Move backward
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        eyePoint -= forwardDir * deltaTime * speed;
    }
    // Strafe right
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        eyePoint += rightDir * deltaTime * speed;
    }
    // Strafe left
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        eyePoint -= rightDir * deltaTime * speed;
    }
    // dive
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS && !isDiving) {
        isDiving = true;
        isRising = false;
    }
    // rise
    if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS && !isRising) {
        isDiving = false;
        isRising = true;
    }

    // smoothly dive
    if (isDiving && eyePoint.y > -20.f) {
        eyePoint.y -= 2.f;

        if (eyePoint.y < -20.f) {
            isDiving = false;
            eyePoint.y = -20.f;
        }
    }
    // smoothly rise
    else if (isRising && eyePoint.y < 15.f) {
        eyePoint.y += 2.f;

        if (eyePoint.y > 15.f) {
            isRising = false;
            eyePoint.y = 15.f;
        }
    }

    // update transform matrix
    view = lookAt(eyePoint, eyePoint + direction, newUp);
    projection = perspective(initialFoV, 1.f * WINDOW_WIDTH / WINDOW_HEIGHT, nearPlane, farPlane);

    // For the next frame, the "last time" will be "now"
    lastTime = currentTime;
}

void keyCallback(GLFWwindow* keyWnd, int key, int scancode, int action,
    int mods) {
    if (action == GLFW_PRESS) {
        switch (key) {
        case GLFW_KEY_ESCAPE: {
            glfwSetWindowShouldClose(keyWnd, GLFW_TRUE);
            break;
        }
        case GLFW_KEY_F: {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            break;
        }
        case GLFW_KEY_L: {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            break;
        }
        case GLFW_KEY_I: {
            std::cout << "eyePoint: " << to_string(eyePoint) << '\n';
            std::cout << "verticleAngle: " << fmod(verticalAngle, 6.28f) << ", "
                << "horizontalAngle: " << fmod(horizontalAngle, 6.28f) << endl;

            break;
        }
        case GLFW_KEY_Y: {
            saveTrigger = !saveTrigger;
            frameNumber = 0;
            break;
        }
        case GLFW_KEY_X: {
            resume = !resume;
            break;
        }
        case GLFW_KEY_M: {
            saveMap = true;
            break;
        }
        default:
            break;
        }
    }
}

void initMatrix() {
    model = translate(mat4(1.f), vec3(0.f, 0.f, -4.f));
    view = lookAt(eyePoint, eyePoint + eyeDirection, up);
    projection = perspective(initialFoV, 1.f * WINDOW_WIDTH / WINDOW_HEIGHT,
        nearPlane, farPlane);
}

void initOther() {
    // initialize random seed
    // this makes the ocean geometry different in every execution
    srand(clock());

    FreeImage_Initialise(true);
}

void releaseResource() {
    delete ocen;
    delete skybox;
    delete screenQuad;

    glfwTerminate();
    FreeImage_DeInitialise();
}

void saveHeightMap() {
    int w, h;
    w = N;
    h = w;

    // find max, min
    float maxHeight = 0, minHeight = 0;

    // find max
    for (int i = 0; i < w; i++) {
        for (int j = 0; j < h; j++) {
            int idx = i * N + j;

            float y = ocen->vertices[idx].y;

            if (y > maxHeight) {
                maxHeight = y;
            }
        }
    }

    // find min
    for (int i = 0; i < w; i++) {
        for (int j = 0; j < h; j++) {
            int idx = i * N + j;

            float y = ocen->vertices[idx].y;

            if (y < minHeight) {
                minHeight = y;
            }
        }
    }

    // height range
    float heightRange = maxHeight - minHeight;

    // std::cout << "maxHeight: " << maxHeight << '\n';
    // std::cout << "minHeight: " << minHeight << '\n';

    FIBITMAP* bitmap = FreeImage_Allocate(w, h, 24);
    RGBQUAD color;

    if (!bitmap) {
        std::cout << "FreeImage: Cannot allocate image." << '\n';
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < w; i++) {
        for (int j = 0; j < h; j++) {
            int idx = i * N + j;

            // float x = ocean->vertices[idx].x;
            // x += 10.f;
            // x *= 10.f;
            // std::cout << x << '\n';

            float y = ocen->vertices[idx].y;
            y += abs(minHeight); // to non-negative
            y /= heightRange;    // normalize
            // std::cout << y << '\n';
            int iy = int(y * 255.0); // to [0, 255]

            // float z = ocean->vertices[idx].z;
            // z += 10.f;
            // z *= 10.f;
            // std::cout << z << '\n';

            color.rgbRed = iy;
            color.rgbGreen = iy;
            color.rgbBlue = iy;
            FreeImage_SetPixelColor(bitmap, i, j, &color);

            // std::cout << float(color.rgbRed) << '\n';
            // std::cout << '\n';
        }
    }

    if (FreeImage_Save(FIF_PNG, bitmap, "height.png", 0))
        cout << "Height map successfully saved!" << endl;
}

void saveNormalMap() {
    int w, h;
    w = N;
    h = w;

    FIBITMAP* bitmap = FreeImage_Allocate(w, h, 24);
    RGBQUAD color;

    if (!bitmap) {
        std::cout << "FreeImage: Cannot allocate image." << '\n';
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < w; i++) {
        for (int j = 0; j < h; j++) {
            int idx = i * N + j;

            vec3 normal(ocen->vertices[idx].nx, ocen->vertices[idx].ny,
                ocen->vertices[idx].nz);
            normal = normalize(normal);      // to [-1, 1]
            normal = (normal + 1.0f) / 2.0f; // to [0, 1]

            // to [0, 255]
            int ix = int(normal.x * 255.0);
            int iy = int(normal.y * 255.0);
            int iz = int(normal.z * 255.0);

            color.rgbRed = ix;
            color.rgbGreen = iy;
            color.rgbBlue = iz;

            // std::cout << ix << ", " << iy << ", " << iz << '\n';

            FreeImage_SetPixelColor(bitmap, i, j, &color);
        }
    }

    if (FreeImage_Save(FIF_PNG, bitmap, "normal.png", 0))
        cout << "Normal map successfully saved!" << endl;
}

