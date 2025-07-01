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

        for(int i = 0; i < Constants::STICKS_NUMBER / 2; ++i) sticksBlue[i] = std::make_unique<Sprite>(Constants::STICK_POSITIONSBLUE[i], Constants::STICK_SCALE, Constants::STICK_TEXTURE);
        for(int i = 0; i < Constants::STICKS_NUMBER / 2; ++i) sticksRed[i] = std::make_unique<Sprite>(Constants::STICK_POSITIONSRED[i], Constants::STICK_SCALE, Constants::STICK_TEXTURE);

        pawn = std::make_unique<Sprite>(Constants::PAWN1_POSITION, Constants::PAWN1_SCALE, Constants::PAWN1_TEXTURE);
        pawn2 = std::make_unique<Sprite>(Constants::PAWN2_POSITION, Constants::PAWN2_SCALE, Constants::PAWN2_TEXTURE); 

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
        if(backgroundMusic) backgroundMusic->returnMusic().play(); 
        if(backgroundMusic) backgroundMusic->returnMusic().setLoop(Constants::BACKGROUNDMUSIC_LOOP);

        buttonClickSound = std::make_unique<SoundClass>(Constants::BUTTONCLICK_SOUNDBUFF, Constants::BUTTONCLICKSOUND_VOLUME);

        // Text
        introText = std::make_unique<TextClass>(Constants::TEXT_POSITION, Constants::TEXT_SIZE, Constants::TEXT_COLOR, Constants::TEXT_FONT, Constants::TEXT_MESSAGE);
        scoreText = std::make_unique<TextClass>(Constants::SCORETEXT_POSITION, Constants::SCORETEXT_SIZE, Constants::SCORETEXT_COLOR, Constants::TEXT_FONT, Constants::SCORETEXT_MESSAGE);
        endingText = std::make_unique<TextClass>(Constants::ENDINGTEXT_POSITION, Constants::ENDINGTEXT_SIZE, Constants::ENDINGTEXT_COLOR, Constants::TEXT_FONT, Constants::ENDINGTEXT_MESSAGE);
        endingText->setVisibleState(false); // hide ending text at the start

        player1Text = std::make_unique<TextClass>(Constants::PLAYER1TEXT_POSITION, Constants::PLAYER1TEXT_SIZE, Constants::PLAYER1TEXT_COLOR, Constants::TEXT_FONT, Constants::PLAYER1TEXT_MESSAGE);
        player2Text = std::make_unique<TextClass>(Constants::PLAYER2TEXT_POSITION, Constants::PLAYER2TEXT_SIZE, Constants::PLAYER2TEXT_COLOR, Constants::TEXT_FONT, Constants::PLAYER2TEXT_MESSAGE); 
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

// // need to have enterance to other side at least one way & player be able to move in at least one direction
// bool gamePlayScene::playerHasExit(const std::unique_ptr<Player>& currentPlayer, bool isPlayer1) const {
//     if (!boardTileMap || !currentPlayer) return false; 

//     return true; 
// }

