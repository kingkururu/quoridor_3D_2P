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

        for(int i = 0; i < Constants::STICKS_NUMBER; ++i) sticks[i] = std::make_unique<Sprite>(Constants::STICK_POSITIONS[i], Constants::STICK_SCALE, Constants::STICK_TEXTURE);

        board = std::make_unique<Sprite>(Constants::BOARD_POSITION, Constants::BOARD_SCALE, Constants::BOARD_TEXTURE); 
        backgroundBig = std::make_unique<Sprite>(Constants::BACKGROUNDBIG_POSITION, Constants::BACKGROUNDBIG_SCALE, Constants::BACKGROUNDBIG_TEXTURE); 

        backgroundBigFinal = std::make_unique<Sprite>(Constants::BACKGROUNDBIGFINAL_POSITION, Constants::BACKGROUNDBIGFINAL_SCALE, Constants::BACKGROUNDBIGFINAL_TEXTURE); 
        
        // button1 = std::make_unique<Button>(Constants::BUTTON1_POSITION, Constants::BUTTON1_SCALE, Constants::BUTTON1_TEXTURE, Constants::BUTTON1_ANIMATIONRECTS, Constants::BUTTON1_INDEXMAX, utils::convertToWeakPtrVector(Constants::BUTTON1_BITMASK)); 
        // button1->setRects(0); 

        // bullets.push_back(std::make_unique<Bullet>(Constants::BULLET_STARTINGPOS, Constants::BULLET_STARTINGSCALE, Constants::BULLET_TEXTURE, Constants::BULLET_INITIALSPEED, Constants::BULLET_ACCELERATION, 
        //                                            Constants::BULLET_ANIMATIONRECTS, Constants::BULLET_INDEXMAX,  utils::convertToWeakPtrVector(Constants::BULLET_BITMASK)));
        // bullets[0]->setRects(0);

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

        boardTileMap = std::make_unique<BoardTileMap>(boardTiles); 

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
    //quadtree.insert(bullets[bullets.size() - 1]); 
}

void gamePlayScene::respawnAssets(){
   
} 

void gamePlayScene::setTime(){
    if(FlagSystem::gameScene1Flags.begin){
        beginTime += MetaComponents::deltaTime;
    }
    if(FlagSystem::flagEvents.spacePressed || MetaComponents::spacePressedElapsedTime) {
        MetaComponents::spacePressedElapsedTime += MetaComponents::deltaTime; 
    } else {
        MetaComponents::spacePressedElapsedTime = 0.0f; 
    }
    
} 

void gamePlayScene::handleInput() {
    handleMouseClick();
    handleSpaceKey(); 
    handleMovementKeys();
}

void gamePlayScene::handleMouseClick() {    
    if(stickIndex < Constants::STICKS_NUMBER) sticks[stickIndex]->returnSpritesShape().setPosition(MetaComponents::middleViewmouseCurrentPosition_f);
    if(FlagSystem::flagEvents.mouseClicked) ++stickIndex;
}

void gamePlayScene::handleSpaceKey() {
    if(MetaComponents::spacePressedElapsedTime) {
       
    }
}

void gamePlayScene::handleMovementKeys() {
    handleEachPlayer(player);
    handleEachPlayer(player2);
}

