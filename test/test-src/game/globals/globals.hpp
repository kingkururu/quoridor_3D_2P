//
//  globals.hpp
//
//

#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <iostream> 
#include <sstream>
#include <fstream> 
#include <yaml-cpp/yaml.h>
#include <filesystem>
#include <random>
#include <stack>
#include <unordered_set>

#include "../test-logging/log.hpp"
#include "../utils/utils.hpp"

namespace SpriteComponents {
    enum Direction { NONE, LEFT, RIGHT, UP, DOWN };

    Direction toDirection(const std::string& direction); // convert string from yaml to Direction
    sf::Color toSfColor(const std::string& color); // convert string from yaml to sf::Color
}

namespace MetaComponents{
    inline sf::Vector2i leftViewmouseClickedPosition_i {}; 
    inline sf::Vector2f leftViewmouseClickedPosition_f {}; 

    inline sf::Vector2i middleViewmouseClickedPosition_i {}; 
    inline sf::Vector2f middleViewmouseClickedPosition_f {}; 
    inline sf::Vector2f middleViewmouseCurrentPosition_f {};
    inline sf::Vector2i middleViewmouseCurrentPosition_i {};
    
    inline float globalTime {};
    inline float deltaTime {}; 
    inline float spacePressedElapsedTime{};

    extern sf::Clock clock;
    extern sf::View middleView;
    extern sf::View leftView;
    extern sf::View rightView;

    extern sf::FloatRect getmiddleViewBounds();
    extern float getmiddleViewMinX();
    extern float getmiddleViewMaxX();
    extern float getmiddleViewMinY();
    extern float getmiddleViewMaxY();
}

namespace Constants { // not actually "constants" in terms of being fixed, but should never be altered after being read from the config.yaml file
    extern void initialize();

    // make random positions each time
    extern sf::Vector2f makeRandomPosition(); 

    // load textures, fonts, music, and sound
    std::shared_ptr<sf::Uint8[]> createBitmask( const std::shared_ptr<sf::Texture>& texture, const sf::IntRect& rect, const float transparency = 0.0f);
    std::shared_ptr<sf::Uint8[]> createBitmaskForBottom( const std::shared_ptr<sf::Texture>& texture, const sf::IntRect& rect, const float transparency = 0.0f, int rows = 1);

    extern void printBitmaskDebug(const std::shared_ptr<sf::Uint8[]>& bitmask, unsigned int width, unsigned int height); // make visible globally for debugging purposes
    void loadAssets(); 
    void readFromYaml(const std::filesystem::path configFile); 
    void makeRectsAndBitmasks(); 

    // Game display settings
    inline float WORLD_SCALE;
    inline unsigned short WORLD_WIDTH;
    inline unsigned short WORLD_HEIGHT;
    inline unsigned short FRAME_LIMIT;
    inline std::string GAME_TITLE;
    inline sf::Vector2f VIEW_INITIAL_CENTER;
    inline float VIEW_SIZE_X;
    inline float VIEW_SIZE_Y;
    inline sf::FloatRect VIEW_RECT;
    inline unsigned short FOV;
    inline size_t RAYS_NUM;
    inline sf::Color WALL_COLOR;

    // Score settings
    inline unsigned short INITIAL_SCORE;

    // Animation settings
    inline float ANIMATION_CHANGE_TIME;
    inline short PASSTHROUGH_OFFSET;

    // Sprite and text settings
    inline unsigned short SPRITE_OUT_OF_BOUNDS_OFFSET;
    inline unsigned short SPRITE_OUT_OF_BOUNDS_ADJUSTMENT;
    inline unsigned short PLAYER_Y_POS_BOUNDS_RUN;
  
    // player paths and settings
    inline short SPRITE1_INDEXMAX;
    inline short SPRITE1_ANIMATIONROWS;  
    inline std::filesystem::path SPRITE1_PATH;
    inline sf::Vector2f SPRITE1_POSITION;
    inline sf::Vector2f SPRITE1_SCALE;
    inline sf::Vector2f SPRITE1_JUMP_ACCELERATION;
    inline float SPRITE1_SPEED;
    inline sf::Vector2f SPRITE1_ACCELERATION;
    inline std::shared_ptr<sf::Texture> SPRITE1_TEXTURE = std::make_shared<sf::Texture>();
    inline std::vector<sf::IntRect> SPRITE1_ANIMATIONRECTS;
    inline std::vector<std::shared_ptr<sf::Uint8[]>> SPRITE1_BITMASK;

