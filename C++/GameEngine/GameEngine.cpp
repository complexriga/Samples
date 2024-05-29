#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>
#include <hidusage.h>
#include <Xinput.h>
//#include <sstream>

#include <GL/glew.h>
#include <GL/glut.h>  // GLUT, include glu.h and gl.h
#include <GL/wglew.h>

#include "GameTools.h"

using namespace std;
using namespace tools::image;
using namespace tools::audio;
using namespace gametools;
using namespace gametools::renderer;
using namespace gametools::sprite;
using namespace gametools::container;
using namespace gametools::utilities;

const size_t MM_CLASS_ID = 10;
const size_t BULLET_CLASS_ID = 20;

shared_ptr <Level> olTestLevel;
shared_ptr <gdi::AlphaRenderer> olGDIRenderer;
shared_ptr <ogl::AlphaRenderer> olOGLRenderer;

// Configuration Variables
unsigned int blAudPlay = 10;

bool blRightRun = true;
size_t ilLastPackeNumber;


//chrono::time_point <chrono::steady_clock> inter_start = chrono::time_point <chrono::steady_clock> ();
//chrono::time_point <chrono::steady_clock> inter_stop = chrono::time_point <chrono::steady_clock> ();
//chrono::milliseconds min_duration = chrono::milliseconds(1000);
//chrono::milliseconds max_duration = chrono::milliseconds(0);
//chrono::milliseconds min_inter_duration = chrono::milliseconds(1000);
//chrono::milliseconds max_inter_duration = chrono::milliseconds(0);

const function <bool(shared_ptr <TileInstance> opInstance, shared_ptr <const EVENT> opEvent)> MMClassBehaviour = [](shared_ptr <TileInstance> opInstance, shared_ptr <const EVENT> opEvent) {
    if(opEvent->code == EVENT::INSTANCE_CREATION)
        opInstance->setActiveStateIndex(blRightRun? 2: 3);
    else if(opEvent->code == EVENT::DRAW) {
        if(opInstance->getPosition().x + (long) opInstance->getClass()->getState(opInstance->getActiveStateIndex())->getWidth() < opInstance->getOwnerLevel()->getScreen().originX) {
            opInstance->setX(opInstance->getOwnerLevel()->getScreen().width);
            opInstance->resetDelta();
        } else if(opInstance->getPosition().x > opInstance->getOwnerLevel()->getScreen().originX + (long) opInstance->getOwnerLevel()->getScreen().width) {
            opInstance->setX(((long) opInstance->getClass()->getState(opInstance->getActiveStateIndex())->getWidth()) * -1);
            opInstance->resetDelta();
        } else
            opInstance->setX(TileInstance::PERCENT_100, TileInstance::POS_PERCENTAGE);
    }

    return true;
};

const function <bool(shared_ptr <TileInstance> opInstance, shared_ptr <const EVENT> opEvent)> MMRunStateBehaviour = [](shared_ptr <TileInstance> opInstance, shared_ptr <const EVENT> opEvent) {
    if(opEvent->code == EVENT::USER_INPUT_KEY_UP) {
        switch(opEvent->value) {
            case L'a':
            case L'A':
                 blRightRun = false;
                 opInstance->setActiveStateIndex(3, TileInstance::ALIGN_BOTTOM_EDGE);
                 break;
            case L'd':
            case L'D':
                 blRightRun = true;
                 opInstance->setActiveStateIndex(2, TileInstance::ALIGN_BOTTOM_EDGE);
                 break;
        }
        return true;
    } else if(opEvent->code == EVENT::USER_INPUT_GAMEPAD_UP) {
        if(opEvent->value & EVENT::GAMEPAD_DPAD_LEFT) {
                blRightRun = false;
                opInstance->setActiveStateIndex(3, TileInstance::ALIGN_BOTTOM_EDGE);
        } else if(opEvent->value & EVENT::GAMEPAD_DPAD_RIGHT) {
            blRightRun = true;
            opInstance->setActiveStateIndex(2, TileInstance::ALIGN_BOTTOM_EDGE);
        }
        return true;
    } else if(opEvent->code == EVENT::CAPABILITY_CHECK && (opEvent->value == EVENT::IS_DISPLAYABLE || opEvent->value == EVENT::IS_PASSIVE_COLLISION_CAPABLE))
        return true;

    return false;
};

