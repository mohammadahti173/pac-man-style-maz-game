#pragma once

#include <SFML/Graphics.hpp>

#include "Constants.h"

class Maze;

// -----------------------------------------------------------------------------
// The Player class handles Pac-Man itself: where he is, which way he is
// facing, and how he glides smoothly from the center of one tile to the next.
// -----------------------------------------------------------------------------
class Player
{
public:
    Player();

    // Moves the player back to a start tile and stops him.
    void reset(sf::Vector2i startTile, const Maze& maze);

    // Called by the keyboard handler. The requested direction is remembered
    // until it becomes possible to turn.
    void setDesiredDirection(Direction direction);

    // Moves the player forward and handles automatic turning at tile centers.
    void update(float deltaTime, const Maze& maze);

    // Draws the animated Pac-Man character.
    void draw(sf::RenderWindow& window,
              float time,
              bool powerActive,
              bool invulnerable);

    sf::Vector2f getPosition() const;
    sf::Vector2i getCurrentTile() const;
    Direction getFacingDirection() const;
    float getRadius() const;
    bool isMoving() const;

private:
    sf::Vector2i currentTile;
    sf::Vector2i nextTile;
    sf::Vector2f position;

    bool moving;
    float progress;       // 0 = at currentTile, 1 = at nextTile
    float animationTime;

    Direction direction;        // direction the player is actually moving
    Direction desiredDirection; // direction requested by the keyboard
    Direction facing;           // direction used for drawing the mouth

    void tryToStartMoving(const Maze& maze);
    void updatePosition();
};
