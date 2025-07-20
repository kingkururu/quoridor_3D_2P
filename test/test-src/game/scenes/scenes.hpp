#pragma once

#include <iostream>
#include <stdexcept>

#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include <array>

#include "../test-assets/sound/sound.hpp"      
#include "../test-assets/fonts/fonts.hpp"      

#include "../physics/physics.hpp"             
#include "../utils/utils.hpp"                 
#include "../camera/window.hpp"

// Base scene class 
class Scene {
public:
  Scene(sf::RenderWindow& gameWindow) : window(gameWindow), quadtree(0.0f, 0.0f, Constants::WORLD_WIDTH, Constants::WORLD_HEIGHT) {}
  virtual ~Scene() = default; 

  // base functions inside scene
  void runScene();  
  virtual void createAssets(){}; 

protected:
  sf::RenderWindow& window; // from game.hpp
  FlagSystem::SceneEvents sceneEvents; // scene's own flag events

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

  template<typename drawableType>
  void drawVisibleObject(drawableType& drawable){ if (drawable && drawable->getVisibleState()) window.draw(*drawable); }

  physics::Quadtree quadtree; 
};

class lobbyScene : public virtual Scene {
public:
  lobbyScene(sf::RenderWindow& gameWindow);
  void createAssets() override;   

private:
  void setTime() override;
  void handleInput() override;
  void handleGameEvents() override;

  void respawnAssets() override{}; // no respawn in lobby scene

  void update() override;
  void draw() override;

  std::unique_ptr<Sprite> background;
  std::unique_ptr<Button> button; // 1 comp button
  std::unique_ptr<Button> button2; // 2 comp button

  std::unique_ptr<TextClass> titleText;
  std::unique_ptr<TextClass> hostCodeText;
};

class lobby2Scene : public virtual Scene {
public:
  lobby2Scene(sf::RenderWindow& gameWindow);
  void createAssets() override;   

private:
  void setTime() override;
  void handleInput() override;
  void handleGameEvents() override;

  void respawnAssets() override{}; // no respawn in lobby scene

  void update() override;
  void draw() override;

  std::unique_ptr<Sprite> background;
  std::unique_ptr<Button> hostButton; 
  std::unique_ptr<Button> joinButton; 

  std::unique_ptr<TextClass> hostIPText;
  std::unique_ptr<TextClass> joinCodeText;
};

// in use (the main scene in test game)
class gamePlayScene : public virtual Scene{
public:
  gamePlayScene(sf::RenderWindow& gameWindow);
  void createAssets() override; 

private:
  void setInitialTimes() override;
  void insertItemsInQuadtree() override; 

  void handleInput() override; 
  //bool playerHasExit(const std::unique_ptr<Player>& currentPlayer, bool isPlayer1) const;
  void handleMouseKey(); 
  void handleSpaceKey();
  void handleMovementKeys(); 
  void handleEachPlayer(std::unique_ptr<Player>& playerNum, std::unique_ptr<Player>& playerToCheck, size_t& moveCount, unsigned int& prevPathIndex);

  void respawnAssets() override; 

  void setTime() override;

  void handleGameEvents() override; 

  void update() override; 
  void changeAnimation();

  void draw() override; 
  void drawInleftView();
  void drawInmiddleView();
  void drawInRightView();

  std::unique_ptr<Player> player; // blue
  std::unique_ptr<Player> player2; // red 

  std::unique_ptr<Button>button1;

  std::unique_ptr<Sprite> pawn; // for 3D rendering purposes; pawn is not player and is a blue sprite
  std::unique_ptr<Sprite> pawn2; // for 3D rendering purposes; pawn is not player and is a red sprite
  std::unique_ptr<Sprite> backgroundBig; 
  std::unique_ptr<Sprite> backgroundBigFinal; 
  std::unique_ptr<Sprite> backgroundBigHalfRed; // for 3D rendering
  std::unique_ptr<Sprite> backgroundBigHalfBlue; // for 3D rendering

  std::array<std::unique_ptr<Sprite>, Constants::STICKS_NUMBER / 2> sticksBlue;
  std::array<std::unique_ptr<Sprite>, Constants::STICKS_NUMBER / 2> sticksRed;
  
  std::array<std::shared_ptr<Tile>, 11> boardTiles;
  std::unique_ptr<BoardTileMap> boardTileMap; // for the board with walls and goals

  // for 3d walls
  sf::VertexArray rays; // player 1
  sf::VertexArray wallLine; // player 1
  sf::VertexArray rays2; // player 2
  sf::VertexArray wallLine2; // player 2

  std::unique_ptr<MusicClass> backgroundMusic;
  std::unique_ptr<SoundClass> buttonClickSound; 

  std::unique_ptr<TextClass> introText; 
  std::unique_ptr<TextClass> scoreText; 
  std::unique_ptr<TextClass> endingText; 
  std::unique_ptr<TextClass> player1Text; // for player 1
  std::unique_ptr<TextClass> player2Text; // for player 2

  unsigned int stickIndexBlue{}; 
  unsigned int stickIndexRed{};

  size_t p1pathCount{};
  unsigned int p1PrevPathIndex{};
  size_t p2pathCount{};
  unsigned int p2PrevPathIndex{};

  bool pawnRedBlocked = false; // for player 2
  bool pawnBlueBlocked = false; // for player 1
};
