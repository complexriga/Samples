#include <windows.h>
#include <vector>
#include <list>
#include <map>
#include <string>
#include <memory>
#include <functional>
#include <thread>
#include <mutex>
#include <chrono>
#include <stdio.h>
#include <gdiplus.h>
#include <Xinput.h>
#include "Tools.h"

#pragma comment (lib,"Gdiplus.lib")

///////////////////////////////////////////////////////////
// DO NOT MOVE CONST CLAUSES, they are neccesary to compile the assigning constructors correctly.
///////////////////////////////////////////////////////////

#ifndef GAME_TOOLS
///////////////////////////////////////////////////////////

#define GAME_TOOLS

// These macros do not support the packing of data types greater than 1/4 the size of size_t.
#define PACK_DISPLACEMENT ((sizeof(size_t) / 4) * 8)
#define COLLISION_DATA_MASK ((size_t) -1 >> ((sizeof(size_t) * 8) - PACK_DISPLACEMENT))
#define PACK_COLLISION_DATA(sprite, frame, tier, border) ((size_t) (((COLLISION_DATA_MASK & (size_t) sprite) << (PACK_DISPLACEMENT * 3)) | ((COLLISION_DATA_MASK & (size_t) frame) << (PACK_DISPLACEMENT * 2)) | ((COLLISION_DATA_MASK & (size_t) tier) << PACK_DISPLACEMENT) | ((COLLISION_DATA_MASK & (size_t) border))))
#define UNPACK_COLLISION_SPRITE(collision) ((size_t) collision >> (PACK_DISPLACEMENT * 3))
#define UNPACK_COLLISION_FRAME(collision) (((size_t) collision >> (PACK_DISPLACEMENT * 2)) & COLLISION_DATA_MASK)
#define UNPACK_COLLISION_TIER(collision) (((size_t) collision >> (PACK_DISPLACEMENT * 1)) & COLLISION_DATA_MASK)
#define UNPACK_COLLISION_BORDER(collision) ((size_t) collision & COLLISION_DATA_MASK)

namespace gametools {

    const unsigned char OUTLINE_VALID_ALPHA = 255;
    const unsigned char OUTLINE_SEGMENT_LENGTH = 10;
    const unsigned int OUTLINE_BORDER_SIZE = 1000;
    const unsigned int OUTLINE_INITIAL_BACKTRACE = 2;
    const unsigned char DECIMAL_CONVERSION_PRECISION = 10;
    const tools::image::RGBA BLACK = {0, 0, 0, 255};
    const tools::image::RGBA WHITE = {255, 255, 255, 255};

    //////////////////////////////////////////////////////////////////////////////////////
    // COORDINATE FORMATS

    struct SI3DPOINT {
        long x = 0;
        long y = 0;
        long z = 0;
    };

    struct UI3DPOINT {
        unsigned long x = 0;
        unsigned long y = 0;
        unsigned long z = 0;
    };

    struct SF2DPOINT {
        float x = 0;
        float y = 0;
    };

    //////////////////////////////////////////////////////////////////////////////////////
    // BORDER STRUCTURES

    typedef std::vector <SI3DPOINT> BORDER;
    typedef std::vector <std::shared_ptr <BORDER>> BORDER_REF;
    typedef std::vector <std::shared_ptr <BORDER_REF>> BORDER_TIER;
    typedef std::vector <std::shared_ptr <BORDER_TIER>> BORDER_DIVISION;

    typedef const BORDER CONST_BORDER;
    typedef const BORDER_REF CONST_BORDER_REF;
    typedef const BORDER_TIER CONST_BORDER_TIER;
    typedef const BORDER_DIVISION CONST_BORDER_DIVISION;

    const std::shared_ptr <BORDER_DIVISION> EMPTY_BORDER = std::make_shared <BORDER_DIVISION> ();
    const tools::image::RGBA DEFAULT_BORDER_COLOR = {0, 255, 255, 255};

    //////////////////////////////////////////////////////////////////////////////////////
    // RENDERERS

    namespace renderer {

        const unsigned int NULL_RENDERER = 0;
        const unsigned int GDI_ALPHA_RENDERER = 1;
        const unsigned int GDI_MASK_RENDERER = 2;
        const unsigned int OGL_ALPHA_RENDERER = 3;
        const unsigned int OGL_MASK_RENDERER = 4;

        class Renderer {
            protected:
                unsigned int igType;

            public:
                Renderer();

                unsigned int getType();
                virtual void drawSprite(const SI3DPOINT& rpPosition, float fpAngle, const SF2DPOINT rpScale, size_t hpBitmap, unsigned long ipXDelta, unsigned long ipYDelta, unsigned int ipSpriteWidth, unsigned int ipSpriteHeight) const = 0;
                virtual void drawBorder(const SI3DPOINT& rpPosition, float fpAngle, const SF2DPOINT rpScale, const std::shared_ptr <CONST_BORDER_TIER> apBorder, unsigned char ipPointSize = 2, tools::image::RGBA rpColor = DEFAULT_BORDER_COLOR) const;

                Renderer(const Renderer& opRenderer) = delete;
                Renderer& operator= (const Renderer& opRenderer) = delete;
        };

        class ScreenRenderer {
            protected:
                unsigned int igScreenWidth;
                unsigned int igScreenHeight;

            public:
                ScreenRenderer() = default;

                virtual void prepareScreen(unsigned int ipScreenWidth, unsigned int ipScreenHeight);
                virtual void drawScreen() const;

                ScreenRenderer(const ScreenRenderer& opRenderer) = delete;
                ScreenRenderer& operator= (const ScreenRenderer& opRenderer) = delete;
        };

        const std::shared_ptr <ScreenRenderer> NULL_SCREEN_RENDERER = std::make_shared <ScreenRenderer> ();

        namespace gdi {

            class GDIScreenRenderer : public ScreenRenderer {
                public:
                    GDIScreenRenderer();

                    void prepareScreen(unsigned int ipScreenWidth, unsigned int ipScreenHeight);
                    void drawScreen() const;
            };

            class AlphaRenderer : public Renderer {
                public:
                    AlphaRenderer();

                    void drawSprite(const SI3DPOINT& rpPosition, float fpAngle, const SF2DPOINT rpScale, size_t hpBitmap, unsigned long ipXDelta, unsigned long ipYDelta, unsigned int ipSpriteWidth, unsigned int ipSpriteHeight) const;
            };

            class MaskRenderer : public Renderer {
                public:
                    MaskRenderer();

                    void drawSprite(const SI3DPOINT& rpPosition, float fpAngle, const SF2DPOINT rpScale, size_t hpBitmap, unsigned long ipXDelta, unsigned long ipYDelta, unsigned int ipSpriteWidth, unsigned int ipSpriteHeight) const;
            };

        }

        namespace ogl {

            class OGLRenderer : public Renderer {
                protected:
                    unsigned int hgProgram;
                    std::string sgVertexShader;
                    std::string sgGeometryShader;
                    std::string sgFragmentShader;
                    static std::shared_ptr<std::vector <float>> agPerspectiveMatrix;
                    static float fgPixelSize;
                    static float fgAspect;

                public:
                    static const size_t DEFAULT_PROGRAM1;
                    static const size_t DEFAULT_PROGRAM2;
                    static const float NORMALIZED_COLOR_UNIT;

                    OGLRenderer();
                    ~OGLRenderer();