    inline short SPRITE2_INDEXMAX;
    inline short SPRITE2_ANIMATIONROWS;  
    inline std::filesystem::path SPRITE2_PATH;
    inline sf::Vector2f SPRITE2_POSITION;
    inline sf::Vector2f SPRITE2_SCALE;
    inline sf::Vector2f SPRITE2_JUMP_ACCELERATION;
    inline float SPRITE2_SPEED;
    inline sf::Vector2f SPRITE2_ACCELERATION;
    inline std::shared_ptr<sf::Texture> SPRITE2_TEXTURE = std::make_shared<sf::Texture>();
    inline std::vector<sf::IntRect> SPRITE2_ANIMATIONRECTS;
    inline std::vector<std::shared_ptr<sf::Uint8[]>> SPRITE2_BITMASK;

    // not in use rn
    inline short BUTTON1_INDEXMAX;
    inline short BUTTON1_ANIMATIONROWS;  
    inline std::filesystem::path BUTTON1_PATH;
    inline sf::Vector2f BUTTON1_POSITION;
    inline sf::Vector2f BUTTON1_SCALE;
    inline std::shared_ptr<sf::Texture> BUTTON1_TEXTURE = std::make_shared<sf::Texture>();
    inline std::vector<sf::IntRect> BUTTON1_ANIMATIONRECTS;
    inline std::vector<std::shared_ptr<sf::Uint8[]>> BUTTON1_BITMASK;
 
    // pawn paths and settings
    inline std::filesystem::path PAWNBLUE_PATH;
    inline sf::Vector2f PAWNBLUE_POSITION;
    inline sf::Vector2f PAWNBLUE_SCALE;
    inline std::shared_ptr<sf::Texture> PAWNBLUE_TEXTURE = std::make_shared<sf::Texture>();

    inline std::filesystem::path PAWNRED_PATH;
    inline sf::Vector2f PAWNRED_POSITION;
    inline sf::Vector2f PAWNRED_SCALE;
    inline std::shared_ptr<sf::Texture> PAWNRED_TEXTURE = std::make_shared<sf::Texture>();

    inline unsigned short const STICKS_NUMBER = 20; // always 20 sticks in the game
    inline std::filesystem::path STICK_PATH;
    inline float STICK_SPACING;
    inline sf::Vector2f STICK_STARTING_POSITION; // starting position of the first stick
    inline float RIGHTSTICK_OFFSET_X; // offset for right stick
    inline float RIGHTSTICK_OFFSET_Y; // offset for right stick
    inline std::array<sf::Vector2f, STICKS_NUMBER> STICK_POSITIONS; 
    inline sf::Vector2f STICK_SCALE;
    inline std::shared_ptr<sf::Texture> STICK_TEXTURE = std::make_shared<sf::Texture>();

    // Background (in the big view) paths and settings
    inline std::filesystem::path BACKGROUNDBIG_PATH;
    inline sf::Vector2f BACKGROUNDBIG_POSITION;
    inline sf::Vector2f BACKGROUNDBIG_SCALE;
    inline std::shared_ptr<sf::Texture> BACKGROUNDBIG_TEXTURE = std::make_shared<sf::Texture>();
    inline std::filesystem::path BACKGROUNDBIGFINAL_PATH;
    inline sf::Vector2f BACKGROUNDBIGFINAL_POSITION;
    inline sf::Vector2f BACKGROUNDBIGFINAL_SCALE;
    inline std::shared_ptr<sf::Texture> BACKGROUNDBIGFINAL_TEXTURE = std::make_shared<sf::Texture>();
    inline std::filesystem::path BACKGROUNDBIGHALF_PATH;
    inline sf::Vector2f BACKGROUNDBIGHALF_POSITION;
    inline sf::Vector2f BACKGROUNDBIGHALF_SCALE;
    inline std::shared_ptr<sf::Texture> BACKGROUNDBIGHALF_TEXTURE = std::make_shared<sf::Texture>();
    
    // Board tile settings
    inline std::filesystem::path BOARDTILES_PATH;
    inline size_t PATH_TILE_INDEX;
    inline size_t P1_GOAL_TILE_INDEX;
    inline size_t P2_GOAL_TILE_INDEX;
    inline size_t BLANKWALL_TILE_INDEX; // for single space between stick and path
    inline size_t BLANKP1_INDEX; 
    inline size_t BLANKP2_INDEX;
    inline size_t WALL_TILEX_INDEX; // horizontal wall tile  
    inline size_t WALL_TILEY_INDEX; // vertical wall tile 
    inline size_t WALL_INDEX;
    inline size_t WALLBLANK_INDEX; // horizontal wall tile  
    inline size_t WALLTOP_INDEX; // vertical wall tile 

