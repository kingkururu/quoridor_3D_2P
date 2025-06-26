#include "tiles.hpp"

Tile::Tile(sf::Vector2f scale, std::weak_ptr<sf::Texture> texture, sf::IntRect textureRect, 
           std::weak_ptr<sf::Uint8[]> bitmask, bool walkableState)
    : scale(scale), texture(texture), textureRect(textureRect), bitmask(bitmask), walkableState(walkableState) {
    
    try {
        tileSprite = std::make_unique<sf::Sprite>(); // Use unique_ptr for tileSprite

        if (auto sharedTexture = texture.lock()) {
            sf::Vector2u textureSize = sharedTexture->getSize(); 
            if (textureSize.x == 0 || textureSize.y == 0) {
                throw std::runtime_error("Loaded tile texture has size 0");
            }

            tileSprite->setTexture(*sharedTexture); // Set the texture
            tileSprite->setScale(scale); // Set the scale
            tileSprite->setTextureRect(textureRect); // Set the texture rectangle
            visibleState = true;

        } else {
            throw std::runtime_error("Tile texture is not available");
        }
    } catch (const std::exception& e) {
        log_error(e.what()); // Log any exceptions that occur
    }
}

// Custom copy constructor
Tile::Tile(const Tile& other)
    : position(other.position), scale(other.scale),
      texture(other.texture), textureRect(other.textureRect),
      bitmask(other.bitmask), walkableState(other.walkableState) {

    // Create a new sprite with the same texture and scale
    tileSprite = std::make_unique<sf::Sprite>();

    // Check if the texture is still valid
    if (auto texturePtr = other.texture.lock()) {
        // Set the texture and texture rectangle for the new sprite
        tileSprite->setTexture(*texturePtr);
        tileSprite->setTextureRect(other.textureRect);
        tileSprite->setScale(scale); // Apply the scale
        tileSprite->setPosition(other.tileSprite->getPosition()); // Copy the position if needed
        visibleState = true;
    } else {
        throw std::runtime_error("Texture for copied tile is not available");
    }
}
 
TileMap::TileMap(std::shared_ptr<Tile>* tileTypesArray, unsigned int tileTypesNumber, size_t tileMapWidth, size_t tileMapHeight, float tileWidth, float tileHeight, std::filesystem::path filePath, sf::Vector2f tileMapPosition) 
    : tileTypesNumber(tileTypesNumber), tileMapWidth(tileMapWidth), tileMapHeight(tileMapHeight), tileWidth(tileWidth), tileHeight(tileHeight), tileMapPosition(tileMapPosition) {

    try{
        tiles.reserve( tileMapWidth * tileMapHeight ); 

        std::ifstream fileStream(filePath);
        
        if (!fileStream.is_open()) {
            throw std::runtime_error("Unable to open file: " + filePath.string());
        }

        std::string line;
        unsigned int currentY = 0; // Track the current row

        while (std::getline(fileStream, line) && currentY < tileMapHeight) {
            std::istringstream lineStream(line);
            std::string tileIndexStr;
            unsigned int currentX = 0; // Track the current column

            while (lineStream >> tileIndexStr && currentX < tileMapWidth) {
                unsigned int tileIndex = std::stoul(tileIndexStr); // Convert to unsigned int
                
                if (tileIndex < tileTypesNumber) {
                    auto tile = tileTypesArray[tileIndex]->clone();
                    tile->getTileSprite().setPosition(tileMapPosition.x + currentX * tileWidth, tileMapPosition.y + currentY * tileHeight); 
                    tiles.emplace_back(std::move(tile));
                    
                } else {
                    throw std::out_of_range("Tile index out of bounds: " + std::to_string(tileIndex));
                }
                currentX++; // Increment column index
            } 
            currentY++; // Increment row index
        }

        fileStream.close();

        log_info("Tile map initialized successfully");
    } catch (const std::exception& e) {
        log_warning("Error in making tilemap: " + std::string(e.what()));
    }
}