                    void drawBorder(const SI3DPOINT& rpPosition, float fpAngle, const SF2DPOINT rpScale, const std::shared_ptr <CONST_BORDER_TIER> apBorder, unsigned char ipPointSize = 2, tools::image::RGBA rpColor = DEFAULT_BORDER_COLOR) const;

                    std::string getVertexShaderSourceCode() const;
                    std::string getGeometryShaderSourceCode() const;
                    std::string getFragmentShaderSourceCode() const;

                    static void setPerspectiveMatrix(float fpFieldOfView, unsigned int ipWidth, unsigned int ipHeight, float fpZNear, float fpZFar);
                    static void setPerspectiveMatrixFromWindow(HWND hpWindow);
                    static std::shared_ptr <std::vector <float>> getPerspetiveMatrix();
                    static float getPixelSize();
            };

            class OGLScreenRenderer : public ScreenRenderer {
                public:
                    OGLScreenRenderer();

                    void prepareScreen(unsigned int ipScreenWidth, unsigned int ipScreenHeight);
                    void drawScreen() const;
            };

            class AlphaRenderer : public OGLRenderer {
                protected:
                    unsigned int hgVertexArray;
                    unsigned int hgVertexBuffer;
                    unsigned int hgElementBuffer;
                    std::vector <unsigned int> agElementIndexes;

                public:
                    AlphaRenderer();
                    ~AlphaRenderer();

                    void drawSprite(const SI3DPOINT& rpPosition, float fpAngle, const SF2DPOINT rpScale, size_t hpBitmap, unsigned long ipXDelta, unsigned long ipYDelta, unsigned int ipSpriteWidth, unsigned int ipSpriteHeight) const;
            };

            class MaskByColorRenderer : public OGLRenderer {
                protected:
                    unsigned int hgVertexArray;
                    unsigned int hgVertexBuffer;
                    unsigned int hgElementBuffer;
                    tools::image::RGBA rgMaskColor;
                    std::vector <unsigned int> agElementIndexes;

                public:
                    MaskByColorRenderer(const tools::image::RGBA& rpColor);
                    ~MaskByColorRenderer();

                    void drawSprite(const SI3DPOINT& rpPosition, float fpAngle, const SF2DPOINT rpScale, size_t hpBitmap, unsigned long ipXDelta, unsigned long ipYDelta, unsigned int ipSpriteWidth, unsigned int ipSpriteHeight) const;

                    tools::image::RGBA getMaskColor() const;
            };

            class MaskRenderer : public OGLRenderer {
                protected:
                    unsigned int hgVertexArray;
                    unsigned int hgVertexBuffer;
                    unsigned int hgElementBuffer;
                    std::vector <unsigned int> agElementIndexes;

                public:
                    MaskRenderer();
                    ~MaskRenderer();

                    void drawSprite(const SI3DPOINT& rpPosition, float fpAngle, const SF2DPOINT rpScale, size_t hpBitmap, unsigned long ipXDelta, unsigned long ipYDelta, unsigned int ipSpriteWidth, unsigned int ipSpriteHeight) const;
            };  

        }

    }

    //////////////////////////////////////////////////////////////////////////////////////
    // SPRITES

    namespace sprite {

        const SI3DPOINT DEFAULT_POS_DELTA = {0, 0, 0};
        const float DEFAULT_ANGLE_DELTA = 0.0f;
        const SF2DPOINT DEFAULT_SCALE_DELTA = {0.0f, 0.0f};

        class Sprite {
            public:
                // The levels of active_borders and passive_borders are:
                //  1- Frame: The entries for each squere have to exist even there are no borders to test.
                //  2- Level: All borders at the same level are evaluated, if no border at the actual level has a collision then the borders at the following levels are not evaluated.
                //  3- Border: Contains the pointers to the effective  borders
                struct FrameDescriptor {
                    unsigned long division_inx; // Index of the division that corresponds to the frame.
                    UI3DPOINT division_pos; // Position of the upper left corner of the division at it's source.
                    unsigned long tick_count; // Number of ticks the frame remains active.
                    std::chrono::milliseconds tick_length; // Duration of a tick.
                    SI3DPOINT delta_pos; // Shift delta of the frame for each tick.
                    SF2DPOINT delta_scale; // Scale up delta of the frame for each tick.
                    float delta_angle; // Rotation degree of the sprite for each tick, this is a fixed decimal value that is the rotation angle represented by the division.
                    unsigned char marker; // Code of the form in which the frame must be interpreted or linked.
                    size_t param; // Value whose interpretation depends on the hint field.
                    std::shared_ptr <BORDER_TIER> active_borders; // Active borders of the frame.
                    std::shared_ptr <BORDER_TIER> passive_borders; // Passive borders of the frame.
                };

                static const unsigned char UPWARD = 0;
                static const unsigned char DOWNWARD = 1;
                static const unsigned char RIGHTWARD = 2;
                static const unsigned char LEFTWARD = 3;
                static const unsigned char REVERSE = 4;

                // Marks that control the sprite animation:
                static const unsigned char MARK_NULL = 0; // The current frame should be treated according to the current state.
                static const unsigned char MARK_FORWARD = 1; // After finishing with the current frame the sprite should pick up the following frames in ascending order.
                static const unsigned char MARK_BACKWARD = 2; // After finishing with the current frame the sprite should pick up the following frames in descending order.
                static const unsigned char MARK_STOP = 4; // After finishing with the current frame the sprite must stop in the last state.
                static const unsigned char MARK_FOR = 8; // After finishing with the current frame the animation flow should jump at the frame that has the last MARK_NEXT for a number of times defined by param.
                static const unsigned char MARK_NEXT = 16; // Points to the frame to which the animation flow should jump after the next frame with MARK_FOR.
                static const unsigned char MARK_JUMP = 32; // Unconditional jump towards the frame pointed by param.
                static const unsigned char MARK_REPEAT = MARK_NEXT | MARK_FOR; // The current grame must be repeated the number of times defined by param.


                static const size_t NOLOOP = -1;
                static const size_t ALWAYSLOOP = -1;

                static std::shared_ptr <std::vector <FrameDescriptor>> EMPTY_FRAMES;

            protected:
                unsigned char igDivisionDirection;
                unsigned int igDivisionCount;
                unsigned int igDivisionWidth;
                unsigned int igDivisionHeight;
                UI3DPOINT rgDivisionOrigin;

                size_t hgBitmap;
                tools::image::RGBA rgBackgroundColor;
                std::shared_ptr <std::vector <FrameDescriptor>> agFrames;

            public:
                Sprite(std::shared_ptr <std::vector <FrameDescriptor>> apFrames = EMPTY_FRAMES);
                Sprite(
                    size_t hpImage,
                    UI3DPOINT& rpDivisionOrigin, // Position on the bitmap pointed by hpImage which serves as base to calculate the positions of the frames.
                    unsigned long ipDivisionWidth, // Wide of the frame.
                    unsigned long ipDivisionHeight, // Hight of the frame.
                    unsigned char ipDivisionDirection, // Direction relative to rpDivisionOrigin in which the frames must be searched.
                    unsigned long ipDivisionCount, // Number of frames in the animation.
                    std::shared_ptr <std::vector <FrameDescriptor>> apFrames, // Frame descriptors.
                    const tools::image::RGBA& ipBackgroundColor = BLACK // Color to build the mask.
                );

