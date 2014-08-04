/* 
 * File:   fbState.cpp
 * Author: miles
 * 
 * Created on July 30, 2014, 9:50 PM
 */

#include "lsRenderer.h"
#include "fbState.h"

using math::vec2i;
using math::vec2;
using math::vec3;
using math::mat4;
using math::quat;

enum {
    TEST_MAX_SCENE_OBJECTS = 50,
    TEST_MAX_SCENE_INSTANCES = TEST_MAX_SCENE_OBJECTS*TEST_MAX_SCENE_OBJECTS*TEST_MAX_SCENE_OBJECTS,
    TEST_MAX_KEYBORD_STATES = 512,
    TEST_FRAMEBUFFER_WIDTH = 320,
    TEST_FRAMEBUFFER_HEIGHT = 240
};

#define TEST_PROJECTION_FOV 60.f
#define TEST_PROJECTION_NEAR 0.01f
#define TEST_PROJECTION_FAR 10.f

#define TEST_INSTANCE_RADIUS 0.5f

#define TEST_FONT_FILE "FiraSans-Regular.otf"

#define VP_MATRIX_UNIFORM "vpMatrix"
#define CAMERA_POSITION_UNIFORM "camPos"

/*
 * This shader uses a Logarithmic Z-Buffer, thanks to
 * http://www.gamasutra.com/blogs/BranoKemen/20090812/2725/Logarithmic_Depth_Buffer.php
 */
static const char meshVSData[] = R"***(
#version 330 core

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec2 inUv;
layout (location = 2) in vec3 inNorm;
layout (location = 3) in mat4 inModelMat;

uniform mat4 vpMatrix;
uniform vec3 camPos = vec3(0.0, 0.0, 1.0);

out vec3 eyeDir;
out vec3 nrmCoords;
out vec2 uvCoords;

const float NEAR = 1.0;
const float FAR = 10.0;

void main() {
    mat4 mvpMatrix = vpMatrix * inModelMat;
    gl_Position = mvpMatrix * vec4(inPos, 1.0);
    gl_Position.z = -log(NEAR * gl_Position.z + 1.0) / log(NEAR * FAR + 1.0);
    
    // Use this to make the camera act as either a specular or point light
    //eyeDir = camPos - inPos;

    eyeDir = camPos;
    nrmCoords = inNorm;
    uvCoords = inUv;
}
)***";

/*
 * Testing Alpha Masking for font rendering.
 */
static const char meshFSData[] = R"***(
#version 330 core

in vec3 eyeDir;
in vec3 nrmCoords;
in vec2 uvCoords;

uniform sampler2D tex;

out vec4 outFragCol;

void main() {
    float lightIntensity = dot(eyeDir, normalize(nrmCoords));
    outFragCol = texture(tex, uvCoords) * lightIntensity;
}
)***";

/*
 * Testing Alpha Masking for font rendering.
 */
static const char fontFSData[] = R"***(
#version 330

precision lowp float;

in vec2 uvCoords;
out vec4 outFragCol;

uniform sampler2DRect texSampler;
uniform vec4 fontColor = vec4(0.0, 1.0, 1.0, 1.0);

void main() {
    float mask = texture(texSampler, uvCoords).r;
    outFragCol = fontColor*step(0.5, mask);
}
)***";

/******************************************************************************
 * Constructor & Destructor
******************************************************************************/
fbState::fbState() {
    SDL_StopTextInput();
    SDL_SetRelativeMouseMode(SDL_TRUE);
}

fbState::~fbState() {
    terminate();
}

/******************************************************************************
 * Hardware Events
******************************************************************************/
/******************************************************************************
 * Key Up Event
******************************************************************************/
void fbState::onKeyboardUpEvent(const SDL_KeyboardEvent& e) {
    const SDL_Keycode key = e.keysym.sym;
    
    if (key < 0 || (unsigned)key >= TEST_MAX_KEYBORD_STATES) {
        return;
    }
    
    if (key == SDLK_ESCAPE) {
        this->setState(LS_GAME_STOPPED);
    }
    else {
        pKeyStates[key] = false;
    }
}