BoardTileMap::BoardTileMap(std::array<std::shared_ptr<Tile>, 8> tileTypesArr) {
    // tileTypesArr[0] = wall x tile (33x9)
    // tileTypesArr[1] = wall y tile (9x33)
    // tileTypesArr[2] = path tile (33x33)
    // tileTypesArr[3] = goal tile for p1 (46x33)
    // tileTypesArr[4] = goal tile for p2 (46x33)
    // tileTypesArr[5] = blank tile (9x9)
    // tileTypesArr[6] = middle bit of even and odd rows starting (46x9)
    // tileTypesArr[7] = middle bit of even and odd rows ending (46x9)
    
    this->tileTypesArr = tileTypesArr;
    
    // Store tile sizes
    wallTileXSize = sf::Vector2i{33, 9};   // horizontal wall
    wallTileYSize = sf::Vector2i{9, 33};   // vertical wall
    pathTileSize = sf::Vector2i{33, 33};   // path tile
    goalTileSize = sf::Vector2i{46, 33};   // goal tiles
    blankWallTileSize = sf::Vector2i{9, 9}; // blank tile
    blankp1TileSize = sf::Vector2i{46, 9}; 
    blankp2TileSize = sf::Vector2i{46, 9}; 

    // DEBUG: Print all tile sizes
    std::cout << "\n=== TILE SIZES DEBUG ===";
    std::cout << "\nwallTileXSize (horizontal wall): " << wallTileXSize.x << "x" << wallTileXSize.y;
    std::cout << "\nwallTileYSize (vertical wall): " << wallTileYSize.x << "x" << wallTileYSize.y;
    std::cout << "\npathTileSize: " << pathTileSize.x << "x" << pathTileSize.y;
    std::cout << "\ngoalTileSize: " << goalTileSize.x << "x" << goalTileSize.y;
    std::cout << "\nblankWallTileSize: " << blankWallTileSize.x << "x" << blankWallTileSize.y;
    std::cout << "\nblankp1TileSize: " << blankp1TileSize.x << "x" << blankp1TileSize.y;
    std::cout << "\nblankp2TileSize: " << blankp2TileSize.x << "x" << blankp2TileSize.y;
    std::cout << "\n========================\n";

    float currentY = 0.0f;
    
    // Initialize the 21x19 board with the specified pattern    
    for(int row = 0; row < 21; ++row) {
        int rowStart = row * 19;
        float currentX = 0.0f;
        float maxRowHeight = 0.0f; // Track the tallest tile in this row
        
        std::cout << "\n--- ROW " << row << " (" << (row % 2 == 0 ? "EVEN" : "ODD") << ") ---";
                
        for(int col = 0; col < 19; ++col) {
            int tileIndex = -1;
            std::shared_ptr<Tile> selectedTile;
            sf::Vector2i tileSize;
            std::string tileTypeName;
            
            if(row % 2 == 0) {
                if(col == 0) {
                    selectedTile = tileTypesArr[3]; // goal tile p1
                    tileSize = goalTileSize;
                    tileIndex = 3;
                    tileTypeName = "goal_p1";
                }
                else if(col == 18) {
                    selectedTile = tileTypesArr[4]; // goal tile p2
                    tileSize = goalTileSize;
                    tileIndex = 4;
                    tileTypeName = "goal_p2";
                }
                else if(col == 1) {
                    selectedTile = tileTypesArr[2]; // path tile
                    tileSize = pathTileSize;
                    tileIndex = 2;
                    tileTypeName = "path";
                }
                else {
                    if(col % 2 == 0) {
                        selectedTile = tileTypesArr[1]; 
                        tileSize = wallTileYSize;
                        tileIndex = 1;
                        tileTypeName = "wall_vertical";
                    } else {
                        selectedTile = tileTypesArr[2]; // path tile
                        tileSize = pathTileSize;
                        tileIndex = 2;
                        tileTypeName = "path";
                    }
                }
            }
            else {
                if(col == 0) {
                    selectedTile = tileTypesArr[6]; // start piece
                    tileSize = blankp1TileSize;
                    tileIndex = 6;
                    tileTypeName = "blank_p1";
                }
                else if(col == 18) {
                    selectedTile = tileTypesArr[7]; // end piece
                    tileSize = blankp2TileSize;
                    tileIndex = 7;
                    tileTypeName = "blank_p2";
                }
                else {
                    if(col % 2 == 1) {
                        selectedTile = tileTypesArr[0]; 
                        tileSize = wallTileXSize;
                        tileIndex = 0;
                        tileTypeName = "wall_horizontal";
                    } else {
                        selectedTile = tileTypesArr[5]; // blank tile
                        tileSize = blankWallTileSize;
                        tileIndex = 5;
                        tileTypeName = "blank";
                    }
                }
            }
            
            // DEBUG: Print tile selection details
            std::cout << "\n  Col " << col << ": " << tileTypeName << " (idx=" << tileIndex 
                      << ") expectedSize=" << tileSize.x << "x" << tileSize.y;
            
            // Clone the tile and set its position
            tiles[rowStart + col] = selectedTile->clone(); 

            // Set the position of the tile sprite
            if (tiles[rowStart + col]) {
                tiles[rowStart + col]->getTileSprite().setPosition(currentX, currentY);

                // DEBUG: Comprehensive tile positioning info
                sf::Vector2f actualPos = tiles[rowStart + col]->getTileSprite().getPosition();
                sf::IntRect texRect = tiles[rowStart + col]->getTileSprite().getTextureRect();
                bool isVisible = tiles[rowStart + col]->getVisibleState();
                
                std::cout << "\n    -> Positioned at (" << currentX << ", " << currentY << ")";
                std::cout << "\n    -> Actual sprite pos: (" << actualPos.x << ", " << actualPos.y << ")";
                std::cout << "\n    -> Texture rect: (" << texRect.left << "," << texRect.top 
                          << "," << texRect.width << "," << texRect.height << ")";
                std::cout << "\n    -> Visible: " << (isVisible ? "YES" : "NO");
                std::cout << "\n    -> Using size: " << tileSize.x << "x" << tileSize.y 
                          << " (texture size: " << texRect.width << "x" << texRect.height << ")";
                          
                // Check if our expected size matches texture size
                if(tileSize.x != texRect.width || tileSize.y != texRect.height) {
                    std::cout << "\n    -> WARNING: Expected size doesn't match texture size!";
                }
            } else {
                std::cout << "\n    -> ERROR: Tile is NULL!";
            }
            
            currentX += tileSize.x;
            
            // Track the tallest tile in this row for proper row advancement
            maxRowHeight = std::max(maxRowHeight, static_cast<float>(tileSize.y));
        }
        
        // DEBUG: Row completion info
        std::cout << "\n  Row " << row << " complete. Total width: " << currentX 
                  << ", Max height: " << maxRowHeight << ", Moving Y from " << currentY 
                  << " to " << (currentY + maxRowHeight);
        
        // Move to next row position using the actual tallest tile in this row
        currentY += maxRowHeight;
    }    
    
    std::cout << "\n\n=== BOARD COMPLETE ===";
    std::cout << "\n======================\n";
    
    log_info("BoardTileMap initialized with 21x19 grid at proper positions");
}

