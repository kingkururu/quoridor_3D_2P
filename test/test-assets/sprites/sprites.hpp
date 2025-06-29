//
//  sprites.hpp
//  sfml game template
//
//

#pragma once

#include <stdio.h>
#include <string> 
#include <iostream>
#include <stdexcept>
#include <map>
#include <SFML/Graphics.hpp>

#include "../../test-logging/log.hpp"
#include "../globals/globals.hpp"


// base class for all sprites; contains position, scale, and texture 
class Sprite : public sf::Drawable {
public:
    explicit Sprite(sf::Vector2f position, sf::Vector2f scale, std::weak_ptr<sf::Texture> texture);
    virtual ~Sprite() = default;

    sf::Vector2f getSpritePos() const { return position; };
    void updateSpritePos(sf::Vector2f position) { this->position = position; spriteCreated->setPosition(position); }
    sf::Sprite& returnSpritesShape() const { return *spriteCreated; } 
    bool getVisibleState() const { return visibleState; }
    void setVisibleState(bool visibleState){ this->visibleState = visibleState; }
    sf::Vector2f getScreenPosition() const { return screenPosition; }
    void setScreenPosition(sf::Vector2f screenPosition){ this->screenPosition = screenPosition; }
    // base template for retreaving radius (based on sprite size) 
    virtual float getRadius() const;

    // blank members for use in Animated class
    virtual sf::IntRect getRects() const { return sf::IntRect(); }
    virtual int getCurrIndex() const { return 0; }
    virtual std::shared_ptr<sf::Uint8[]> const getBitmask(size_t index) const { return nullptr; }
    virtual bool isAnimated() const { return false; }
    // blank members for use in NonStatic class
    virtual sf::Vector2f getDirectionVector() const { return sf::Vector2f(); }
    virtual float getSpeed() const { return 0.0f; }
    virtual sf::Vector2f getAcceleration() const { return sf::Vector2f(); }
    bool getMoveState() const { return false; }
    bool isCentered() const { return false; }

    // draws sprite using window.draw(*sprite)
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override { if (visibleState && spriteCreated) target.draw(*spriteCreated, states); }
    virtual void updateVisibility(); 

protected:
    sf::Vector2f position {};
    sf::Vector2f scale {};
    std::weak_ptr<sf::Texture> texture;
    std::unique_ptr<sf::Sprite> spriteCreated;
    bool visibleState {};
    float radius{}; 
    sf::Vector2f screenPosition {};
};

class Animated : public virtual Sprite {
public:
    explicit Animated( sf::Vector2f position, sf::Vector2f scale, std::weak_ptr<sf::Texture> texture, const std::vector<sf::IntRect> animationRects, unsigned const int indexMax,  const std::vector<std::weak_ptr<sf::Uint8[]>>& bitMask) 
        : Sprite(position, scale, texture), animationRects(animationRects), indexMax(indexMax), bitMask(bitMask) {}
    
        std::vector<sf::IntRect> const getAnimationRects() const { return animationRects; } 
    void setAnimation(std::vector<sf::IntRect> AnimationRects) { animationRects = AnimationRects; } 
    
    void setAnimChangeState(bool newState) { animChangeState = newState; }
    virtual void changeAnimation(); 
    void setRects(int animNum); 

    float getRadius() const override; 
    sf::IntRect getRects() const override;
    int getCurrIndex() const override { return currentIndex; } 
    std::shared_ptr<sf::Uint8[]> const getBitmask(size_t index) const override; 
    bool isAnimated() const override { return true; } // for checking type

protected:
    std::vector<sf::IntRect> animationRects{}; 
    int currentIndex {};
    int indexMax {}; 
    float elapsedTime {};
    bool animChangeState = true; 
    std::vector<std::weak_ptr<sf::Uint8[]>> bitMask{}; 
};

class NonAnimated : public virtual Sprite { // add something inside later if necessary
    using Sprite::Sprite;
    ~NonAnimated() override{};
};

// background class deriving from sprites; the background doesn't "actually move with physics", but moves constantly to the left 
class Background : public Sprite{
public:
   explicit Background(sf::Vector2f position, sf::Vector2f scale, std::weak_ptr<sf::Texture> texture);
    ~Background() override{};

    // make background (can put any direction if only using primaryDirection, but need to put up/down in primary and right/left in secondary if using both)
    void updateBackground(float speed, SpriteComponents::Direction primaryDirection, SpriteComponents::Direction secondaryDirection = SpriteComponents::Direction::NONE);  

    sf::Sprite& returnSpritesShape2() { return *spriteCreated2; }
    sf::Sprite& returnSpritesShape3() { return *spriteCreated3; }
    sf::Sprite& returnSpritesShape4() { return *spriteCreated4; }

    sf::FloatRect getViewBounds(sf::Sprite& spriteNum) const;

    bool getBackgroundMoveState() const { return backgroundMoveState; } 
    void setBackgroundMoveState(bool newState) { backgroundMoveState = newState; }
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override; 

private:
    // for right and left
    std::unique_ptr<sf::Sprite> spriteCreated2;
    // for up and down 
    std::unique_ptr<sf::Sprite> spriteCreated3;
    // for left-over blank space 
    std::unique_ptr<sf::Sprite> spriteCreated4;

