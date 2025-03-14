#include "scenes.hpp"

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
// Base scene functions  
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

// Scene constructure sets up window and sprite respawn times 
Scene::Scene( sf::RenderWindow& gameWindow ) : window(gameWindow), quadtree(0.0f, 0.0f, Constants::WORLD_WIDTH, Constants::WORLD_HEIGHT){ 
    MetaComponents::smallView = sf::View(Constants::VIEW_RECT); 
    MetaComponents::smallView.setViewport(sf::FloatRect(0.75f, 0.f, 0.25f, 0.25f));

    MetaComponents::bigView = sf::View(sf::FloatRect(0, 0, Constants::WORLD_WIDTH, Constants::WORLD_HEIGHT)); 
    MetaComponents::bigView.setViewport(sf::FloatRect(0.0f, 0.f, 1.0f, 1.0f)); 

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
        MetaComponents::smallView.move(sf::Vector2f(-10, 0)); 
    }
    if(FlagSystem::flagEvents.dPressed){
        MetaComponents::smallView.move(sf::Vector2f(10, 0)); 
    }
    if(FlagSystem::flagEvents.sPressed){
        MetaComponents::smallView.move(sf::Vector2f(0, 10)); 
    }
    if(FlagSystem::flagEvents.wPressed){
        MetaComponents::smallView.move(sf::Vector2f(0, -10)); 
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
        
        frame = std::make_unique<Sprite>(Constants::FRAME_POSITION, Constants::FRAME_SCALE, Constants::FRAME_TEXTURE); 
        backgroundBig = std::make_unique<Sprite>(Constants::BACKGROUNDBIG_POSITION, Constants::BACKGROUNDBIG_SCALE, Constants::BACKGROUNDBIG_TEXTURE); 
        // button1 = std::make_unique<Button>(Constants::BUTTON1_POSITION, Constants::BUTTON1_SCALE, Constants::BUTTON1_TEXTURE, 
        //                            Constants::BUTTON1_ANIMATIONRECTS, Constants::BUTTON1_INDEXMAX, utils::convertToWeakPtrVector(Constants::BUTTON1_BITMASK));
        // button1->setRects(0); 
        // button1->setVisibleState(false); 
        backgroundBigFinal = std::make_unique<Sprite>(Constants::BACKGROUNDBIGFINAL_POSITION, Constants::BACKGROUNDBIGFINAL_SCALE, Constants::BACKGROUNDBIGFINAL_TEXTURE); 
        backgroundBigStart = std::make_unique<Sprite>(Constants::BACKGROUNDBIGSTART_POSITION, Constants::BACKGROUNDBIGSTART_SCALE, Constants::BACKGROUNDBIGSTART_TEXTURE);

        enemy = std::make_unique<Obstacle>(Constants::ENEMY_POSITION, Constants::ENEMY_SCALE, Constants::ENEMY_TEXTURE, Constants::ENEMY_SPEED, Constants::ENEMY_ACCELERATION, Constants::ENEMY_ANIMATIONRECTS, Constants::ENEMY_INDEXMAX, utils::convertToWeakPtrVector(Constants::ENEMY_BITMASK)); 
         
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

        // // Sound
        // playerJumpSound = std::make_unique<SoundClass>(Constants::PLAYERJUMP_SOUNDBUFF, Constants::PLAYERJUMPSOUND_VOLUME); 
        // coinHitSound = std::make_unique<SoundClass>(Constants::COINHIT_SOUNDBUFF, Constants::COINHITSOUND_VOLUME); 
        // buttonClickSound = std::make_unique<SoundClass>(Constants::BUTTONCLICK_SOUNDBUFF, Constants::BUTTONCLICKSOUND_VOLUME);

        // Text
        introText = std::make_unique<TextClass>(Constants::TEXT_POSITION, Constants::TEXT_SIZE, Constants::TEXT_COLOR, Constants::TEXT_FONT, Constants::TEXT_MESSAGE);
        scoreText = std::make_unique<TextClass>(Constants::SCORETEXT_POSITION, Constants::SCORETEXT_SIZE, Constants::SCORETEXT_COLOR, Constants::TEXT_FONT, Constants::SCORETEXT_MESSAGE);
        // endingText = std::make_unique<TextClass>(Constants::ENDINGTEXmake T_POSITION, Constants::ENDINGTEXT_SIZE, Constants::ENDINGTEXT_COLOR, Constants::TEXT_FONT, Constants::ENDINGTEXT_MESSAGE);
        // endingText->setVisibleState(false);

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
    quadtree.insert(enemy); 
}

void gamePlayScene::respawnAssets(){
   
} 

void gamePlayScene::handleInvisibleSprites() {
    
}

