#include "physics.hpp"

// physics namespace to have sprites move 
namespace physics {
    Quadtree::Quadtree(float x, float y, float width, float height, size_t level, size_t maxObjects, size_t maxLevels)
        : maxObjects(maxObjects), maxLevels(maxLevels), level(level), bounds(x, y, width, height) {}

    void Quadtree::clear() {
        objects.clear();
        log_info("objects cleared.");
        nodes.clear();
        log_info("Quadtree cleared.");
    }

    std::vector<Sprite*> Quadtree::query(const sf::FloatRect& area) const {
        try {
            std::vector<Sprite*> result;
            if (!bounds.intersects(area)) {
                log_warning("Area does not intersect with the quadtree bounds at level " + std::to_string(level));
                return result;
            }

            for (const auto& obj : objects) {
                if (area.intersects(obj->returnSpritesShape().getGlobalBounds())) {
                    result.push_back(obj);
                    log_info("Sprite added to query result at level " + std::to_string(level));
                }
            }

            for (const auto& node : nodes) {
                auto nodeResult = node->query(area);
                result.insert(result.end(), nodeResult.begin(), nodeResult.end());
            }
            return result;

        } catch (const std::exception& e) {
            log_error("Error during query at level " + std::to_string(level) + ": " + std::string(e.what()));
            return std::vector<Sprite*>();
        }
    }

    bool Quadtree::contains(const sf::FloatRect& bounds) const {
        try {
            bool result = this->bounds.contains(bounds.left, bounds.top) && this->bounds.contains(bounds.left + bounds.width, bounds.top + bounds.height);
            log_info(result ? "Bounds are contained in the quadtree at level " + std::to_string(level) :
                            "Bounds are not contained in the quadtree at level " + std::to_string(level));
            return result;
        } catch (const std::exception& e) {
            log_error("Error during contains check at level " + std::to_string(level) + ": " + std::string(e.what()));
            return false;
        }
    }

    void Quadtree::subdivide() {
        try {
            // Check if we've reached the max level
            if (level >= maxLevels) {
                log_info("Maximum level reached, cannot subdivide further.");
                return;
            }

            float halfWidth = bounds.width / 2;
            float halfHeight = bounds.height / 2;
            float x = bounds.left;
            float y = bounds.top;

            // Create four child nodes with smaller bounds and increment the level
            nodes.push_back(std::make_unique<Quadtree>(x, y, halfWidth, halfHeight, level + 1, maxObjects, maxLevels));
            nodes.push_back(std::make_unique<Quadtree>(x + halfWidth, y, halfWidth, halfHeight, level + 1, maxObjects, maxLevels));
            nodes.push_back(std::make_unique<Quadtree>(x, y + halfHeight, halfWidth, halfHeight, level + 1, maxObjects, maxLevels));
            nodes.push_back(std::make_unique<Quadtree>(x + halfWidth, y + halfHeight, halfWidth, halfHeight, level + 1, maxObjects, maxLevels));

            log_info("Quadtree subdivided into 4 child nodes at level " + std::to_string(level));

            // Redistribute the objects into the appropriate child nodes
            for (auto it = objects.begin(); it != objects.end(); ) {
                bool inserted = false;
                for (auto& node : nodes) {
                    if (node->bounds.intersects((*it)->returnSpritesShape().getGlobalBounds())) {
                        node->objects.push_back(*it);
                        it = objects.erase(it); // Remove object from the current node
                        inserted = true;
                        log_info("Sprite moved to child node at level " + std::to_string(node->level));
                        break;
                    }
                }
                if (!inserted) {
                    ++it;
                }
            }
        } catch (const std::exception& e) {
            log_error("Error during subdivision at level " + std::to_string(level) + ": " + std::string(e.what()));
        }
    }