                size_t getBitmapHandle() const;
                const UI3DPOINT& getDivisionOrigin() const;
                unsigned int getDivisionHeight() const;
                unsigned int getDivisionWidth() const;
                unsigned char getDivisionDirection() const;
                unsigned long getDivisionCount() const;
                const tools::image::RGBA& getBackgroundColor() const;
                size_t getFrameCount() const;
                std::shared_ptr <const std::vector <FrameDescriptor>> getFrames() const;

                Sprite(const Sprite& opSprite) = delete;
                Sprite& operator= (const Sprite& opSprite) = delete;
        };

        class SpriteCreator {
            public:
                // Modes of findBorder for searching the border of an sprite, passed in ipSearchMode parameter:
                static const unsigned char BY_ALPHA = 0; // Builds the borders of a sprite using the alpha channel.
                static const unsigned char BY_COLOR = 1; // Builds the borders of the sprite using a color.

                // Types of borders returned by findBorder, passed in ipBorderType parameter:
                static const unsigned char BORDER_NULL = 0; // Returns an empty border.
                static const unsigned char BORDER_BOX = 1; // Returns a border for each minimal box that contains the frame of the sprite.
                static const unsigned char BORDER_QUADRANT = 2; // Returns the borders that correspond to a BOXBORDER, but divided into 4 quadrants.
                static const unsigned char BORDER_TIGHT_CONTOUR = 4; // Returns a border for each framee of the sprite, such that each border encloses exactly the content of the corresponding frame.
                static const unsigned char BORDER_LOOSE_CONTOUR = 8; // Returns a border for each framee of the sprite, such that each border encloses approximately the content of the corresponding frame.

                static std::shared_ptr <tools::image::RGBAImage> getFrameBitmap(std::shared_ptr <Sprite> opSprite, unsigned long ipFrameIndex, bool blMask = false);
                static std::shared_ptr <CONST_BORDER_TIER> getDeltaBorder(std::shared_ptr <Sprite> opSprite, size_t ipFrameIndex, bool bpActive, SI3DPOINT rpPosition, float fpAngle, SF2DPOINT rpScale, SI3DPOINT rpPosDelta, float fpAngleDelta, SF2DPOINT rpScaleDelta);

                static std::shared_ptr <Sprite> create(
                    size_t hpImage,
                    UI3DPOINT& rpSourcePosition,
                    unsigned long ipDivisionWidth,
                    unsigned long ipDivisionHeight,
                    unsigned char ipDirection,
                    unsigned long ipDivisionCount,
                    bool bpReverse,
                    std::shared_ptr <std::vector <Sprite::FrameDescriptor>> apFrames,
                    unsigned char ipSearchMode,
                    unsigned char ipActiveBorderType = BORDER_QUADRANT,
                    unsigned char ipPassiveBorderType = BORDER_QUADRANT,
                    const tools::image::RGBA& ipBackgroundColor = BLACK,
                    unsigned char ipValidAlpha = OUTLINE_VALID_ALPHA,
                    unsigned char ipSegmentLength = OUTLINE_SEGMENT_LENGTH,
                    unsigned int ipBorderSize = OUTLINE_BORDER_SIZE,
                    unsigned int ipBackTrace = OUTLINE_INITIAL_BACKTRACE
                );

                static std::shared_ptr <Sprite> create(
                    size_t hpImage,
                    bool bpReverse,
                    std::shared_ptr <std::vector <Sprite::FrameDescriptor>> apFrames,
                    unsigned char ipSearchMode,
                    unsigned char ipActiveBorderType = BORDER_QUADRANT,
                    unsigned char ipPassiveBorderType = BORDER_QUADRANT,
                    const tools::image::RGBA& ipBackgroundColor = BLACK,
                    unsigned char ipValidAlpha = OUTLINE_VALID_ALPHA,
                    unsigned char ipSegmentLength = OUTLINE_SEGMENT_LENGTH,
                    unsigned int ipBorderSize = OUTLINE_BORDER_SIZE,
                    unsigned int ipBackTrace = OUTLINE_INITIAL_BACKTRACE
                );

                // Build the borders that enclose the content of each frame in a sprite, these borders are automatically defined by the alpha channel of each frame (ipSearchMode == BY_ALPHA) or by color (ipSearchMode == BY_COLOR), 
                // and the borders for each frame are organized by proximity level to the content of the frame, that's given by the aggregation of the flags on the parameter ipBorderType:
                // Level 1) BOXBORDER: This is the first and outmost border.
                // Level 2) QUADRANTBORDER.
                // Level 3) CONTOURBORDER.
                static std::shared_ptr <BORDER_DIVISION> findBorders(
                    size_t hpImage,
                    UI3DPOINT& rpSourcePosition,
                    unsigned long ipDivisionWidth,
                    unsigned long ipDivisionHeight,
                    unsigned char ipDirection,
                    unsigned long ipDivisionCount,
                    unsigned char ipSearchMode,
                    unsigned char ipBorderType,
                    const tools::image::RGBA& ipBackgroundColor,
                    unsigned char ipValidAlpha,
                    unsigned char ipSegmentLength,
                    unsigned int ipBorderSize,
                    unsigned int ipBackTrace
                );
        };

    }

    //////////////////////////////////////////////////////////////////////////////////////
    // CONTAINERS

    namespace container {
        
        class TileInstance;
        class Level;

        struct EVENT {
            // System Events:
            static const unsigned char CLASS_REGISTRATION = 0;
            static const unsigned char CLASS_UNREGISTRATION = 1;
            static const unsigned char INSTANCE_CREATION = 2;
            static const unsigned char INSTANCE_DESTRUCTION = 3;
            static const unsigned char DRAW = 4;
            static const unsigned char ACTIVE_COLLISION = 5;
            static const unsigned char PASSIVE_COLLISION = 6;
            static const unsigned char COLLISION_REQUEST = 7;
            static const unsigned char CAPABILITY_CHECK = 8;
            static const unsigned char PROPAGATE_EVENT = 9;
            static const unsigned char LEVEL_CREATION = 10;
            static const unsigned char LEVEL_DESTRUCTION = 11;
            static const unsigned char LEVEL_START = 12;
            static const unsigned char LEVEL_END = 13;
            // General Events:
            static const unsigned char USER_INPUT_KEY_DOWN = 14;
            static const unsigned char USER_INPUT_KEY_UP = 15;
            static const unsigned char USER_INPUT_GAMEPAD_DOWN = 16;
            static const unsigned char USER_INPUT_GAMEPAD_UP = 17;
            static const unsigned char USER_INPUT_GAMEPAD_CHANGE = 18;

            static const WORD GAMEPAD_DPAD_UP = 0x0001;
            static const WORD GAMEPAD_DPAD_DOWN = 0x0002;
            static const WORD GAMEPAD_DPAD_LEFT = 0x0004;
            static const WORD GAMEPAD_DPAD_RIGHT = 0x0008;
            static const WORD GAMEPAD_START = 0x0010;
            static const WORD GAMEPAD_BACK = 0x0020;
            static const WORD GAMEPAD_LEFT_THUMB = 0x0040;
            static const WORD GAMEPAD_RIGHT_THUMB = 0x0080;
            static const WORD GAMEPAD_LEFT_SHOULDER = 0x0100;
            static const WORD GAMEPAD_RIGHT_SHOULDER = 0x0200;
            static const WORD GAMEPAD_LEFT_TRIGGER = 0x0400;
            static const WORD GAMEPAD_RIGHT_TRIGGER = 0x0800;
            static const WORD GAMEPAD_A = 0x1000;
            static const WORD GAMEPAD_B = 0x2000;
            static const WORD GAMEPAD_X = 0x4000;
            static const WORD GAMEPAD_Y = 0x8000;

