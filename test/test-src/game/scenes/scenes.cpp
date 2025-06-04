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
    handleGameFlags();
    handleSceneFlags();

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
    // re-play background music

    // set sprite movestates to true
   
    // re-set sprite and text positions 

    // clear respawn time vectors or any other unecessary vectors 

    // re-set flagEvents
    sceneEvents.resetFlags(); 
}

// Handles events from flagEvents
void Scene::handleGameFlags(){
    // if flagEvents.gameEnd is true or some event ... do somthing 
    if(FlagSystem::flagEvents.gameEnd){
       
    }
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
        
        // Animated sprites
        player = std::make_unique<Player>(Constants::SPRITE1_POSITION, Constants::SPRITE1_SCALE, Constants::SPRITE1_TEXTURE, Constants::SPRITE1_SPEED, Constants::SPRITE1_ACCELERATION, 
                                          Constants::SPRITE1_ANIMATIONRECTS, Constants::SPRITE1_INDEXMAX, utils::convertToWeakPtrVector(Constants::SPRITE1_BITMASK));
        player->setRects(0); 
        
        pawn1 = std::make_unique<Sprite>(Constants::PAWN1_POSITION, Constants::PAWN1_SCALE, Constants::PAWN1_TEXTURE);
        pawn2 = std::make_unique<Sprite>(Constants::PAWN2_POSITION, Constants::PAWN2_SCALE, Constants::PAWN2_TEXTURE);
        
        for(int i = 0; i < Constants::STICKS_NUMBER; ++i) {
            sticks[i] = std::make_unique<Sprite>(Constants::STICK_POSITIONS[i], Constants::STICK_SCALE, Constants::STICK_TEXTURE);
        }

        board = std::make_unique<Sprite>(Constants::BOARD_POSITION, Constants::BOARD_SCALE, Constants::BOARD_TEXTURE); 
        backgroundBig = std::make_unique<Sprite>(Constants::BACKGROUNDBIG_POSITION, Constants::BACKGROUNDBIG_SCALE, Constants::BACKGROUNDBIG_TEXTURE); 

        backgroundBigFinal = std::make_unique<Sprite>(Constants::BACKGROUNDBIGFINAL_POSITION, Constants::BACKGROUNDBIGFINAL_SCALE, Constants::BACKGROUNDBIGFINAL_TEXTURE); 
        
        button1 = std::make_unique<Button>(Constants::BUTTON1_POSITION, Constants::BUTTON1_SCALE, Constants::BUTTON1_TEXTURE, Constants::BUTTON1_ANIMATIONRECTS, Constants::BUTTON1_INDEXMAX, utils::convertToWeakPtrVector(Constants::BUTTON1_BITMASK)); 
        button1->setRects(0); 

        bullets.push_back(std::make_unique<Bullet>(Constants::BULLET_STARTINGPOS, Constants::BULLET_STARTINGSCALE, Constants::BULLET_TEXTURE, Constants::BULLET_INITIALSPEED, Constants::BULLET_ACCELERATION, 
                                                   Constants::BULLET_ANIMATIONRECTS, Constants::BULLET_INDEXMAX,  utils::convertToWeakPtrVector(Constants::BULLET_BITMASK)));
        bullets[0]->setRects(0);

        // Tiles and tilemap
        for (int i = 0; i < Constants::TILES_NUMBER; ++i) {
            tiles1.at(i) = std::make_shared<Tile>(Constants::TILES_SCALE, Constants::TILES_TEXTURE, Constants::TILES_SINGLE_RECTS[i], Constants::TILES_BITMASKS[i], Constants::TILES_BOOLS[i]); 
        }
       
        tileMap1 = std::make_unique<TileMap>(tiles1.data(), Constants::TILES_NUMBER, Constants::TILEMAP_WIDTH, Constants::TILEMAP_HEIGHT, Constants::TILE_WIDTH, Constants::TILE_HEIGHT, Constants::TILEMAP_FILEPATH, Constants::TILEMAP_POSITION); 
        rays = sf::VertexArray(sf::Lines, Constants::RAYS_NUM);
        rays = sf::VertexArray(sf::Quads, Constants::RAYS_NUM);
   
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
    quadtree.insert(bullets[bullets.size() - 1]); 
}