    void Quadtree::update() {
        try {
            for (auto& sprite : objects) {
              //  log_info("Updating quadtree at level " + std::to_string(level));

                if (sprite->getMoveState()) {
                    // Check which node the sprite was in
                    for (auto& node : nodes) {
                        if (node->contains(sprite->returnSpritesShape().getGlobalBounds())) {
                            // Remove sprite from the old node
                            node->objects.erase(std::remove(node->objects.begin(), node->objects.end(), sprite), node->objects.end());
                            log_info("Sprite removed from old node at level " + std::to_string(node->level));
                            break;
                        }
                    }

                    // Insert the sprite back into the quadtree
                    std::unique_ptr<Sprite> spritePtr(sprite);
                    insert(spritePtr);
                    log_info("Sprite updated and inserted into quadtree at level " + std::to_string(level));
                }
            }
        } catch (const std::exception& e) {
            log_error("Error during update at level " + std::to_string(level) + ": " + std::string(e.what()));
        }
    }

    // struct to hold raycast operation results that use vector of sprites
    RaycastResult cachedRaycastResult {}; 

    // falling objects 
    sf::Vector2f freeFall( float speed, sf::Vector2f originalPos){
        return { originalPos.x, originalPos.y += speed * MetaComponents::deltaTime };
    }

    // object moving in a direction vector
    sf::Vector2f followDirVec( float speed, sf::Vector2f originalPos, sf::Vector2f acceleration, const sf::Vector2f& direction){
         sf::Vector2f movement(direction.x * speed * MetaComponents::deltaTime * acceleration.x, direction.y * speed * MetaComponents::deltaTime * acceleration.y);
        return originalPos + movement;
    }
    sf::Vector2f followDirVecOpposite( float speed, sf::Vector2f originalPos, sf::Vector2f acceleration, const sf::Vector2f& direction){
        sf::Vector2f movement(direction.x * speed * MetaComponents::deltaTime * acceleration.x * -1, direction.y * speed * MetaComponents::deltaTime * acceleration.y * -1);
       return originalPos + movement;
   }

    // moving x or y positions based on directions
    sf::Vector2f moveLeft( float speed, sf::Vector2f originalPos, sf::Vector2f acceleration){
        // return { originalPos.x -= speed * MetaComponents::deltaTime * acceleration.x, originalPos.y };
        return { originalPos.x -= speed * MetaComponents::deltaTime, originalPos.y };
    }
    sf::Vector2f moveRight( float speed, sf::Vector2f originalPos, sf::Vector2f acceleration){
        // return { originalPos.x += speed * MetaComponents::deltaTime * acceleration.x, originalPos.y };
        return { originalPos.x += speed * MetaComponents::deltaTime, originalPos.y };
    }
    sf::Vector2f moveUp( float speed, sf::Vector2f originalPos, sf::Vector2f acceleration){
        // return { originalPos.x, originalPos.y -= speed * MetaComponents::deltaTime * acceleration.y};
        return { originalPos.x, originalPos.y -= speed * MetaComponents::deltaTime };
    }
    sf::Vector2f moveDown( float speed, sf::Vector2f originalPos, sf::Vector2f acceleration){
        // return { originalPos.x, originalPos.y += speed * MetaComponents::deltaTime * acceleration.y};
        return { originalPos.x, originalPos.y += speed * MetaComponents::deltaTime};
    }

    sf::Vector2f jump(float& elapsedTime, float speed, sf::Vector2f originalPos, sf::Vector2f acceleration){
        float jumpDuration = 0.8f; 
        if (elapsedTime <= jumpDuration) {   // If elapsedTime is within jump duration
            if (elapsedTime <= jumpDuration / 2) {
                originalPos.y -= speed * MetaComponents::deltaTime * (1.0f - elapsedTime / (jumpDuration / 2.0)) * acceleration.y * gravity;
            }
            else {
                originalPos.y += speed * MetaComponents::deltaTime * (elapsedTime - jumpDuration / 2.0) / ( jumpDuration / 2.0)* acceleration.y * gravity;
            }
            originalPos.x += speed * MetaComponents::deltaTime;
            // log_info("Jump in progress. Elapsed time: {}, Position: ({}, {})" + std::to_string(elapsedTime) + ", "+ std::to_string(originalPos.x) + ", " + std::to_string(originalPos.y));
        } else {
            elapsedTime = 0.0f;
            originalPos.y = std::round(originalPos.y); // Correct minor float inaccuracies

            //log_info("Jump done, "+ std::to_string(originalPos.x) + ", " + std::to_string(originalPos.y));
        }
        return originalPos;
    }

