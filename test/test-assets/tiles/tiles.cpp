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

BoardTileMap::BoardTileMap(std::array<std::shared_ptr<Tile>, 11> tileTypesArr) {
    // tileTypesArr[0] = wall x tile (33x9)
    // tileTypesArr[1] = wall y tile (9x33)
    // tileTypesArr[2] = path tile (33x33)
    // tileTypesArr[3] = goal tile for p1 (46x33)
    // tileTypesArr[4] = goal tile for p2 (46x33)
    // tileTypesArr[5] = blank tile (9x9)
    // tileTypesArr[6] = middle bit of even and odd rows starting (46x9)
    // tileTypesArr[7] = middle bit of even and odd rows ending (46x9)
    // tileTypesArr[8] = long part of border (11x33)
    // tileTypesArr[9] = short part of border (11x9)
    // tileTypesArr[10] = top and bottom of border (23x9)
    
    this->tileTypesArr = tileTypesArr;
    
    // Store tile sizes
    wallTileXSize = sf::Vector2i{33, 9};   // horizontal wall
    wallTileYSize = sf::Vector2i{9, 33};   // vertical wall
    pathTileSize = sf::Vector2i{33, 33};   // path tile
    goalTileSize = sf::Vector2i{46, 33};   // goal tiles
    blankWallTileSize = sf::Vector2i{9, 9}; // blank tile
    blankp1TileSize = sf::Vector2i{46, 9};
    blankp2TileSize = sf::Vector2i{46, 9};
    
    // Border tile sizes
    sf::Vector2i borderLongSize = sf::Vector2i{11, 33};    // long part of border
    sf::Vector2i borderShortSize = sf::Vector2i{11, 9};    // short part of border
    sf::Vector2i borderTopBottomSize = sf::Vector2i{23, 9}; // top and bottom of border
    
    float currentY = -10.0f; // Start 10 pixels higher
    
    // Initialize the 23x21 board with border (21 rows + 2 border rows, 19 cols + 2 border cols)
    for(int row = 0; row < 23; ++row) {
        int rowStart = row * 21;
        float currentX = -15.0f; // Start pixels to the left
        float maxRowHeight = 0.0f; // Track the tallest tile in this row
        
        for(int col = 0; col < 21; ++col) {
            std::shared_ptr<Tile> selectedTile;
            sf::Vector2i tileSize;
            
            // Top border row (row 0)
            if(row == 0) {
                selectedTile = tileTypesArr[10]; // top border
                tileSize = borderTopBottomSize;
            }
            // Bottom border row (row 22)
            else if(row == 22) {
                selectedTile = tileTypesArr[10]; // bottom border
                tileSize = borderTopBottomSize;
            }
            // Left border column (col 0)
            else if(col == 0) {
                if((row - 1) % 2 == 0) { // Even rows in original grid (accounting for top border offset)
                    selectedTile = tileTypesArr[8]; // long part of border
                    tileSize = borderLongSize;
                } else { // Odd rows in original grid
                    selectedTile = tileTypesArr[9]; // short part of border
                    tileSize = borderShortSize;
                }
            }
            // Right border column (col 20)
            else if(col == 20) {
                if((row - 1) % 2 == 0) { // Even rows in original grid (accounting for top border offset)
                    selectedTile = tileTypesArr[8]; // long part of border
                    tileSize = borderLongSize;
                } else { // Odd rows in original grid
                    selectedTile = tileTypesArr[9]; // short part of border
                    tileSize = borderShortSize;
                }
            }
            // Interior tiles (original 21x19 grid logic, but offset by 1 row and 1 col)
            else {
                int originalRow = row - 1; // Offset for top border
                int originalCol = col - 1; // Offset for left border
                
                if(originalRow % 2 == 0) {
                    if(originalCol == 0) {
                        selectedTile = tileTypesArr[3]; // goal tile p1
                        tileSize = goalTileSize;
                    }
                    else if(originalCol == 18) {
                        selectedTile = tileTypesArr[4]; // goal tile p2
                        tileSize = goalTileSize;
                    }
                    else if(originalCol == 1) {
                        selectedTile = tileTypesArr[2]; // path tile
                        tileSize = pathTileSize;
                    }
                    else {
                        if(originalCol % 2 == 0) {
                            selectedTile = tileTypesArr[1];
                            tileSize = wallTileYSize;
                        } else {
                            selectedTile = tileTypesArr[2]; // path tile
                            tileSize = pathTileSize;
                        }
                    }
                }
                else {
                    if(originalCol == 0) {
                        selectedTile = tileTypesArr[6]; // start piece
                        tileSize = blankp1TileSize;
                    }
                    else if(originalCol == 18) {
                        selectedTile = tileTypesArr[7]; // end piece
                        tileSize = blankp2TileSize;
                    }
                    else {
                        if(originalCol % 2 == 1) {
                            selectedTile = tileTypesArr[0];
                            tileSize = wallTileXSize;
                        } else {
                            selectedTile = tileTypesArr[5]; // blank tile
                            tileSize = blankWallTileSize;
                        }
                    }
                }
            }
            
            // Clone the tile and set its position
            tiles[rowStart + col] = selectedTile->clone();
            
            // Set the position of the tile sprite
            if (tiles[rowStart + col]) {
                tiles[rowStart + col]->getTileSprite().setPosition(currentX, currentY);
            }
            
            currentX += tileSize.x;
            
            // Track the tallest tile in this row for proper row advancement
            maxRowHeight = std::max(maxRowHeight, static_cast<float>(tileSize.y));
        }
        
        // Move to next row position using the actual tallest tile in this row
        currentY += maxRowHeight;
    }
    log_info("Boardtilemap initialized"); 
}