void gamePlayScene::respawnAssets(){
   
} 

void gamePlayScene::handleInvisibleSprites() {
    
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
    if(!player->getAutoNavigate()) handleMovementKeys();
}

void gamePlayScene::handleMouseClick() {    
    if(FlagSystem::flagEvents.mouseClicked) {
        if(physics::collisionHelper(button1, MetaComponents::leftViewmouseClickedPosition_f)){
            button1->setVisibleState(false); 
            button1->setClickedBool(true);
            FlagSystem::gameScene1Flags.begin = true;
            buttonClickSound->returnSound().play();
        }
    }
}

void gamePlayScene::handleSpaceKey() {
    if(MetaComponents::spacePressedElapsedTime) {
       
    }
}

void gamePlayScene::handleMovementKeys() {
    if(!player->getMoveState()) return;

    int tileX = static_cast<int>((player->getSpritePos().x - Constants::TILEMAP_POSITION.x) / Constants::TILE_WIDTH);
    int tileY = static_cast<int>((player->getSpritePos().y - Constants::TILEMAP_POSITION.y) / Constants::TILE_HEIGHT);
    int tileIndexInMap = tileY * Constants::TILEMAP_WIDTH + tileX;
    bool canWalkOnTile = tileMap1->getTile(tileIndexInMap)->getWalkable(); 

    sf::FloatRect playerBounds = player->returnSpritesShape().getGlobalBounds();
    sf::Vector2f originalPlayerPos = player->getSpritePos();

    // std::cout << "Tile X: " << tileX << ", Tile Y: " << tileY << ", inex: " << tileIndexInMap << "can walk: "<< canWalkOnTile<< std::endl;
    
    if(FlagSystem::flagEvents.aPressed){ // turn left
        player->returnSpritesShape().rotate(-1.0f); // degrees
        float newAngle = player->returnSpritesShape().getRotation();
        player->setHeadingAngle(newAngle);
        FlagSystem::gameScene1Flags.begin = true;
       // player->setAutoNavigate(false); // stop auto navigation
    }
    if(FlagSystem::flagEvents.dPressed){ // turn right 
        player->returnSpritesShape().rotate(1.0f); // degrees
        float newAngle = player->returnSpritesShape().getRotation();
        player->setHeadingAngle(newAngle);
        FlagSystem::gameScene1Flags.begin = true;
       // player->setAutoNavigate(false); // stop auto navigation
    }
    if(FlagSystem::flagEvents.wPressed && canWalkOnTile){ // front 
        physics::spriteMover(player, physics::followDirVec); 
        FlagSystem::gameScene1Flags.begin = true;
       // player->setAutoNavigate(false); // stop auto navigation
    }
    if(FlagSystem::flagEvents.sPressed && canWalkOnTile){ // back
        physics::spriteMover(player, physics::followDirVecOpposite); 
        FlagSystem::gameScene1Flags.begin = true;
       // player->setAutoNavigate(true); // stop auto navigation
    }   

    int newTileX = static_cast<int>((player->getSpritePos().x - Constants::TILEMAP_POSITION.x) / Constants::TILE_WIDTH);
    int newTileY = static_cast<int>((player->getSpritePos().y - Constants::TILEMAP_POSITION.y) / Constants::TILE_HEIGHT);
    int newTileIndexInMap = newTileY * Constants::TILEMAP_WIDTH + newTileX;
    bool canWalkOnTileAgain = tileMap1->getTile(newTileIndexInMap)->getWalkable(); 

    if(!canWalkOnTileAgain){
        player->changePosition(originalPlayerPos);
        player->updatePos(); 
    }
    sf::Vector2f playerPos = player->getSpritePos();
    float spriteWidth = playerBounds.width;
    float spriteHeight = playerBounds.height;

    float newX = std::clamp(playerPos.x, spriteWidth, Constants::VIEW_SIZE_X - spriteWidth);
    float newY = std::clamp(playerPos.y, spriteHeight, Constants::VIEW_SIZE_Y - spriteHeight);

    player->changePosition(sf::Vector2f{newX, newY});
    player->updatePos(); 
}

