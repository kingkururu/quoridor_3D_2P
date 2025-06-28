#pragma once

#include <iostream>
#include <stdexcept>

#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include <array>

#include "../test-assets/sprites/sprites.hpp"  
#include "../test-assets/sound/sound.hpp"      
#include "../test-assets/fonts/fonts.hpp"      
#include "../test-assets/tiles/tiles.hpp"      

#include "../globals/globals.hpp"              
#include "../physics/physics.hpp"             
#include "../utils/utils.hpp"                 

#include "../test-logging/log.hpp"              

// Base scene class 
class Scene {
public:
  Scene( sf::RenderWindow& gameWindow );
  virtual ~Scene() = default; 

  // base functions inside scene
  void runScene();  
  virtual void createAssets(){}; 

protected:
  sf::RenderWindow& window; // from game.hpp
  FlagSystem::SceneEvents sceneEvents; // scene's own flag events

  // blank templates here
  virtual void setInitialTimes(){};
  virtual void insertItemsInQuadtree(){}; 

  virtual void setTime(){}; 

  virtual void handleInput(){};

  virtual void respawnAssets(){}; 

  virtual void handleGameEvents(){};

  virtual void update(){};
  virtual void draw(); 
  virtual void moveViewPortWASD();

  void restartScene();

  physics::Quadtree quadtree; 
};

// in use (the main scene in test game)
class gamePlayScene : public virtual Scene{
public:
  using Scene::Scene; 
  ~gamePlayScene() override = default; 
 
  void createAssets() override; 

private:
  void setInitialTimes() override;
  void insertItemsInQuadtree() override; 

  void handleInput() override; 
  void handleMouseKey(); 
  void handleSpaceKey();
  void handleMovementKeys(); 
  void handleEachPlayer(std::unique_ptr<Player>& playerNum, size_t& moveCount, unsigned int& prevPathIndex);

  void respawnAssets() override; 

  void setTime() override;

  void handleGameEvents() override; 

  void update() override; 
  void changeAnimation();

  void draw() override; 
  void drawInleftView();
  void drawInmiddleView();
  void drawInRightView();

  template<typename drawableType>
  void drawVisibleObject(drawableType& drawable){ if (drawable && drawable->getVisibleState()) window.draw(*drawable); }

  std::unique_ptr<Player> player; 
  std::unique_ptr<Player> player2; 

  // std::vector<std::unique_ptr<Bullet>> bullets; // not in use 
  //std::unique_ptr<Button>button1; // not in use

  std::unique_ptr<Sprite> pawnBlue; // for 3D rendering
  std::unique_ptr<Sprite> pawnRed;
  std::unique_ptr<Sprite> backgroundBig; 
  std::unique_ptr<Sprite> backgroundBigFinal; 

  std::array<std::unique_ptr<Sprite>, Constants::STICKS_NUMBER> sticks;
  
  std::array<std::shared_ptr<Tile>, 11> boardTiles;
  std::unique_ptr<BoardTileMap> boardTileMap; // for the board with walls and goals

  // for 3d walls
  sf::VertexArray rays; // player 1
  sf::VertexArray wallLine; // player 2
  sf::VertexArray rays2; // player 2
  sf::VertexArray wallLine2; // player 2

  std::unique_ptr<MusicClass> backgroundMusic;
  std::unique_ptr<SoundClass> buttonClickSound; 

  std::unique_ptr<TextClass> introText; 
  std::unique_ptr<TextClass> scoreText; 
  std::unique_ptr<TextClass> endingText; 

  float beginTime{};

  unsigned int stickIndex{}; // even is player 2 odd is player 1

  size_t p1pathCount{};
  unsigned int p1PrevPathIndex{};
  size_t p2pathCount{};
  unsigned int p2PrevPathIndex{};
};