const function <bool(shared_ptr <TileInstance> opInstance, shared_ptr <const EVENT> opEvent)> MMStandStateBehaviour = [](shared_ptr <TileInstance> opInstance, shared_ptr <const EVENT> opEvent) {
    if(opEvent->code == EVENT::USER_INPUT_KEY_DOWN) {
        switch(opEvent->value) {
            case L'a':
            case L'A':
                opInstance->setActiveStateIndex(1, TileInstance::ALIGN_BOTTOM_EDGE | TileInstance::ALIGN_RIGHT_EDGE);
                blRightRun = false;
                break;
            case L'd':
            case L'D':
                opInstance->setActiveStateIndex(0, TileInstance::ALIGN_BOTTOM_EDGE | TileInstance::ALIGN_LEFT_EDGE);
                blRightRun = true;
                break;
            case L'w':
            case L'W':
                opInstance->setActiveStateIndex(blRightRun? 4: 5, TileInstance::ALIGN_BOTTOM_EDGE | (blRightRun ? TileInstance::ALIGN_LEFT_EDGE: TileInstance::ALIGN_RIGHT_EDGE));
                break;
        }
        return true;
    } else if(opEvent->code == EVENT::USER_INPUT_GAMEPAD_DOWN) {
        if(opEvent->value & EVENT::GAMEPAD_DPAD_LEFT) {
            opInstance->setActiveStateIndex(1, TileInstance::ALIGN_BOTTOM_EDGE | TileInstance::ALIGN_RIGHT_EDGE);
            blRightRun = false;
        } else if(opEvent->value & EVENT::GAMEPAD_DPAD_RIGHT) {
            opInstance->setActiveStateIndex(0, TileInstance::ALIGN_BOTTOM_EDGE | TileInstance::ALIGN_LEFT_EDGE);
            blRightRun = true;
        } else if(opEvent->value & EVENT::GAMEPAD_X)
                opInstance->setActiveStateIndex(blRightRun ? 4 : 5, TileInstance::ALIGN_BOTTOM_EDGE | (blRightRun ? TileInstance::ALIGN_LEFT_EDGE : TileInstance::ALIGN_RIGHT_EDGE));
        return true;
    } else if(opEvent->code == EVENT::CAPABILITY_CHECK && (opEvent->value == EVENT::IS_DISPLAYABLE || opEvent->value == EVENT::IS_PASSIVE_COLLISION_CAPABLE))
        return true;

    return false;
};

const function <bool(shared_ptr <TileInstance> opInstance, shared_ptr <const EVENT> opEvent)> MMJumpStateBehaviour = [](shared_ptr <TileInstance> opInstance, shared_ptr <const EVENT> opEvent) {
    if(opEvent->code == EVENT::DRAW) {
        if(opInstance->isSpriteActive(0) && opInstance->isSpriteFinished(0))
            opInstance->setActiveStateIndex(blRightRun? 2: 3, TileInstance::ALIGN_BOTTOM_EDGE | (blRightRun ? TileInstance::ALIGN_LEFT_EDGE : TileInstance::ALIGN_RIGHT_EDGE));
        return true;
    } else if(opEvent->code == EVENT::CAPABILITY_CHECK && (opEvent->value == EVENT::IS_DISPLAYABLE || opEvent->value == EVENT::IS_PASSIVE_COLLISION_CAPABLE))
        return true;

    return false;
};

const function <bool(shared_ptr <TileInstance> opInstance, shared_ptr <const EVENT> opEvent)> BulletClassBehaviour = [](shared_ptr <TileInstance> opInstance, shared_ptr <const EVENT> opEvent) {
    if(!opInstance->isInitialized())
        return false;

    if(opEvent->code == EVENT::CLASS_REGISTRATION) {
        opInstance->setActiveStateIndex(0);
        opInstance->setFrameIndex(0, 0);
        opInstance->setCollisionType(TileInstance::COLLISION_INTERCEPTION);
    }

    return true;
};

const function <bool(shared_ptr <TileInstance> opInstance, shared_ptr <const EVENT> opEvent)> BulletStateBehaviour = [](shared_ptr <TileInstance> opInstance, shared_ptr <const EVENT> opEvent) {
    if(opEvent->code == EVENT::DRAW) {
        if(opInstance->getPosition().x + (long) opInstance->getClass()->getState(opInstance->getActiveStateIndex())->getWidth() <= opInstance->getOwnerLevel()->getScreen().originX) {
            opInstance->setX(opInstance->getOwnerLevel()->getScreen().width);
            opInstance->resetDelta();
        } else
            opInstance->setX(TileInstance::PERCENT_100, TileInstance::POS_PERCENTAGE);
        blAudPlay++;
        return true;
    } else if(opEvent->code == EVENT::COLLISION_REQUEST)
        return opEvent->value == MM_CLASS_ID;
    else if(blAudPlay >= 10 && opEvent->code == EVENT::ACTIVE_COLLISION) {
        opInstance->setAudioCommand(0, AudioCommand::PLAY_SOUND);
        blAudPlay = 0;
    } else if(opEvent->code == EVENT::CAPABILITY_CHECK && (opEvent->value == EVENT::IS_DISPLAYABLE || opEvent->value == EVENT::IS_ACTIVE_COLLISION_CAPABLE))
        return true;

    return false;
};

