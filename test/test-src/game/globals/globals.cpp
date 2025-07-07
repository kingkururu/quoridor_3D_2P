//
//  globals.cpp
//
//

#include "globals.hpp"  
    
namespace MetaComponents {
    sf::Clock clock;
    sf::View middleView; 
    sf::View leftView; 
    sf::View rightView;

    sf::FloatRect getmiddleViewBounds(){
        return {
            middleView.getCenter().x - middleView.getSize().x / 2,
            middleView.getCenter().y - middleView.getSize().y / 2,
            middleView.getSize().x,
            middleView.getSize().y
        };
    }

    float getmiddleViewMinX(){
        return middleView.getCenter().x - middleView.getSize().x / 2;
    }
    float getmiddleViewMaxX(){
        return middleView.getCenter().x + middleView.getSize().x / 2;
    }
    float getmiddleViewMinY(){
        return middleView.getCenter().y - middleView.getSize().y / 2;
    }
    float getmiddleViewMaxY(){
        return middleView.getCenter().y + middleView.getSize().y / 2;       
    }
}   

namespace SpriteComponents {
    Direction toDirection(const std::string& direction) {
        static const std::unordered_map<std::string, Direction> directionMap = {
            {"LEFT", Direction::LEFT},
            {"RIGHT", Direction::RIGHT},
            {"UP", Direction::UP},
            {"DOWN", Direction::DOWN}
        };

        auto it = directionMap.find(direction);
        return (it != directionMap.end()) ? it->second : Direction::NONE;
    }

    sf::Color toSfColor(const std::string& color) {
        static const std::unordered_map<std::string, sf::Color> colorMap = {
            {"RED", sf::Color::Red},
            {"GREEN", sf::Color::Green},
            {"BLUE", sf::Color::Blue},
            {"YELLOW", sf::Color::Yellow},
            {"MAGENTA", sf::Color::Magenta},
            {"CYAN", sf::Color::Cyan},
            {"WHITE", sf::Color::White},
            {"BLACK", sf::Color::Black},
            {"TRANSPARENT", sf::Color::Transparent},
            {"CUSTOMCOLOR_LIGHTCORAL", sf::Color(240, 128, 128)}, // add colors like this 
            {"CUSTOMCOLOR_BROWN", sf::Color(225, 190, 153)}
        };

        auto it = colorMap.find(color);
        return (it != colorMap.end()) ? it->second : sf::Color::Black; // Default to Black if not found
    }
}

namespace Constants {
    // make random position from upper right corner
    sf::Vector2f makeRandomPosition(){
        float xPos = static_cast<float>(WORLD_WIDTH - std::rand() % static_cast<int>(WORLD_WIDTH / 2));
        float yPos = 0.0f;
        return sf::Vector2f{ xPos, yPos }; 
    }

    void initialize(){
        std::srand(static_cast<unsigned int>(std::time(nullptr))); 

        readFromYaml(std::filesystem::path("test/test-src/game/globals/config.yaml"));

        loadAssets();
        makeRectsAndBitmasks(); 
    }