void TileMap::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    for (const auto& tile : tiles) {
        if (tile && tile->getVisibleState()) {
            target.draw(tile->getTileSprite(), states);
        }
    }
}

// Add a tile to the map at the specified grid position (x, y)
void TileMap::addTile(unsigned int x, unsigned int y, std::unique_ptr<Tile> tile) {
    try{
        if (x >= tileMapWidth || y >= tileMapHeight) {
            throw std::out_of_range("Tile position out of bounds: (" + std::to_string(x) + ", " + std::to_string(y) + ")");
        }

        // Calculate the index in the tiles vector
        unsigned int index = y * tileMapWidth + x;

        // Ensure the tile is valid (not nullptr)
        if (!tile) {
            throw std::runtime_error("Attempted to add a null tile.");
        }

        // Move the tile into the tiles vector at the calculated index
        tiles[index] = std::move(tile);

        // Optionally set the position of the tile if the Tile class has a method for that
        tiles[index]->getTileSprite().setPosition(tileMapPosition.x + x * tileWidth, tileMapPosition.y + y * tileHeight);
    } catch (const std::exception& e) {
        log_error(e.what()); // Log any exceptions that occur
    }
}

std::unique_ptr<Tile>& TileMap::getTile(size_t index) {
    if (index < tiles.size()) {
        return tiles[index]; // Return the tile at the specified index
    } else {
        // Handle the case where the index is out of bounds, throw an exception or return a nullptr
        throw std::out_of_range("Index is out of range in getTile");
    }
}

void BoardTileMap::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    for (const auto& tile : tiles) {
        if (tile && tile->getVisibleState()) {
            target.draw(tile->getTileSprite(), states);
        }
    }
}