void gamePlayScene::handleMouseKey() { 

    unsigned int stickIndex;
    if(FlagSystem::gameScene1Flags.playerBlueTurn) stickIndex = stickIndexBlue;
    else if(FlagSystem::gameScene1Flags.playerRedTurn) stickIndex = stickIndexRed;

    if (!boardTileMap || stickIndex >= Constants::STICKS_NUMBER / 2 ) return;

    sf::Vector2f mousePos = MetaComponents::middleViewmouseCurrentPosition_f;
    if (mousePos.x == 0.0f && mousePos.y == 0.0f)  mousePos = sf::Vector2f{20.0f, 20.0f}; // Default position

    int currentTileIndex = -1;
    try {
        currentTileIndex = boardTileMap->getTileIndex(mousePos);
    } catch (const std::out_of_range& e) {
        return;  // Mouse is outside tilemap bounds - exit early
    }

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

    int row = targetTileIndex / Constants::BOARDTILES_ROW;
    int col = targetTileIndex % Constants::BOARDTILES_ROW;
    bool isVertical = boardTileMap->isVerticalWallTile(targetTileIndex);

    sf::Vector2f stickPos = boardTileMap->getTile(targetTileIndex)->getTileSprite().getPosition();
    if (isVertical) stickPos.x += 9.0f;

    if(FlagSystem::gameScene1Flags.playerBlueTurn){
        sticksBlue[stickIndex]->returnSpritesShape().setPosition(stickPos);
        sticksBlue[stickIndex]->returnSpritesShape().setRotation(isVertical ? 90.0f : 0.0f);
    } else if (FlagSystem::gameScene1Flags.playerRedTurn){
        sticksRed[stickIndex]->returnSpritesShape().setPosition(stickPos);
        sticksRed[stickIndex]->returnSpritesShape().setRotation(isVertical ? 90.0f : 0.0f);
    }

    if (!FlagSystem::flagEvents.mouseClicked) return; // Only apply changes on click

    unsigned int nextBlankTileIndex, nextTileIndex;
    bool validPlacement = false;

    if (isVertical) {
        if (row < Constants::BOARDTILES_ROW) {
            nextBlankTileIndex = targetTileIndex + Constants::BOARDTILES_ROW + 2;
            nextTileIndex = nextBlankTileIndex + Constants::BOARDTILES_ROW + 2;
            validPlacement = true;
        }
    } else {
        if (col < Constants::BOARDTILES_ROW) {
            nextBlankTileIndex = targetTileIndex + 1;
            nextTileIndex = nextBlankTileIndex + 1;
            validPlacement = true;
        }
    }

    // bool player1HasExit = playerHasExit(player, true);
    // bool player2HasExit = playerHasExit(player2, false);

    bool player1HasExit = true;
    bool player2HasExit = true;

    if (!validPlacement || nextBlankTileIndex >= boardTileMap->getTileMapNumber() || nextTileIndex >= boardTileMap->getTileMapNumber() || !player1HasExit || !player2HasExit) return;

    // Before placing, verify all involved tiles are walkable
    if (!boardTileMap->getTile(targetTileIndex)->getWalkable() || !boardTileMap->getTile(nextBlankTileIndex)->getWalkable() || !boardTileMap->getTile(nextTileIndex)->getWalkable()) return;

    boardTileMap->getTile(targetTileIndex)->setWalkable(false);
    boardTileMap->getTile(nextBlankTileIndex)->setWalkable(false);
    boardTileMap->getTile(nextTileIndex)->setWalkable(false);

    if(FlagSystem::gameScene1Flags.playerBlueTurn) ++stickIndexBlue;
    else if (FlagSystem::gameScene1Flags.playerRedTurn) ++stickIndexRed;
    FlagSystem::gameScene1Flags.stickPlaced = true;
    if (FlagSystem::flagEvents.mouseClicked && buttonClickSound) buttonClickSound->returnSound().play();

    // std::cout << "red index: " << stickIndexRed << std::endl;
    // std::cout << "blue index: " << stickIndexBlue << std::endl;
    // std::cout << "total index: " << stickIndex << std::endl;
}

void gamePlayScene::handleSpaceKey() {
    if(MetaComponents::spacePressedElapsedTime) {
       
    }
}

void gamePlayScene::handleMovementKeys() {
    handleEachPlayer(player, player2, p1pathCount, p1PrevPathIndex);
    handleEachPlayer(player2, player, p2pathCount, p2PrevPathIndex);
}