    void readFromYaml(const std::filesystem::path configFile) {
        try{ 
            YAML::Node config = YAML::LoadFile(configFile);

            // Load game display settings
            WORLD_SCALE = config["world"]["scale"].as<float>();
            WORLD_WIDTH = config["world"]["width"].as<unsigned short>();
            WORLD_HEIGHT = config["world"]["height"].as<unsigned short>();
            FRAME_LIMIT = config["world"]["frame_limit"].as<unsigned short>();
            GAME_TITLE = config["world"]["title"].as<std::string>();
            VIEW_SIZE_X = config["world"]["view"]["size_x"].as<float>();
            VIEW_SIZE_Y = config["world"]["view"]["size_y"].as<float>();
            VIEW_INITIAL_CENTER = {config["world"]["view"]["initial_center"]["x"].as<float>(),
                                config["world"]["view"]["initial_center"]["y"].as<float>()};
            VIEW_RECT = { 0.0f, 0.0f, VIEW_SIZE_X, VIEW_SIZE_Y };
            FOV = config["world"]["FOV"].as<unsigned short>(); 
            RAYS_NUM = config["world"]["rays_num"].as<size_t>(); 
            WALL_COLOR = SpriteComponents::toSfColor(config["world"]["wall_color"].as<std::string>());

            // Load score settings
            INITIAL_SCORE = config["score"]["initial"].as<unsigned short>(); 

            // Load animation settings
            ANIMATION_CHANGE_TIME = config["animation"]["change_time"].as<float>();
            PASSTHROUGH_OFFSET = config["animation"]["passthrough_offset"].as<short>();

            // Load sprite and text settings
            SPRITE_OUT_OF_BOUNDS_OFFSET = config["sprite"]["out_of_bounds_offset"].as<unsigned short>();
            SPRITE_OUT_OF_BOUNDS_ADJUSTMENT = config["sprite"]["out_of_bounds_adjustment"].as<unsigned short>();
            PLAYER_Y_POS_BOUNDS_RUN = config["sprite"]["player_y_pos_bounds_run"].as<unsigned short>();

            ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            /////////////////////////////////////////////////// Lobby Scene ///////////////////////////////////////////////////////////
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            
            LOBBYTEXT_SIZE = config["lobby"]["text"]["size"].as<unsigned short>();
            LOBBYTEXT_PATH = config["lobby"]["text"]["path"].as<std::string>();
            LOBBYTEXT_MESSAGE = config["lobby"]["text"]["message"].as<std::string>();
            LOBBYTEXT_POSITION = {config["lobby"]["text"]["position"]["x"].as<float>(),
                                config["lobby"]["text"]["position"]["y"].as<float>()};
            LOBBYTEXT_COLOR = SpriteComponents::toSfColor(config["lobby"]["text"]["color"].as<std::string>());
            LOBBYTEXT_FONT = std::make_shared<sf::Font>();
            
            HOSTCODETEXT_SIZE = config["lobby"]["hostcode_text"]["size"].as<unsigned short>();
            HOSTCODETEXT_MESSAGE = config["lobby"]["hostcode_text"]["message"].as<std::string>();
            HOSTCODETEXT_POSITION = {config["lobby"]["hostcode_text"]["position"]["x"].as<float>(),
                                config["lobby"]["hostcode_text"]["position"]["y"].as<float>()};
            HOSTCODETEXT_COLOR = SpriteComponents::toSfColor(config["lobby"]["hostcode_text"]["color"].as<std::string>());

            ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            /////////////////////////////////////////////////// Game Play Scene 1 /////////////////////////////////////////////////////
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

            // Load player paths and settings
            SPRITE1_PATH = config["sprites"]["sprite1"]["path"].as<std::string>();
            SPRITE1_SPEED = config["sprites"]["sprite1"]["speed"].as<float>();
            SPRITE1_ACCELERATION = {config["sprites"]["sprite1"]["acceleration"]["x"].as<float>(),
                                config["sprites"]["sprite1"]["acceleration"]["y"].as<float>()};   
            SPRITE1_JUMP_ACCELERATION = {config["sprites"]["sprite1"]["jump_acceleration"]["x"].as<float>(),
                                config["sprites"]["sprite1"]["jump_acceleration"]["y"].as<float>()};           
            SPRITE1_INDEXMAX = config["sprites"]["sprite1"]["index_max"].as<short>();
            SPRITE1_ANIMATIONROWS = config["sprites"]["sprite1"]["animation_rows"].as<short>();
            SPRITE1_POSITION = {config["sprites"]["sprite1"]["position"]["x"].as<float>(),
                                config["sprites"]["sprite1"]["position"]["y"].as<float>()};
            SPRITE1_SCALE = {config["sprites"]["sprite1"]["scale"]["x"].as<float>(),
                            config["sprites"]["sprite1"]["scale"]["y"].as<float>()};

            // Load second player paths and settings
            SPRITE2_PATH = config["sprites"]["sprite2"]["path"].as<std::string>();
            SPRITE2_SPEED = config["sprites"]["sprite2"]["speed"].as<float>();
            SPRITE2_ACCELERATION = {config["sprites"]["sprite2"]["acceleration"]["x"].as<float>(),
                                config["sprites"]["sprite2"]["acceleration"]["y"].as<float>()};
            SPRITE2_JUMP_ACCELERATION = {config["sprites"]["sprite2"]["jump_acceleration"]["x"].as<float>(),
                                config["sprites"]["sprite2"]["jump_acceleration"]["y"].as<float>()};            
            SPRITE2_INDEXMAX = config["sprites"]["sprite2"]["index_max"].as<short>();
            SPRITE2_ANIMATIONROWS = config["sprites"]["sprite2"]["animation_rows"].as<short>();
            SPRITE2_POSITION = {config["sprites"]["sprite2"]["position"]["x"].as<float>(),
                                config["sprites"]["sprite2"]["position"]["y"].as<float>()};
            SPRITE2_SCALE = {config["sprites"]["sprite2"]["scale"]["x"].as<float>(),
                            config["sprites"]["sprite2"]["scale"]["y"].as<float>()};

            // Load enemy paths and settings
            BUTTON1_PATH = config["sprites"]["button1"]["path"].as<std::string>();
            BUTTON1_INDEXMAX = config["sprites"]["button1"]["index_max"].as<short>();
            BUTTON1_ANIMATIONROWS = config["sprites"]["button1"]["animation_rows"].as<short>();
            BUTTON1_POSITION = {config["sprites"]["button1"]["position"]["x"].as<float>(),
                                config["sprites"]["button1"]["position"]["y"].as<float>()};
            BUTTON1_SCALE = {config["sprites"]["button1"]["scale"]["x"].as<float>(),
                            config["sprites"]["button1"]["scale"]["y"].as<float>()};                
        
            // Load pawn paths and settings
            PAWN1_PATH = config["sprites"]["pawn_1"]["path"].as<std::string>();
            PAWN1_POSITION = {config["sprites"]["pawn_1"]["position"]["x"].as<float>(),
                                config["sprites"]["pawn_1"]["position"]["y"].as<float>()};
            PAWN1_SCALE = {config["sprites"]["pawn_1"]["scale"]["x"].as<float>(),
                            config["sprites"]["pawn_1"]["scale"]["y"].as<float>()};

            PAWN2_PATH = config["sprites"]["pawn_2"]["path"].as<std::string>();
            PAWN2_POSITION = {config["sprites"]["pawn_2"]["position"]["x"].as<float>(),
                                config["sprites"]["pawn_2"]["position"]["y"].as<float>()};
            PAWN2_SCALE = {config["sprites"]["pawn_2"]["scale"]["x"].as<float>(),
                            config["sprites"]["pawn_2"]["scale"]["y"].as<float>()};
  
            // Load stick paths and settings
            STICK_PATH = config["sprites"]["stick"]["path"].as<std::string>();
            STICK_SPACING = config["sprites"]["stick"]["spacing"].as<float>();
            STICK_STARTING_POSITION = {config["sprites"]["stick"]["starting_position"]["x"].as<float>(),
                                config["sprites"]["stick"]["starting_position"]["y"].as<float>()};
            STICK_SCALE = {config["sprites"]["stick"]["scale"]["x"].as<float>(),
                            config["sprites"]["stick"]["scale"]["y"].as<float>()};
            RIGHTSTICK_OFFSET_X = config["sprites"]["stick"]["right_stick_offset_x"].as<float>();
            RIGHTSTICK_OFFSET_Y = config["sprites"]["stick"]["right_stick_offset_y"].as<float>();
          
            unsigned short blueIndex = 0; // Index for STICK_POSITIONSBLUE
            unsigned short redIndex = 0;  // Index for STICK_POSITIONSRED
            for (unsigned short i = 0; i < STICKS_NUMBER; ++i) {
                if (i % 2) {
                    STICK_POSITIONSRED[redIndex] = { STICK_STARTING_POSITION.x, STICK_STARTING_POSITION.y + i * STICK_SPACING };
                    redIndex++;
                } else { 
                    STICK_POSITIONSBLUE[blueIndex] = { VIEW_SIZE_X + RIGHTSTICK_OFFSET_X + STICK_STARTING_POSITION.x, RIGHTSTICK_OFFSET_Y + STICK_STARTING_POSITION.y + i * STICK_SPACING };
                    blueIndex++;
                }
            }

            // Load background (in the big screen) settings
            BACKGROUNDBIG_PATH = config["sprites"]["background_big"]["path"].as<std::string>();
            BACKGROUNDBIG_POSITION = {config["sprites"]["background_big"]["position"]["x"].as<float>(),
                                    config["sprites"]["background_big"]["position"]["y"].as<float>()};
            BACKGROUNDBIG_SCALE = {config["sprites"]["background_big"]["scale"]["x"].as<float>(),
                                config["sprites"]["background_big"]["scale"]["y"].as<float>()};

            BACKGROUNDBIGFINAL_PATH = config["sprites"]["background_big_final"]["path"].as<std::string>();
            BACKGROUNDBIGFINAL_POSITION = {config["sprites"]["background_big_final"]["position"]["x"].as<float>(),
                                    config["sprites"]["background_big_final"]["position"]["y"].as<float>()};
            BACKGROUNDBIGFINAL_SCALE = {config["sprites"]["background_big_final"]["scale"]["x"].as<float>(),
                                config["sprites"]["background_big_final"]["scale"]["y"].as<float>()};

            BACKGROUNDBIGHALF_PATH = config["sprites"]["background_big_half"]["path"].as<std::string>();
            BACKGROUNDBIGHALF_POSITION = {config["sprites"]["background_big_half"]["position"]["x"].as<float>(),
                                    config["sprites"]["background_big_half"]["position"]["y"].as<float>()};
            BACKGROUNDBIGHALF_SCALE = {config["sprites"]["background_big_half"]["scale"]["x"].as<float>(),
                                config["sprites"]["background_big_half"]["scale"]["y"].as<float>()};

            // Load board tile settings
            BOARDTILES_PATH = config["board"]["tiles_path"].as<std::string>();
            BOARDTILES_SCALE = {config["board"]["scale"]["x"].as<float>(),
                            config["board"]["scale"]["y"].as<float>()};   
            BOARDTILES_ROW = config["board"]["tiles_row"].as<size_t>();
            BOARDTILES_COL = config["board"]["tiles_col"].as<size_t>(); 
            
            WALL_TILEX_INDEX = config["board"]["wall_tileX_index"].as<size_t>();
            WALL_TILEY_INDEX = config["board"]["wall_tileY_index"].as<size_t>();
            PATH_TILE_INDEX = config["board"]["path_tile_index"].as<size_t>();
            P1_GOAL_TILE_INDEX = config["board"]["p1_goal_tile_index"].as<size_t>();
            P2_GOAL_TILE_INDEX = config["board"]["p2_goal_tile_index"].as<size_t>();
            BLANKWALL_TILE_INDEX = config["board"]["blankwall_tile_index"].as<size_t>(); // additional tile type for walls that are not there yet
            BLANKP1_INDEX = config["board"]["blank_p1_index"].as<size_t>(); // additional tile type for player 1
            BLANKP2_INDEX = config["board"]["blank_p2_index"].as<size_t>(); // additional tile type for player 2
            WALL_INDEX = config["board"]["wall_index"].as<size_t>();
            WALLBLANK_INDEX = config["board"]["wall_blank_index"].as<size_t>(); 
            WALLTOP_INDEX = config["board"]["wall_top_index"].as<size_t>(); // both top and bottom
            TILE_THRESHOLD = config["board"]["tile_threshold"].as<float>(); // threshold for tile movement
                                      
            // Load text settings
            TEXT_SIZE = config["text"]["size"].as<unsigned short>();
            TEXT_PATH = config["text"]["font_path"].as<std::string>();
            TEXT_MESSAGE = config["text"]["message"].as<std::string>();
            TEXT_POSITION = {config["text"]["position"]["x"].as<float>(),
                            config["text"]["position"]["y"].as<float>()};
            TEXT_COLOR = SpriteComponents::toSfColor(config["text"]["color"].as<std::string>());

            SCORETEXT_SIZE = config["score_text"]["size"].as<unsigned short>();
            SCORETEXT_MESSAGE = config["score_text"]["message"].as<std::string>();
            SCORETEXT_POSITION = {config["score_text"]["position"]["x"].as<float>(),
                                config["score_text"]["position"]["y"].as<float>()};
            SCORETEXT_COLOR = SpriteComponents::toSfColor(config["score_text"]["color"].as<std::string>());

            ENDINGTEXT_SIZE = config["ending_text"]["size"].as<unsigned short>();
            ENDINGTEXT_MESSAGE = config["ending_text"]["message"].as<std::string>();
            ENDINGTEXT_POSITION = {config["ending_text"]["position"]["x"].as<float>(),
                                config["ending_text"]["position"]["y"].as<float>()};
            ENDINGTEXT_COLOR = SpriteComponents::toSfColor(config["ending_text"]["color"].as<std::string>());   
            
            PLAYER1TEXT_SIZE = config["player1_text"]["size"].as<unsigned short>();
            PLAYER1TEXT_MESSAGE = config["player1_text"]["message"].as<std::string>();
            PLAYER1TEXT_POSITION = {config["player1_text"]["position"]["x"].as<float>(),
                                config["player1_text"]["position"]["y"].as<float>()};
            PLAYER1TEXT_COLOR = SpriteComponents::toSfColor(config["player1_text"]["color"].as<std::string>());

            PLAYER2TEXT_SIZE = config["player2_text"]["size"].as<unsigned short>();
            PLAYER2TEXT_MESSAGE = config["player2_text"]["message"].as<std::string>();
            PLAYER2TEXT_POSITION = {config["player2_text"]["position"]["x"].as<float>(),
                                config["player2_text"]["position"]["y"].as<float>()};
            PLAYER2TEXT_COLOR = SpriteComponents::toSfColor(config["player2_text"]["color"].as<std::string>());

            // Load music settings
            BACKGROUNDMUSIC_PATH = config["music"]["background_music"]["path"].as<std::string>();
            BACKGROUNDMUSIC_VOLUME = config["music"]["background_music"]["volume"].as<float>();
            BACKGROUNDMUSIC_LOOP = config["music"]["background_music"]["loop"].as<bool>();
            BACKGROUNDMUSIC_ENDINGVOLUME = config["music"]["background_music"]["ending_volume"].as<float>();
            
            // Load sound settings
            BUTTONCLICKSOUND_PATH = config["sound"]["button_click"]["path"].as<std::string>();
            BUTTONCLICKSOUND_VOLUME = config["sound"]["button_click"]["volume"].as<float>();
            
            log_info("Succesfuly read yaml file");
        } 
        catch (const YAML::BadFile& e) {
            log_error("Failed to load config file: " + std::string(e.what()));
        } 
        catch (const YAML::Exception& e) {
            log_error("YAML parsing error: " + std::string(e.what()));
        }

    }

