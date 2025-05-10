#include "glframework/core.h"
#include "wrapper/checkError.h"

#include "application/application.h"
#include "application/assimpLoader.h"
#include "application/camera/perspectiveCamera.h"
#include "application/camera/orthographicCamera.h"
#include "application/camera/trackBallCameraControl.h"
#include "application/camera/gameCameraControl.h"

#include "glframework/geometry.h"
#include "glframework/shader.h"
#include "glframework/material/screenMaterial.h"
#include "glframework/material/gBufferMaterial.h"
#include "glframework/material/lightingMaterial.h"
#include "glframework/texture.h"

#include "glframework/mesh/mesh.h"
#include "glframework/light/pointLight.h"
#include "glframework/light/spotLight.h"

#include "glframework/renderer/renderer.h"
#include "glframework/scene.h"
#include "glframework/framebuffer/framebuffer.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"


// 1 Variables
// 1.1 Window
int WIDTH = 2560;
int HEIGHT = 1080;
glm::vec3 clearColor{};

// 1.2 Objects
std::vector<PointLight*> pointLights{};
Camera* camera = nullptr;
GameCameraControl* cameraControl = nullptr;

// 1.3 Render
Renderer* renderer = nullptr;
Scene* sceneOff = nullptr;

Framebuffer* gBuffer = nullptr;
Mesh* lightingMesh = nullptr;

// --------------------------------------------

// 2 Mouse and keyboard event
void OnResize(int width, int height) {

    GL_CALL(glViewport(0, 0, width, height));
}

void OnKey(int key, int action, int mods) {

    cameraControl->onKey(key, action, mods);
}

void OnMouse(int button, int action, int mods) {

    double x, y;
    glApp->getCursorPosition(&x, &y);
    cameraControl->onMouse(button, action, x, y);

}

void OnCursor(double xpos, double ypos) {

    cameraControl->onCursor(xpos, ypos);
}

void OnScroll(double offset) {

    cameraControl->onScroll(offset);
}


// --------------------------------------------

// 3 Prepare render objects and camera

void prepare() {

    gBuffer = Framebuffer::createGBufferFbo(WIDTH, HEIGHT);
    renderer = new Renderer();
    sceneOff = new Scene();

    // Geometry pass
    auto cyborg = AssimpLoader::load("assets/cyborg/cyborg.obj");
    cyborg->setPosition(glm::vec3(0.0f, 0.0f, 0.0f));
    sceneOff->addChild(cyborg);


    // Lighting pass
    auto lightingGeo = Geometry::createScreenPlane();
    auto lightingMat = new LightingMaterial();
    lightingMat->mPosition = gBuffer->mGBufferAttachment[0];
    lightingMat->mNormal = gBuffer->mGBufferAttachment[1];
    lightingMat->mAlbedoSpec = gBuffer->mGBufferAttachment[2];
    lightingMesh = new Mesh(lightingGeo, lightingMat);


    // 4 Create light
    glm::vec3 lightPositions[] = {
            glm::vec3(-3.0f,  3.0f, 10.0f),
            glm::vec3(3.0f,  3.0f, 10.0f),
            glm::vec3(-3.0f, -3.0f, 10.0f),
            glm::vec3(3.0f, -3.0f, 10.0f),
    };
    glm::vec3 lightColors[] = {
        glm::vec3(1.0f, 1.0f, 1.0f),
        glm::vec3(1.0f, 1.0f, 1.0f),
        glm::vec3(1.0f, 1.0f, 1.0f),
        glm::vec3(1.0f, 1.0f, 1.0f)
    };
    for (int i = 0; i < 4; i++) {
        auto pointLight = new PointLight();
        pointLight->setPosition(lightPositions[i]);
        pointLight->mColor = lightColors[i];
        pointLights.push_back(pointLight);
    }
}


void prepareCamera() {

    camera = new PerspectiveCamera(60.0f, (float)glApp->getWidth() / glApp->getHeight(), 0.1f, 1000.0f);

    cameraControl = new GameCameraControl();
    cameraControl->setCamera(camera);
    cameraControl->setSensitivity(0.4f);
    cameraControl->setSpeed(0.1f);
}

// --------------------------------------------

// 4 Prepare UI

void initIMGUI() {

    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(glApp->getWindow(), true);
    ImGui_ImplOpenGL3_Init("#version 460");
}

void renderIMGUI() {

    // 1 Initial
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // 2 GUI widget
    ImGui::Begin("MaterialEditor");
    ImGui::End();

    // 3 Render
    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(glApp->getWindow(), &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}



// 5 Render
int main() {

    // 5.1 Initial the window
    if (!glApp->init(WIDTH, HEIGHT)) {
        return -1;
    }

    // 5.2 Size and keyboard callback
    glApp->setResizeCallback(OnResize);
    glApp->setKeyBoardCallback(OnKey);
    glApp->setMouseCallback(OnMouse);
    glApp->setCursorCallback(OnCursor);
    glApp->setScrollCallback(OnScroll);

    // 5.3 Set openGl rendering viewport and clear canvas color
    GL_CALL(glViewport(0, 0, WIDTH, HEIGHT));
    GL_CALL(glClearColor(0.0f, 0.18f, 0.65f, 1.0f));

    // 4 Set up camera, objects, UI
    prepareCamera();
    prepare();
    initIMGUI();

    // 4 Set window loop
    while (glApp->update()) {

        cameraControl->update();

        renderer->setClearColor(clearColor);
        renderer->renderGBuffer(sceneOff, camera, gBuffer->mFBO);

        renderer->renderLighting(lightingMesh, camera, pointLights);

        renderIMGUI();
    }

    glApp->destroy();

    return 0;
}