void TileMap::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    for (const auto& tile : tiles) {
        if (tile && tile->getVisibleState()) target.draw(tile->getTileSprite(), states);
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

std::shared_ptr<Tile>& BoardTileMap::getTile(size_t index) {
    if (index < tiles.size()) {
        return tiles[index]; // Return the tile at the specified index
    } else {
        // Handle the case where the index is out of bounds, throw an exception or return a nullptr
        throw std::out_of_range("Index is out of range in getTile");
    }
}

void BoardTileMap::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    for (const auto& tile : tiles) {
        if (tile && tile->getVisibleState()) target.draw(tile->getTileSprite(), states);
    }
}

size_t BoardTileMap::getTileIndex(sf::Vector2i position) {
    return getTileIndex(sf::Vector2f(static_cast<float>(position.x), static_cast<float>(position.y)));
}

size_t BoardTileMap::getTileIndex(sf::Vector2f position) {
    // Starting position offset (same as in constructor)
    float startX = -15.0f;
    float startY = -10.0f;
    
    // Adjust position relative to the tilemap's starting position
    float relativeX = position.x - startX;
    float relativeY = position.y - startY;
    
    // If position is before the tilemap starts, return invalid index
    if (relativeX < 0 || relativeY < 0) {
        throw std::out_of_range("Position is before tilemap bounds");
    }
    
    // Find the row by iterating through row heights
    float currentY = 0.0f;
    int targetRow = -1;
    
    for (int row = 0; row < 23; ++row) {
        float maxRowHeight = 0.0f;
        
        // Calculate the tallest tile height in this row
        for (int col = 0; col < 21; ++col) {
            sf::Vector2i tileSize;
            
            // Determine tile size based on position (same logic as constructor)
            if (row == 0 || row == 22) { // Top/bottom border
                tileSize = sf::Vector2i{23, 9}; // borderTopBottomSize
            }
            else if (col == 0 || col == 20) { // Left/right border
                if ((row - 1) % 2 == 0) {
                    tileSize = sf::Vector2i{11, 33}; // borderLongSize
                } else {
                    tileSize = sf::Vector2i{11, 9}; // borderShortSize
                }
            }
            else { // Interior tiles
                int originalRow = row - 1;
                int originalCol = col - 1;
                
                if (originalRow % 2 == 0) {
                    if (originalCol == 0 || originalCol == 18) {
                        tileSize = goalTileSize; // sf::Vector2i{46, 33}
                    }
                    else if (originalCol == 1) {
                        tileSize = pathTileSize; // sf::Vector2i{33, 33}
                    }
                    else {
                        if (originalCol % 2 == 0) {
                            tileSize = wallTileYSize; // sf::Vector2i{9, 33}
                        } else {
                            tileSize = pathTileSize; // sf::Vector2i{33, 33}
                        }
                    }
                }
                else {
                    if (originalCol == 0) {
                        tileSize = blankp1TileSize; // sf::Vector2i{46, 9}
                    }
                    else if (originalCol == 18) {
                        tileSize = blankp2TileSize; // sf::Vector2i{46, 9}
                    }
                    else {
                        if (originalCol % 2 == 1) {
                            tileSize = wallTileXSize; // sf::Vector2i{33, 9}
                        } else {
                            tileSize = blankWallTileSize; // sf::Vector2i{9, 9}
                        }
                    }
                }
            }
            
            maxRowHeight = std::max(maxRowHeight, static_cast<float>(tileSize.y));
        }
        
        // Check if the Y position falls within this row
        if (relativeY >= currentY && relativeY < currentY + maxRowHeight) {
            targetRow = row;
            break;
        }
        
        currentY += maxRowHeight;
    }
    
    // If no row was found, position is beyond tilemap bounds
    if (targetRow == -1) {
        throw std::out_of_range("Position is beyond tilemap bounds");
    }
    
    // Now find the column within the target row
    float currentX = 0.0f;
    int targetCol = -1;
    
    for (int col = 0; col < 21; ++col) {
        sf::Vector2i tileSize;
        
        // Determine tile size for this specific tile (same logic as above)
        if (targetRow == 0 || targetRow == 22) { // Top/bottom border
            tileSize = sf::Vector2i{23, 9};
        }
        else if (col == 0 || col == 20) { // Left/right border
            if ((targetRow - 1) % 2 == 0) {
                tileSize = sf::Vector2i{11, 33};
            } else {
                tileSize = sf::Vector2i{11, 9};
            }
        }
        else { // Interior tiles
            int originalRow = targetRow - 1;
            int originalCol = col - 1;
            
            if (originalRow % 2 == 0) {
                if (originalCol == 0 || originalCol == 18) {
                    tileSize = goalTileSize;
                }
                else if (originalCol == 1) {
                    tileSize = pathTileSize;
                }
                else {
                    if (originalCol % 2 == 0) {
                        tileSize = wallTileYSize;
                    } else {
                        tileSize = pathTileSize;
                    }
                }
            }
            else {
                if (originalCol == 0) {
                    tileSize = blankp1TileSize;
                }
                else if (originalCol == 18) {
                    tileSize = blankp2TileSize;
                }
                else {
                    if (originalCol % 2 == 1) {
                        tileSize = wallTileXSize;
                    } else {
                        tileSize = blankWallTileSize;
                    }
                }
            }
        }
        
        // Check if the X position falls within this column
        if (relativeX >= currentX && relativeX < currentX + tileSize.x) {
            targetCol = col;
            break;
        }
        
        currentX += tileSize.x;
    }
    
    // If no column was found, position is beyond tilemap bounds
    if (targetCol == -1) {
        throw std::out_of_range("Position is beyond tilemap bounds");
    }
    
    // Calculate and return the tile index
    size_t index = targetRow * 21 + targetCol;
    
    // Ensure index is within valid range
    if (index >= tiles.size()) {
        throw std::out_of_range("Calculated tile index is out of bounds");
    }
    
    return index;
}