    sf::Vector2f jumpToSurface(float& elapsedTime, float speed, sf::Vector2f originalPos, sf::Vector2f acceleration){
        static sf::Vector2f startPos = originalPos; // Save the start position
        float jumpDuration = 0.4f; 

        // If elapsedTime is within jump duration
        if (elapsedTime <= jumpDuration) {
            // Calculate jump height
            float jumpHeight = speed * acceleration.y * gravity;

            if (elapsedTime <= (jumpDuration / 2.0)) {
                originalPos.y = startPos.y - (jumpHeight * (elapsedTime / (jumpDuration / 2.0)));
            }
            else {
                originalPos.y = startPos.y - jumpHeight + (jumpHeight * ((elapsedTime - (jumpDuration / 2.0)) / (jumpDuration / 2.0)));
            }
            // log_info("Jump in progress. Elapsed time: {}, Position: ({}, {})" + std::to_string(elapsedTime) + ", "+ std::to_string(originalPos.x) + ", " + std::to_string(originalPos.y));
        } else {
            elapsedTime = 0.0f;
            originalPos.y = startPos.y; // Reset to the initial position
            // log_info("Jump done, "+ std::to_string(originalPos.x) + ", " + std::to_string(originalPos.y));

        }
        return originalPos;
    }

    // void navigateMaze(std::unique_ptr<Player>& player, std::unique_ptr<TileMap>& tileMap, std::vector<size_t>& tileVisitCounts, sf::Vector2i& lastDirection) {
    //     if (!player || !tileMap) {
    //         log_error("Tile or player is not initialized");
    //         return;
    //     }
    
    //     // Get the player's current position
    //     sf::Vector2f start = player->getSpritePos();
    //     sf::Vector2f goal = {
    //         tileMap->getTileMapWidth() * tileMap->getTileWidth() - tileMap->getTileWidth(),
    //         tileMap->getTileMapHeight() * tileMap->getTileHeight() - tileMap->getTileHeight()
    //     };
    
    //     // Calculate the player's current tile position
    //     int tileX = static_cast<int>((start.x - Constants::TILEMAP_POSITION.x) / Constants::TILE_WIDTH);
    //     int tileY = static_cast<int>((start.y - Constants::TILEMAP_POSITION.y) / Constants::TILE_HEIGHT);
    
    //     // Directions to try (right, left, down, up)
    //     std::array<sf::Vector2i, 4> directions = {
    //         sf::Vector2i{tileX + 1, tileY},  // Right
    //         sf::Vector2i{tileX - 1, tileY},  // Left
    //         sf::Vector2i{tileX, tileY + 1},  // Down
    //         sf::Vector2i{tileX, tileY - 1}   // Up
    //     };
    
    //     // Variables to keep track of the best next tile
    //     sf::Vector2i nextTile = {tileX, tileY}; // Initialize to current tile
    //     int minSteps = std::numeric_limits<int>::max(); // Initialize to a large value
    
    //     // Loop through directions to find the least visited valid tile
    //     for (auto& dir : directions) {
    //         int newTileX = dir.x;
    //         int newTileY = dir.y;
    
    //         // Check if the tile is out of bounds
    //         if (newTileX < 0 || newTileY < 0 || 
    //             newTileX >= tileMap->getTileMapWidth() || 
    //             newTileY >= tileMap->getTileMapHeight()) {
    //             continue; // Skip out-of-bounds tiles
    //         }
    
    //         // Calculate index for visit count
    //         int index = newTileY * tileMap->getTileMapWidth() + newTileX;
    