/******************************************************************************
 * Key Down Event
******************************************************************************/
void fbState::onKeyboardDownEvent(const SDL_KeyboardEvent& e) {
    const SDL_Keycode key = e.keysym.sym;
    
    if (key < 0 || (unsigned)key >= TEST_MAX_KEYBORD_STATES) {
        return;
    }
    
    if (key == SDLK_SPACE) {
        if (getState() == LS_GAME_RUNNING) {
            setState(LS_GAME_PAUSED);
            
            // testing mouse capture for framebuffer/window resizing
            SDL_SetRelativeMouseMode(SDL_FALSE);
        }
        else  {
            setState(LS_GAME_RUNNING);
            SDL_SetRelativeMouseMode(SDL_TRUE);
        }
    }
    
    pKeyStates[key] = true;
}

/******************************************************************************
 * Keyboard States
******************************************************************************/
void fbState::updateKeyStates(float dt) {
    const float moveSpeed = 0.05f * dt;
    vec3 pos = {0.f};
    
    if (pKeyStates[SDLK_w]) {
        pos[2] += moveSpeed;
    }
    if (pKeyStates[SDLK_s]) {
        pos[2] -= moveSpeed;
    }
    if (pKeyStates[SDLK_a]) {
        pos[0] += moveSpeed;
    }
    if (pKeyStates[SDLK_d]) {
        pos[0] -= moveSpeed;
    }
    
    const vec3 translation = {
        math::dot(math::getAxisX(orientation), pos),
        math::dot(math::getAxisY(orientation), pos),
        math::dot(math::getAxisZ(orientation), pos)
    };
    
    const mat4& viewMatrix = pMatStack->getMatrix(LS_VIEW_MATRIX);
    const mat4&& movement = math::translate(viewMatrix, translation);
    pMatStack->loadMatrix(LS_VIEW_MATRIX, movement);
}

/******************************************************************************
 * Text Events
******************************************************************************/
void fbState::onKeyboardTextEvent(const SDL_TextInputEvent&) {
}

/******************************************************************************
 * Window Event
******************************************************************************/
void fbState::onWindowEvent(const SDL_WindowEvent& e) {
    switch (e.event) {
        case SDL_WINDOWEVENT_CLOSE:
            this->setState(LS_GAME_STOPPED);
            break;
        case SDL_WINDOWEVENT_RESIZED:
            resetGlViewport();
            pMatStack->loadMatrix(LS_PROJECTION_MATRIX, get3dViewport());
            break;
        default:
            break;
    }
}

/******************************************************************************
 * Mouse Move Event
******************************************************************************/
void fbState::onMouseMoveEvent(const SDL_MouseMotionEvent& e) {
    // Prevent the orientation from drifting by keeping track of the relative mouse offset
    if (this->getState() == LS_GAME_PAUSED
    || (mouseX == e.xrel && mouseY == e.yrel)
    ) {
        // I would rather quit the function than have unnecessary LERPs and
        // quaternion multiplications.
        return;
    }
    
    mouseX = e.xrel;
    mouseY = e.yrel;
    
    // Get the current mouse position and LERP from the previous mouse position.
    // The mouse position is divided by the window's resolution in order to normalize
    // the mouse delta between 0 and 1. This allows for the camera's orientation to
    // be LERPed without the need for multiplying it by the last time delta.
    // As a result, the camera's movement becomes as smooth and natural as possible.
    
    const vec2&& fRes = (vec2)getParentSystem().getDisplay().getResolution();
    const vec2&& mouseDelta = vec2{(float)mouseX, (float)mouseY} / fRes;
    
    // Always lerp to the 
    const quat&& lerpX = math::lerp(
        quat{0.f, 0.f, 0.f, 1.f}, quat{mouseDelta[1], 0.f, 0.f, 1.f}, 1.f
    );
    
    const quat&& lerpY = math::lerp(
        quat{0.f, 0.f, 0.f, 1.f}, quat{0.f, mouseDelta[0], 0.f, 1.f}, 1.f
    );
        
    orientation *= lerpY * lerpX;
    orientation = math::normalize(orientation);
}

/******************************************************************************
 * Mouse Button Up Event
******************************************************************************/
void fbState::onMouseButtonUpEvent(const SDL_MouseButtonEvent&) {
}

