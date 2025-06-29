#include "scenes.hpp"

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
// Base scene functions  
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

// Scene constructure sets up window and sprite respawn times 
Scene::Scene( sf::RenderWindow& gameWindow ) : window(gameWindow), quadtree(0.0f, 0.0f, Constants::WORLD_WIDTH, Constants::WORLD_HEIGHT){ 
    MetaComponents::leftView.setSize(Constants::VIEW_SIZE_X, Constants::VIEW_SIZE_Y);
    MetaComponents::leftView.setCenter(Constants::VIEW_SIZE_X / 2, Constants::VIEW_SIZE_Y / 2);
    MetaComponents::leftView.setViewport(sf::FloatRect(0.0f, 0.0f, 0.333f, 1.0f));

    MetaComponents::middleView.setSize(Constants::VIEW_SIZE_X, Constants::VIEW_SIZE_Y);
    MetaComponents::middleView.setCenter(Constants::VIEW_SIZE_X / 2, Constants::VIEW_SIZE_Y / 2);
    MetaComponents::middleView.setViewport(sf::FloatRect(0.333f, 0.0f, 0.333f, 1.0f));

    MetaComponents::rightView.setSize(Constants::VIEW_SIZE_X, Constants::VIEW_SIZE_Y);
    MetaComponents::rightView.setCenter(Constants::VIEW_SIZE_X / 2, Constants::VIEW_SIZE_Y / 2);
    MetaComponents::rightView.setViewport(sf::FloatRect(0.667f, 0.0f, 0.333f, 1.0f));

    log_info("scene made"); 
}

void Scene::runScene() {
    if (FlagSystem::flagEvents.gameEnd) return; // Early exit if game ended
    
    setTime();

    handleInput();

    respawnAssets();

    handleGameEvents();

    update();
    draw();
}

void Scene::draw(){
    window.clear(sf::Color::Black);
    window.display(); 
 }

void Scene::moveViewPortWASD(){
    // move view port 
    if(FlagSystem::flagEvents.aPressed){
        MetaComponents::middleView.move(sf::Vector2f(-10, 0)); 
    }
    if(FlagSystem::flagEvents.dPressed){
        MetaComponents::middleView.move(sf::Vector2f(10, 0)); 
    }
    if(FlagSystem::flagEvents.sPressed){
        MetaComponents::middleView.move(sf::Vector2f(0, 10)); 
    }
    if(FlagSystem::flagEvents.wPressed){
        MetaComponents::middleView.move(sf::Vector2f(0, -10)); 
    }
}

// Resets everything for scene to start again. The position, moveState, flagEvents, etc are all reset 
void Scene::restartScene() {
    sceneEvents.resetFlags(); 
}

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
// Game Scene #1 down below 
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