    void loadAssets(){  // load all sprites textures and stuff across scenes 
        // lobby
        if (!LOBBYTEXT_FONT->loadFromFile(LOBBYTEXT_PATH)) log_warning("Failed to load lobby text font");

        // sprites
        if (!SPRITE1_TEXTURE->loadFromFile(SPRITE1_PATH)) log_warning("Failed to load sprite1 texture");
        if (!SPRITE2_TEXTURE->loadFromFile(SPRITE2_PATH)) log_warning("Failed to load sprite2 texture");
        if (!PAWN1_TEXTURE->loadFromFile(PAWN1_PATH)) log_warning("Failed to load board texture"); 
        if (!PAWN2_TEXTURE->loadFromFile(PAWN2_PATH)) log_warning("Failed to load board texture");  
        if (!BUTTON1_TEXTURE->loadFromFile(BUTTON1_PATH)) log_warning("Failed to load enemy texture");  
        if (!BACKGROUNDBIG_TEXTURE->loadFromFile(BACKGROUNDBIG_PATH)) log_warning("Failed to load background big texture");
        if (!BACKGROUNDBIGFINAL_TEXTURE->loadFromFile(BACKGROUNDBIGFINAL_PATH)) log_warning("Failed to load background big final texture");
        if (!BACKGROUNDBIGHALF_TEXTURE->loadFromFile(BACKGROUNDBIGHALF_PATH)) log_warning("Failed to load background big half texture");
        if (!STICK_TEXTURE->loadFromFile(STICK_PATH)) log_warning("Failed to load stick texture");
        if (!BOARDTILES_TEXTURE->loadFromFile(BOARDTILES_PATH)) log_warning("Failed to load board tiles texture");

        // music
        if (!BACKGROUNDMUSIC_MUSIC->openFromFile(BACKGROUNDMUSIC_PATH)) log_warning("Failed to load background music");

        // sounds
        if (!BUTTONCLICK_SOUNDBUFF->loadFromFile(BUTTONCLICKSOUND_PATH)) log_warning("Failed to load button click sound");
        
        // font
        if (!TEXT_FONT->loadFromFile(TEXT_PATH)) log_warning("Failed to load text font");
    }