/******************************************************************************
 * Mouse Button Down Event
******************************************************************************/
void fbState::onMouseButtonDownEvent(const SDL_MouseButtonEvent&) {
}

/******************************************************************************
 * Mouse Wheel Event
******************************************************************************/
void fbState::onMouseWheelEvent(const SDL_MouseWheelEvent&) {
}

/******************************************************************************
 * termination assistant
******************************************************************************/
void fbState::terminate() {
    mouseX = 0;
    mouseY = 0;
    
    meshProg.terminate();
    fontProg.terminate();
    
    testFb.terminate();
    
    orientation = quat{0.f, 0.f, 0.f, 1.f};

    delete pMatStack;
    pMatStack = nullptr;

    delete pScene;
    pScene = nullptr;

    delete [] pKeyStates;
    pKeyStates = nullptr;

    delete [] pModelMatrices;
    pModelMatrices = nullptr;
    
    delete pBlender;
    pBlender = nullptr;
}

/******************************************************************************
 * Allocate internal class memory
******************************************************************************/
bool fbState::initMemory() {
    pMatStack       = new lsMatrixStack{};
    pScene          = new lsSceneManager{};
    pKeyStates      = new bool[TEST_MAX_KEYBORD_STATES];
    pModelMatrices  = new mat4[TEST_MAX_SCENE_INSTANCES];
    pBlender        = new lsBlender{};
    
    if (pMatStack == nullptr
    ||  pScene == nullptr
    ||  !pScene->init()
    ||  pKeyStates == nullptr
    ||  pModelMatrices == nullptr
    ||  !testFb.init()
    ||  !pBlender
    ) {
        terminate();
        return false;
    }
    
    return true;
}

/******************************************************************************
 * Initialize resources from files
******************************************************************************/
bool fbState::initFileData() {
    
    lsMeshResource* pMeshLoader = new lsMeshResource{};
    lsFontResource* pFontLoader = new lsFontResource{};
    lsMesh* pSphereMesh         = new lsMesh{};
    lsMesh* pFontMesh           = new lsMesh{};
    lsAtlas* pAtlas             = new lsAtlas{};
    lsTexture* fbDepthTex       = new lsTexture{};
    lsTexture* fbColorTex       = new lsTexture{};
    bool ret                    = true;
    
    if (!pMeshLoader
    || !pFontLoader
    || !pSphereMesh
    || !pFontMesh
    || !pAtlas
    || !pMeshLoader->loadSphere(16)
    || !pSphereMesh->init(*pMeshLoader)
    || !pFontLoader->loadFile(TEST_FONT_FILE)
    || !pAtlas->init(*pFontLoader)
    || !pFontMesh->init(*pAtlas, "Hello World")
    || !fbDepthTex->init(0, LS_GRAY_8, vec2i{TEST_FRAMEBUFFER_WIDTH, TEST_FRAMEBUFFER_HEIGHT}, LS_GRAY, LS_UNSIGNED_BYTE, nullptr)
    || !fbColorTex->init(0, LS_RGB_8, vec2i{TEST_FRAMEBUFFER_WIDTH, TEST_FRAMEBUFFER_HEIGHT}, LS_RGB, LS_UNSIGNED_BYTE, nullptr)
    ) {
        ret = false;
    }
    else {
        pScene->manageMesh(pSphereMesh); // test data at the mesh index 0
        pScene->manageMesh(pFontMesh);
        pScene->manageAtlas(pAtlas);
        pScene->manageTexture(fbDepthTex);
        pScene->manageTexture(fbColorTex);
    }
    
    delete pMeshLoader;
    delete pFontLoader;
    
    return ret;
}