const function <bool(shared_ptr <Level> opLevel, shared_ptr <const EVENT> opEvent)> TestLevelBehaviour = [](shared_ptr <Level> opLevel, shared_ptr <const EVENT> opEvent) {
    bool blResult = false;

    switch(opEvent->code) {
        case EVENT::LEVEL_CREATION:
            {
                shared_ptr <ResourceManager> olManager = ResourceManager::getInstance();
                shared_ptr <vector <size_t>> alBitmaps = make_shared <vector <size_t>> ();
        
                RGBA rlMaskColor = {0, 0, 255, 255};
                shared_ptr <vector <size_t>> alTileAudios = make_shared <vector <size_t>> ();
                shared_ptr <vector <size_t>> alLevelAudios = make_shared <vector <size_t>> ();

                alBitmaps->push_back(olManager->addBitmap("D:\\Users\\Andrés Giraldo\\Desktop\\Sprites\\Test3\\walking_right_132x132_10.png", rlMaskColor, true));
                alBitmaps->push_back(olManager->addBitmap("D:\\Users\\Andrés Giraldo\\Desktop\\Sprites\\Test3\\walking_left_132x132_10.png", rlMaskColor, true));
                alBitmaps->push_back(olManager->addBitmap("D:\\Users\\Andrés Giraldo\\Desktop\\Sprites\\Test3\\stand_right_120x120_6.png", rlMaskColor, true));
                alBitmaps->push_back(olManager->addBitmap("D:\\Users\\Andrés Giraldo\\Desktop\\Sprites\\Test3\\stand_left_120x120_6.png", rlMaskColor, true));
                alBitmaps->push_back(olManager->addBitmap("D:\\Users\\Andrés Giraldo\\Desktop\\Sprites\\Test3\\jump_right_162x162_5.png", rlMaskColor, true));
                alBitmaps->push_back(olManager->addBitmap("D:\\Users\\Andrés Giraldo\\Desktop\\Sprites\\Test3\\jump_left_162x162_5.png", rlMaskColor, true));

                alBitmaps->push_back(olManager->addBitmap("D:\\Users\\Andrés Giraldo\\Desktop\\Sprites\\Test2\\Alpha\\bullet_120x120_1.png", rlMaskColor, true));
                alTileAudios->push_back(olManager->addAudio("D:\\Users\\Andrés Giraldo\\Desktop\\Sprites\\Audios\\WAV\\Effects\\07 - MegamanDamage.wav", true));
                alLevelAudios->push_back(olManager->addAudio("D:\\Users\\Andrés Giraldo\\Desktop\\Sprites\\Audios\\MIDI\\megaman2intro.mid", true));

                // Given that the Sprites are building their own Borders, Images must be loaded before Sprites are created.
                olManager->loadBitmaps(alBitmaps);
                olManager->waitBitmapsLoad();
                olManager->loadAudios(alTileAudios);
                olManager->waitAudiosLoad();

                unsigned long ilRunTicks = 8;
                unsigned int ilRunTime = 4;
                long ilRunLenght = 8;

                shared_ptr <vector <shared_ptr <TileState>>> alMMStates = make_shared <vector <shared_ptr <TileState>>> ();

                shared_ptr <vector <shared_ptr <Sprite>>> olRightRunSprite = make_shared <vector <shared_ptr <Sprite>>> ();
                shared_ptr <vector <Sprite::FrameDescriptor>> olRightRunFrames = make_shared <vector <Sprite::FrameDescriptor>> ();
                for(unsigned int i = 0; i < 10; i++)
                    if(i == 0)
                        olRightRunFrames->push_back({i, {0, 0, 0}, ilRunTicks, chrono::milliseconds(ilRunTime), {ilRunLenght, 0, 0}, {0, 0}, 0, Sprite::MARK_FORWARD | Sprite::MARK_NEXT, 0, make_shared <BORDER_TIER> (), make_shared <BORDER_TIER> ()});
                    else if(i < 9)
                        olRightRunFrames->push_back({i, {0, 0, 0}, ilRunTicks, chrono::milliseconds(ilRunTime), {ilRunLenght, 0, 0}, {0, 0}, 0, Sprite::MARK_NULL, 0, make_shared <BORDER_TIER>(), make_shared <BORDER_TIER>()});
                    else
                        olRightRunFrames->push_back({i, {0, 0, 0}, ilRunTicks, chrono::milliseconds(ilRunTime), {ilRunLenght, 0, 0}, {0, 0}, 0, Sprite::MARK_FOR, Sprite::ALWAYSLOOP, make_shared <BORDER_TIER>(), make_shared <BORDER_TIER>()});
                olRightRunSprite->push_back(SpriteCreator::create(alBitmaps->at(0), false, olRightRunFrames, SpriteCreator::BY_ALPHA, SpriteCreator::BORDER_LOOSE_CONTOUR, SpriteCreator::BORDER_LOOSE_CONTOUR));
                alMMStates->push_back(shared_ptr <TileState>(make_shared <TileState>(olRightRunSprite, make_shared <vector <size_t>>(), MMRunStateBehaviour)));

                shared_ptr <vector <shared_ptr <Sprite>>> olLeftRunSprites = make_shared <vector <shared_ptr <Sprite>>>();
                shared_ptr <vector <Sprite::FrameDescriptor>> olLeftRunFrames = make_shared <vector <Sprite::FrameDescriptor>> ();
                for(unsigned int i = 0; i < 10; i++)
                    if(i == 0)
                        olLeftRunFrames->push_back({i, {0, 0, 0}, ilRunTicks, chrono::milliseconds(ilRunTime), {-ilRunLenght, 0, 0}, {0, 0}, 0, Sprite::MARK_FORWARD | Sprite::MARK_NEXT, 0, make_shared <BORDER_TIER> (), make_shared <BORDER_TIER> ()});
                    else if(i < 9)
                        olLeftRunFrames->push_back({i, {0, 0, 0}, ilRunTicks, chrono::milliseconds(ilRunTime), {-ilRunLenght, 0, 0}, {0, 0}, 0, Sprite::MARK_NULL, 0, make_shared <BORDER_TIER>(), make_shared <BORDER_TIER>()});
                    else
                        olLeftRunFrames->push_back({i, {0, 0, 0}, ilRunTicks, chrono::milliseconds(ilRunTime), {-ilRunLenght, 0, 0}, {0, 0}, 0, Sprite::MARK_FOR, Sprite::ALWAYSLOOP, make_shared <BORDER_TIER>(), make_shared <BORDER_TIER>()});
                olLeftRunSprites->push_back(SpriteCreator::create(alBitmaps->at(1), false, olLeftRunFrames, SpriteCreator::BY_ALPHA, SpriteCreator::BORDER_LOOSE_CONTOUR, SpriteCreator::BORDER_LOOSE_CONTOUR));
                alMMStates->push_back(shared_ptr <TileState> (make_shared <TileState> (olLeftRunSprites, make_shared <vector <size_t>> (), MMRunStateBehaviour)));

                int ilStandTime = 80;

                shared_ptr <vector <shared_ptr <Sprite>>> olRightStandSprites = make_shared <vector <shared_ptr <Sprite>>> ();
                shared_ptr <vector <Sprite::FrameDescriptor>> olRightStandFrames = make_shared <vector <Sprite::FrameDescriptor>> ();
                for(unsigned int i = 0; i < 6; i++)
                        if(i == 0)
                            olRightStandFrames->push_back({i, {0, 0, 0}, 1, chrono::milliseconds(ilStandTime), {0, 0, 0}, {0, 0}, 0, Sprite::MARK_FORWARD, 0, make_shared <BORDER_TIER> (), make_shared <BORDER_TIER> ()});
                        else if(i < 5)
                            olRightStandFrames->push_back({i, {0, 0, 0}, 1, chrono::milliseconds(ilStandTime), {0, 0, 0}, {0, 0}, 0, Sprite::MARK_NULL, 0, make_shared <BORDER_TIER>(), make_shared <BORDER_TIER>()});
                        else
                            olRightStandFrames->push_back({i, {0, 0, 0}, 1, chrono::milliseconds(ilStandTime), {0, 0, 0}, {0, 0}, 0, Sprite::MARK_JUMP, 0, make_shared <BORDER_TIER>(), make_shared <BORDER_TIER>()});
                olRightStandSprites->push_back(SpriteCreator::create(alBitmaps->at(2), false, olRightStandFrames, SpriteCreator::BY_ALPHA, SpriteCreator::BORDER_LOOSE_CONTOUR, SpriteCreator::BORDER_LOOSE_CONTOUR));
                alMMStates->push_back(shared_ptr <TileState>(make_shared <TileState>(olRightStandSprites, make_shared <vector <size_t>>(), MMStandStateBehaviour)));

                shared_ptr <vector <shared_ptr <Sprite>>> olLeftStandSprites = make_shared <vector <shared_ptr <Sprite>>>();
                shared_ptr <vector <Sprite::FrameDescriptor>> olLeftStandFrames = make_shared <vector <Sprite::FrameDescriptor>> ();
                for(unsigned int i = 0; i < 6; i++)
                    if(i == 0)
                        olLeftStandFrames->push_back({i, {0, 0, 0}, 1, chrono::milliseconds(ilStandTime), {0, 0, 0}, {0, 0}, 0, Sprite::MARK_FORWARD, 0, make_shared <BORDER_TIER> (), make_shared <BORDER_TIER> ()});
                    else if(i < 5)
                        olLeftStandFrames->push_back({i, {0, 0, 0}, 1, chrono::milliseconds(ilStandTime), {0, 0, 0}, {0, 0}, 0, Sprite::MARK_NULL, 0, make_shared <BORDER_TIER>(), make_shared <BORDER_TIER>()});
                    else
                        olLeftStandFrames->push_back({i, {0, 0, 0}, 1, chrono::milliseconds(ilStandTime), {0, 0, 0}, {0, 0}, 0, Sprite::MARK_JUMP, 0, make_shared <BORDER_TIER>(), make_shared <BORDER_TIER>()});
                olLeftStandSprites->push_back(SpriteCreator::create(alBitmaps->at(3), false, olLeftStandFrames, SpriteCreator::BY_ALPHA, SpriteCreator::BORDER_LOOSE_CONTOUR, SpriteCreator::BORDER_LOOSE_CONTOUR));
                alMMStates->push_back(shared_ptr <TileState> (make_shared <TileState> (olLeftStandSprites, make_shared <vector <size_t>> (), MMStandStateBehaviour)));

                int ilJumpTime = 0;

                UI3DPOINT pos = {0, 0, 0};
                shared_ptr <vector <shared_ptr <Sprite>>> olRightJumpSprites = make_shared <vector <shared_ptr <Sprite>>> ();
                shared_ptr <vector <Sprite::FrameDescriptor>> olRightJumpFrames = make_shared <vector <Sprite::FrameDescriptor>> ();
                olRightJumpFrames->push_back({0, {0, 0, 0},  6, chrono::milliseconds(ilJumpTime)    , {0, -15, 0}, {0, 0}, 0, Sprite::MARK_FORWARD, 0, make_shared <BORDER_TIER> (), make_shared <BORDER_TIER> ()});
                olRightJumpFrames->push_back({1, {0, 0, 0},  8, chrono::milliseconds(ilJumpTime)    , {0, -13, 0}, {0, 0}, 0, Sprite::MARK_REPEAT, 10, make_shared <BORDER_TIER> (), make_shared <BORDER_TIER> ()});
                olRightJumpFrames->push_back({1, {0, 0, 0}, 10, chrono::milliseconds(ilJumpTime)    , {0, -10, 0}, {0, 0}, 0, Sprite::MARK_NULL, 0, make_shared <BORDER_TIER>(), make_shared <BORDER_TIER>()});
                olRightJumpFrames->push_back({2, {0, 0, 0}, 12, chrono::milliseconds(ilJumpTime)    , {0,  -8, 0}, {0, 0}, 0, Sprite::MARK_NULL, 0, make_shared <BORDER_TIER> (), make_shared <BORDER_TIER> ()});
                olRightJumpFrames->push_back({2, {0, 0, 0}, 13, chrono::milliseconds(ilJumpTime)    , {0,  -6, 0}, {0, 0}, 0, Sprite::MARK_NULL, 0, make_shared <BORDER_TIER>(), make_shared <BORDER_TIER>()});
                olRightJumpFrames->push_back({2, {0, 0, 0}, 14, chrono::milliseconds(ilJumpTime * 2), {0,  -5, 0}, {0, 0}, 0, Sprite::MARK_NULL, 0, make_shared <BORDER_TIER>(), make_shared <BORDER_TIER>()});
                olRightJumpFrames->push_back({3, {0, 0, 0}, 15, chrono::milliseconds(ilJumpTime * 3), {0,   0, 0}, {0, 0}, 0, Sprite::MARK_NULL, 0, make_shared <BORDER_TIER> (), make_shared <BORDER_TIER> ()});
                olRightJumpFrames->push_back({4, {0, 0, 0}, 14, chrono::milliseconds(ilJumpTime * 2), {0,   5, 0}, {0, 0}, 0, Sprite::MARK_NULL, 0, make_shared <BORDER_TIER> (), make_shared <BORDER_TIER> ()});
                olRightJumpFrames->push_back({4, {0, 0, 0}, 13, chrono::milliseconds(ilJumpTime)    , {0,   6, 0}, {0, 0}, 0, Sprite::MARK_NULL, 0, make_shared <BORDER_TIER> (), make_shared <BORDER_TIER> ()});
                olRightJumpFrames->push_back({4, {0, 0, 0}, 12, chrono::milliseconds(ilJumpTime)    , {0,   8, 0}, {0, 0}, 0, Sprite::MARK_NULL, 0, make_shared <BORDER_TIER> (), make_shared <BORDER_TIER> ()});
                olRightJumpFrames->push_back({4, {0, 0, 0}, 10, chrono::milliseconds(ilJumpTime)    , {0,  10, 0}, {0, 0}, 0, Sprite::MARK_NULL, 0, make_shared <BORDER_TIER>(), make_shared <BORDER_TIER>()});
                olRightJumpFrames->push_back({4, {0, 0, 0},  8, chrono::milliseconds(ilJumpTime)    , {0,  13, 0}, {0, 0}, 0, Sprite::MARK_REPEAT, 10, make_shared <BORDER_TIER>(), make_shared <BORDER_TIER>()});
                olRightJumpFrames->push_back({4, {0, 0, 0},  6, chrono::milliseconds(ilJumpTime)    , {0,  15, 0}, {0, 0}, 0, Sprite::MARK_STOP, 0, make_shared <BORDER_TIER>(), make_shared <BORDER_TIER>()});
                olRightJumpSprites->push_back(SpriteCreator::create(alBitmaps->at(4), false, olRightJumpFrames, SpriteCreator::BY_ALPHA, SpriteCreator::BORDER_LOOSE_CONTOUR, SpriteCreator::BORDER_LOOSE_CONTOUR));
                alMMStates->push_back(make_shared <TileState>(olRightJumpSprites, make_shared <vector <size_t>>(), MMJumpStateBehaviour));

                shared_ptr <vector <shared_ptr <Sprite>>> olLeftJumpSprites = make_shared <vector <shared_ptr <Sprite>>>();
                shared_ptr <vector <Sprite::FrameDescriptor>> olLeftJumpFrames = make_shared <vector <Sprite::FrameDescriptor>> ();
                olLeftJumpFrames->push_back({0, {0, 0, 0},  6, chrono::milliseconds(ilJumpTime)    , {0, -15, 0}, {0, 0}, 0, Sprite::MARK_FORWARD, 0, make_shared <BORDER_TIER>(), make_shared <BORDER_TIER>()});
                olLeftJumpFrames->push_back({1, {0, 0, 0},  8, chrono::milliseconds(ilJumpTime)    , {0, -13, 0}, {0, 0}, 0, Sprite::MARK_NULL, 0, make_shared <BORDER_TIER>(), make_shared <BORDER_TIER>()});
                olLeftJumpFrames->push_back({1, {0, 0, 0}, 10, chrono::milliseconds(ilJumpTime)    , {0, -10, 0}, {0, 0}, 0, Sprite::MARK_NULL, 0, make_shared <BORDER_TIER>(), make_shared <BORDER_TIER>()});
                olLeftJumpFrames->push_back({2, {0, 0, 0}, 12, chrono::milliseconds(ilJumpTime)    , {0,  -8, 0}, {0, 0}, 0, Sprite::MARK_NULL, 0, make_shared <BORDER_TIER>(), make_shared <BORDER_TIER>()});
                olLeftJumpFrames->push_back({2, {0, 0, 0}, 13, chrono::milliseconds(ilJumpTime)    , {0,  -6, 0}, {0, 0}, 0, Sprite::MARK_NULL, 0, make_shared <BORDER_TIER>(), make_shared <BORDER_TIER>()});
                olLeftJumpFrames->push_back({2, {0, 0, 0}, 14, chrono::milliseconds(ilJumpTime * 2), {0,  -5, 0}, {0, 0}, 0, Sprite::MARK_NULL, 0, make_shared <BORDER_TIER>(), make_shared <BORDER_TIER>()});
                olLeftJumpFrames->push_back({3, {0, 0, 0}, 15, chrono::milliseconds(ilJumpTime * 3), {0,   0, 0}, {0, 0}, 0, Sprite::MARK_NULL, 0, make_shared <BORDER_TIER>(), make_shared <BORDER_TIER>()});
                olLeftJumpFrames->push_back({4, {0, 0, 0}, 14, chrono::milliseconds(ilJumpTime * 2), {0,   5, 0}, {0, 0}, 0, Sprite::MARK_NULL, 0, make_shared <BORDER_TIER>(), make_shared <BORDER_TIER>()});
                olLeftJumpFrames->push_back({4, {0, 0, 0}, 13, chrono::milliseconds(ilJumpTime)    , {0,   6, 0}, {0, 0}, 0, Sprite::MARK_NULL, 0, make_shared <BORDER_TIER>(), make_shared <BORDER_TIER>()});
                olLeftJumpFrames->push_back({4, {0, 0, 0}, 12, chrono::milliseconds(ilJumpTime)    , {0,   8, 0}, {0, 0}, 0, Sprite::MARK_NULL, 0, make_shared <BORDER_TIER>(), make_shared <BORDER_TIER>()});
                olLeftJumpFrames->push_back({4, {0, 0, 0}, 10, chrono::milliseconds(ilJumpTime)    , {0,  10, 0}, {0, 0}, 0, Sprite::MARK_NULL, 0, make_shared <BORDER_TIER>(), make_shared <BORDER_TIER>()});
                olLeftJumpFrames->push_back({4, {0, 0, 0},  8, chrono::milliseconds(ilJumpTime)    , {0,  13, 0}, {0, 0}, 0, Sprite::MARK_NULL, 0, make_shared <BORDER_TIER>(), make_shared <BORDER_TIER>()});
                olLeftJumpFrames->push_back({4, {0, 0, 0},  6, chrono::milliseconds(ilJumpTime)    , {0,  15, 0}, {0, 0}, 0, Sprite::MARK_STOP, 0, make_shared <BORDER_TIER>(), make_shared <BORDER_TIER>()});
                olLeftJumpSprites->push_back(SpriteCreator::create(alBitmaps->at(5), pos, olRightJumpSprites->at(0)->getDivisionWidth(), olRightJumpSprites->at(0)->getDivisionHeight(), Sprite::LEFTWARD, 5, false, olLeftJumpFrames, SpriteCreator::BY_ALPHA, SpriteCreator::BORDER_LOOSE_CONTOUR, SpriteCreator::BORDER_LOOSE_CONTOUR));
                alMMStates->push_back(make_shared <TileState> (olLeftJumpSprites, make_shared <vector <size_t>> (), MMJumpStateBehaviour));

                shared_ptr <TileClass> olMM = make_shared <TileClass> (alMMStates, blRightRun? 2: 3, MMClassBehaviour);
                olMM->setAlternativeClassId(MM_CLASS_ID);
                opLevel->registerTileClass(olMM);

                shared_ptr <vector <shared_ptr <TileState>>> alBulletStates = make_shared <vector <shared_ptr <TileState>>> ();
                shared_ptr <vector <shared_ptr <Sprite>>> olBulletSprites = make_shared <vector <shared_ptr <Sprite>>> ();
                shared_ptr <vector <Sprite::FrameDescriptor>> olBulletFrames = make_shared <vector <Sprite::FrameDescriptor>> ();
                olBulletFrames->push_back({0, {0, 0, 0}, 1, chrono::milliseconds(50), {-10, 0, 0}, {0, 0}, 0, Sprite::MARK_REPEAT, Sprite::ALWAYSLOOP, make_shared <BORDER_TIER> (), make_shared <BORDER_TIER> ()});
                olBulletSprites->push_back(SpriteCreator::create(alBitmaps->at(6), false, olBulletFrames, SpriteCreator::BY_ALPHA, SpriteCreator::BORDER_BOX));
                alBulletStates->push_back(shared_ptr <TileState> (make_shared <TileState> (olBulletSprites, alTileAudios, BulletStateBehaviour)));

                shared_ptr <TileClass> olBullet = make_shared <TileClass> (alBulletStates, TileClass::MAINSTATEINDEX, BulletClassBehaviour);
                olBullet->setAlternativeClassId(BULLET_CLASS_ID);
                opLevel->registerTileClass(olBullet);

                opLevel->setAudios(alLevelAudios);
                opLevel->loadResources();
                opLevel->setGamepadCount(1);

                SCREEN olScr;
                olScr.originX = 0;
                olScr.originY = 0;
                olScr.width = GetSystemMetrics(SM_CXSCREEN);
                olScr.height = GetSystemMetrics(SM_CYSCREEN);
                opLevel->setScreen(olScr);

                blResult = true;
            }
            break;
        case EVENT::LEVEL_START:
            //opLevel->setAudioCommand(0, AudioCommand::PLAY_SOUND, true, 0, 5000);
            blResult = true;
            break;
        case EVENT::CLASS_REGISTRATION:
            {
                unsigned char ilMode = ResourceManager::getInstance()->getMode();

                if(ilMode == ResourceManager::GDI_ALPHA_MODE || ilMode == ResourceManager::GDI_MASK_MODE) {
                    if(!olGDIRenderer)
                        olGDIRenderer = make_shared <gdi::AlphaRenderer> ();

                    if(opEvent->value == MM_CLASS_ID)
                        opLevel->createTileInstance({500, 500, 0}, opEvent->srcclassid, olGDIRenderer);

                    if(opEvent->value == BULLET_CLASS_ID)
                        opLevel->createTileInstance({900, 500, 0}, opEvent->srcclassid, olGDIRenderer);
                } else {
                    if(!olOGLRenderer)
                        olOGLRenderer = make_shared <ogl::AlphaRenderer> ();

                    if(opEvent->value == MM_CLASS_ID)
                        opLevel->createTileInstance({500, 500, 0}, opEvent->srcclassid, olOGLRenderer);

                    if(opEvent->value == BULLET_CLASS_ID)
                        opLevel->createTileInstance({900, 500, 0}, opEvent->srcclassid, olOGLRenderer);
                }

                blResult = true;
            }
    }
    
    return blResult;
};