    //         if (tileMap->getTile(index)->getWalkable()) {
    //             // Prevent backtracking: Skip if the direction is the reverse of the last direction
    //             if (dir.x == tileX - lastDirection.x && dir.y == tileY - lastDirection.y) {
    //                 continue;
    //             }
    
    //             int steps = tileVisitCounts[index]; // Get visit count for this tile
    //             if (steps < minSteps) {
    //                 minSteps = steps;
    //                 nextTile = {newTileX, newTileY}; // Update to the least visited tile
    //             }
    //         }
    //     }
    
    //     // Determine the direction to move the player based on the next tile
    //     float playerAngle = player->getHeadingAngle();
    //     if (tileX < nextTile.x) {
    //         playerAngle = 0.0f;   // Moving right
    //         physics::spriteMover(player, physics::moveRight);
    //     } else if (tileX > nextTile.x) {
    //         playerAngle = 180.0f; // Moving left
    //         physics::spriteMover(player, physics::moveLeft);
    //     } else if (tileY < nextTile.y) {
    //         playerAngle = 90.0f;  // Moving down
    //         physics::spriteMover(player, physics::moveDown);
    //     } else if (tileY > nextTile.y) {
    //         playerAngle = 270.0f; // Moving up
    //         physics::spriteMover(player, physics::moveUp);
    //     }
    
    //     // Update the player's heading angle after moving
    //     player->setHeadingAngle(playerAngle);
    
    //     // Update the visit count for the newly visited tile
    //     int nextTileIndex = nextTile.y * tileMap->getTileMapWidth() + nextTile.x;
    //     ++tileVisitCounts[nextTileIndex];
    
    //     std::cout << "Player moved to tile (" << nextTile.x << ", " << nextTile.y << ") at index" << nextTileIndex << std::endl;
    //     std::cout << "Player's heading angle: " << playerAngle << std::endl;
    //     std::cout << "tileVisitCounts[" << nextTileIndex << "]: " << tileVisitCounts[nextTileIndex] << std::endl;
    
    //     lastDirection = nextTile;

    //     // Check if the player has reached the goal
    //     start = player->getSpritePos();
    //     if (std::hypot(start.x - goal.x, start.y - goal.y) < Constants::TILE_WIDTH) {
    //         log_info("Player has reached the goal!");
    //     }
    // }     
    