/******************************************************************************
 * Initialize the model, view, and projection matrices
******************************************************************************/
bool fbState::initMatrices() {
    // setup the matrix stacks
    pMatStack->loadMatrix(
        LS_PROJECTION_MATRIX,
        math::perspective(
            TEST_PROJECTION_FOV, 4.f/3.f,
            TEST_PROJECTION_NEAR, TEST_PROJECTION_FAR
        )
    );
    pMatStack->loadMatrix(
        LS_VIEW_MATRIX,
        math::lookAt(vec3((float)TEST_MAX_SCENE_OBJECTS), vec3(0.f), vec3(0.f, 1.f, 0.f))
    );
    pMatStack->constructVp();
    
    meshProg.bind();
    const GLint mvpId = meshProg.getUniformLocation(VP_MATRIX_UNIFORM);
    LOG_GL_ERR();
    
    if (mvpId == -1) {
        return false;
    }
    else {
        meshProg.setUniformValue(mvpId, pMatStack->getVpMatrix());
    }
    
    // initialize the test mesh translations
    unsigned matIter = 0;
    const int numObjects = TEST_MAX_SCENE_OBJECTS/2;
    for (int i = -numObjects; i < numObjects; ++i) {
        for (int j = -numObjects; j < numObjects; ++j) {
            for (int k = -numObjects; k < numObjects; ++k) {
                pModelMatrices[matIter] = math::translate(mat4{TEST_INSTANCE_RADIUS}, vec3{(float)i,(float)j,(float)k});
                ++matIter;
            }
        }
    }
    
    return true;
}

/******************************************************************************
 * Initialize the program shaders
******************************************************************************/
bool fbState::initShaders() {
    vertexShader vertShader;
    fragmentShader fragShader;
    fragmentShader fontFragShader;

    if (!vertShader.compile(meshVSData)
    ||  !fragShader.compile(meshFSData)
    ||  !fontFragShader.compile(fontFSData)
    ) {
        return false;
    }
    else {
        LOG_GL_ERR();
    }
    
    if (!meshProg.attachShaders(vertShader, fragShader)
    ||  !meshProg.link()
    ||  !fontProg.attachShaders(vertShader, fontFragShader)
    ||  !fontProg.link()
    ) {
        return false;
    }
    else {
        LOG_GL_ERR();
    }
    
    return true;
}

/******************************************************************************
 * Create the draw models that will be used for rendering
******************************************************************************/
bool fbState::initDrawModels() {
    // test model 1
    lsDrawModel* const pModel = new lsDrawModel{};
    if (pModel == nullptr) {
        LS_LOG_ERR("Unable to generate test draw model 1");
        return false;
    }
    else {
        pScene->manageModel(pModel);
        lsMesh* pMesh = pScene->getMeshList()[0];
        pModel->init(*pMesh, pScene->getDefaultTexture());

         // lights, camera, batch!
        pModel->setNumInstances(TEST_MAX_SCENE_INSTANCES, pModelMatrices);
    }
    
    // font/text model
    lsDrawModel* const pTextModel = new lsDrawModel{};
    if (pTextModel == nullptr) {
        LS_LOG_ERR("Unable to generate test draw model 2");
        return false;
    }
    else {
        pScene->manageModel(pTextModel);
        lsMesh* pTextMesh = pScene->getMeshList()[1];
        pTextModel->init(*pTextMesh, pScene->getAtlas(0)->getTexture());

        mat4 modelMat = {1.f};
        pTextModel->setNumInstances(1, &modelMat);
    }
    
    LOG_GL_ERR();
    
    return true;
}

/******************************************************************************
 * Initialize the framebuffers
******************************************************************************/
bool fbState::initFramebuffers() {
    lsTexture* const pDepthTex = pScene->getTexture(0);
    lsTexture* const pColorTex = pScene->getTexture(1);
    
    // setup the test framebuffer depth texture
    pDepthTex->bind();
        pDepthTex->setParameter(LS_TEX_MIN_FILTER, LS_FILTER_LINEAR);
        pDepthTex->setParameter(LS_TEX_MAG_FILTER, LS_FILTER_LINEAR);
        pDepthTex->setParameter(LS_TEX_WRAP_S, LS_TEX_CLAMP_EDGE);
        pDepthTex->setParameter(LS_TEX_WRAP_T, LS_TEX_CLAMP_EDGE);
    pDepthTex->unbind();
    
    LOG_GL_ERR();
    
    // framebuffer color texture
    pColorTex->bind();
        pColorTex->setParameter(LS_TEX_MIN_FILTER, LS_FILTER_LINEAR);
        pColorTex->setParameter(LS_TEX_MAG_FILTER, LS_FILTER_LINEAR);
        pColorTex->setParameter(LS_TEX_WRAP_S, LS_TEX_CLAMP_EDGE);
        pColorTex->setParameter(LS_TEX_WRAP_T, LS_TEX_CLAMP_EDGE);
    pColorTex->unbind();
    
    LOG_GL_ERR();
    
    testFb.bind();
        testFb.attachTexture(LS_DEPTH_ATTACHMENT, LS_FBO_2D_TARGET, *pDepthTex);
        testFb.attachTexture(LS_COLOR_ATTACHMENT0, LS_FBO_2D_TARGET, *pColorTex);
    testFb.unbind();
    
    LOG_GL_ERR();
    
    if (lsFramebuffer::getStatus(testFb) != LS_FBO_COMPLETE) {
        return false;
    }
    
    return true;
}