void LibraryInitialization() {
    glewInit();

    //Setting up swap intervals
    if(glewIsSupported("WGL_EXT_swap_control")) {
        PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = NULL;
        PFNWGLGETSWAPINTERVALEXTPROC wglGetSwapIntervalEXT = NULL;
        // Extension is supported, init pointers.
        wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC) wglGetProcAddress("wglSwapIntervalEXT");
        // this is another function from WGL_EXT_swap_control extension
        wglGetSwapIntervalEXT = (PFNWGLGETSWAPINTERVALEXTPROC) wglGetProcAddress("wglGetSwapIntervalEXT");
        if(wglGetSwapIntervalEXT() != 1)
            wglSwapIntervalEXT(1);
    }
}

LRESULT CALLBACK WndProc(
    _In_ HWND       hWnd,
    _In_ UINT       uMsg,
    _In_ WPARAM     wParam,
    _In_ LPARAM     lParam
) {
    unsigned char ilMode = uMsg == WM_NCCREATE || uMsg == WM_NCCALCSIZE || uMsg == WM_CREATE ? ResourceManager::OGL_ALPHA_MODE : ResourceManager::getInstance()->getMode();
    switch(uMsg) {
        case WM_CREATE:
            try {
                if(ilMode == ResourceManager::GDI_ALPHA_MODE || ilMode == ResourceManager::GDI_MASK_MODE)
                    ResourceManager::getInstance(ilMode, hWnd);
                else {
                    ResourceManager::getInstance(ilMode, hWnd, LibraryInitialization);
                    ogl::OGLRenderer::setPerspectiveMatrixFromWindow(hWnd);
                }

                RAWINPUTDEVICE rlInputDevice;
                rlInputDevice.usUsagePage = HID_USAGE_PAGE_GENERIC;
                rlInputDevice.usUsage = HID_USAGE_GENERIC_GAMEPAD;
                rlInputDevice.dwFlags = 0;
                rlInputDevice.hwndTarget = hWnd;
                if(!RegisterRawInputDevices(&rlInputDevice, 1, sizeof(RAWINPUTDEVICE))) {
                    MessageBox(
                        NULL,
                        (LPCWSTR) L"Device not recognized.",
                        (LPCWSTR) L"Exception",
                        MB_ICONWARNING | MB_OK
                    );
                }

                if(ilMode == ResourceManager::GDI_ALPHA_MODE || ilMode == ResourceManager::GDI_MASK_MODE)
                    olTestLevel = make_shared <Level>(make_shared <gdi::GDIScreenRenderer> (), TestLevelBehaviour);
                else
                    olTestLevel = make_shared <Level> (make_shared <ogl::OGLScreenRenderer> (), TestLevelBehaviour);
                olTestLevel->createLevel();
                olTestLevel->start();
            } catch(string excp) {
                std::wstring stemp = std::wstring(excp.begin(), excp.end());
                MessageBox(
                    NULL,
                    (LPCWSTR) stemp.c_str(),
                    (LPCWSTR) L"Excepción",
                    MB_ICONWARNING | MB_OK
                );
                //PostQuitMessage(0);
            }
            break;
        case WM_PAINT:
            PAINTSTRUCT ps;
            BeginPaint(hWnd, &ps); // En Modos OPENGL deben llamarse BeginPaing y EndPaint para poder retirar WM_PAINT de la Cola de Procesamiento de Mensajes.

            if((ilMode == ResourceManager::GDI_ALPHA_MODE || ilMode == ResourceManager::GDI_MASK_MODE) && olTestLevel->isLoaded() && olTestLevel->isInitialized()) { // Coordina WM_PAINT y WM_ERASEBKGND para prevenir el Sobredibujo.
                try {
                    /*
                    if(inter_start.time_since_epoch().count() != 0) {
                        inter_stop = chrono::high_resolution_clock::now();
                        auto prev_duration = chrono::duration_cast <chrono::milliseconds> (inter_stop - inter_start);
                        if(prev_duration < min_inter_duration)
                            min_inter_duration = prev_duration;
                        else if(prev_duration > max_inter_duration)
                            max_inter_duration = prev_duration;
                    }
                        
                    auto start = chrono::high_resolution_clock::now();
                    */

                    olTestLevel->playLevel(true);

                    /*
                    auto stop = chrono::high_resolution_clock::now();
                    auto duration = chrono::duration_cast<chrono::milliseconds>(stop - start);
                    if(duration < min_duration)
                        min_duration = duration;
                    else if(duration > max_duration)
                        max_duration = duration;

                    inter_start = chrono::high_resolution_clock::now();
                    */
                } catch(string excp) {
                    std::wstring stemp = std::wstring(excp.begin(), excp.end());
                    MessageBox(
                        NULL,
                        (LPCWSTR) stemp.c_str(),
                        (LPCWSTR) L"Excepción",
                        MB_ICONWARNING | MB_OK
                    );
                }

            }
            EndPaint(hWnd, &ps);
            break;
        case WM_SIZE:
            if(ilMode == ResourceManager::OGL_ALPHA_MODE || ilMode == ResourceManager::OGL_MASK_MODE)
                ogl::OGLRenderer::setPerspectiveMatrixFromWindow(hWnd);
            break;
        case WM_CLOSE:
            olTestLevel->end();
            olTestLevel->destroyLevel();
            ResourceManager::getInstance()->destroy();
            /*
            {
                wstringstream slMessage;
                slMessage << L"Min: " << min_duration.count() << L"\nMax: " << max_duration.count() << L"\nInter Min: " << min_inter_duration.count() << L"\nInter Max: " << max_inter_duration.count() << endl;
                MessageBox(
                    NULL,
                    (LPCWSTR) slMessage.str().c_str(),
                    (LPCWSTR) L"Excepción",
                    MB_ICONWARNING | MB_OK | MB_SYSTEMMODAL
                );
            }
            */
            DestroyWindow(hWnd);
            break;
        case WM_DESTROY:
            ResourceManager::getInstance()->clear();
            PostQuitMessage(0);
            break;
        case WM_KEYDOWN:
            olTestLevel->keyDown(wParam);
            break;
        case WM_KEYUP:
            olTestLevel->keyUp(wParam);
            break;
        case WM_INPUT:
            {
                const unsigned int DEVICE_ID = 0;
                XINPUT_STATE rlState;
                ZeroMemory(&rlState, sizeof(XINPUT_STATE));
                
                if(XInputGetState(DEVICE_ID, &rlState)) {
                    MessageBox(
                        NULL,
                        (LPCWSTR) L"Gamecontroler was not found.",
                        (LPCWSTR) L"Exception",
                        MB_ICONWARNING | MB_OK
                    );
                }

                if(rlState.dwPacketNumber != ilLastPackeNumber) {
                    shared_ptr <Level::GamepadState> rlGamepadState = make_shared <Level::GamepadState> (rlState);
                    olTestLevel->setGamepadState(DEVICE_ID, rlGamepadState);
                    ilLastPackeNumber = rlState.dwPacketNumber;
                }
            }
            break;
        default:
            return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }

    return 0;
}