bool BoardTileMap::isGreyTile(size_t index) {
    // Check if index is valid
    if (index >= tiles.size()) {
        throw std::out_of_range("Tile index out of bounds in isGreyTile");
    }
    
    // Calculate row and column from index
    int row = index / 21;
    int col = index % 21;
    
    // Border tiles are considered grey (non-playable)
    if (row == 0 || row == 22 || col == 0 || col == 20) {
        return true;
    }
    
    // Interior tiles (original 21x19 grid logic, offset by 1)
    int originalRow = row - 1;
    int originalCol = col - 1;
    
    if (originalRow % 2 == 0) { // Even rows
        // Goal tiles (p1 and p2) are not grey - they're playable
        if (originalCol == 0 || originalCol == 18) {
            return false; // Goal tiles
        }
        // Path tiles are not grey - they're playable
        else if (originalCol == 1) {
            return false; // Path tile
        }
        else {
            if (originalCol % 2 == 0) {
                return true; // Wall Y tiles are grey
            } else {
                return false; // Path tiles are not grey
            }
        }
    }
    else { // Odd rows
        // Start and end pieces are grey (non-playable decorative tiles)
        if (originalCol == 0 || originalCol == 18) {
            return true; // Start/end pieces
        }
        else {
            if (originalCol % 2 == 1) {
                return true; // Wall X tiles are grey
            } else {
                return false; // Blank tiles (9x9) are NOT grey anymore
            }
        }
    }
}
bool BoardTileMap::isVerticalWallTile(size_t index) {
    // Check if index is valid
    if (index >= tiles.size()) {
        return false;
    }
    
    // Calculate row and column from index
    int row = index / 21;
    int col = index % 21;
    
    // Only interior tiles can be vertical walls
    if (row == 0 || row == 22 || col == 0 || col == 20) {
        return false;
    }
    
    // Interior tiles (original 21x19 grid logic, offset by 1)
    int originalRow = row - 1;
    int originalCol = col - 1;
    
    // Vertical wall tiles (9x33) only exist in even rows at even columns (excluding special cases)
    if (originalRow % 2 == 0) {
        // Skip goal tiles and first path tile
        if (originalCol == 0 || originalCol == 18 || originalCol == 1) {
            return false;
        }
        // Even columns in even rows are vertical wall tiles (9x33)
        if (originalCol % 2 == 0) {
            return true; // This is a vertical wall tile (wallTileYSize = 9x33)
        }
    }
    
    return false;
}