    void calculateRayCast3d(std::unique_ptr<Player>& player, std::unique_ptr<TileMap>& tileMap, sf::VertexArray& lines, sf::VertexArray& wallLine) {
        if(!player || !tileMap){
            log_error("tile or player is not initialized");
            return;
        }

        float startX = player->getSpritePos().x;
        float startY = player->getSpritePos().y;
        float playerAngle = player->getHeadingAngle(); // Player's rotation angle in degrees

        size_t itCount = Constants::RAYS_NUM / 2;
        float screenWidth = static_cast<float>(MetaComponents::bigView.getSize().x);
        float screenHeight = static_cast<float>(MetaComponents::bigView.getSize().y);
        float centerY = screenHeight / 2.0f;

        const float wallHeightScale = 2500.0f;  // Scale factor for wall height
        float angleStep = Constants::FOV / static_cast<float>(itCount);  // Angle step between rays
        const float maxRayDistance = 1000.0f; // Maximum allowed ray distance to prevent infinite loops

        wallLine.clear();
        wallLine.setPrimitiveType(sf::Quads);  // Use quads for filled walls
        lines.clear();
        lines.setPrimitiveType(sf::Lines);
        lines.resize(2 * itCount); // Ensure enough space for ray visualization

        float sliceWidth = screenWidth / static_cast<float>(itCount); // Corrected wall slice width

        for (size_t i = 0; i < itCount; ++i) {
            float angleOffset = (i - itCount / 2.0f) * angleStep;
            float rayAngle = playerAngle + angleOffset;
            float radian = rayAngle * 3.14159f / 180.0f; // Convert to radians

            float dirX = cos(radian);
            float dirY = sin(radian);
            float stepSize = 1.0f;

            float rayX = startX;
            float rayY = startY;
            bool hit = false;
            float rayDistance = 0.0f;

            while (!hit && rayDistance < maxRayDistance) {
                rayX += dirX * stepSize;
                rayY += dirY * stepSize;
                rayDistance += stepSize;

                int tileX = static_cast<int>(rayX) / Constants::TILE_WIDTH;
                int tileY = static_cast<int>(rayY) / Constants::TILE_HEIGHT;

                if (tileX < 0 || tileY < 0 || tileX >= tileMap->getTileMapWidth() || tileY >= tileMap->getTileMapHeight()) break; // Exit if ray goes out of bounds
            
                // Store raycasting lines for debugging (2D representation)
                lines[2 * i].position = sf::Vector2f(startX, startY);
                lines[2 * i + 1].position = sf::Vector2f(rayX, rayY);
                lines[2 * i].color = sf::Color::Red;
                lines[2 * i + 1].color = sf::Color::Red;

                auto& tile = tileMap->getTile(tileY * tileMap->getTileMapWidth() + tileX);

                if (!tile->getWalkable()) { hit = true;

                // Correct fish-eye effect
                float correctedDistance = rayDistance * cos((rayAngle - playerAngle) * 3.14159f / 180.0f);
                correctedDistance = std::max(1.0f, correctedDistance); // Prevent division by zero or extreme values

                // Compute projected wall height
                float wallHeight = wallHeightScale / correctedDistance;

                // Compute screen position for this wall slice
                float screenX = i * sliceWidth;
                float wallTopY = centerY - wallHeight / 2.0f;
                float wallBottomY = centerY + wallHeight / 2.0f;

                // Adjust brightness based on distance
                const float maxDistance = 100.0f; // Adjust based on game scale
                float brightnessFactor = std::max(0.2f, 1.0f - (correctedDistance / maxDistance));
                sf::Uint8 color = static_cast<sf::Uint8>(50 + 150 * brightnessFactor);
               // sf::Color wallColor(0, static_cast<sf::Uint8>(50 + 150 * brightnessFactor), 0);
                sf::Color wallColor(color, color, color);

                // Define quad vertices for the wall slice
                wallLine.append(sf::Vertex(sf::Vector2f(screenX, wallTopY), wallColor));     // Top Left
                wallLine.append(sf::Vertex(sf::Vector2f(screenX + sliceWidth, wallTopY), wallColor));   // Top Right
                wallLine.append(sf::Vertex(sf::Vector2f(screenX + sliceWidth, wallBottomY), wallColor)); // Bottom Right
                wallLine.append(sf::Vertex(sf::Vector2f(screenX, wallBottomY), wallColor));  // Bottom Left
                }
            }
        }
    }
    
// collisions 
    // circle collision 
    bool circleCollision(sf::Vector2f pos1, float radius1, sf::Vector2f pos2, float radius2) {
        // Calculate the distance between the centers of the circles
        float dx = pos1.x - pos2.x;
        float dy = pos1.y - pos2.y;
        float distanceSquared = dx * dx + dy * dy;

        // Calculate the sum of the radii
        float radiusSum = radius1 + radius2;
        float radiusSumSquared = radiusSum * radiusSum;

        // Check if the distance squared is less than or equal to the sum of radii squared
        return distanceSquared <= radiusSumSquared;
    }