void drawScene() {
    unsigned char ilMode = ResourceManager::getInstance()->getMode();
    if(ilMode != ResourceManager::OGL_ALPHA_MODE && ilMode != ResourceManager::OGL_MASK_MODE)
        return;

    try {
        /*
        if(inter_start.time_since_epoch().count() != 0) {
            inter_stop = chrono::high_resolution_clock::now();
            auto prev_duration = chrono::duration_cast <chrono::milliseconds> (inter_stop - inter_start);
            if(prev_duration < min_inter_duration)
                min_inter_duration = prev_duration;
            else if(prev_duration > max_inter_duration)
                max_inter_duration = prev_duration;
        }

        auto start = chrono::high_resolution_clock::now();
        */

        olTestLevel->playLevel(true);

        /*
        auto stop = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<chrono::milliseconds>(stop - start);
        if(duration < min_duration)
            min_duration = duration;
        else if(duration > max_duration)
            max_duration = duration;

        inter_start = chrono::high_resolution_clock::now();
        */
    } catch(string excp) {
        std::wstring stemp = std::wstring(excp.begin(), excp.end());
        MessageBox(
            NULL,
            (LPCWSTR) stemp.c_str(),
            (LPCWSTR) L"Excepción",
            MB_ICONWARNING | MB_OK
        );
    }
}