    bool backgroundMoveState = true; 
};

// static class deriving from sprites; refers to non-moving sprites 
class Static : public virtual Sprite{
public:
    using Sprite::Sprite;
    ~Static() override{};
};

// NonStatic class deriving from sprites; refers to moving sprites 
class NonStatic : public virtual Sprite{
public:
   explicit NonStatic(sf::Vector2f position, sf::Vector2f scale, std::weak_ptr<sf::Texture> texture, float speed, sf::Vector2f acceleration)
        : Sprite(position, scale, texture), speed(speed), acceleration(acceleration) {}
    ~NonStatic() override{}; 

    bool getMoveState() const { return moveState; }
    void setMoveState(bool newState) { moveState = newState; }
    void changePosition(sf::Vector2f newPos) { position = newPos; }  
    void setSpeed(float newSpeed) { speed = newSpeed; } 
    void setAcceleration( sf::Vector2f newAcc) { acceleration = newAcc; } 

    virtual void setDirectionVector(sf::Vector2f dir) {directionVector = dir; } 

    using Sprite::getDirectionVector;
    virtual sf::Vector2f getDirectionVector() const override { return directionVector; }
    virtual float getSpeed() const override { return speed; }
    virtual sf::Vector2f getAcceleration() const override{ return acceleration; }
    virtual void updatePos() { spriteCreated->setPosition(position); }

protected:
    bool moveState = true;
    sf::Vector2f directionVector{}; 
    float speed {}; 
    sf::Vector2f acceleration{}; 
};

// player class deriving from NonStatic; refers to movable player 
class Player : public NonStatic, public Animated {
 public:
   explicit Player(sf::Vector2f position, sf::Vector2f scale, std::weak_ptr<sf::Texture> texture,
                float speed, sf::Vector2f acceleration,  
                const std::vector<sf::IntRect> animationRects, unsigned int indexMax, 
                const std::vector<std::weak_ptr<sf::Uint8[]>>& bitMask)
    : Sprite(position, scale, texture), 
      NonStatic(position, scale, texture, speed, acceleration), 
      Animated(position, scale, texture, animationRects, indexMax, bitMask) {
        setHeadingAngle(spriteCreated->getRotation());
        sf::Vector2f playerCenter = sf::Vector2f( getRects().width / 2, getRects().height / 2);
        spriteCreated->setOrigin(playerCenter);  // Adjust the origin for the player sprite
      }
    ~Player() override = default;
    
    void updatePlayer(sf::Vector2f newPos); 
    void changeAnimation() override; 
    bool getJumpingState() const { return isJumping; }
    bool getFallingState() const { return isFalling; }
    void setJumpingState(bool jumpState) { isJumping = jumpState; }  
    void setFallingState(bool fallState) { isFalling = fallState; } 
    void setHeadingAngle(float headingAngle);
    float getHeadingAngle() const { return headingAngle; }
    bool isCentered() const { return true; }
   
    bool getTurnInProgress() const { return turnInProgress; }
    int getTlesMovedThisTurn() const { return tilesMovedThisTurn; }
    bool getIsMoving() const { return isMoving; }
    int getCurrentDirection() const { return currentDirection; }
    sf::Vector2f getTargetPosition() const { return targetPosition; }

    void setTurnInProgress(bool turnInProgress) { this->turnInProgress = turnInProgress; }
    void setTilesMovedThisTurn(int tilesMovedThisTurn) { this->tilesMovedThisTurn = tilesMovedThisTurn; }
    void setIsMoving(bool isMoving) { this->isMoving = isMoving; }
    void setCurrentDirection(int currentDirection) { this->currentDirection = currentDirection; }
    void setTargetPosition(sf::Vector2f targetPosition) { this->targetPosition = targetPosition; spriteCreated->setPosition(targetPosition); }

 private:
    bool firstTurnInstance = true; 
    bool prevTurnBool{}; 
    bool isJumping = false;  
    bool isFalling = false; 
    float headingAngle{}; 

    bool turnInProgress = false;
    int tilesMovedThisTurn = 0;
    bool isMoving = false;
    int currentDirection = -1; 
    sf::Vector2f targetPosition {}; 
};

class Button : public Animated {
public:
    explicit Button(sf::Vector2f position, sf::Vector2f scale, std::weak_ptr<sf::Texture> texture, 
                      const std::vector<sf::IntRect> animationRects, unsigned int indexMax, 
                      const std::vector<std::weak_ptr<sf::Uint8[]>>& bitMask)
        : Sprite(position, scale, texture),
          Animated(position, scale, texture, animationRects, indexMax, bitMask)
    {}
    ~Button() override = default;

    void setClickedBool(bool click) { clicked = click; }
    bool getClickedBool() const { return clicked; }
    void setPosition(sf::Vector2f newPos) { position = newPos; spriteCreated->setPosition(position); }
    void updatePos() { spriteCreated->setPosition(position); }

private:
    bool clicked {}; 
}; 