// Gets called once before the main game loop 
void gamePlayScene::createAssets() {
    try {
        globalTimer.Reset();  
        
        player = std::make_unique<Player>(Constants::SPRITE1_POSITION, Constants::SPRITE1_SCALE, Constants::SPRITE1_TEXTURE, Constants::SPRITE1_SPEED, Constants::SPRITE1_ACCELERATION, 
                                          Constants::SPRITE1_ANIMATIONRECTS, Constants::SPRITE1_INDEXMAX, utils::convertToWeakPtrVector(Constants::SPRITE1_BITMASK));
        player2 = std::make_unique<Player>(Constants::SPRITE2_POSITION, Constants::SPRITE2_SCALE, Constants::SPRITE2_TEXTURE, Constants::SPRITE2_SPEED, Constants::SPRITE2_ACCELERATION, 
                                          Constants::SPRITE2_ANIMATIONRECTS, Constants::SPRITE2_INDEXMAX, utils::convertToWeakPtrVector(Constants::SPRITE2_BITMASK));
        player2->returnSpritesShape().rotate(180.0f);
        player2->setHeadingAngle(player2->returnSpritesShape().getRotation());

        for(int i = 0; i < Constants::STICKS_NUMBER; ++i) sticks[i] = std::make_unique<Sprite>(Constants::STICK_POSITIONS[i], Constants::STICK_SCALE, Constants::STICK_TEXTURE);

        pawnBlue = std::make_unique<Sprite>(Constants::PAWNBLUE_POSITION, Constants::PAWNBLUE_SCALE, Constants::PAWNBLUE_TEXTURE); 
        pawnRed = std::make_unique<Sprite>(Constants::PAWNRED_POSITION, Constants::PAWNRED_SCALE, Constants::PAWNRED_TEXTURE);

        backgroundBig = std::make_unique<Sprite>(Constants::BACKGROUNDBIG_POSITION, Constants::BACKGROUNDBIG_SCALE, Constants::BACKGROUNDBIG_TEXTURE); 

        backgroundBigFinal = std::make_unique<Sprite>(Constants::BACKGROUNDBIGFINAL_POSITION, Constants::BACKGROUNDBIGFINAL_SCALE, Constants::BACKGROUNDBIGFINAL_TEXTURE); 
        backgroundBigFinal->setVisibleState(false); // hide final background at the start

        backgroundBigHalfRed = std::make_unique<Sprite>(Constants::BACKGROUNDBIGHALF_POSITION, Constants::BACKGROUNDBIGHALF_SCALE, Constants::BACKGROUNDBIGHALF_TEXTURE);
        backgroundBigHalfRed->setVisibleState(false); // hide half background at the start

        backgroundBigHalfBlue = std::make_unique<Sprite>(Constants::BACKGROUNDBIGHALF_POSITION, Constants::BACKGROUNDBIGHALF_SCALE, Constants::BACKGROUNDBIGHALF_TEXTURE);
        backgroundBigHalfBlue->setVisibleState(false); // hide half background at the start

        button1 = std::make_unique<Button>(Constants::BUTTON1_POSITION, Constants::BUTTON1_SCALE, Constants::BUTTON1_TEXTURE, Constants::BUTTON1_ANIMATIONRECTS, Constants::BUTTON1_INDEXMAX, utils::convertToWeakPtrVector(Constants::BUTTON1_BITMASK)); 
        button1->setRects(0); 

        boardTiles[0] = std::make_shared<Tile>(Constants::BOARDTILES_SCALE, Constants::BOARDTILES_TEXTURE, Constants::BOARDTILES_RECTS[Constants::WALL_TILEX_INDEX], Constants::BOARDTILES_BITMASK[Constants::WALL_TILEX_INDEX], true); // walkable all true
        boardTiles[1] = std::make_shared<Tile>(Constants::BOARDTILES_SCALE, Constants::BOARDTILES_TEXTURE, Constants::BOARDTILES_RECTS[Constants::WALL_TILEY_INDEX], Constants::BOARDTILES_BITMASK[Constants::WALL_TILEY_INDEX], true); 
        boardTiles[2] = std::make_shared<Tile>(Constants::BOARDTILES_SCALE, Constants::BOARDTILES_TEXTURE, Constants::BOARDTILES_RECTS[Constants::PATH_TILE_INDEX], Constants::BOARDTILES_BITMASK[Constants::PATH_TILE_INDEX], true); 
        boardTiles[3] = std::make_shared<Tile>(Constants::BOARDTILES_SCALE, Constants::BOARDTILES_TEXTURE, Constants::BOARDTILES_RECTS[Constants::P1_GOAL_TILE_INDEX], Constants::BOARDTILES_BITMASK[Constants::P1_GOAL_TILE_INDEX], true); 
        boardTiles[4] = std::make_shared<Tile >(Constants::BOARDTILES_SCALE, Constants::BOARDTILES_TEXTURE, Constants::BOARDTILES_RECTS[Constants::P2_GOAL_TILE_INDEX], Constants::BOARDTILES_BITMASK[Constants::P2_GOAL_TILE_INDEX], true); 
        boardTiles[5] = std::make_shared<Tile>(Constants::BOARDTILES_SCALE, Constants::BOARDTILES_TEXTURE, Constants::BOARDTILES_RECTS[Constants::BLANKWALL_TILE_INDEX], Constants::BOARDTILES_BITMASK[Constants::BLANKWALL_TILE_INDEX], true); 
        boardTiles[6] = std::make_shared<Tile>(Constants::BOARDTILES_SCALE, Constants::BOARDTILES_TEXTURE, Constants::BOARDTILES_RECTS[Constants::BLANKP1_INDEX], Constants::BOARDTILES_BITMASK[Constants::BLANKP1_INDEX], true); 
        boardTiles[7] = std::make_shared<Tile>(Constants::BOARDTILES_SCALE, Constants::BOARDTILES_TEXTURE, Constants::BOARDTILES_RECTS[Constants::BLANKP2_INDEX], Constants::BOARDTILES_BITMASK[Constants::BLANKP2_INDEX], true);  

        boardTiles[8] = std::make_shared<Tile>(Constants::BOARDTILES_SCALE, Constants::BOARDTILES_TEXTURE, Constants::BOARDTILES_RECTS[Constants::WALL_INDEX], Constants::BOARDTILES_BITMASK[Constants::WALL_INDEX], false); 
        boardTiles[9] = std::make_shared<Tile>(Constants::BOARDTILES_SCALE, Constants::BOARDTILES_TEXTURE, Constants::BOARDTILES_RECTS[Constants::WALLBLANK_INDEX], Constants::BOARDTILES_BITMASK[Constants::WALLBLANK_INDEX], false); 
        boardTiles[10] = std::make_shared<Tile>(Constants::BOARDTILES_SCALE, Constants::BOARDTILES_TEXTURE, Constants::BOARDTILES_RECTS[Constants::WALLTOP_INDEX], Constants::BOARDTILES_BITMASK[Constants::WALLTOP_INDEX], false); 

        boardTileMap = std::make_unique<BoardTileMap>(boardTiles, Constants::BOARDTILES_ROW, Constants::BOARDTILES_COL); // 19 x 21 tiles including walls

        rays = sf::VertexArray(sf::Quads, Constants::RAYS_NUM);
        rays2 = sf::VertexArray(sf::Quads, Constants::RAYS_NUM);

        // Music
        backgroundMusic = std::make_unique<MusicClass>(std::move(Constants::BACKGROUNDMUSIC_MUSIC), Constants::BACKGROUNDMUSIC_VOLUME);
        // if(backgroundMusic) backgroundMusic->returnMusic().play(); 
        // if(backgroundMusic) backgroundMusic->returnMusic().setLoop(Constants::BACKGROUNDMUSIC_LOOP);

        buttonClickSound = std::make_unique<SoundClass>(Constants::BUTTONCLICK_SOUNDBUFF, Constants::BUTTONCLICKSOUND_VOLUME);

        // Text
        introText = std::make_unique<TextClass>(Constants::TEXT_POSITION, Constants::TEXT_SIZE, Constants::TEXT_COLOR, Constants::TEXT_FONT, Constants::TEXT_MESSAGE);
        scoreText = std::make_unique<TextClass>(Constants::SCORETEXT_POSITION, Constants::SCORETEXT_SIZE, Constants::SCORETEXT_COLOR, Constants::TEXT_FONT, Constants::SCORETEXT_MESSAGE);
        endingText = std::make_unique<TextClass>(Constants::ENDINGTEXT_POSITION, Constants::ENDINGTEXT_SIZE, Constants::ENDINGTEXT_COLOR, Constants::TEXT_FONT, Constants::ENDINGTEXT_MESSAGE);

        insertItemsInQuadtree(); 
        setInitialTimes();

        globalTimer.End("initializing assets in scene 1"); // for logging purposes
    } 
    catch (const std::exception& e) {
        log_error("Exception in createAssets: " + std::string(e.what()));
    }
}