            // Queries that can be done using CAPABILITY_CHECK:
            static const unsigned int IS_ACTIVE_COLLISION_CAPABLE = 0; // Is able to do active collision?
            static const unsigned int IS_PASSIVE_COLLISION_CAPABLE = 1; // Is able to do passive collision?
            static const unsigned int IS_DISPLAYABLE = 2; // Is It displayables?

            unsigned char code;
            size_t value;
            size_t srcclassid;
            size_t srcinstanceid;
            size_t dstclassid;
            size_t dstinstanceid;
            std::shared_ptr <BORDER> collision;
        };

        struct SCREEN {
            long originX;
            long originY;
            unsigned int width;
            unsigned int height;
        };

        struct AudioCommand {
            // Audio control commands:
            static const unsigned char NO_COMMAND_SOUND = 0;
            static const unsigned char PLAY_SOUND = 1;
            static const unsigned char PAUSE_SOUND = 2;
            static const unsigned char RESUME_SOUND = 3;
            static const unsigned char STOP_SOUND = 4;
            static const unsigned char WAIT_SOUND = 5;

            unsigned char command; // Command.
            bool loop; // Should loop the audio?
            unsigned int limit; // How many times should the audio be looped?
            unsigned long delay; // Ellapsed time between loops.
        };

        const AudioCommand NULL_AUDIO_COMMAND = {AudioCommand::NO_COMMAND_SOUND, false, 0};
        bool NULL_TILE_BEHAVIOUR(std::shared_ptr <TileInstance> opInstance, std::shared_ptr <const EVENT> opEvent);
        bool NULL_LEVEL_BEHAVIOUR(std::shared_ptr <Level> opLevel, std::shared_ptr <const EVENT> opEvent);

        class TileState {
            protected:
                bool bgIsLoaded;
                size_t igAltId;

                std::function <bool (std::shared_ptr <TileInstance> opInstance, std::shared_ptr <const EVENT> opEvent)> pgFunction;
                std::vector <std::shared_ptr <sprite::Sprite>> agSprites;
                std::shared_ptr <std::vector <size_t>> agAudios;

            public:
                static const size_t UNINITIALIZED = -1;
                static const size_t MAINSPRITEINDEX = 0;
                static const size_t MAINFRAMEINDEX = 0;

                TileState();
                TileState(
                    std::shared_ptr <std::vector <std::shared_ptr <sprite::Sprite>>> apSprites,
                    std::shared_ptr <std::vector <size_t>> apAudios,
                    std::function <bool (std::shared_ptr <TileInstance> opInstance, std::shared_ptr <const EVENT> opEvent)> ppFunction = NULL_TILE_BEHAVIOUR
                );

                // API for state identification.
                void setAlternativeStateId(size_t ipId);
                size_t getAlternativeStateId() const;

                // API for querying the characteristics of the state.
                unsigned int getWidth() const;
                unsigned int getHeight() const;
                size_t getSpriteCount() const;
                size_t getAudioCount() const;

                bool catchEvent(std::shared_ptr <TileInstance> opInstance, std::shared_ptr <const EVENT> opEvent) const;
                std::shared_ptr <CONST_BORDER_REF> getFrameBorder(size_t ipSpriteInx, size_t ipFrameInx, size_t ipTier, bool bpActive, SI3DPOINT rpPosition, float fpAngle, SF2DPOINT rpScale, SI3DPOINT rpPosDelta = sprite::DEFAULT_POS_DELTA, float fpAngleDelta = sprite::DEFAULT_ANGLE_DELTA, SF2DPOINT rpScaleDelta = sprite::DEFAULT_SCALE_DELTA) const;
                const std::shared_ptr <const sprite::Sprite> getSprite(size_t ipSpriteInx) const;
                const std::shared_ptr <const tools::audio::Audio> getAudio(size_t ipAudioInx) const;

                // API for resource management.
                bool isLoaded() const;
                void loadResources(bool bpAsynchronousLoad = false);
                void unloadResources(bool bpUnloadAudio = false);
                bool isLoadingAudio();
                void waitResourcesLoad();

                TileState(const TileState& opState) = delete;
                TileState& operator= (const TileState& opState) = delete;
        };

        class TileClass {
            protected:
                bool bgIsLoaded;
                size_t igId;
                size_t igAltId;
                size_t igStartStateIndex;
                std::vector <std::shared_ptr <TileState>> agStates;
                std::function <bool (std::shared_ptr <TileInstance> opInstance, std::shared_ptr <const EVENT> opEvent)> pgFunction;

                static size_t getLastClassId();

            public:
                static const size_t UNINITIALIZED = -1;
                static const size_t MAINSTATEINDEX = 0;

                TileClass();
                TileClass(
                    std::shared_ptr <std::vector <std::shared_ptr <TileState>>> apStates,
                    size_t ipStartState = MAINSTATEINDEX,
                    std::function <bool (std::shared_ptr <TileInstance> opInstance, std::shared_ptr <const EVENT> opEvent)> ppFunction = NULL_TILE_BEHAVIOUR
                );

                size_t getStartStateIndex() const;
                size_t getStatesCount() const;
                const std::shared_ptr <const TileState> getState(size_t ipStateInx) const;


                // API for class identification.
                size_t getClassId() const;
                void setAlternativeClassId(size_t ipId);
                size_t getAlternativeClassId() const;

                // API for resource management.
                bool isLoaded() const;
                void loadResources(bool bpAsynchronousLoad = false);
                void unloadResources(bool bpUnloadAudio = false);
                bool isLoadingAudio();
                void waitResourcesLoad();

                bool catchEvent(std::shared_ptr <TileInstance> opInstance, std::shared_ptr <const EVENT> opEvent) const;

                TileClass(const TileClass& opClass) = delete;
                TileClass& operator= (const TileClass& opClass) = delete;
        };

        class TileInstance : public std::enable_shared_from_this <TileInstance> {
            protected:
                struct SpriteControlDescriptor {
                    bool active;
                    bool finished;
                    bool stop;
                    bool reverse;
                    bool show;
                    size_t frame_inx;
                    size_t loop_counter;
                    size_t loop_index;
                    unsigned long tick_accumulator;
                    std::chrono::milliseconds next_tick_time;
                    SI3DPOINT pos;
                    float angle;
                    SF2DPOINT scale;
                    std::shared_ptr <const std::vector <gametools::sprite::Sprite::FrameDescriptor>> buffer;
                };
                static const SpriteControlDescriptor NULL_SPRITE_CONTROL;