    void makeRectsAndBitmasks(){
        SPRITE1_ANIMATIONRECTS.reserve(SPRITE1_INDEXMAX); 
        SPRITE1_ANIMATIONRECTS.emplace_back(sf::IntRect{0, 0, 31, 31});
        SPRITE1_BITMASK.reserve(SPRITE1_INDEXMAX); 
        // make bitmasks for tiles 
        for (const auto& rect : SPRITE1_ANIMATIONRECTS ) {
            SPRITE1_BITMASK.emplace_back(createBitmask(SPRITE1_TEXTURE, rect, 0));
        }
        SPRITE2_ANIMATIONRECTS.reserve(SPRITE2_INDEXMAX); 
        SPRITE2_ANIMATIONRECTS.emplace_back(sf::IntRect{0, 0, 31, 31});
        SPRITE2_BITMASK.reserve(SPRITE2_INDEXMAX); 
        // make bitmasks for tiles 
        for (const auto& rect : SPRITE2_ANIMATIONRECTS ) {
            SPRITE2_BITMASK.emplace_back(createBitmask(SPRITE2_TEXTURE, rect, 0));
        }

        BUTTON1_ANIMATIONRECTS.reserve(BUTTON1_INDEXMAX);
        for (int row = 0; row < BUTTON1_ANIMATIONROWS; ++row) {
            for (int col = 0; col < BUTTON1_INDEXMAX / BUTTON1_ANIMATIONROWS; ++col) {
                BUTTON1_ANIMATIONRECTS.emplace_back(sf::IntRect{col * 96, row * 38, 96, 38});
            }
        }
        BUTTON1_BITMASK.reserve(BUTTON1_INDEXMAX); 
        // make bitmasks for tiles 
        for (const auto& rect : BUTTON1_ANIMATIONRECTS ) {
            BUTTON1_BITMASK.emplace_back(createBitmask(BUTTON1_TEXTURE, rect));
        }

        BOARDTILES_RECTS[P1_GOAL_TILE_INDEX] = BOARDTILES_RECTS[P2_GOAL_TILE_INDEX] = sf::IntRect{0, 0, 46, 33}; // p1,p2 goal block
        BOARDTILES_RECTS[PATH_TILE_INDEX] = sf::IntRect{0, 33, 33, 33}; // path block
        BOARDTILES_RECTS[WALL_TILEX_INDEX] = sf::IntRect{0, 66, 33, 9}; // stick horizontal
        BOARDTILES_RECTS[WALL_TILEY_INDEX] = sf::IntRect{0, 75, 9, 33}; // stick vertical
        BOARDTILES_RECTS[BLANKWALL_TILE_INDEX] = sf::IntRect{0, 75, 9, 9}; // single space between stick and path
        BOARDTILES_RECTS[BLANKP1_INDEX] = BOARDTILES_RECTS[BLANKP2_INDEX] = sf::IntRect{0, 0, 46, 9}; 
        BOARDTILES_RECTS[WALL_INDEX] = sf::IntRect{0, 66, 11, 33}; // wall right and left long
        BOARDTILES_RECTS[WALLBLANK_INDEX] = sf::IntRect{0, 66, 11, 9}; // wall right and left short blank block
        BOARDTILES_RECTS[WALLTOP_INDEX] = sf::IntRect{0, 66, 23, 9}; // wall top and down

        for (size_t i = 0; i < BOARDTILES_RECTS.size(); ++i) {
            BOARDTILES_BITMASK[i] = createBitmask(BOARDTILES_TEXTURE, BOARDTILES_RECTS[i]);
        }

        log_info("\tConstants initialized");
    }

