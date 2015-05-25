#include <string>
//#include "sgct.h"
#include "scene.h"
#include "mesh.h"
#include "floor.h"
#include "sphere.h"

void init();
void draw();
void preSync();
void encode();
void decode();
void cleanUp();
void keyCallback(int key, int action);
void mouseButtonCallback(int button, int action);

// Declare an engine object for sgct
sgct::Engine * gEngine;
// Declare a scene for our simulation
Scene * scene;

// Some bodies for the scene
Body * cloth;
Body * sphere;
Body * floor_;

// Mouse stuff
bool mouseLeftButton = false;
double mouseDx = 0.0;
double mouseXPos[] = { 0.0, 0.0 };

// Toggle draw functions
unsigned int drawType = 0;

glm::vec3 view(0.0f, 0.0f, 1.0f);

sgct::SharedObject<glm::mat4> cameraRot;

float rotationSpeed = 0.2f;

// If time is required
sgct::SharedDouble curr_time(0.0);


int main(int argc, char* argv[]) {

    gEngine = new sgct::Engine(argc, argv);
    
    scene = new Scene();

    scene->setAcceleration(glm::vec3(0.0f, -1.0f, 0.0f) * 9.82f);

    gEngine->setInitOGLFunction(init);
    gEngine->setDrawFunction(draw);
    gEngine->setPreSyncFunction(preSync);
    gEngine->setCleanUpFunction(cleanUp);
    gEngine->setKeyboardCallbackFunction(keyCallback);
    gEngine->setMouseButtonCallbackFunction(mouseButtonCallback);
    sgct::SharedData::instance()->setEncodeFunction(encode);
    sgct::SharedData::instance()->setDecodeFunction(decode);

    if(!gEngine->init(sgct::Engine::OpenGL_3_3_Core_Profile)) {
        
        delete gEngine;
        return EXIT_FAILURE;
    }

    gEngine->render();

    delete gEngine;

    exit(EXIT_SUCCESS);
}


void init() {

    // Create our cloth mesh
    cloth = new Body(new Mesh(7, 1.0f, glm::vec3(0.0f, 2.5f, 0.0f)));
    //cloth->setup1();
    cloth->getShape()->setBodyStatic(42);
    //cloth->getShape()->setBodyStatic(43);
    //cloth->getShape()->setBodyStatic(44);
    //cloth->getShape()->setBodyStatic(45);
    //cloth->getShape()->setBodyStatic(46);
    //cloth->getShape()->setBodyStatic(47);
    cloth->getShape()->setBodyStatic(48);
    scene->addBody(cloth);


    sphere = new Body(new Sphere(2.0f, glm::vec3(0.0f, 0.0f, 3.0f)));
    scene->addBody(sphere);

    // Create a floor for some orientation help
    floor_ = new Body(new Floor(glm::vec3(0.0f, -3.0f, 0.0f), 30.0f, "checker"));
    scene->addBody(floor_);
    
    scene->init();
    
}


void draw() {
    scene->setTime(static_cast<float>(curr_time.getVal()));
    scene->setDt(gEngine->getDt());
    scene->step();
    scene->draw(gEngine->getActiveModelViewProjectionMatrix(), gEngine->getActiveModelViewMatrix(), cameraRot.getVal(), drawType);
}


void preSync() {
    if(gEngine->isMaster()) {
        curr_time.setVal(sgct::Engine::getTime());

        // Camera movement
        if( mouseLeftButton ) {
            double tmpYPos;
            //get the mouse pos from first window
            sgct::Engine::getMousePos( gEngine->getFocusedWindowIndex(), &mouseXPos[0], &tmpYPos );
            mouseDx = mouseXPos[0] - mouseXPos[1];
        }
        else {
            mouseDx = 0.0;
        }

        static float panRot = 0.0f;
        panRot += (static_cast<float>(mouseDx) * rotationSpeed * static_cast<float>(gEngine->getDt()));

        glm::mat4 ViewRotateX = glm::rotate(
            glm::mat4(1.0f),
            panRot,
            glm::vec3(0.0f, 1.0f, 0.0f)); //rotation around the y-axis

        // Some camera interaction matrices, make the camera
        // rotate around the scene
        glm::mat4 result = ViewRotateX;
        result *= glm::translate( glm::mat4(1.0f), sgct::Engine::getUserPtr()->getPos() );
        result *= glm::translate( glm::mat4(1.0f), -sgct::Engine::getUserPtr()->getPos() );
        
        cameraRot.setVal(result);
    }
}


void encode() {
    sgct::SharedData::instance()->writeDouble(&curr_time);
    sgct::SharedData::instance()->writeObj(&cameraRot);
}


void decode() {
    sgct::SharedData::instance()->readDouble(&curr_time);
    sgct::SharedData::instance()->readObj(&cameraRot);
}


void keyCallback(int key, int action)
{
    if( gEngine->isMaster() )
    {
        switch( key )
        {
        // Draw vertices or ploygons?
        case SGCT_KEY_K:
            if(action == SGCT_PRESS)
                drawType = (drawType == 0) ? 1 : 0;
            break;

        // Reset the simulation
        case SGCT_KEY_R:
            if(action == SGCT_PRESS)
                scene->reset();
            break;

        case SGCT_KEY_W:
            sphere->getShape()->setPosition(sphere->getShape()->getPosition() + glm::vec3(0.0f, 0.0f, -0.2f));
            break;

        case SGCT_KEY_S:
            sphere->getShape()->setPosition(sphere->getShape()->getPosition() + glm::vec3(0.0f, 0.0f, 0.2f));
            break;

        case SGCT_KEY_A:
            sphere->getShape()->setPosition(sphere->getShape()->getPosition() + glm::vec3(-0.2f, 0.0f, 0.0f));
            break;

        case SGCT_KEY_D:
            sphere->getShape()->setPosition(sphere->getShape()->getPosition() + glm::vec3(0.2f, 0.0f, 0.0f));
            break;

        case SGCT_KEY_Q:
            sphere->getShape()->setPosition(sphere->getShape()->getPosition() + glm::vec3(0.0f, -0.2f, 0.0f));
            break;

        case SGCT_KEY_E:
            sphere->getShape()->setPosition(sphere->getShape()->getPosition() + glm::vec3(0.0f, 0.2f, 0.0f));
            break;

        case SGCT_KEY_1:
            if(action == SGCT_PRESS)
                cloth->getShape()->setup1();
            break;

        case SGCT_KEY_2:
            if(action == SGCT_PRESS)
                cloth->getShape()->setup2();
            break;
        }
    }
}


void mouseButtonCallback(int button, int action) {

    if(gEngine->isMaster()) {
        switch(button) {
            
        case SGCT_MOUSE_BUTTON_LEFT:
            mouseLeftButton = (action == SGCT_PRESS ? true : false);
            double tmpYPos;
            sgct::Engine::getMousePos( gEngine->getFocusedWindowIndex(), &mouseXPos[1], &tmpYPos );
            break;
        }
    }
}


void cleanUp() {
    delete scene;
}