                bool bgIsInitialized;
                bool bgIsAudioLoaded;
                bool bgResetDelta;
                unsigned char cgCollisionType;
                unsigned char igXPosType;
                unsigned char igYPosType;
                unsigned char igAngleType;
                unsigned char igXScaleType;
                unsigned char igYScaleType;
                unsigned char igStateChangeCounter;
                size_t igId;
                size_t igAltId;
                size_t igActiveStateIndex;
                SI3DPOINT rgPositionModifier;
                SI3DPOINT rgPosition;
                SI3DPOINT rgPositionDelta;
                float fgAngleModifier;
                float fgAngle;
                float fgAngleDelta;
                SF2DPOINT rgScaleModifier;
                SF2DPOINT rgScale;
                SF2DPOINT rgScaleDelta;
                std::shared_ptr <gametools::renderer::Renderer> ogRenderer;
                std::shared_ptr <TileClass> pgClass;
                std::shared_ptr <Level> pgLevel;
                std::vector <std::vector <SpriteControlDescriptor>> pgSpriteControl;
                std::vector <std::vector <std::shared_ptr <tools::audio::AsynchronousPlayer>>> agPlayers;
                std::vector <std::vector <AudioCommand>> agAudioCommands;
                std::vector <int> agIntegerVariables;
                std::vector <std::string> agStringVariables;
                std::vector <float> agFloatVariables;
                std::vector <float> agTransformation;

                void initialize();
                bool isTimeUp(size_t ipSpriteInx, std::chrono::milliseconds tpReferenceTime);
                gametools::sprite::Sprite::FrameDescriptor getNextFrame(size_t ipSpriteInx);

            public:
                static const size_t UNINITIALIZED = -1;

                // Types of collision detections:
                static const unsigned char COLLISION_INTERCEPTION = 0; // Checks if the borders of the instance are intersected by the border of other instance.
                static const unsigned char COLLISION_INSIDE = 1; // Checks if some border of the instance is contained completely within the border of other instance.
                static const unsigned char COLLISION_VECTOR = 2; // Checks if the border of other instance has been intersected from the previous position to the actual position.

                // Typs of instance coordinates:
                static const unsigned char POS_ABSOLUTE = 0; // Absolute position.
                static const unsigned char POS_DELTA = 1; // Relative position with respect to the actual position.
                static const unsigned char POS_PERCENTAGE = 2; // Percentage of the new position according to the sprite, real format of fixed point with 2 decimal positions (RE: E+DD; E: Integer; D: Decimal).

                static const long PERCENT_0 = 0;
                static const long PERCENT_100 = 10000;

                // Type of alignment of the new state with respect to the active state:
                static const unsigned char ALIGN_TOP_EDGE = 0;
                static const unsigned char ALIGN_BOTTOM_EDGE = 1;
                static const unsigned char ALIGN_VERTICAL_CENTERED = 2;
                static const unsigned char ALIGN_LEFT_EDGE = 0;
                static const unsigned char ALIGN_RIGHT_EDGE = 4;
                static const unsigned char ALIGN_HORIZONTAL_CENTERED = 8;

                TileInstance();
                TileInstance(SI3DPOINT rpPosition, std::shared_ptr <TileClass> ppClass, std::shared_ptr <gametools::renderer::Renderer> opRenderer, float fpAngle = 0, SF2DPOINT rpScale = {1, 1});

                // General API
                const std::shared_ptr <const TileClass> getClass() const;
                const SI3DPOINT& getPosition() const;
                const SI3DPOINT& getPositionDelta() const; // Returns the previous position of the instance in order to do the analysis of the collision on vector mode.
                const float getAngle() const;
                const float getAngleDelta() const;
                const SF2DPOINT& getScale() const;
                const SF2DPOINT& getScaleDelta() const;
                void resetDelta();
                long getZ() const;
                void setX(long ipX, unsigned char ipPositionType = POS_ABSOLUTE);
                void setY(long ipY, unsigned char ipPositionType = POS_ABSOLUTE);
                void setZ(long ipZ); // The type of the z coordinate is always absolute.
                void setAngle(float fpAngle, unsigned char ipAngleType);
                void setXScale(float fpXScale, unsigned char ipXScaleType);
                void setYScale(float fpYScale, unsigned char ipYScaleType);
                bool isInitialized() const;

                void setAlternateInstanceId(size_t ipId);
                size_t getAlternateInstanceId() const;

                // Level API
                void setInstanceId(size_t ipInstanceId);
                size_t getInstanceId() const;
                void setOwnerLevel(std::shared_ptr <Level> opLevel);
                const std::shared_ptr <Level> getOwnerLevel() const;
                void setCollisionType(unsigned char cpType);
                unsigned char getCollisionType() const;

                void setRenderer(std::shared_ptr <gametools::renderer::Renderer> opRenderer);
                const std::shared_ptr <gametools::renderer::Renderer> getRenderer() const;
                void playInstance(bool bpDrawBorder = false, bool bpActive = false, unsigned char ipPixelSize = 2, tools::image::RGBA rlBorderColor = DEFAULT_BORDER_COLOR);

                // TileClass and TileState API
                size_t createIntegerVariable();
                size_t createStringVariable();
                size_t createFloatVariable();

                int& getIntegerVariable(size_t ipIntVariableInx);
                std::string& getStringVariable(size_t ipStrVariableInx);
                float& getFloatVariable(size_t ipDblVariableInx);

                // API for management of the components of the instance.
                unsigned char getStateChangeCount() const; // Returns true after calling setActiveStateIndex and until catchEvent process an event DRAW.
                size_t getActiveStateIndex() const; // Retorna el Indice del Estado Activo para la Instancia.
                void setActiveStateIndex(size_t ipStateIndex, unsigned char ipAlignment = ALIGN_TOP_EDGE); // Changes the index of the active state, this index is determined by the order in which the states were introduced into the TileClass.
                bool isSpriteActive(size_t ipSpriteInx) const;
                void setSpriteActive(size_t ipSpriteInx, size_t ipFrameInx = 0, bool bpChangeFrameBuffer = false, std::shared_ptr <std::vector <gametools::sprite::Sprite::FrameDescriptor>> apFrameBuffer = std::shared_ptr <std::vector <gametools::sprite::Sprite::FrameDescriptor>> (nullptr)); // Changes the index of the current frame of the sprite with index ipSpriteInx, such that the sprite's index is determined by the order in which the sprites were introduced into the current state.
                void setSpriteInactive(size_t ipSpriteInx);
                size_t getFrameIndex(size_t ipSpriteInx) const; // Returns the index of the frame of the sprite wieh index ipSpriteInx.
                void setFrameIndex(size_t ipSpriteInx, size_t ipFrameInx);
                std::chrono::milliseconds getFrameNextTickTime(size_t ipSpriteInx) const;
                void setFrameNextTickTime(size_t ipSpriteInx, std::chrono::milliseconds tpTime);
                unsigned long getFrameTickAccumulator(size_t ipSpriteInx) const;
                void setFrameTickAccumulator(size_t ipSpriteInx, unsigned long ipValue);
                bool isSpriteReverse(size_t ipSpriteInx) const;
                void setSpriteReverse(size_t ipSpriteInx, bool bpReverse);
                bool isSpriteFinished(size_t ipSpriteInx) const;
                void setSpriteFinished(size_t ipSpriteInx, bool bpFinished);
                bool isSpriteStoped(size_t ipSpriteInx) const;
                void setSpriteStoped(size_t ipSpriteInx, bool bpStoped);

                // API for audio management.
                AudioCommand getAudioCommand(size_t ipAudioInx) const;
                void setAudioCommand(size_t ipAudioInx, unsigned char ipCommand = AudioCommand::NO_COMMAND_SOUND, bool bpLoop = false, unsigned int ipLoopLimit = 0, unsigned long ipLoopDelay = 0);
                bool executeAudioCommand(size_t ipAudioInx, unsigned char ipCommand, bool bpLoop = false, unsigned int ipLoopLimit = 0, unsigned long ipLoopDelay = 0);
                bool isAudioPlaying(size_t ipAudioInx);
                bool isAudioLooping(size_t ipAudioInx) const;
                unsigned int getAudioLoopLimit(size_t ipAudioInx) const;
                unsigned int getActualAudioLoop(size_t ipAudioInx) const;
                unsigned long getAudioLoopDelay(size_t ipAudioInx) const;