void gamePlayScene::setInitialTimes(){

}

void gamePlayScene::insertItemsInQuadtree(){
    quadtree.insert(player);  
    quadtree.insert(player2);
}

void gamePlayScene::respawnAssets(){
   
} 

void gamePlayScene::setTime(){
    if(FlagSystem::gameScene1Flags.begin){
        MetaComponents::globalTime += MetaComponents::deltaTime;
    }
    if(FlagSystem::flagEvents.spacePressed || MetaComponents::spacePressedElapsedTime) {
        MetaComponents::spacePressedElapsedTime += MetaComponents::deltaTime; 
    } else {
        MetaComponents::spacePressedElapsedTime = 0.0f; 
    }
    
} 

void gamePlayScene::handleInput() {
    handleMouseKey();
    handleSpaceKey(); 
    handleMovementKeys();
}

void gamePlayScene::handleMouseKey() {
    if (FlagSystem::flagEvents.mouseClicked && buttonClickSound) buttonClickSound->returnSound().play();

    if (!boardTileMap || stickIndex >= Constants::STICKS_NUMBER) return;

    int currentTileIndex = -1;
    
    // Safe bounds checking before calling getTileIndex
    sf::Vector2f mousePos = MetaComponents::middleViewmouseCurrentPosition_f;
    
    // Check if mouse position is valid and within reasonable bounds
    if (mousePos.x == 0.0f && mousePos.y == 0.0f) mousePos = sf::Vector2f{20.0f, 20.0f}; // default position
    
    // Try to get tile index with exception handling
    try {
        currentTileIndex = boardTileMap->getTileIndex(mousePos);
    } 
    catch (const std::out_of_range& e) {  // Mouse is outside tilemap bounds - exit early
        return;        
    }
    
    // Additional safety check
    if (currentTileIndex < 0 || currentTileIndex >= boardTileMap->getTileMapNumber()) return;
    
    size_t targetTileIndex = currentTileIndex;

    // Find closest grey tile if current isn't grey
    if (!boardTileMap->isGreyTile(currentTileIndex)) {
        float minDistance = std::numeric_limits<float>::max();
        bool foundGreyTile = false;
        
        for (size_t i = 0; i < boardTileMap->getTileMapNumber(); ++i) {
            if (boardTileMap->isGreyTile(i)) {
                sf::Vector2f tilePos = boardTileMap->getTile(i)->getTileSprite().getPosition();
                float dx = mousePos.x - tilePos.x;
                float dy = mousePos.y - tilePos.y;
                float distance = std::sqrt(dx * dx + dy * dy);
                
                if (distance < minDistance) {
                    minDistance = distance;
                    targetTileIndex = i;
                    foundGreyTile = true;
                }
            }
        }
        if (!foundGreyTile) return;
    }

    // Convert tile index to row/col coordinates
    int row = targetTileIndex / Constants::BOARDTILES_ROW;
    int col = targetTileIndex % Constants::BOARDTILES_ROW;
    
    bool isVertical = boardTileMap->isVerticalWallTile(targetTileIndex);

    if (stickIndex < Constants::STICKS_NUMBER) {
        sf::Vector2f stickPos = boardTileMap->getTile(targetTileIndex)->getTileSprite().getPosition();
        
        if (isVertical) stickPos.x += 9.0f;
        
        sticks[stickIndex]->returnSpritesShape().setPosition(stickPos);
        sticks[stickIndex]->returnSpritesShape().setRotation(isVertical ? 90.0f : 0.0f);
    }

    // Early exit if not actually clicking
    if (!FlagSystem::flagEvents.mouseClicked) return;

    // BOUNDARY CHECKS - Prevent placement near borders
    unsigned int nextBlankTileIndex, nextTileIndex;
    bool validPlacement = false;

    if (isVertical) {
        if (row < Constants::BOARDTILES_ROW - 1) {
            nextBlankTileIndex = targetTileIndex + Constants::BOARDTILES_ROW + 2;
            nextTileIndex = nextBlankTileIndex + Constants::BOARDTILES_ROW + 2;
            validPlacement = true;
        }
    } else {
        if (col >= 1 && col < Constants::BOARDTILES_ROW - 1) {
            nextBlankTileIndex = targetTileIndex + 1;  // tile to the right
            nextTileIndex = nextBlankTileIndex + 1;    // tile to the right
            validPlacement = true;
        }
    }

    // Only place stick if within valid boundaries and variables are properly set
    if (!validPlacement || 
        nextBlankTileIndex >= boardTileMap->getTileMapNumber() || 
        nextTileIndex >= boardTileMap->getTileMapNumber()) {
        return;
    }

    // Update tile walkability
    boardTileMap->getTile(targetTileIndex)->setWalkable(false);
    boardTileMap->getTile(nextBlankTileIndex)->setWalkable(false);
    boardTileMap->getTile(nextTileIndex)->setWalkable(false);

    ++stickIndex;
    FlagSystem::gameScene1Flags.stickPlaced = true;
}
void gamePlayScene::handleSpaceKey() {
    if(MetaComponents::spacePressedElapsedTime) {
       
    }
}