int CALLBACK WinMain(
    _In_ HINSTANCE  hInstance,
    _In_ HINSTANCE  hPrevInstance,
    _In_ LPSTR      lpCmdLine,
    _In_ int        nCmdShow
) {
    static TCHAR szWindowClass[] = _T("DesktopApp");
    static TCHAR szTitle[] = _T("GameEngine");
    WNDCLASSEX wcex;
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;

    // Initialize GDI+.
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
    
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH) (COLOR_WINDOW + 2);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(hInstance, IDI_APPLICATION);

    if(!RegisterClassEx(&wcex)) {
        MessageBox(
            NULL,
            _T("Call to RegisterClassEx failed!"),
            _T("GameEngine"),
            NULL
        );

        return 1;
    }

    HWND hWnd = CreateWindow(
        szWindowClass,
        szTitle,
        WS_POPUP,
        0, 0,
        GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN),
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if(!hWnd) {
        MessageBox(
            NULL,
            _T("Call to CreateWindow failed!"),
            _T("GameEngine"),
            NULL
        );

        return 1;
    }
    
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
 
    /*
    FILE *stream_in;
    FILE *stream_out;
    FILE *stream_err;

    AllocConsole();
    freopen_s(&stream_in, "conin$", "r", stdin);
    freopen_s(&stream_out, "conout$", "w", stdout);
    freopen_s(&stream_err, "conout$", "w", stderr);
    printf("Debugging Window:\n");
    */
    MSG msg;
    while(true) {
        while(PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) {
            if(GetMessage(&msg, NULL, 0, 0)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            } else
                return TRUE;
        }
        drawScene();
    }

    Gdiplus::GdiplusShutdown(gdiplusToken);
    return (int) msg.wParam;
}