    inline sf::Vector2f BOARDTILES_SCALE;
    inline std::shared_ptr<sf::Texture> BOARDTILES_TEXTURE = std::make_shared<sf::Texture>();
    inline std::array<sf::IntRect, 11> BOARDTILES_RECTS; 
    inline std::array<std::shared_ptr<sf::Uint8[]>, 11> BOARDTILES_BITMASK; 
    inline size_t BOARDTILES_ROW;
    inline size_t BOARDTILES_COL;

    // Text settings
    inline unsigned short TEXT_SIZE;
    inline std::filesystem::path TEXT_PATH;
    inline std::string TEXT_MESSAGE;
    inline sf::Vector2f TEXT_POSITION;
    inline sf::Color TEXT_COLOR;
    inline std::shared_ptr<sf::Font> TEXT_FONT = std::make_shared<sf::Font>(); 

    inline unsigned short SCORETEXT_SIZE;
    inline std::string SCORETEXT_MESSAGE;
    inline sf::Vector2f SCORETEXT_POSITION;
    inline sf::Color SCORETEXT_COLOR;

    inline unsigned short ENDINGTEXT_SIZE;
    inline std::string ENDINGTEXT_MESSAGE;
    inline sf::Vector2f ENDINGTEXT_POSITION;
    inline sf::Color ENDINGTEXT_COLOR;

    // Music settings
    inline std::filesystem::path BACKGROUNDMUSIC_PATH;
    inline float BACKGROUNDMUSIC_VOLUME;
    inline std::unique_ptr<sf::Music> BACKGROUNDMUSIC_MUSIC = std::make_unique<sf::Music>(); 
    inline bool BACKGROUNDMUSIC_LOOP;
    inline float BACKGROUNDMUSIC_ENDINGVOLUME;

    // Sound settings
    inline std::filesystem::path BUTTONCLICKSOUND_PATH;
    inline float BUTTONCLICKSOUND_VOLUME;
    inline std::shared_ptr<sf::SoundBuffer> BUTTONCLICK_SOUNDBUFF = std::make_shared<sf::SoundBuffer>();
}

// New namespace for flag events
namespace FlagSystem {
    
    struct FlagEvents {
        // game states
        bool gameEnd; 

        // keyboard inputs
        bool wPressed;
        bool aPressed;
        bool sPressed;
        bool dPressed;
        bool bPressed;
        bool mPressed;

        bool leftPressed;
        bool rightPressed;
        bool downPressed;
        bool upPressed;

        bool spacePressed; 
        bool mouseClicked;

        FlagEvents() : wPressed(false), aPressed(false), sPressed(false), dPressed(false), bPressed(false), mPressed(false), 
                       leftPressed(false), rightPressed(false), downPressed(false), upPressed(false),  
                       spacePressed(false), mouseClicked(false) {}

        // resets every flag
        void resetFlags() {
            gameEnd = wPressed = aPressed = sPressed = dPressed = bPressed = mPressed = 
                      leftPressed = rightPressed = downPressed = upPressed = 
                      spacePressed = mouseClicked = false;
            log_info("General game flags reset complete");
        }

        // resets keyboard flags only 
        void allFlagKeyReleased() {
            wPressed = false;
            aPressed = false;
            sPressed = false;
            dPressed = false;
            bPressed = false;
            mPressed = false;
            leftPressed = false;
            rightPressed = false;
            downPressed = false;
            upPressed = false;
            spacePressed = false;
        }
    };

    inline FlagEvents flagEvents; 

    struct SceneEvents {
        bool sceneEnd;
        bool sceneStart; 

        SceneEvents() : sceneEnd(false), sceneStart(false) {} 

        void resetFlags() {
            sceneEnd = false;
            sceneStart = false;
            log_info("Reset sceneEvents flags");
        }
    };

    struct GameSceneEvents1 {
        bool sceneEnd;
        bool sceneStart; 

        bool begin; 

        bool playerBlueTurn;
        bool playerRedTurn;

        bool stickPlaced; // true if player placed a stick
        bool moved; // true if player moved

        GameSceneEvents1() : sceneEnd(false), sceneStart(true), begin(false), playerBlueTurn(true), playerRedTurn(false), stickPlaced(false), moved(false) {}

          void resetFlags() {
            sceneEnd = false;
            sceneStart = false;
            begin = false;
            playerBlueTurn = true;  // Default to player 1's turn
            playerRedTurn = false;
            stickPlaced = false;
            moved = false;
            log_info("Reset GameSceneEvents1 flags");
        }
    };
    inline GameSceneEvents1 gameScene1Flags; // accesible from everywhere 
}
