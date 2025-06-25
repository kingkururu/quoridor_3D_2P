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

BoardTileMap::BoardTileMap(std::array<std::shared_ptr<Tile>, 6> tileTypesArr) {
    // tileTypesArr[0] = wall x tile (36x9)
    // tileTypesArr[1] = wall y tile (9x36)
    // tileTypesArr[2] = path tile (33x33)
    // tileTypesArr[3] = goal tile for p1 (46x33)
    // tileTypesArr[4] = goal tile for p2 (46x33)
    // tileTypesArr[5] = blank tile (9x9)
    
    this->tileTypesArr = tileTypesArr;
    
    // Store tile sizes
    wallTileXSize = sf::Vector2i{36, 9};   // horizontal wall
    wallTileYSize = sf::Vector2i{9, 36};   // vertical wall
    pathTileSize = sf::Vector2i{33, 33};   // path tile
    goalTileSize = sf::Vector2i{46, 33};   // goal tiles
    sf::Vector2i blankTileSize = sf::Vector2i{9, 9}; // blank tile
    
    // Pre-calculate row heights based on the tallest tile in each row
    std::vector<float> rowHeights(21);
    for(int row = 0; row < 21; ++row) {
        if(row % 2 == 0) {
            // Even rows have goal tiles (33), path tiles (33), and horizontal walls (9)
            // The tallest tiles are goal and path tiles at 33
            rowHeights[row] = 33.0f;
        } else {
            // Odd rows have goal tiles (33) and vertical walls (36)
            // The tallest tiles are vertical walls at 36
            rowHeights[row] = 36.0f;
        }
    }
    
    float currentY = 0.0f;
    
    // Initialize the 21x19 board with the specified pattern    
    for(int row = 0; row < 21; ++row) {
        int rowStart = row * 19;
        float currentX = 0.0f;
        
        std::cout << std::endl << "Row " << std::setw(2) << row << ": ";
        
        for(int col = 0; col < 19; ++col) {
            int tileIndex = -1;
            std::shared_ptr<Tile> selectedTile;
            sf::Vector2i tileSize;
            
            if(row % 2 == 0) {
                // Even rows: Pattern 3-2-1-2-1-2-1-2-1-2-1-2-1-2-1-2-1-2-4
                if(col == 0) {
                    selectedTile = tileTypesArr[3]; // goal tile p1
                    tileSize = goalTileSize;
                    tileIndex = 3;
                }
                else if(col == 18) {
                    selectedTile = tileTypesArr[4]; // goal tile p2
                    tileSize = goalTileSize;
                    tileIndex = 4;
                }
                else if(col == 1) {
                    selectedTile = tileTypesArr[2]; // path tile
                    tileSize = pathTileSize;
                    tileIndex = 2;
                }
                else {
                    // Alternate between horizontal wall (1) and path (2)
                    if(col % 2 == 0) {
                        selectedTile = tileTypesArr[1]; 
                        tileSize = wallTileXSize;
                        tileIndex = 1;
                    } else {
                        selectedTile = tileTypesArr[2]; // path tile
                        tileSize = pathTileSize;
                        tileIndex = 2;
                    }
                }
            }
            else {
                // Odd rows: Pattern 3-0-5-0-5-0-5-0-5-0-5-0-5-0-5-0-5-0-4
                if(col == 0) {
                    selectedTile = tileTypesArr[3]; // goal tile p1
                    tileSize = goalTileSize;
                    tileIndex = 3;
                }
                else if(col == 18) {
                    selectedTile = tileTypesArr[4]; // goal tile p2
                    tileSize = goalTileSize;
                    tileIndex = 4;
                }
                else {
                    if(col % 2 == 1) {
                        selectedTile = tileTypesArr[0]; 
                        tileSize = wallTileYSize;
                        tileIndex = 0;
                    } else {
                        selectedTile = tileTypesArr[5]; // blank tile
                        tileSize = blankTileSize;
                        tileIndex = 5;
                    }
                }
            }
            
            // Clone the tile and set its position
            tiles[rowStart + col] = selectedTile->clone(); 

            // Set the position of the tile sprite
            if (tiles[rowStart + col]) {
                tiles[rowStart + col]->getTileSprite().setPosition(currentX, currentY);

                // Debug print for tile positioning
                std::cout << "\n  Tile[" << row << "][" << col << "] (type " << tileIndex 
                          << ") positioned at (" << currentX << ", " << currentY 
                          << ") size(" << tileSize.x << "x" << tileSize.y << ")";
                          
                // Verify the position was set correctly
                sf::Vector2f actualPos = tiles[rowStart + col]->getTileSprite().getPosition();
                std::cout << " -> actual pos(" << actualPos.x << ", " << actualPos.y << ")";
                
                // Check if tile is visible
                bool isVisible = tiles[rowStart + col]->getVisibleState();
                std::cout << " visible:" << (isVisible ? "YES" : "NO");
                
                // Check texture rect
                sf::IntRect texRect = tiles[rowStart + col]->getTileSprite().getTextureRect();
                std::cout << " texRect(" << texRect.left << "," << texRect.top << "," 
                          << texRect.width << "," << texRect.height << ")";
            } else {
                std::cout << "\n  ERROR: Tile[" << row << "][" << col << "] is NULL!";
            }
            
            // Move to next column position using THIS tile's width
            currentX += tileSize.x;
            
            std::cout << tileIndex;
            if(col < 18) std::cout << "-";
        }
        
        // Move to next row position using the pre-calculated row height
        currentY += rowHeights[row];
        
        std::cout << " (row height: " << rowHeights[row] << ")" << std::endl;
        std::cout << "  Row " << row << " complete, next row starts at Y=" << currentY << std::endl;
    }    
    
    log_info("BoardTileMap initialized with 21x19 grid at proper positions");
    
    // Calculate total board dimensions
    float totalBoardHeight = currentY;
    
    // Calculate total board width (sum of widths in any row)
    float totalBoardWidth = 0.0f;
    // Even row pattern: 46 + 33 + (8 * (36 + 33)) + 46 = 46 + 33 + 552 + 46 = 677
    // But let's calculate it properly based on the actual pattern
    if(true) { // Calculate from even row pattern
        totalBoardWidth = 46 + 33; // goal + path
        for(int i = 2; i < 18; i++) {
            if(i % 2 == 0) {
                totalBoardWidth += 36; // horizontal wall (but we're using type 1 which is 9x36, so width is 9)
            } else {
                totalBoardWidth += 33; // path
            }
        }
        totalBoardWidth += 46; // final goal
    }
    
    std::cout << "\n=== BOARDTILEMAP DIMENSIONS ===" << std::endl;
    std::cout << "Total board size: " << totalBoardWidth << "x" << totalBoardHeight << std::endl;
    std::cout << "Grid dimensions: 19 columns x 21 rows" << std::endl;
    
    // Print row heights
    std::cout << "Row heights: ";
    for(int i = 0; i < 21; i++) {
        std::cout << rowHeights[i];
        if(i < 20) std::cout << ", ";
    }
    std::cout << std::endl;
    std::cout << "===============================" << std::endl;
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