/******************************************************************************
 * Post-Initialization renderer parameters
******************************************************************************/
void fbState::setRendererParams() {
    lsRenderer renderer;
    renderer.setDepthTesting(true);
    renderer.setFaceCulling(true);
    pBlender->setBlendEquation(LS_BLEND_ADD, LS_BLEND_ADD);
    pBlender->setBlendFunction(LS_ONE, LS_ONE_MINUS_SRC_ALPHA, LS_ONE, LS_ZERO);
}

/******************************************************************************
 * Starting state
******************************************************************************/
bool fbState::onStart() {
    if (!initMemory()) {
        LS_LOG_ERR("An error occurred while initializing the batch state.");
        terminate();
        return false;
    }
    
    if (!initFileData()
    ||  !initShaders()
    ||  !initFramebuffers()
    ||  !initMatrices()
    ||  !initDrawModels()
    ) {
        LS_LOG_ERR("An error occurred while initializing the test state's resources");
        terminate();
        return false;
    }
    else {
        setRendererParams();
    }
    
    return true;
}

/******************************************************************************
 * Stopping state
******************************************************************************/
void fbState::onStop() {
    terminate();
}

/******************************************************************************
 * Running state
******************************************************************************/
void fbState::onRun(float dt) {
    updateKeyStates(dt);
    
    // Meshes all contain their own model matrices. no need to use the ones in
    // the matrix stack. Just greab the view matrix
    pMatStack->pushMatrix(LS_VIEW_MATRIX, math::quatToMat4(orientation));
    pMatStack->constructVp();
    
    const math::mat4& viewDir = pMatStack->getMatrix(LS_VIEW_MATRIX);
    const math::vec3 camPos = vec3{viewDir[0][2], viewDir[1][2], viewDir[2][2]};
    
    meshProg.bind();
    const GLuint mvpId = meshProg.getUniformLocation(CAMERA_POSITION_UNIFORM);
    meshProg.setUniformValue(mvpId, camPos);
    
    drawScene();
    
    pMatStack->popMatrix(LS_VIEW_MATRIX);
}

/******************************************************************************
 * Pausing state
******************************************************************************/
void fbState::onPause(float dt) {
    (void)dt;
    pMatStack->pushMatrix(LS_VIEW_MATRIX, math::quatToMat4(orientation));
    pMatStack->constructVp();
    
    drawScene();
    
    pMatStack->popMatrix(LS_VIEW_MATRIX);
}

/******************************************************************************
 * Get a string representing the current Ms/s and F/s
******************************************************************************/
std::string fbState::getTimingStr() const {
    const float tickTime = getParentSystem().getTickTime() * 0.001f;
    return lsUtil::toString(tickTime) + "MS\n" + lsUtil::toString(1.f/tickTime) + "FPS";
}

/******************************************************************************
 * get a 2d viewport for 2d/gui drawing
******************************************************************************/
mat4 fbState::get2dViewport() const {
    const vec2&& displayRes = (vec2)getParentSystem().getDisplay().getResolution();
    return math::ortho(
        0.f, displayRes[0],
        0.f, displayRes[1],
        -1.f, 1.f
    );
}