void gamePlayScene::handleMovementKeys() {
    // handleEachPlayer(player, player2, p1pathCount, p1PrevPathIndex);
    // handleEachPlayer(player2, player, p2pathCount, p2PrevPathIndex);

     handleEachPlayer(player, p1pathCount, p1PrevPathIndex);
    handleEachPlayer(player2, p2pathCount, p2PrevPathIndex);
}

void gamePlayScene::handleEachPlayer(std::unique_ptr<Player>& playerNum, size_t& moveCount, unsigned int& prevPathIndex) {
    if (!playerNum || !playerNum->getMoveState()) return;
    
    // Static variables for turn tracking
    bool turnInProgress = playerNum->getTurnInProgress();
    int tilesMovedThisTurn = playerNum->getTlesMovedThisTurn();
    bool isMoving = playerNum->getIsMoving();
    sf::Vector2f targetPosition = playerNum->getTargetPosition();
    int currentDirection = playerNum->getCurrentDirection();

    // Reset turn state
    if (moveCount == 0) {
        prevPathIndex = boardTileMap->getTileIndex(playerNum->getSpritePos());
        playerNum->setTurnInProgress(false);
        playerNum->setTilesMovedThisTurn(0);
        playerNum->setIsMoving(false);
    }
    ++moveCount;

    sf::Vector2f currentPos = playerNum->getSpritePos();
    unsigned int currentTileIndex = boardTileMap->getTileIndex(currentPos);
    
    // Check collision at position
    auto canWalkAtPosition = [&](sf::Vector2f pos, bool isBackwardMove = false) -> bool {
        sf::FloatRect playerBounds = playerNum->returnSpritesShape().getGlobalBounds();
        sf::FloatRect testBounds(
            pos.x - playerBounds.width * 0.3f,
            pos.y - playerBounds.height * 0.2f,
            playerBounds.width * 0.6f,
            playerBounds.height * 0.4f
        );
        
        // Count collisions
        int collisionCount = 0;
        for (int i = 0; i < Constants::BOARDTILES_ROW * Constants::BOARDTILES_COL; ++i) {
            try {
                auto& tile = boardTileMap->getTile(i);
                if (tile && tile->getVisibleState() && !tile->getWalkable()) {
                    if (tile->getTileSprite().getGlobalBounds().intersects(testBounds)) collisionCount++;
                }
            } catch (const std::exception& e) {
                log_warning("Tile collision check error: " + std::string(e.what()));
            }
        }
        
        // Anti-stuck logic for backward moves
        if (isBackwardMove) {
            sf::FloatRect currentBounds(
                currentPos.x - playerBounds.width * 0.3f,
                currentPos.y - playerBounds.height * 0.2f,
                playerBounds.width * 0.6f,
                playerBounds.height * 0.4f
            );
            
            int currentCollisions = 0;
            for (int i = 0; i < Constants::BOARDTILES_ROW * Constants::BOARDTILES_COL; ++i) {
                try {
                    auto& tile = boardTileMap->getTile(i);
                    if (tile && tile->getVisibleState() && !tile->getWalkable()) {
                        if (tile->getTileSprite().getGlobalBounds().intersects(currentBounds)) currentCollisions++;
                    }
                } catch (const std::exception& e) {
                    log_warning("Current collision check error: " + std::string(e.what()));
                }
            }
            return collisionCount <= currentCollisions;
        }
        return collisionCount == 0;
    };
    
    // Get max tiles allowed in direction
    auto getMaxTilesInDirection = [&](unsigned int fromTileIndex, int direction) -> int {
        int col = fromTileIndex % Constants::BOARDTILES_COL;
        
        // Check start tile restrictions for horizontal movement
        if (direction == 1) { // moving right
            if (boardTileMap->isP1StartTile(fromTileIndex) || 
                (col + 1 < Constants::BOARDTILES_COL && boardTileMap->isP2StartTile(fromTileIndex + 1))) return 1;
        } else if (direction == 3) { // moving left
            if ((col > 0 && boardTileMap->isP1StartTile(fromTileIndex - 1)) || boardTileMap->isP2StartTile(fromTileIndex)) return 1;
        }
        
        // Standard movement rules
        switch (direction) {
            case 0: case 2: return 2; // up/down: always 2
            case 1: // right
                if (col >= Constants::BOARDTILES_COL - 1) return 0;
                return (col == 0 || col == Constants::BOARDTILES_COL - 2) ? 1 : 2;
            case 3: // left
                if (col == 0) return 0;
                return (col == 1 || col == Constants::BOARDTILES_COL - 1) ? 1 : 2;
        }
        return 0;
    };
    
    // Get adjacent tile index
    auto getAdjacentTileIndex = [&](unsigned int fromIndex, int direction, int distance) -> int {
        int row = fromIndex / Constants::BOARDTILES_COL;
        int col = fromIndex % Constants::BOARDTILES_COL;
        
        switch (direction) {
            case 0: return (row >= distance) ? (row - distance) * Constants::BOARDTILES_COL + col : -1; // up
            case 1: return (col + distance < Constants::BOARDTILES_COL) ? row * Constants::BOARDTILES_COL + (col + distance) : -1; // right
            case 2: return (row + distance < Constants::BOARDTILES_ROW) ? (row + distance) * Constants::BOARDTILES_COL + col : -1; // down
            case 3: return (col >= distance) ? row * Constants::BOARDTILES_COL + (col - distance) : -1; // left
        }
        return -1;
    };
    
    // Get tile center position
    auto getTileCenter = [&](unsigned int tileIndex) -> sf::Vector2f {
        try {
            auto& tile = boardTileMap->getTile(tileIndex);
            if (tile && tile->getVisibleState()) {
                sf::FloatRect bounds = tile->getTileSprite().getGlobalBounds();
                return sf::Vector2f(bounds.left + bounds.width / 2.0f, bounds.top + bounds.height / 2.0f);
            }
        } catch (const std::exception& e) {
            log_warning("Get tile center error: " + std::string(e.what()));
        }
        return sf::Vector2f(0, 0);
    };
    
    // Get movement direction from player rotation
    auto getMovementDirection = [&](bool forward) -> int {
        float angle = playerNum->returnSpritesShape().getRotation();
        while (angle < 0) angle += 360;
        while (angle >= 360) angle -= 360;
        
        int direction;
        if (angle >= 315 || angle < 45) direction = 1;      // right
        else if (angle >= 45 && angle < 135) direction = 2;  // down
        else if (angle >= 135 && angle < 225) direction = 3; // left
        else direction = 0;                                  // up
        
        return forward ? direction : (direction + 2) % 4;
    };
    
    // Handle ongoing movement animation
    if (isMoving) {
        sf::Vector2f playerPos = playerNum->getSpritePos();
        float distance = std::sqrt(std::pow(targetPosition.x - playerPos.x, 2) + std::pow(targetPosition.y - playerPos.y, 2));
        
        if (distance <= Constants::TILE_THRESHOLD) {
            playerNum->changePosition(targetPosition);
            playerNum->updatePos();
            playerNum->setIsMoving(false); 
            playerNum->setCurrentDirection(-1);
        } else {
            // Continue movement
            switch (currentDirection) {
                case 0: physics::spriteMover(playerNum, physics::moveUp, Constants::SPRITE1_SPEED, playerPos); break;
                case 1: physics::spriteMover(playerNum, physics::moveRight, Constants::SPRITE1_SPEED, playerPos); break;
                case 2: physics::spriteMover(playerNum, physics::moveDown, Constants::SPRITE1_SPEED, playerPos); break;
                case 3: physics::spriteMover(playerNum, physics::moveLeft, Constants::SPRITE1_SPEED, playerPos); break;
            }
        }
        return;
    }
    
    // Handle rotation (only when not moving or turn hasn't started)
    if (FlagSystem::flagEvents.aPressed && (!turnInProgress || tilesMovedThisTurn == 0)) {
        playerNum->returnSpritesShape().rotate(-1.0f);
        playerNum->setHeadingAngle(playerNum->returnSpritesShape().getRotation());
    }
    
    if (FlagSystem::flagEvents.dPressed && (!turnInProgress || tilesMovedThisTurn == 0)) {
        playerNum->returnSpritesShape().rotate(1.0f);
        playerNum->setHeadingAngle(playerNum->returnSpritesShape().getRotation());
    }
    
    // Handle movement input
    auto attemptMovement = [&](bool isForward) {
        if (tilesMovedThisTurn > 0) return; // Already moved this turn
        
        if (!turnInProgress) playerNum->setTurnInProgress(true);
        
        int direction = getMovementDirection(isForward);
        int maxTiles = getMaxTilesInDirection(currentTileIndex, direction);
        
        if (maxTiles <= 0) return;
        
        // Find furthest valid position
        int validDistance = 0;
        for (int dist = 1; dist <= maxTiles; ++dist) {
            int testTileIndex = getAdjacentTileIndex(currentTileIndex, direction, dist);
            if (testTileIndex == -1) break;
            
            sf::Vector2f testPos = getTileCenter(testTileIndex);
            if (canWalkAtPosition(testPos, !isForward)) validDistance = dist;
            else break;
        }
        
        if (validDistance > 0) {
            int finalTileIndex = getAdjacentTileIndex(currentTileIndex, direction, validDistance);
            playerNum->setTargetPosition(getTileCenter(finalTileIndex));
            playerNum->setCurrentDirection(direction);
            playerNum->setIsMoving(true);
            playerNum->setTilesMovedThisTurn(validDistance);
        }
    };
    
    if (FlagSystem::flagEvents.wPressed) attemptMovement(true);   // forward
    if (FlagSystem::flagEvents.sPressed) attemptMovement(false);  // backward
    
    // Apply screen boundaries
    sf::Vector2f finalPos = playerNum->getSpritePos();
    sf::FloatRect bounds = playerNum->returnSpritesShape().getGlobalBounds();
    
    float newX = std::clamp(finalPos.x, bounds.width / 2, Constants::VIEW_SIZE_X - bounds.width / 2);
    float newY = std::clamp(finalPos.y, bounds.height / 2, Constants::VIEW_SIZE_Y - bounds.height / 2);
    
    playerNum->changePosition(sf::Vector2f{newX, newY});
    playerNum->updatePos();

    // Check if turn is complete
    unsigned int newTileIndex = boardTileMap->getTileIndex(playerNum->getSpritePos());
    if (newTileIndex != prevPathIndex && !isMoving) {
        FlagSystem::gameScene1Flags.moved = true;
        moveCount = 0;
        playerNum->setTurnInProgress(false);
        playerNum->setTilesMovedThisTurn(0);
        prevPathIndex = newTileIndex;
    }
}
// Keeps sprites inside screen bounds, checks for collisions, update scores, and sets flagEvents.gameEnd to true in an event of collision 
void gamePlayScene::handleGameEvents() { 

    if(MetaComponents::globalTime >= 5.0) introText->setVisibleState(false); // hide intro text after 5 seconds

    physics::calculateRayCast3d(player, boardTileMap, rays, wallLine); // board specific
    physics::calculateRayCast3d(player2, boardTileMap, rays2, wallLine2); // board specific

    pawnRed->updateSpritePos(player2->getSpritePos()); // pawn red is player 2
    pawnBlue->updateSpritePos(player->getSpritePos()); // pawn blue is player

    physics::calculateSprite3D(pawnRed, player, boardTileMap, pawnRedBlocked); // pawn red is player 2
    physics::calculateSprite3D(pawnBlue, player2, boardTileMap, pawnBlueBlocked); // pawn blue is player 1

    backgroundBigHalfRed->setVisibleState(pawnRedBlocked);
    backgroundBigHalfBlue->setVisibleState(pawnBlueBlocked);

    // check which players turn
    if(FlagSystem::gameScene1Flags.playerRedTurn) {
        player->setMoveState(true); // player 1 can move
        player2->setMoveState(false); // player 2 cannot move

        if(FlagSystem::gameScene1Flags.moved || FlagSystem::gameScene1Flags.stickPlaced) {
            FlagSystem::gameScene1Flags.playerRedTurn = false; // switch to player 2's turn
            FlagSystem::gameScene1Flags.playerBlueTurn = true; // set player 2's turn
            FlagSystem::gameScene1Flags.moved = false; // reset moved flag
            FlagSystem::gameScene1Flags.stickPlaced = false; // reset stick placed flag
        }

        if(boardTileMap->isP2StartTile(boardTileMap->getTileIndex(player->getSpritePos()))) {
            FlagSystem::flagEvents.gameEnd = true; // player 1 reached goal tile
            endingText->updateText(Constants::ENDINGTEXT_MESSAGE + " Player Blue wins!");
            endingText->setVisibleState(true);
            backgroundBigFinal->setVisibleState(true); // show final background
            backgroundBig->setVisibleState(false); // hide initial background
        }
    }
    else if(FlagSystem::gameScene1Flags.playerBlueTurn) {
        player2->setMoveState(true); // player 1 can move
        player->setMoveState(false); // player 1 cannot move

        if(FlagSystem::gameScene1Flags.moved || FlagSystem::gameScene1Flags.stickPlaced) {
            FlagSystem::gameScene1Flags.playerBlueTurn = false; // switch to player 1's turn
            FlagSystem::gameScene1Flags.playerRedTurn = true; // set player 1's turn
            FlagSystem::gameScene1Flags.moved = false; // reset moved flag
            FlagSystem::gameScene1Flags.stickPlaced = false; // reset stick placed flag
        }

        if(boardTileMap->isP1StartTile(boardTileMap->getTileIndex(player2->getSpritePos()))) {
            FlagSystem::flagEvents.gameEnd = true; // player 2 reached goal tile
            endingText->updateText(Constants::ENDINGTEXT_MESSAGE + " Player Red wins!");
            endingText->setVisibleState(true);
            backgroundBigFinal->setVisibleState(true); // show final background
            backgroundBig->setVisibleState(false); // hide initial background
        }
    }
} 