                // Catch and throw of events.
                bool throwEvent(std::shared_ptr <const EVENT> opEvent);
                // ADVERTENCIA: Puede quedar atrapado en un Ciclo Infinito si los Cambios de Estado no se planean correctamente.
                bool catchEvent(std::shared_ptr <const EVENT> opEvent);

                // API for resource management.
                bool isAudioLoaded() const;
                void loadAudios();
                void unloadAudios();
                bool isLoadingAudio();

                TileInstance(const TileInstance& opInstance) = delete;
                TileInstance& operator= (const TileInstance& opInstance) = delete;
        };

        class Level : public std::enable_shared_from_this <Level> {
            public:
                struct GamepadState {
                    unsigned int buttons;
                    short leftTrigger;
                    short rightTrigger;
                    short thumbLX;
                    short thumbLY;
                    short thumbRX;
                    short thumbRY;

                    GamepadState();
                    GamepadState(const XINPUT_STATE& rpState);
                    GamepadState& operator = (const XINPUT_STATE& rpState);
                };

            protected:
                struct InstanceDescriptor {
                    std::shared_ptr <TileInstance> instance;
                    bool has_zbufferpos = false;
                    long zpos;
                    std::list <std::shared_ptr <TileInstance>>::iterator zbufferpos;
                    bool has_acbufferpos = false;
                    std::list <std::shared_ptr <TileInstance>>::iterator acbufferpos;
                    bool has_pcbufferpos = false;
                    std::list <std::shared_ptr <TileInstance>>::iterator pcbufferpos;
                };

                struct ClassEntry {
                    std::shared_ptr <TileClass> tileclass;
                    std::map <size_t, InstanceDescriptor> tileinstances;
                    std::list <std::shared_ptr <TileInstance>> passivecollisionbuffer;
                    unsigned long lastinstancekey;
                };

                bool bgIsLoaded;
                bool bgInitialized;
                bool bgStarted;

                SCREEN rgScreen; // Window proyected on the screen from the level's space.

                std::map <size_t, ClassEntry> agTiles;
                std::shared_ptr <std::map <long, std::list <std::shared_ptr <TileInstance>>>> agZBuffer; // Depth queue, defines the order in which the Tiels are drawn starting with those farthest from the camera.
                std::list <std::shared_ptr <TileInstance>> agActiveCollisionBuffer; // List of tiles that support active collisions.
                std::function <bool (std::shared_ptr <Level> opLevel, std::shared_ptr <const EVENT> opEvent)> pgFunction;
                std::shared_ptr <gametools::renderer::ScreenRenderer> ogRenderer; // Postprocessing rederer.

                std::shared_ptr <std::vector <size_t>> agAudios; // Internal handlers of audio tracks used by the level, not by the tiles.
                std::vector <std::shared_ptr <tools::audio::AsynchronousPlayer>> agPlayers; // Audio players used by the level, not by the tiles.
                std::vector <AudioCommand> agAudioCommands; // Command queue used by the audio players of the level.
                std::vector <GamepadState> agGamepads; Estado de los Controladores. // Gamepads state.

                std::vector <int> agIntegerVariables;
                std::vector <std::string> agStringVariables;
                std::vector <float> agFloatVariables;

                bool gateway(std::shared_ptr <const EVENT> opEvent, bool bpIsInternalCall); // event throwing gateway.
                void processCollisions();

            public:
                Level(std::shared_ptr <gametools::renderer::ScreenRenderer> opRenderer = gametools::renderer::NULL_SCREEN_RENDERER, std::function <bool (std::shared_ptr <Level> opLevel, std::shared_ptr <const EVENT> opEvent)> ppFunction = NULL_LEVEL_BEHAVIOUR);

                // API for variable creation and use in the level.
                size_t createIntegerVariable();
                size_t createStringVariable();
                size_t createFloatVariable();

                int& getIntegerVariable(size_t ipIntVariableInx);
                std::string& getStringVariable(size_t ipStrVariableInx);
                float& getFloatVariable(size_t ipDblVariableInx);

                // API for throwing level events.
                void createLevel();
                void destroyLevel();
                void start();
                void end();
                
                bool throwEvent(std::shared_ptr <const EVENT> opEvent);

                // API for level graphic management.
                void playLevel(bool bpDrawBorder = false, bool bpActive = false, unsigned char ipPixelSize = 2, tools::image::RGBA rlBorderColor = DEFAULT_BORDER_COLOR);
                void setScreenRenderer(std::shared_ptr <gametools::renderer::ScreenRenderer> opRenderer);
                std::shared_ptr <gametools::renderer::ScreenRenderer> getPostRenderer();
                void setScreen(SCREEN& rpScreen, bool blIsSubScreen = false);
                const SCREEN& getScreen() const;

                // API for level component management.
                size_t registerTileClass(std::shared_ptr <TileClass> opClass);
                bool unregisterTileClass(size_t ipClassIndex);
                size_t createTileInstance(SI3DPOINT rpPosition, size_t ipClassIndex, std::shared_ptr <gametools::renderer::Renderer> opRenderer, float fpAngle = 0, SF2DPOINT rpScale = {1, 1});
                bool destroyTileInstance(size_t ipClassIndex, size_t ipInstanceIndex);

                // API for level audio management.
                void setAudios(std::shared_ptr <std::vector <size_t>> apAudios);
                size_t getAudioCount() const;
                AudioCommand getAudioCommand(size_t ipAudioInx) const;
                void setAudioCommand(size_t ipAudioInx, unsigned char ipCommand = AudioCommand::NO_COMMAND_SOUND, bool bpLoop = false, unsigned int ipLoopLimit = 0, unsigned long ipLoopDelay = 0);
                bool executeAudioCommand(size_t ipAudioInx, unsigned char ipCommand, bool bpLoop = false, unsigned int ipLoopLimit = 0, unsigned long ipLoopDelay = 0);
                bool isAudioPlaying(size_t ipAudioInx);
                bool isAudioLooping(size_t ipAudioInx) const;
                unsigned int getAudioLoopLimit(size_t ipAudioInx) const;
                unsigned int getActualAudioLoop(size_t ipAudioInx) const;
                unsigned long getAudioLoopDelay(size_t ipAudioInx) const;

                // API for level component resources management.
                bool isInitialized() const;
                bool isLoaded() const;
                void loadResources(bool bpAsynchronousLoad = false);
                void unloadResources(bool bpUnloadAudio = false);
                bool isLoadingAudio();
                void waitResourcesLoad();

                // API for user input management.
                void setGamepadCount(size_t ipGamepadCount);
                void setGamepadState(size_t ipId, std::shared_ptr <GamepadState> rpState);
                std::shared_ptr <GamepadState> getGamepadState(size_t ipId) const;
                void keyDown(size_t ipCharacter);
                void keyUp(size_t ipCharacter);

                Level(const Level& opLevel) = delete;
                Level& operator= (const Level& opLevel) = delete;
        };