// Keeps sprites inside screen bounds, checks for collisions, update scores, and sets flagEvents.gameEnd to true in an event of collision 
void gamePlayScene::handleGameEvents() { 
    // scoreText->getText().setPosition(MetaComponents::middleView.getCenter().x - 460, MetaComponents::middleView.getCenter().y - 270);
    if(button1->getClickedBool() && player && player->getMoveState()){
        physics::navigateMaze(player, tileMap1, Constants::TILEPATH_INSTRUCTION);
        player->setAutoNavigate(true); 
    }
    physics::calculateRayCast3d(player, tileMap1, rays, wallLine); // modifies the ray 
} 

void gamePlayScene::handleSceneFlags(){
    if(!FlagSystem::flagEvents.gameEnd && FlagSystem::gameScene1Flags.begin){
        scoreText->getText().setString("Seconds elapsed: " + std::to_string(beginTime));
    }
}

void gamePlayScene::update() {
    try {
        updateEntityStates();
        changeAnimation();
        updateDrawablesVisibility(); 
        handleInvisibleSprites();

        updatePlayerAndView(); 
        quadtree.update(); 

        // Set the view for the window
        window.setView(MetaComponents::middleView);
        
    } catch (const std::exception& e) {
        log_error("Exception in updateSprites: " + std::string(e.what()));
    }
}

void gamePlayScene::updateEntityStates(){ // manually change the sprite's state
   
}

void gamePlayScene::changeAnimation(){ // change animation for sprites. change animation for texts if necessary     if (button1 && button1->getVisibleState()) button1->changeAnimation(); 
   // for (const auto& bullet : bullets) if (bullet) bullet->changeAnimation();
   if (button1) button1->changeAnimation(); 
}

void gamePlayScene::updatePlayerAndView() {
    
}

void gamePlayScene::updateDrawablesVisibility(){
    try{
        if(MetaComponents::globalTime > 3.0) introText->setVisibleState(false);
        button1->setVisibleState(!FlagSystem::flagEvents.mPressed && !player->getAutoNavigate());
    }
    catch(const std::exception & e){
        log_error("Exception in updateDrawablesVisibility: " + std::string(e.what()));
    }
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

    int tileX = static_cast<int>((player->getSpritePos().x - Constants::TILEMAP_POSITION.x) / Constants::TILE_WIDTH);
    int tileY = static_cast<int>((player->getSpritePos().y - Constants::TILEMAP_POSITION.y) / Constants::TILE_HEIGHT);
    int tileIndexInMap = tileY * Constants::TILEMAP_WIDTH + tileX;

    if(tileIndexInMap == Constants::TILEMAP_GOALINDEX){ 
        FlagSystem::flagEvents.gameEnd = true;
        drawVisibleObject(backgroundBigFinal);
        drawVisibleObject(endingText);
    } else {
        drawVisibleObject(backgroundBig);
    }
    window.draw(wallLine);

  //  drawVisibleObject(bullets[0]); 
    drawVisibleObject(scoreText); 
    drawVisibleObject(introText);
    drawVisibleObject(button1);
}

void gamePlayScene::drawInmiddleView(){
    window.setView(MetaComponents::middleView);

    drawVisibleObject(player);
    drawVisibleObject(board);

    for(const auto& stick : sticks) drawVisibleObject(stick);

    drawVisibleObject(pawn1);
    drawVisibleObject(pawn2);

}

void gamePlayScene::drawInRightView(){
    window.setView(MetaComponents::rightView);
    
    int tileX = static_cast<int>((player->getSpritePos().x - Constants::TILEMAP_POSITION.x) / Constants::TILE_WIDTH);
    int tileY = static_cast<int>((player->getSpritePos().y - Constants::TILEMAP_POSITION.y) / Constants::TILE_HEIGHT);
    int tileIndexInMap = tileY * Constants::TILEMAP_WIDTH + tileX;

    if(tileIndexInMap == Constants::TILEMAP_GOALINDEX){ 
        FlagSystem::flagEvents.gameEnd = true;
        drawVisibleObject(backgroundBigFinal);
        drawVisibleObject(endingText);
    } else {
        drawVisibleObject(backgroundBig);
    }
    window.draw(wallLine);

  //  drawVisibleObject(bullets[0]); 
    drawVisibleObject(scoreText); 
    drawVisibleObject(introText);

    drawVisibleObject(button1);
}