    std::shared_ptr<sf::Uint8[]> createBitmask( const std::shared_ptr<sf::Texture>& texture, const sf::IntRect& rect, const float transparency) {
        if (!texture) {
            log_warning("\tfailed to create bitmask ( texture is empty )");
            return nullptr;
        }

        // Ensure the rect is within the bounds of the texture
        sf::Vector2u textureSize = texture->getSize();
        if (rect.left < 0 || rect.top < 0 || 
            rect.left + rect.width > static_cast<int>(textureSize.x) || 
            rect.top + rect.height > static_cast<int>(textureSize.y)) {
            log_warning("\tfailed to create bitmask ( rect is out of bounds)");
            return nullptr;
        }

        sf::Image image = texture->copyToImage();
        unsigned int width = rect.width;
        unsigned int height = rect.height;

        unsigned int bitmaskSize = (width * height) / 8 + ((width * height) % 8 != 0); // rounding up
        std::shared_ptr<sf::Uint8[]> bitmask(new sf::Uint8[bitmaskSize](), std::default_delete<sf::Uint8[]>());

        for (unsigned int y = 0; y < height; ++y) {
            for (unsigned int x = 0; x < width; ++x) {
                sf::Color pixelColor = image.getPixel(rect.left + x, rect.top + y);
                unsigned int bitIndex = y * width + x;
                unsigned int byteIndex = bitIndex / 8;
                unsigned int bitPosition = bitIndex % 8;

                // Use transparency threshold if provided, otherwise default to alpha > 128
                if ((transparency > 0.0f && pixelColor.a >= static_cast<sf::Uint8>(transparency * 255)) || 
                    (transparency <= 0.0f && pixelColor.a > 128)) {
                    bitmask[byteIndex] |= (1 << bitPosition);
                }
            }
        }
        return bitmask;
    }

