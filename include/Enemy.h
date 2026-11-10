#pragma once

#include <SFML/Graphics.hpp>

#include "Constants.h"

class Maze;

// -----------------------------------------------------------------------------
// An Enemy is one ghost. Every ghost walks through the maze and chooses a new
// direction whenever it reaches the center of a tile. The AI is deliberately
// simple: compare the four possible next tiles and pick the one closest to a
// target tile.
// -----------------------------------------------------------------------------
class Enemy
{
public:
    Enemy();

    // Gives the ghost its color and its AI personality.
    void setup(sf::Color bodyColor, int personalityType);

    // Puts the ghost back at its start tile.
    void reset(sf::Vector2i startTile, const Maze& maze);

    // Updates movement and AI.
    void update(float deltaTime,
                const Maze& maze,
                sf::Vector2i playerTile,
                Direction playerDirection,
                bool frightened,
                float speedFactor);

    void draw(sf::RenderWindow& window, float time, bool frightened, bool flash);

    // Called when the player eats this ghost in power mode.
    void eat();

    bool isActive() const;
    sf::Vector2f getPosition() const;
    sf::Vector2i getCurrentTile() const;
    sf::Vector2i getStartTile() const;
    float getRadius() const;

private:
    sf::Vector2i startTile;
    sf::Vector2i currentTile;
    sf::Vector2i nextTile;
    sf::Vector2f position;

    bool moving;
    bool active;
    float progress;
    float respawnTimer;
    float animationTime;

    sf::Color bodyColor;
    int personality;

    Direction direction;
    Direction facing;

    void chooseDirection(const Maze& maze,
                         sf::Vector2i playerTile,
                         Direction playerDirection,
                         bool frightened);
    void updatePosition();
    sf::Vector2i getTargetTile(sf::Vector2i playerTile,
                               Direction playerDirection) const;
};