void gamePlayScene::handleEachPlayer(std::unique_ptr<Player>& playerNum, std::unique_ptr<Player>& playerToCheck, size_t& moveCount, unsigned int& prevPathIndex) { 
    if (!playerNum || !playerNum->getMoveState()) return; 

    // Get current state
    bool turnInProgress = playerNum->getTurnInProgress(); 
    int tilesMovedThisTurn = playerNum->getTlesMovedThisTurn(); 
    bool isMoving = playerNum->getIsMoving(); 

    sf::Vector2f targetPosition = playerNum->getTargetPosition(); 
    int currentDirection = playerNum->getCurrentDirection(); 
    bool isSpecialMovement = playerNum->getIsSpecialMovement();
    bool hasReachedOtherPlayer = playerNum->getHasReachedOtherPlayer();

    // Reset turn state at beginning of new move cycle 
    if (moveCount == 0) { 
        prevPathIndex = boardTileMap->getTileIndex(playerNum->getSpritePos()); 
        playerNum->setTurnInProgress(false); 
        playerNum->setTilesMovedThisTurn(0); 
        playerNum->setIsMoving(false); 
        playerNum->setIsSpecialMovement(false);
        playerNum->setHasReachedOtherPlayer(false);
    } 
    ++moveCount; 

    sf::Vector2f currentPos = playerNum->getSpritePos(); 
    unsigned int currentTileIndex = boardTileMap->getTileIndex(currentPos); 

    // Check if player is on their start tile
    bool isOnStartTile = boardTileMap->isP1StartTile(currentTileIndex) || boardTileMap->isP2StartTile(currentTileIndex);
    
    // Check walkability at position
    auto canWalkAtPosition = [&](sf::Vector2f pos, bool isBackward = false) -> bool { 
        sf::FloatRect playerBounds = playerNum->returnSpritesShape().getGlobalBounds(); 
        sf::FloatRect testBounds(pos.x - playerBounds.width * 0.3f, pos.y - playerBounds.height * 0.2f, 
                                playerBounds.width * 0.6f, playerBounds.height * 0.4f); 

        int collisionCount = 0; 
        for (int i = 0; i < Constants::BOARDTILES_ROW * Constants::BOARDTILES_COL; ++i) { 
            try { 
                auto& tile = boardTileMap->getTile(i); 
                if (tile && tile->getVisibleState() && !tile->getWalkable() && 
                    tile->getTileSprite().getGlobalBounds().intersects(testBounds)) collisionCount++;
            } catch (const std::exception& e) { 
                log_warning("Tile collision check error: " + std::string(e.what())); 
            } 
        } 

        if (isBackward) { 
            sf::FloatRect currentBounds(currentPos.x - playerBounds.width * 0.3f, currentPos.y - playerBounds.height * 0.2f, 
                                       playerBounds.width * 0.6f, playerBounds.height * 0.4f); 
            int currentCollisions = 0; 
            for (int i = 0; i < Constants::BOARDTILES_ROW * Constants::BOARDTILES_COL; ++i) { 
                try { 
                    auto& tile = boardTileMap->getTile(i); 
                    if (tile && tile->getVisibleState() && !tile->getWalkable() && 
                        tile->getTileSprite().getGlobalBounds().intersects(currentBounds)) currentCollisions++;
                } catch (const std::exception& e) { 
                    log_warning("Current collision check error: " + std::string(e.what())); 
                } 
            } 
            return collisionCount <= currentCollisions; 
        } 
        return collisionCount == 0; 
    }; 

    // Get max tiles in direction
    auto getMaxTilesInDirection = [&](unsigned int fromTileIndex, int direction, bool isSpecialMove = false) -> int { 
        int col = fromTileIndex % Constants::BOARDTILES_COL; 
        if ((direction == 1 && (boardTileMap->isP1StartTile(fromTileIndex) || (col + 1 < Constants::BOARDTILES_COL && boardTileMap->isP2StartTile(fromTileIndex + 1)))) || 
            (direction == 3 && ((col > 0 && boardTileMap->isP1StartTile(fromTileIndex - 1)) || boardTileMap->isP2StartTile(fromTileIndex)))) return 1; 

        int baseTiles;
        switch (direction) { 
            case 0: case 2: baseTiles = 2; break;
            case 1: baseTiles = (col >= Constants::BOARDTILES_COL - 1) ? 0 : ((col == 0 || col == Constants::BOARDTILES_COL - 2) ? 1 : 2); break;
            case 3: baseTiles = (col == 0) ? 0 : ((col == 1 || col == Constants::BOARDTILES_COL - 1) ? 1 : 2); break;
            default: baseTiles = 0; break;
        } 
        return isSpecialMove ? (baseTiles * 2) : baseTiles;
    }; 

    // Get adjacent tile index
    auto getAdjacentTileIndex = [&](unsigned int fromIndex, int direction, int distance) -> int { 
        int row = fromIndex / Constants::BOARDTILES_COL, col = fromIndex % Constants::BOARDTILES_COL; 
        switch (direction) { 
            case 0: return (row >= distance) ? (row - distance) * Constants::BOARDTILES_COL + col : -1; 
            case 1: return (col + distance < Constants::BOARDTILES_COL) ? row * Constants::BOARDTILES_COL + (col + distance) : -1; 
            case 2: return (row + distance < Constants::BOARDTILES_ROW) ? (row + distance) * Constants::BOARDTILES_COL + col : -1; 
            case 3: return (col >= distance) ? row * Constants::BOARDTILES_COL + (col - distance) : -1; 
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

    // Get movement direction from rotation
    auto getMovementDirection = [&](bool forward) -> int { 
        float angle = playerNum->returnSpritesShape().getRotation(); 
        while (angle < 0) angle += 360; 
        while (angle >= 360) angle -= 360; 

        int direction = (angle >= 315 || angle < 45) ? 1 : 
                       (angle >= 45 && angle < 135) ? 2 : 
                       (angle >= 135 && angle < 225) ? 3 : 0;
        return forward ? direction : (direction + 2) % 4; 
    }; 

    // Check if other player is 2 tiles away
    auto isOtherPlayerAt2Tiles = [&](int direction) -> bool {
        if (!playerToCheck) return false;
        unsigned int otherPlayerTileIndex = boardTileMap->getTileIndex(playerToCheck->getSpritePos());
        int expectedTileIndex = getAdjacentTileIndex(currentTileIndex, direction, 2);
        return (expectedTileIndex != -1 && expectedTileIndex == otherPlayerTileIndex);
    };

    // Handle ongoing movement
    if (isMoving) { 
        sf::Vector2f playerPos = playerNum->getSpritePos(); 
        float distance = std::sqrt(std::pow(targetPosition.x - playerPos.x, 2) + std::pow(targetPosition.y - playerPos.y, 2)); 

        if (distance <= Constants::TILE_THRESHOLD) {  // problematic here 
            playerNum->changePosition(targetPosition); 
            playerNum->updatePos(); 
            playerNum->setIsMoving(false); 
            playerNum->setCurrentDirection(-1); 
            
            if (isSpecialMovement && !hasReachedOtherPlayer) playerNum->setHasReachedOtherPlayer(true);
        } else { 
            switch (currentDirection) { 
                case 0: physics::spriteMover(playerNum, physics::moveUp); break; 
                case 1: physics::spriteMover(playerNum, physics::moveRight); break; 
                case 2: physics::spriteMover(playerNum, physics::moveDown); break; 
                case 3: physics::spriteMover(playerNum, physics::moveLeft); break; 
            } 
        } 
        return; 
    } 

    // Handle rotation input - Allow infinite rotation on start tiles
    if ((FlagSystem::flagEvents.aPressed || FlagSystem::flagEvents.dPressed) && 
        (isOnStartTile || !turnInProgress || tilesMovedThisTurn == 0 || (isSpecialMovement && hasReachedOtherPlayer))) { 
        if (FlagSystem::flagEvents.aPressed) playerNum->returnSpritesShape().rotate(-1.0f); 
        if (FlagSystem::flagEvents.dPressed) playerNum->returnSpritesShape().rotate(1.0f); 
        playerNum->setHeadingAngle(playerNum->returnSpritesShape().getRotation()); 
    } 

    // Attempt movement
    auto attemptMovement = [&](bool isForward) { 
        // MODIFIED: Allow infinite movement if on start tile, otherwise apply normal restrictions
        if (!isOnStartTile) {
            // Normal movement restrictions when NOT on start tile
            if (!isSpecialMovement && tilesMovedThisTurn > 0) return;
            if (isSpecialMovement && tilesMovedThisTurn >= 4) return;
            if (isSpecialMovement && hasReachedOtherPlayer && tilesMovedThisTurn >= 4) return;
        }

        // If on start tile, no movement restrictions apply - player can move infinitely
        if (!turnInProgress) playerNum->setTurnInProgress(true); 
        int direction = getMovementDirection(isForward); 
        int movementDistance = 2;
        
        // Determine special movement (only if not on start tile to avoid interference)
        if (!isOnStartTile && !isSpecialMovement && tilesMovedThisTurn == 0 && isOtherPlayerAt2Tiles(direction)) playerNum->setIsSpecialMovement(true);
        else if (!isOnStartTile && isSpecialMovement && !hasReachedOtherPlayer) return;

        int maxTiles = getMaxTilesInDirection(currentTileIndex, direction, isSpecialMovement);
        if (maxTiles <= 0) return;
        
        movementDistance = std::min(movementDistance, maxTiles);

        // Find furthest valid position
        int validDistance = 0; 
        for (int dist = 1; dist <= movementDistance; ++dist) { 
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
            
            if (!isOnStartTile) playerNum->setTilesMovedThisTurn(tilesMovedThisTurn + validDistance); 
        } 
    }; 

    // Trigger movement
    if (FlagSystem::flagEvents.wPressed) attemptMovement(true);
    if (FlagSystem::flagEvents.sPressed) attemptMovement(false);

    // Apply screen boundaries
    sf::Vector2f finalPos = playerNum->getSpritePos(); 
    sf::FloatRect bounds = playerNum->returnSpritesShape().getGlobalBounds(); 
    playerNum->changePosition(sf::Vector2f{
        std::clamp(finalPos.x, bounds.width / 2, Constants::VIEW_SIZE_X - bounds.width / 2), 
        std::clamp(finalPos.y, bounds.height / 2, Constants::VIEW_SIZE_Y - bounds.height / 2) 
    }); 
    playerNum->updatePos(); 

    // Complete turn logic - MODIFIED: Only end turn if not on start tile
    unsigned int newTileIndex = boardTileMap->getTileIndex(playerNum->getSpritePos()); 
    bool shouldEndTurn = (!isSpecialMovement) || (isSpecialMovement && hasReachedOtherPlayer && tilesMovedThisTurn >= 4);
    // std::cout  << "new tile index: " << newTileIndex << std::endl;
    // std::cout << "previous tile index: " << prevPathIndex << std::endl;     
    // std::cout << "is moving: " << isMoving << std::endl;
    // std::cout << "should end turn: " << shouldEndTurn << std::endl;
    // std::cout << "is on start tile: " << isOnStartTile << std::endl;

    // MODIFIED: Don't end turn if still on start tile, allow infinite turns
    if (newTileIndex != prevPathIndex && !isMoving && shouldEndTurn && !isOnStartTile) { 
        FlagSystem::gameScene1Flags.moved = true; 
        moveCount = 0; 
        playerNum->setTurnInProgress(false); 
        playerNum->setTilesMovedThisTurn(0); 
        playerNum->setIsSpecialMovement(false);
        playerNum->setHasReachedOtherPlayer(false);
        prevPathIndex = newTileIndex; 
    } 
}

void gamePlayScene::handleGameEvents() { 
    player1Text->updateText(Constants::PLAYER1TEXT_MESSAGE + " " + std::to_string(Constants::STICKS_NUMBER / 2 - stickIndexRed) + "/" + std::to_string(Constants::STICKS_NUMBER / 2));
    player2Text->updateText(Constants::PLAYER2TEXT_MESSAGE + " " + std::to_string(Constants::STICKS_NUMBER / 2 - stickIndexBlue) + "/" + std::to_string(Constants::STICKS_NUMBER / 2));

    physics::calculateRayCast3d(player, boardTileMap, rays, wallLine); // board specific
    physics::calculateRayCast3d(player2, boardTileMap, rays2, wallLine2); // board specific

    pawn2->updateSpritePos(player2->getSpritePos()); 
    pawn->updateSpritePos(player->getSpritePos());

    physics::calculateSprite3D(pawn2, player, boardTileMap, pawnRedBlocked);
    physics::calculateSprite3D(pawn, player2, boardTileMap, pawnBlueBlocked); 

    backgroundBigHalfRed->setVisibleState(pawnRedBlocked);
    backgroundBigHalfBlue->setVisibleState(pawnBlueBlocked);

    // check which players turn
    if(FlagSystem::gameScene1Flags.playerRedTurn) {
        introText->updateText("BLUE turn");
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
            backgroundBigFinal->setVisibleState(true); // show final background
            backgroundBig->setVisibleState(false); // hide initial background
            introText->updateText("Player blue wins!"); 

            endingText->setVisibleState(true); 
        }
    }
    else if(FlagSystem::gameScene1Flags.playerBlueTurn) {
        introText->updateText("RED turn");
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
            backgroundBigFinal->setVisibleState(true); // show final background
            backgroundBig->setVisibleState(false); // hide initial background
            introText->updateText("Player red wins!");

            endingText->setVisibleState(true);
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

    if(pawnRedBlocked){
        drawVisibleObject(pawn2);
        drawVisibleObject(backgroundBigHalfRed);
        window.draw(wallLine);
    } else {
        window.draw(wallLine);
        drawVisibleObject(pawn2);
    }
    drawVisibleObject(endingText);
}

void gamePlayScene::drawInmiddleView(){
    window.setView(MetaComponents::middleView);

    drawVisibleObject(boardTileMap); 

    for(const auto& stick : sticksBlue) drawVisibleObject(stick);
    for(const auto& stick : sticksRed) drawVisibleObject(stick);

    drawVisibleObject(player);
    drawVisibleObject(player2);

    drawVisibleObject(player1Text);
    drawVisibleObject(player2Text);
    drawVisibleObject(introText);

}

void gamePlayScene::drawInRightView(){
    window.setView(MetaComponents::rightView);
    
    drawVisibleObject(backgroundBig);
    drawVisibleObject(backgroundBigFinal);

  //  drawVisibleObject(button1); 

    if(pawnBlueBlocked){
        drawVisibleObject(pawn);
        drawVisibleObject(backgroundBigHalfBlue);
        window.draw(wallLine2);
    } else {
        window.draw(wallLine2);
        drawVisibleObject(pawn); 
    }

    drawVisibleObject(endingText);

}