    // raycast collision 
    bool raycastPreCollision(const sf::Vector2f obj1position, const sf::Vector2f obj1direction, float obj1Speed, const sf::FloatRect obj1Bounds, sf::Vector2f obj1Acceleration, 
                                const sf::Vector2f obj2position, const sf::Vector2f obj2direction, float obj2Speed, const sf::FloatRect obj2Bounds, sf::Vector2f obj2Acceleration) { // 2d collision pre check
            
        ++cachedRaycastResult.counter;
        std::cout << "calculating raycast collision time" << std::endl; 

        // Calculate the initial relative velocity (obj1 velocity minus obj2 velocity)
        sf::Vector2f relativeVelocity = obj1direction * obj1Speed - obj2direction * obj2Speed;
        sf::Vector2f relativePosition = obj1position - obj2position;

        // Relative acceleration as a scalar difference
        sf::Vector2f relativeAcceleration = obj1Acceleration - obj2Acceleration;

        // Calculate velocity dot products
        float velocityDot = relativeVelocity.x * relativeVelocity.x + relativeVelocity.y * relativeVelocity.y;
        float positionVelocityDot = relativePosition.x * relativeVelocity.x + relativePosition.y * relativeVelocity.y;

        // Avoid division by zero or invalid values
        if (velocityDot == 0  && (relativeAcceleration.x == 0 && relativeAcceleration.y == 0)) {
            std::cout << "No relative motion or acceleration; no collision possible." << std::endl;
            return false;
        }

        // Time of closest approach considering relative acceleration
        float timeToClosestApproach = 0.0f;
        // If there is no relative acceleration, calculate using relative velocity only
        if (relativeAcceleration.x == 0 && relativeAcceleration.y == 0) {
            // Standard case: objects have relative velocity, no relative acceleration
            if (velocityDot != 0) {
                timeToClosestApproach = -positionVelocityDot / velocityDot;
            } else {
                std::cout << "No relative velocity detected; no collision possible." << std::endl;
                return false;
            }
        } else {
            // Case with relative acceleration: solve the quadratic equation
            // Compute a quadratic form: at² + bt + c = 0, where:
            // a = 0.5 * relative acceleration · relative acceleration
            // b = relative velocity · relative acceleration
            // c = relative position · relative velocity
            
            float a = 0.5f * (relativeAcceleration.x * relativeAcceleration.x + relativeAcceleration.y * relativeAcceleration.y);
            float b = relativeVelocity.x * relativeAcceleration.x + relativeVelocity.y * relativeAcceleration.y;
            float c = relativePosition.x * relativeVelocity.x + relativePosition.y * relativeVelocity.y;

            // Solve the quadratic equation for time (if applicable)
            float discriminant = b * b - 4.0f * a * c;

            if (discriminant < 0) {
                std::cout << "No collision; discriminant < 0." << std::endl;
                return false;
            }

            // Calculate the two possible times of closest approach
            float sqrtDiscriminant = std::sqrt(discriminant);
            float time1 = (-b - sqrtDiscriminant) / (2.0f * a);
            float time2 = (-b + sqrtDiscriminant) / (2.0f * a);

            // Choose the smallest positive time
            timeToClosestApproach = std::min(time1, time2);
            
            if (timeToClosestApproach < 0) {
                std::cout << "Closest approach is in the past." << std::endl;
                return false;
            }
        }

        // Store the calculated time of closest approach
        cachedRaycastResult.collisionTimes.emplace_back(timeToClosestApproach);

        // Log the calculated time for debugging
        std::cout << "Calculated Time to Closest Approach: " << timeToClosestApproach << std::endl;

        return true;
    }

    bool boundingBoxCollision(const sf::Vector2f &position1, const sf::Vector2f &size1,
                                const sf::Vector2f &position2, const sf::Vector2f &size2) {

        float xOverlapStart = std::max(position1.x, position2.x);
        float yOverlapStart = std::max(position1.y, position2.y);
        float xOverlapEnd = std::min(position1.x + size1.x, position2.x + size2.x);
        float yOverlapEnd = std::min(position1.y + size1.y, position2.y + size2.y);

        return !(xOverlapStart >= xOverlapEnd || yOverlapStart >= yOverlapEnd); 
    }

