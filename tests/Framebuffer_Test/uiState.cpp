/* 
 * File:   uiState.cpp
 * Author: miles
 * 
 * Created on August 5, 2014, 9:18 PM
 */

#include <utility>
#include "uiState.h"

#define TEST_FONT_FILE L"FiraSans-Regular.otf"

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

out vec2 uvCoords;

void main() {
    gl_Position = vpMatrix * inModelMat * vec4(inPos, 1.0);
    uvCoords = inUv;
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
uiState::uiState() {
}

uiState::uiState(uiState&& state) :
    lsGameState{}
{
    *this = std::move(state);
}

uiState::~uiState() {
}

uiState& uiState::operator=(uiState&& state) {
    lsGameState::operator=(std::move(state));
    
    fontProg = std::move(state.fontProg);
    
    pScene = state.pScene;
    state.pScene = nullptr;
    
    pBlender = std::move(state.pBlender);
    
    return *this;
}

/******************************************************************************
 * Allocate internal class memory
******************************************************************************/
bool uiState::initMemory() {
    pScene          = new lsSceneManager{};
    pBlender        = new lsBlendObject{};
    
    if (pScene == nullptr
    ||  !pScene->init()
    ||  !pBlender
    ) {
        return false;
    }
    
    return true;
}

/******************************************************************************
 * Initialize resources from files
******************************************************************************/
bool uiState::initFileData() {
    
    lsFontResource* pFontLoader = new lsFontResource{};
    lsMesh* pFontMesh           = new lsMesh{};
    lsAtlas* pAtlas             = new lsAtlas{};
    bool ret                    = true;
    
    if (!pFontLoader
    || !pFontMesh
    || !pAtlas
    || !pFontLoader->loadFile(TEST_FONT_FILE)
    || !pAtlas->init(*pFontLoader)
    || !pFontMesh->init(*pAtlas, "Hello World")
    ) {
        ret = false;
    }
    else {
        pScene->manageMesh(pFontMesh);
        pScene->manageAtlas(pAtlas);
    }
    
    delete pFontLoader;
    
    return ret;
}

/******************************************************************************
 * Initialize the program shaders
******************************************************************************/
bool uiState::initShaders() {
    vertexShader vertShader;
    fragmentShader fontFragShader;

    if (!vertShader.compile(meshVSData)
    ||  !fontFragShader.compile(fontFSData)
    ) {
        return false;
    }
    else {
        LOG_GL_ERR();
    }
    
    if (!fontProg.attachShaders(vertShader, fontFragShader) || !fontProg.link()) {
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
bool uiState::initDrawModels() {
    // font/text model
    lsDrawModel* const pTextModel = new lsDrawModel{};
    if (pTextModel == nullptr) {
        LS_LOG_ERR("Unable to generate test text model");
        return false;
    }
    else {
        pScene->manageModel(pTextModel);
        lsMesh* const pTextMesh = pScene->getMeshList()[0];
        pTextModel->init(*pTextMesh, pScene->getAtlas(0)->getTexture());

        math::mat4 modelMat = {1.f};
        pTextModel->setNumInstances(1, &modelMat);
    }
    
    LOG_GL_ERR();
    
    return true;
}

/******************************************************************************
 * Post-Initialization renderer parameters
******************************************************************************/
void uiState::setRendererParams() {
    pBlender->setState(true);
    pBlender->setBlendEquation(LS_BLEND_ADD, LS_BLEND_ADD);
    pBlender->setBlendFunction(LS_ONE, LS_ONE_MINUS_SRC_ALPHA, LS_ONE, LS_ZERO);
}

/******************************************************************************
 * Starting state
 * 
 * Resources that were already allocated are removed during "onStop()"
******************************************************************************/
bool uiState::onStart() {
    if (!initMemory()) {
        LS_LOG_ERR("An error occurred while initializing the batch state.");
        return false;
    }
    
    if (!initFileData()
    ||  !initShaders()
    ||  !initDrawModels()
    ) {
        LS_LOG_ERR("An error occurred while initializing the test state's resources");
        return false;
    }
    else {
        setRendererParams();
        getParentSystem().getDisplay().setFullScreenMode(LS_FULLSCREEN_WINDOW);
    }
    
    return true;
}

/******************************************************************************
 * Stopping state
******************************************************************************/
void uiState::onStop() {
    delete pScene;
    pScene = nullptr;
    
    delete pBlender;
    pBlender = nullptr;
}

/******************************************************************************
 * Running state
******************************************************************************/
void uiState::onRun(float) {
    drawScene();
}

/******************************************************************************
 * Pausing state
******************************************************************************/
void uiState::onPause(float) {
    drawScene();
}

/******************************************************************************
 * Get a string representing the current Ms/s and F/s
******************************************************************************/
std::string uiState::getTimingStr() const {
    const float tickTime = getParentSystem().getTickTime() * 0.001f;
    return lsUtil::toString(tickTime) + "MS\n" + lsUtil::toString(1.f/tickTime) + "FPS";
}

/******************************************************************************
 * get a 2d viewport for 2d/gui drawing
******************************************************************************/
math::mat4 uiState::get2dViewport() const {
    const lsDisplay& display = getParentSystem().getDisplay();
    const math::vec2&& displayRes = (math::vec2)display.getResolution();
    
    return math::ortho(
        0.f, displayRes[0],
        0.f, displayRes[1],
        0.f, 1.f
    );
}

/******************************************************************************
 * Update the renderer's viewport with the current window resolution
******************************************************************************/
void uiState::resetGlViewport() {
    const lsDisplay& disp = getParentSystem().getDisplay();
    lsRenderer renderer;
    renderer.setViewport(math::vec2i{0}, disp.getResolution());
}

/******************************************************************************
 * Drawing a scene
******************************************************************************/
void uiState::drawScene() {
    LOG_GL_ERR();
    
    fontProg.bind();
    const GLint fontMvpId           = fontProg.getUniformLocation("vpMatrix");
    const math::mat4&& orthoProj    = get2dViewport();
    fontProg.setUniformValue(fontMvpId, orthoProj);
    
    // setup some UI parameters
    const float screenResY  = (float)getParentSystem().getDisplay().getResolution()[1];
    math::mat4 modelMat     = math::translate(math::mat4{1.f}, math::vec3{0.f, screenResY, 0.f});
    modelMat                = math::scale(modelMat, math::vec3{10.f});
    
    // Regenerate a string mesh using the frame's timing information.
    lsAtlas* const pStringAtlas     = pScene->getAtlas(0);
    lsMesh* const pStringMesh       = pScene->getMesh(0);
    
    pStringMesh->init(*pStringAtlas, getTimingStr());
    
    // model 1 has the string mesh already bound
    lsDrawModel* const pStringModel = pScene->getModelList()[0];
    pStringModel->setNumInstances(1, &modelMat);

    // setup parameters to draw a transparent mesh as a screen overlay/UI
    lsRenderer renderer;
    renderer.setDepthTesting(false);
    pBlender->bind();
    pStringModel->draw();
    pBlender->unbind();
    renderer.setDepthTesting(true);
}