        class ResourceManager {
            protected:
                struct BitmapDescriptor {
                    std::shared_ptr <tools::image::RGBAImage> bitmap;
                    size_t mask_inx;
                    bool isMemoryReloadable;
                    bool isCacheReloadable;
                    std::string path;
                    size_t handler;
                    tools::image::RGBA background;
                };
                struct AudioDescriptor {
                    std::shared_ptr <tools::audio::Audio> audio;
                    bool isReloadable;
                    std::string path;
                    size_t handle;
                };

                unsigned char igMode;
                HWND hgWindow;
                HDC hgScreen;
                HGLRC hgPrimaryContext; // Used by open gl.
                HGLRC hgSecondaryContext; // Used by open gl.
                std::vector <BitmapDescriptor> agBitmaps;
                std::vector <AudioDescriptor> agAudios;
                std::vector <unsigned int> agPrograms;

                std::thread ogLoadBitmapsThread;
                std::thread ogLoadAudiosThread;
                std::mutex ogLoadBitmapsMutex;
                std::mutex ogLoadAudiosMutex;

                ResourceManager(unsigned char ipMode, HWND hpWindow, std::function <void ()> ppLibraryInitializationFunction);

                bool setPixelFormat();

                void loadBitmapsAsynchronous(std::shared_ptr <const std::vector <size_t>> apBitmapInx);
                void loadAudiosAsynchronous(std::shared_ptr <const std::vector <size_t>> apAudioInx);

            public:
                // Graphic renderization mode contants.
                static const unsigned char GDI_ALPHA_MODE = 0;
                static const unsigned char GDI_MASK_MODE = 1;
                static const unsigned char OGL_ALPHA_MODE = 2;
                static const unsigned char OGL_MASK_MODE = 3;
                static const size_t CONTROL_BITMAP_INX = 0;
                static const std::function <void ()> NULL_INITIALIZATION_FUNCTION;

                ~ResourceManager();
                static std::shared_ptr <ResourceManager> getInstance(unsigned char ipMode = -1, HWND hpWindow = NULL, std::function <void ()> ppLibraryInitializationFunction = NULL_INITIALIZATION_FUNCTION);
                void clear(); // Free the resources contained by the resource manager to add new resources.
                void destroy(); //Destroys the resource manager and leave It in unusable state, should be called at the end of life of the manager.

                unsigned char getMode();

                // Device context management for the GDI modes.
                void setDeviceContext(HWND hpWindow);
                bool setRenderContext(HWND hpWindow);
                HDC getDeviceContext();
                HWND getWindowHandle();

                // Add resources to the manager.
                size_t addBitmap(
                    std::string spBitmapFile,
                    const tools::image::RGBA& rpBackgroundColor = BLACK,
                    bool bpMemoryReloadable = false, // Must unload the image from main memory when unloadBitmaps is called, and load It again when loadBitmaps is called.
                    bool bpCacheReloadable = false // Must unload the image from the GPU's memory when unloadBitmaps is called, and load It again when loadBitmaps is called, bpMemoryReloadable == true implies bpCacheReloadable == true.
                );
                size_t addAudio(
                    std::string spAudioFile,
                    bool bpReloadable = false // Must unload audio from memory whe unloadAudios is called, and load It again when loadAudios is called.
                );
                size_t compileShaderProgram(std::string spVertexShader, std::string spGeometryShader, std::string spFragmentShader);

                // API for mask management on the MASK mode.
                size_t makeMask(size_t ipBitmapInx, const tools::image::RGBA& ipBackgroundColor = BLACK);
                void associateMask(size_t ipBitmapInx, size_t ipMaskInx);
                size_t getMaskIndex(size_t ipBitmapInx);

                // Load and unload resources from main memory and GPU memory.
                void loadBitmaps(std::shared_ptr <const std::vector <size_t>> apBitmapInx);
                void unloadBitmaps(std::shared_ptr <const std::vector <size_t>> apBitmapInx);
                bool isLoadingBitmaps();
                void waitBitmapsLoad();
                void loadAudios(std::shared_ptr <const std::vector <size_t>> apAudioInx);
                void unloadAudios(std::shared_ptr <const std::vector <size_t>> apAudioInx);
                bool isLoadingAudios();
                void waitAudiosLoad();

                // Regurns the resources and their handlers.
                std::shared_ptr <tools::image::RGBAImage> getBitmap(size_t ipInx);
                size_t getBitmapHandle(size_t ipInx);
                std::shared_ptr <tools::audio::Audio> getAudio(size_t ipInx);
                unsigned int getShaderProgramHandle(size_t ipProgramInx);

                ResourceManager(const ResourceManager& opManager) = delete;
                ResourceManager& operator= (const ResourceManager& opManager) = delete;
        };

    } // END CONTAINERS
    
    //////////////////////////////////////////////////////////////////////////////////////
    // TOOLS

    namespace utilities {

        const unsigned char OUTLINE_VALID_ALPHA = 255;
        const unsigned char OUTLINE_SEGMENT_LENGTH = 10;
        const unsigned int OUTLINE_BORDER_SIZE = 1000;
        const unsigned int OUTLINE_INITIAL_BACKTRACE = 2;
        const unsigned char DECIMAL_CONVERSION_PRECISION = 10;
        const tools::image::RGBA OUTLINE_BACKGROUND_COLOR;

        //////////////////////////////////////////////////////////
        // FUNCTIONS TO REGISTER IMAGES IN GDI/OPENGL
        //////////////////////////////////////////////////////////

        HBITMAP writeToGDIBuffer(HDC hpDeviceContext, std::shared_ptr <const tools::image::RGBAImage> opImage);
        unsigned int writeToOGLBuffer(std::shared_ptr <const tools::image::RGBAImage> opImage, bool bpGenerateMipmap = false);

        //////////////////////////////////////////////////////////
        // FUNCTION TO PRORCESS BORDERS
        // The borders are vectors of points, where each pair of consecutive points form a segment, and additionally the first and last elements of the border form a segment.
        //////////////////////////////////////////////////////////

        // Finds the border according to the tight contour of the image based on ppFunc, and having as origin the origin of opImage.
        std::shared_ptr <BORDER> getImageTightContourBorder(
            std::shared_ptr <const tools::image::BaseImage> opImage,
            std::function <bool (const tools::image::RGBA&)> ppFunc, // Function that checks if the current point is on the border.
            unsigned char ipSegmentLength = OUTLINE_SEGMENT_LENGTH, // Distance between the elements of the border, but the distance between the first and last point of the border may be shorter.
            unsigned int ipBorderSize = OUTLINE_BORDER_SIZE, // Maximum Number of points on the border, if the number of points is enough then It is possible that the distance between the first and last point on the border is greater than ipSegmentLength.
            unsigned int ipBackTrace = OUTLINE_INITIAL_BACKTRACE // Number of points that are considered for proximity search.
        );

        // Finds the border according to the loose contour of the image based on ppFunc, and having as origin the origin of opImage.
        std::shared_ptr <BORDER> getImageLooseContourBorder(
            std::shared_ptr <const tools::image::BaseImage> opImage,
            std::function <bool(const tools::image::RGBA&)> ppFunc // Function that validates if a point is on the border.
        );

        std::shared_ptr <BORDER> getImageTightContourBorderByAlpha(
            std::shared_ptr <const tools::image::BaseImage> opImage,
            unsigned char ipValidAlpha = OUTLINE_VALID_ALPHA, // Minimum alpha used to search for the border.
            unsigned char ipSegmentLength = OUTLINE_SEGMENT_LENGTH, // Distance between the elements of the border, but the distance between the first and last point on the boder may be shorter.
            unsigned int ipBorderSize = OUTLINE_BORDER_SIZE, // Maximum Number of points on the border, if the number of points is enough then It is possible that the distance between the first and last point on the border is greater than ipSegmentLength.
            unsigned int ipBackTrace = OUTLINE_INITIAL_BACKTRACE // Number of points that are considered for proximity search.
        );