void gamePlayScene::setTime(){
    // count respawn time here 
    if (FlagSystem::flagEvents.spacePressed || MetaComponents::spacePressedElapsedTime) {
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
    if (FlagSystem::flagEvents.mouseClicked) {
       
    }
}

void gamePlayScene::handleSpaceKey() {
    if (MetaComponents::spacePressedElapsedTime) {
       
    }
}

void gamePlayScene::handleMovementKeys() {
    if (!player->getMoveState()) return;

    int tileX = static_cast<int>((player->getSpritePos().x - Constants::TILEMAP_POSITION.x) / Constants::TILE_WIDTH);
    int tileY = static_cast<int>((player->getSpritePos().y - Constants::TILEMAP_POSITION.y) / Constants::TILE_HEIGHT);
    int tileIndexInMap = tileY * Constants::TILEMAP_WIDTH + tileX;
    bool canWalkOnTile = tileMap1->getTile(tileIndexInMap)->getWalkable(); 

    sf::FloatRect playerBounds = player->returnSpritesShape().getGlobalBounds();
    sf::Vector2f originalPlayerPos = player->getSpritePos();

    // std::cout << "Tile X: " << tileX << ", Tile Y: " << tileY << ", inex: " << tileIndexInMap << "can walk: "<< canWalkOnTile<< std::endl;
    
    if (FlagSystem::flagEvents.aPressed){ // turn left
        player->returnSpritesShape().rotate(-1.0f); // degrees
        float newAngle = player->returnSpritesShape().getRotation();
        player->setHeadingAngle(newAngle);
    }
    if (FlagSystem::flagEvents.dPressed){ // turn right 
        player->returnSpritesShape().rotate(1.0f); // degrees
        float newAngle = player->returnSpritesShape().getRotation();
        player->setHeadingAngle(newAngle);
    }
    if (FlagSystem::flagEvents.wPressed && canWalkOnTile){ // front 
        physics::spriteMover(player, physics::followDirVec); 
    }
    if (FlagSystem::flagEvents.sPressed && canWalkOnTile){ // back
        physics::spriteMover(player, physics::followDirVecOpposite); 
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
    // scoreText->getText().setPosition(MetaComponents::smallView.getCenter().x - 460, MetaComponents::smallView.getCenter().y - 270);

    physics::calculateRayCast3d(player, tileMap1, rays, wallLine); // modifies the ray 
   // physics::calculateRayCast3d(player, enemy, &quadtree); 
    // physics::calculateRayCast3d(player, enemy); 

} 

void gamePlayScene::handleSceneFlags(){
   if(!FlagSystem::flagEvents.gameEnd){
        scoreText->getText().setString("Seconds elapsed: " + std::to_string(MetaComponents::globalTime));
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
        window.setView(MetaComponents::smallView);
        
    } catch (const std::exception& e) {
        log_error("Exception in updateSprites: " + std::string(e.what()));
    }
}

void gamePlayScene::updateEntityStates(){ // manually change the sprite's state
   
}

void gamePlayScene::changeAnimation(){ // change animation for sprites. change animation for texts if necessary     if (button1 && button1->getVisibleState()) button1->changeAnimation(); 
   // for (const auto& bullet : bullets) if (bullet) bullet->changeAnimation();
   if (enemy) enemy->changeAnimation(); 
}

void gamePlayScene::updatePlayerAndView() {

}

void gamePlayScene::updateDrawablesVisibility(){
    try{
        if(MetaComponents::globalTime > 3.0) introText->setVisibleState(false);
    }
    catch(const std::exception & e){
        log_error("Exception in updateDrawablesVisibility: " + std::string(e.what()));
    }
}

// Draws only the visible sprite and texts
void gamePlayScene::draw() {
    try {
        window.clear(sf::Color::Black); // set the base baskground color black

        drawInBigView();
        drawInSmallView();

        window.display(); 
    } 
    catch (const std::exception& e) {
        log_error("Exception in draw: " + std::string(e.what()));
    }
}

void gamePlayScene::drawInBigView(){
    window.setView(MetaComponents::bigView);

    int tileX = static_cast<int>((player->getSpritePos().x - Constants::TILEMAP_POSITION.x) / Constants::TILE_WIDTH);
    int tileY = static_cast<int>((player->getSpritePos().y - Constants::TILEMAP_POSITION.y) / Constants::TILE_HEIGHT);
    int tileIndexInMap = tileY * Constants::TILEMAP_WIDTH + tileX;

    if(tileIndexInMap == 20){ // start tile
        drawVisibleObject(backgroundBigStart);
    } else if (tileIndexInMap == 226){ // end tile
        FlagSystem::flagEvents.gameEnd = true;
        drawVisibleObject(backgroundBigFinal);
    } else {
        drawVisibleObject(backgroundBig);
    }

    window.draw(wallLine);

  //  drawVisibleObject(bullets[0]); 
    drawVisibleObject(frame); 
    drawVisibleObject(scoreText); 
    drawVisibleObject(introText);

}

void gamePlayScene::drawInSmallView(){
    window.setView(MetaComponents::smallView);

    // temporary 
    sf::RectangleShape mainRect(sf::Vector2f(Constants::VIEW_SIZE_X, Constants::VIEW_SIZE_Y));
    mainRect.setFillColor(sf::Color::Magenta); // background for small view
    mainRect.setPosition(0,0);

    window.draw(mainRect);

    drawVisibleObject(tileMap1);
    drawVisibleObject(player);

    window.draw(rays); 
}

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
// Game Scene #2 from down below 
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void gamePlayScene2::createAssets() {
    try {
        
    } 
    catch (const std::exception& e) {
        log_error("Exception in createAssets: " + std::string(e.what()));
    }
}

void gamePlayScene2::handleInput() {
    moveViewPortWASD(); // change position of the view port based on keyboard input flags
}

void gamePlayScene2::draw() {
    try {
        window.clear(); // clear elements from previous screen 
        
        window.display(); 
    } 
    catch (const std::exception& e) {
        log_error("Exception in gamePlayScene2 draw: " + std::string(e.what()));
    }
}

void gamePlayScene2::update() {
    try {
        handleInvisibleSprites(); // do a sprite pooling or actually delete all

        window.setView(MetaComponents::smallView); 
    }
    catch (const std::exception& e) {
        log_error("Exception in updateSprites: " + std::string(e.what()));
    }
}