void gamePlayScene::update() {
    try {
        changeAnimation();
        quadtree.update();         
    } 
    catch (const std::exception& e) {
        log_error("Exception in updateSprites: " + std::string(e.what()));
    }
}

void gamePlayScene::changeAnimation(){ // change animation for sprites. change animation for texts if necessary   
    if (button1) button1->changeAnimation(); 
}

// Draws only the visible sprite and texts
void gamePlayScene::draw() {
    try {
        window.clear(sf::Color::Black); // set the base baskground color black

        drawInleftView();
        drawInmiddleView();
        drawInRightView();

        window.display(); 
    } 
    catch (const std::exception& e) {
        log_error("Exception in draw: " + std::string(e.what()));
    }
}

void gamePlayScene::drawInleftView(){
    window.setView(MetaComponents::leftView);

    drawVisibleObject(backgroundBig);
    drawVisibleObject(backgroundBigFinal);

    drawVisibleObject(scoreText); 
    drawVisibleObject(introText);
    drawVisibleObject(endingText);

    if(pawnRedBlocked){
        drawVisibleObject(pawnRed);
        drawVisibleObject(backgroundBigHalfRed);
        window.draw(wallLine);
    } else {
        window.draw(wallLine);
        drawVisibleObject(pawnRed); // pawn red is player 2
    }
}

void gamePlayScene::drawInmiddleView(){
    window.setView(MetaComponents::middleView);

    drawVisibleObject(boardTileMap); 

    for(const auto& stick : sticks) drawVisibleObject(stick);

    drawVisibleObject(player);
    window.draw(rays); // direct sf object

    drawVisibleObject(player2);
    window.draw(rays2); 
}

void gamePlayScene::drawInRightView(){
    window.setView(MetaComponents::rightView);
    
    drawVisibleObject(backgroundBig);
    drawVisibleObject(backgroundBigFinal);

    drawVisibleObject(endingText);
    drawVisibleObject(button1); 

    if(pawnBlueBlocked){
        drawVisibleObject(pawnBlue);
        drawVisibleObject(backgroundBigHalfBlue);
        window.draw(wallLine2);
    } else {
        window.draw(wallLine2);
        drawVisibleObject(pawnBlue); // pawn red is player 2
    }
}