    bool pixelPerfectCollision( const std::shared_ptr<sf::Uint8[]>& bitmask1, const sf::Vector2f& position1, const sf::Vector2f& size1,
                                const std::shared_ptr<sf::Uint8[]>& bitmask2, const sf::Vector2f& position2, const sf::Vector2f& size2) {

        // Helper function to get the pixel index in the bitmask
        auto getPixelIndex = [](const sf::Vector2f& size, int x, int y) -> int {
            return (y * static_cast<int>(size.x) + x) * 4; // Each pixel has 4 bytes (RGBA)
        };

        // Calculate the overlapping area between the two objects
        float left = std::max(position1.x, position2.x);
        float top = std::max(position1.y, position2.y);
        float right = std::min(position1.x + size1.x, position2.x + size2.x);
        float bottom = std::min(position1.y + size1.y, position2.y + size2.y);

        // Check AABB collision first
        if (left >= right || top >= bottom) return false; 
        //std::cout << "AABB collision passed." << std::endl;
        // Check each pixel in the overlapping area
        for (int y = static_cast<int>(top); y < static_cast<int>(bottom); ++y) {
            for (int x = static_cast<int>(left); x < static_cast<int>(right); ++x) {
                // Calculate the position in each bitmask
                int x1 = x - static_cast<int>(position1.x);
                int y1 = y - static_cast<int>(position1.y);
                int x2 = x - static_cast<int>(position2.x);
                int y2 = y - static_cast<int>(position2.y);

                // Get the index of the pixel in each bitmask
                int index1 = getPixelIndex(size1, x1, y1);
                int index2 = getPixelIndex(size2, x2, y2);

                // Check if the pixels' values are non-zero (i.e., not transparent)
                if (bitmask1[index1] == 1 && bitmask2[index2] == 1) {
                    return true; // Collision detected
                }
            }
        }
        //std::cout << "Pixel perfect collision passed." << std::endl;
        return false; 
    }
    bool pixelPerfectCollision(const std::shared_ptr<sf::Uint8[]>& bitmask1, const sf::Vector2f& position1, const sf::Vector2f& size1,
        const std::shared_ptr<sf::Uint8[]>& bitmask2, const sf::Vector2f& position2, const sf::Vector2f& size2,
        float angle1, float angle2) {

        // Helper function to get the pixel index in the bitmask
        auto getPixelIndex = [](const sf::Vector2f& size, int x, int y) -> int {
            return (y * static_cast<int>(size.x) + x) * 4; // Each pixel has 4 bytes (RGBA)
        };

        // Calculate the overlapping area between the two objects
        float left = std::max(position1.x, position2.x);
        float top = std::max(position1.y, position2.y);
        float right = std::min(position1.x + size1.x, position2.x + size2.x);
        float bottom = std::min(position1.y + size1.y, position2.y + size2.y);

        // Check AABB collision first
        if (left >= right || top >= bottom) return false;

        // Helper function to rotate a point (x, y) around the center of the sprite
        auto rotatePoint = [](float x, float y, float angle) -> sf::Vector2f {
            float rad = angle * 3.14159f / 180.0f;
            float cosAngle = std::cos(rad);
            float sinAngle = std::sin(rad);
            return sf::Vector2f(x * cosAngle - y * sinAngle, x * sinAngle + y * cosAngle);
        };

        // Check each pixel in the overlapping area
        for (int y = static_cast<int>(top); y < static_cast<int>(bottom); ++y) {
            for (int x = static_cast<int>(left); x < static_cast<int>(right); ++x) {

                // Transform the pixel position to local coordinates of each sprite
                int x1 = x - static_cast<int>(position1.x);
                int y1 = y - static_cast<int>(position1.y);
                int x2 = x - static_cast<int>(position2.x);
                int y2 = y - static_cast<int>(position2.y);

                // Apply rotation by reversing the rotation angle for each sprite
                auto rotated1 = rotatePoint(x1, y1, -angle1);
                auto rotated2 = rotatePoint(x2, y2, -angle2);

                // Get the index of the pixel in each bitmask
                int index1 = getPixelIndex(size1, static_cast<int>(rotated1.x), static_cast<int>(rotated1.y));
                int index2 = getPixelIndex(size2, static_cast<int>(rotated2.x), static_cast<int>(rotated2.y));

                // Check if the pixels' values are non-zero (i.e., not transparent)
                if (bitmask1[index1] == 1 && bitmask2[index2] == 1) {
                    return true; // Collision detected
                }
            }
        }

        return false;
        }

}