    std::shared_ptr<sf::Uint8[]> createBitmaskForBottom(const std::shared_ptr<sf::Texture>& texture, const sf::IntRect& rect, const float transparency, int rows) {
        if (!texture) {
            log_warning("\tfailed to create bitmask ( texture is empty )");
            return nullptr;
        }

        // Ensure the rect is within the bounds of the texture
        sf::Vector2u textureSize = texture->getSize();
        if (rect.left < 0 || rect.top < 0 || 
            rect.left + rect.width > static_cast<int>(textureSize.x) || 
            rect.top + rect.height > static_cast<int>(textureSize.y)) {
            log_warning("\tfailed to create bitmask ( rect is out of bounds)");
            return nullptr;
        }

        sf::Image image = texture->copyToImage();
        unsigned int width = rect.width;
        unsigned int height = rect.height;

        unsigned int bitmaskSize = (width * height) / 8 + ((width * height) % 8 != 0); // rounding up
        std::shared_ptr<sf::Uint8[]> bitmask(new sf::Uint8[bitmaskSize](), std::default_delete<sf::Uint8[]>());

        // Start processing only the last selected rows of the rectangle
        unsigned int startRow = (height >= rows) ? height - rows : 0;

        for (unsigned int y = startRow; y < height; ++y) {
            for (unsigned int x = 0; x < width; ++x) {
                sf::Color pixelColor = image.getPixel(rect.left + x, rect.top + y);
                unsigned int bitIndex = y * width + x;
                unsigned int byteIndex = bitIndex / 8;
                unsigned int bitPosition = bitIndex % 8;

                // Use transparency threshold if provided, otherwise default to alpha > 128
                if ((transparency > 0.0f && pixelColor.a >= static_cast<sf::Uint8>(transparency * 255)) || 
                    (transparency <= 0.0f && pixelColor.a > 128)) {
                    bitmask[byteIndex] |= (1 << bitPosition);
                }
            }
        }

        return bitmask;
    }
    void printBitmaskDebug(const std::shared_ptr<sf::Uint8[]>& bitmask, unsigned int width, unsigned int height) {
        std::stringstream bitmaskStream;

        for (unsigned int y = 0; y < height; ++y) {
            for (unsigned int x = 0; x < width; ++x) {
                unsigned int bitIndex = y * width + x;
                unsigned int byteIndex = bitIndex / 8;
                int bitPosition = 7 - (bitIndex % 8); // High bit to low bit order

                if (bitmask[byteIndex] & (1 << bitPosition)) {
                    bitmaskStream << '1';
                } else {
                    bitmaskStream << '0';
                }
            }
            bitmaskStream << std::endl; // Move to the next row
        }
        log_info(bitmaskStream.str());
    }

}