        // NOTE: Even though this method can find the border on any image, the idea is to use It to find the border using the masks of the masked sprites.
        std::shared_ptr <BORDER> getImageTightContourBorderByColor(
            std::shared_ptr <const tools::image::BaseImage> opImage,
            const tools::image::RGBA& ipBackgroundColor = OUTLINE_BACKGROUND_COLOR, // This is the foreground color that will be used to search for the border.
            unsigned char ipSegmentLength = OUTLINE_SEGMENT_LENGTH, // Distance between the elements of the border, but the distance between the first and last point on the boder may be shorter.
            unsigned int ipBorderSize = OUTLINE_BORDER_SIZE, // Maximum Number of points on the border, if the number of points is enough then It is possible that the distance between the first and last point on the border is greater than ipSegmentLength.
            unsigned int ipBackTrace = OUTLINE_INITIAL_BACKTRACE // Number of points that are considered for proximity search.
        );

        std::shared_ptr <BORDER> getImageLooseContourBorderByAlpha(
            std::shared_ptr <const tools::image::BaseImage> opImage,
            unsigned char ipValidAlpha = OUTLINE_VALID_ALPHA // Minimum alpha used to search for the border.
        );

        // NOTE: Even though this method can find the border on any image, the idea is to use It to find the border using the masks of the masked sprites.
        std::shared_ptr <BORDER> getImageLooseContourBorderByColor(
            std::shared_ptr <const tools::image::BaseImage> opImage,
            const tools::image::RGBA& ipBackgroundColor = OUTLINE_BACKGROUND_COLOR // This is the foreground color that will be used to search for the border.
        );

        // Finds the border according to the minimum square that contains the Image, and having as origin the origin of olImage.
        // The borders are defined by bpDivideByQuandrants, where:
        //  true: Produce 4 borders, one for each triangle defined by the vertexes of the square and its center, being the 0 index the leftmos triangle, and continuing clockwise, being the last point of each border the center of the square. 
        //  false: Produce a border defined by the vertexes of the square.
        BORDER_REF getImageBoxBorder(
            std::shared_ptr <const tools::image::BaseImage> opImage,
            std::function <bool(const tools::image::RGBA&)> ppFunc, // Function that validates if a point is on the border.
            bool bpDivideByQuandrants = false
        );

        BORDER_REF getImageBoxBorderByAlpha(
            std::shared_ptr <const tools::image::BaseImage> olImage,
            unsigned char ipValidAlpha = OUTLINE_VALID_ALPHA, // Minimum alpha used to search for the border.
            bool bpDivideByQuandrants = false
        );

        // NOTE: Even though this method can find the border on any image, the idea is to use It to find the border using the masks of the masked sprites.
        BORDER_REF getImageBoxBorderByColor(
            std::shared_ptr <const tools::image::BaseImage> olImage,
            const tools::image::RGBA& ipBackgroundColor = OUTLINE_BACKGROUND_COLOR, // This is the foreground color that will be used to search for the border.
            bool bpDivideByQuandrants = false
        );

        //////////////////////////////////////////////////////////
        // FUNCTIONS TO FIND BORDERS COLLISIONS
        //////////////////////////////////////////////////////////

        // How many times the segment defined by rpAPoint and rpBPoint intersects the segments defined by apBorder.
        std::shared_ptr <BORDER> findIntersections(
            const SI3DPOINT& rpAPoint, // Point A of the segment, should be adjusted to the current position of the tile that It belongs to.
            const SI3DPOINT& rpBPoint, // Point B of the segment, should be adjusted to the current position of the tile that It belongs to.
            const std::shared_ptr <CONST_BORDER> apBorder, // Border, their points should be adjusted to the current position of the tile that they belong to.
            bool bpReturnOnFirstCross = true, // true: Returns when It finds the first intersection between the segment and the border.
            bool bpCountOneSidedNodeAsOne = false, // true: If the segment A goes through a point that is shared by 2 segments that are at both sides of A, then count those segments as just one intersection.
            bool bpCountTwoSidedNodeAsOne = true, // true: If the segment A goes through a point that is shared by 2 segments that are at the same side of A, then count those segments as just one intersection.
            unsigned char ipPrecision = DECIMAL_CONVERSION_PRECISION // Precision of the convertion from real to integer.
        );

        // There are segments defined by apABorder and apBBorder that interect each other?
        bool isIntersectedBy(
            const std::shared_ptr <CONST_BORDER> apABorder, // Border A, their points should be adjusted to the current position of the tile that they belong to.
            const std::shared_ptr <CONST_BORDER> apBBorder, // Border B, their points should be adjusted to the current position of the tile that they belong to.
            unsigned char ipPrecision = DECIMAL_CONVERSION_PRECISION
        );

        // There is a point in apPoints inside the area defined by apBorder?
        bool isInsideOf(
            const std::shared_ptr <CONST_BORDER> apPoints, // Border A, their points should be adjusted to the current position of the tile that they belong to.
            const std::shared_ptr <CONST_BORDER> apBorder, // Border B, their points should be adjusted to the current position of the tile that they belong to.
            unsigned char ipPrecision = DECIMAL_CONVERSION_PRECISION
        );

        // Is the distance between rpACenter and rpBCenter smaller than the greatest radio between ipARadius and ipBRadius?
        bool isWithinRadiusOf(
            const SI3DPOINT& rpACenter,
            unsigned short ipARadius,
            const SI3DPOINT& rpAReference,
            const SI3DPOINT& rpBCenter,
            unsigned short ipBRadius,
            const SI3DPOINT& rpBReference
        );

        //////////////////////////////////////////////////////////
        // FUNCTION FOR MASK CREATION
        //////////////////////////////////////////////////////////

        std::shared_ptr <tools::image::RGBAImage> getMaskedBitmap(std::shared_ptr <const tools::image::RGBAImage> opImage, bool bpIsMask, bool blInvert, const tools::image::RGBA& ipBackgroundColor = OUTLINE_BACKGROUND_COLOR);

        //////////////////////////////////////////////////////////
        // HELPER FUNCTIONS FOR OPEN/GL
        //////////////////////////////////////////////////////////

        std::shared_ptr <std::vector <float>> getPerspectiveMatrix(float fpFieldOfView, float fpAspect, float fpZNear, float fpZFar);
        std::shared_ptr <std::vector <float>> getTransformationMatrix(float fpNewX, float fpNewY, float fpNewZ, float fpAngle, float fpXScale, float fpYScale);
        std::shared_ptr <std::vector <float>> getIdentityMatrix();
        SI3DPOINT getNewVector(long fpX, long fpY, long fpZ, long ipNewX, long ipNewY, long ipNewZ, float fpAngle, float fpXScale, float fpYScale);

        //////////////////////////////////////////////////////////////////////////////////////
        // MISCELLANEOUS FUNTIONS
        //////////////////////////////////////////////////////////

        std::wstring convertoToWString(const std::string& spSource);
        std::string convertoToString(const std::wstring& spSource);

    }

}

///////////////////////////////////////////////////////////
#endif