/******************************************************************************
 * get a 2d viewport for 2d/gui drawing
******************************************************************************/
mat4 fbState::get3dViewport() const {
    const vec2i displayRes = getParentSystem().getDisplay().getResolution();
    return math::perspective(
        TEST_PROJECTION_FOV, (float)displayRes[0]/displayRes[1],
        TEST_PROJECTION_NEAR, TEST_PROJECTION_FAR
    );
}

/******************************************************************************
 * Update the renderer's viewport with the current window resolution
******************************************************************************/
void fbState::resetGlViewport() {
    lsDisplay& disp = getParentSystem().getDisplay();
    lsRenderer renderer;
    
    renderer.setViewport(vec2i{0}, disp.getResolution());
}

/******************************************************************************
 * Drawing a scene
******************************************************************************/
void fbState::drawScene() {
    LOG_GL_ERR();
    
    drawMeshes();
    drawStrings();
}

/******************************************************************************
 * Drawing the scene's opaque meshes
******************************************************************************/
void fbState::drawMeshes() {
    // setup a viewport for a custom framebuffer
    lsRenderer renderer;
    renderer.setViewport(vec2i{0}, vec2i{TEST_FRAMEBUFFER_WIDTH, TEST_FRAMEBUFFER_HEIGHT});

    // use render to the framebuffer's color attachment
    static const ls_fbo_attach_t fboDrawAttachments[] = {LS_COLOR_ATTACHMENT0};

    // setup the framebuffer for draw operations
    testFb.setAccessType(LS_DRAW_FRAMEBUFFER);
    testFb.bind();
    testFb.setDrawTargets(1, fboDrawAttachments);
    testFb.clear((ls_fbo_mask_t)(LS_DEPTH_MASK | LS_COLOR_MASK));
    
    // setup a view matrix for the opaque mesh shader
    meshProg.bind();
    const GLuint mvpId = meshProg.getUniformLocation(VP_MATRIX_UNIFORM);
    meshProg.setUniformValue(mvpId, pMatStack->getVpMatrix());
    
    // draw a test mesh
    lsDrawModel* const pTestModel = pScene->getModelList()[0];
    pTestModel->draw();
    
    // restore draw operations to the default GL framebuffer
    testFb.unbind();
    
    // setup the custom framebuffer for read operations
    testFb.setAccessType(LS_READ_FRAMEBUFFER);
    testFb.bind();

    // blit the custom framebuffer to OpenGL's backbuffer
    testFb.blit(
        vec2i{0}, vec2i{TEST_FRAMEBUFFER_WIDTH, TEST_FRAMEBUFFER_HEIGHT},
        vec2i{0}, this->getParentSystem().getDisplay().getResolution(),
        LS_COLOR_MASK
    );
    
    // restore framebuffer reads to OpenGL's backbuffer
    testFb.unbind();
    
    // reset the GL viewport back to normal
    resetGlViewport();
}

/******************************************************************************
 * Drawing the scene's string models
******************************************************************************/
void fbState::drawStrings() {
    fontProg.bind();
    const GLint fontMvpId = meshProg.getUniformLocation(VP_MATRIX_UNIFORM);
    const mat4&& orthoProj = get2dViewport();
    meshProg.setUniformValue(fontMvpId, orthoProj);
    
    // setup some UI parameters
    const float screenResY = (float)getParentSystem().getDisplay().getResolution()[1];
    mat4 modelMat = math::translate(mat4{1.f}, vec3{0.f, screenResY, 0.f});
    modelMat = math::scale(modelMat, vec3{10.f});
    
    // Regenerate a string mesh using the frame's timing information.
    lsAtlas* const pStringAtlas = pScene->getAtlas(0);
    lsMesh* const pStringMesh = pScene->getMesh(1);
    pStringMesh->init(*pStringAtlas, getTimingStr());
    
    // model 1 has the string mesh already bound
    lsDrawModel* const pStringModel = pScene->getModelList()[1];
    pStringModel->setNumInstances(1, &modelMat);

    // setup parameters to draw a transparent mesh as a screen overlay/UI
    lsRenderer renderer;
    renderer.setDepthTesting(false);
    pBlender->bind();
    pStringModel->draw();
    pBlender->unbind();
    renderer.setDepthTesting(true);
}