void gamePlayScene::handleEachPlayer(std::unique_ptr<Player>& playerNum) {

    if(!playerNum || !playerNum->getMoveState()) return;
    
    // Get player's current position and bounds
    sf::Vector2f playerPos = playerNum->getSpritePos();
    sf::FloatRect playerBounds = playerNum->returnSpritesShape().getGlobalBounds();
    sf::Vector2f originalPlayerPos = playerPos;

    // Function to check if player can walk at a given position
    auto canWalkAtPosition = [&](sf::Vector2f pos) -> bool {
        // Custom player collision box (smaller than full sprite)
        float collisionWidth = playerBounds.width * 0.6f;
        float collisionHeight = playerBounds.height * 0.4f;

        sf::FloatRect testPlayerBounds(
            pos.x - collisionWidth / 2.f,
            pos.y - collisionHeight / 2.f,
            collisionWidth,
            collisionHeight
        );

        // Check collision with all tiles in the BoardTileMap
        for (int i = 0; i < 23 * 21; ++i) { // num or rows and columns from boardTileMap
            try {
                auto& tile = boardTileMap->getTile(i);
                if (tile && tile->getVisibleState()) {
                    sf::FloatRect tileBounds = tile->getTileSprite().getGlobalBounds();

                    // If player intersects with a non-walkable tile, return false
                    if (tileBounds.intersects(testPlayerBounds) && !tile->getWalkable()) return false;
                }
            } catch (const std::exception& e) {
                log_warning("Error checking tile collision: " + std::string(e.what()));
            }
        }
        return true;
    };
        
    // Check if current position is valid
    bool canWalkOnCurrentTile = canWalkAtPosition(playerPos);
    
    // Handle player input
    if(FlagSystem::flagEvents.aPressed) { // turn left
        playerNum->returnSpritesShape().rotate(-1.0f); // degrees
        float newAngle = playerNum->returnSpritesShape().getRotation();
        playerNum->setHeadingAngle(newAngle);
       // FlagSystem::gameScene1Flags.begin = true;
    }
    
    if(FlagSystem::flagEvents.dPressed) { // turn right
        playerNum->returnSpritesShape().rotate(1.0f); // degrees
        float newAngle = playerNum->returnSpritesShape().getRotation();
        playerNum->setHeadingAngle(newAngle);
       // FlagSystem::gameScene1Flags.begin = true;
    }
    
    // Store position before movement
    sf::Vector2f positionBeforeMovement = playerNum->getSpritePos();
    
    if(FlagSystem::flagEvents.wPressed && canWalkOnCurrentTile) { // forward
        physics::spriteMover(playerNum, physics::followDirVec);
        // FlagSystem::gameScene1Flags.begin = true;
    }
    
    if(FlagSystem::flagEvents.sPressed && canWalkOnCurrentTile) { // backward
        physics::spriteMover(playerNum, physics::followDirVecOpposite);
       // FlagSystem::gameScene1Flags.begin = true;
    }
    
    // Check if new position after movement is valid
    sf::Vector2f newPlayerPos = playerNum->getSpritePos();
    bool canWalkAtNewPosition = canWalkAtPosition(newPlayerPos);
    
    // If new position is invalid, revert to original position
    if(!canWalkAtNewPosition) {
        playerNum->changePosition(originalPlayerPos);
        playerNum->updatePos();
    }
    
    // Apply screen boundary constraints
    sf::Vector2f finalPlayerPos = playerNum->getSpritePos();
    sf::FloatRect finalPlayerBounds = playerNum->returnSpritesShape().getGlobalBounds();
    
    float newX = std::clamp(finalPlayerPos.x, 
                           finalPlayerBounds.width / 2, 
                           Constants::VIEW_SIZE_X - finalPlayerBounds.width / 2);
    float newY = std::clamp(finalPlayerPos.y, 
                           finalPlayerBounds.height / 2, 
                           Constants::VIEW_SIZE_Y - finalPlayerBounds.height / 2);
    
    playerNum->changePosition(sf::Vector2f{newX, newY});
    playerNum->updatePos();
}

// Keeps sprites inside screen bounds, checks for collisions, update scores, and sets flagEvents.gameEnd to true in an event of collision 
void gamePlayScene::handleGameEvents() { 
    physics::calculateRayCast3d(player, boardTileMap, rays, wallLine); // board specific
    physics::calculateRayCast3d(player2, boardTileMap, rays2, wallLine2); // board specific

                     boardTileMap->getTile(44)->setWalkable(false); // temporary for testing

    // check which players turn
    if(FlagSystem::gameScene1Flags.player1turn) {
        player2->setMoveState(false); // player 2 cannot move
    }
    else if(FlagSystem::gameScene1Flags.player2turn) {
        player->setMoveState(false); // player 1 cannot move
    }

    // check move made (moved path or placed stick)
    if(moved || stickPlaced){

    }
    
    // check if player reached goal tile

    // victory
        // FlagSystem::flagEvents.gameEnd to true if player reached goal tile
        // score? 
} 

void gamePlayScene::update() {
    try {
        changeAnimation();
        quadtree.update(); 

        // Set the view for the window
        window.setView(MetaComponents::middleView);
        
    } catch (const std::exception& e) {
        log_error("Exception in updateSprites: " + std::string(e.what()));
    }
}

void gamePlayScene::changeAnimation(){ // change animation for sprites. change animation for texts if necessary     if (button1 && button1->getVisibleState()) button1->changeAnimation(); 
   // for (const auto& bullet : bullets) if (bullet) bullet->changeAnimation();
   // if (button1) button1->changeAnimation(); 
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

    window.draw(wallLine);

    drawVisibleObject(scoreText); 
    drawVisibleObject(introText);
}

void gamePlayScene::drawInmiddleView(){
    window.setView(MetaComponents::middleView);

    // drawVisibleObject(board);
    drawVisibleObject(boardTileMap); // temporary

    for(const auto& stick : sticks) drawVisibleObject(stick);

    drawVisibleObject(player);
    window.draw(rays); // direct sf object

    drawVisibleObject(player2);
    window.draw(rays2); 
}

void gamePlayScene::drawInRightView(){
    window.setView(MetaComponents::rightView);
    
    drawVisibleObject(backgroundBig);

    window.draw(wallLine2);

  //  drawVisibleObject(bullets[0]); 
}
