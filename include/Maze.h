#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

#include "Constants.h"

// -----------------------------------------------------------------------------
// The Maze class owns the text-based maze layout. It knows which tiles are
// walls, where the dots are, where the player starts and where the ghosts
// start. It also draws the complete maze with a neon arcade look.
// -----------------------------------------------------------------------------
class Maze
{
public:
    Maze();

    // Loads one of the three built-in layouts. Level numbers start at 1.
    void loadLevel(int levelNumber);

    // Draws walls, dots and power pellets.
    void draw(sf::RenderWindow& window, float time);

    // Basic grid questions.
    bool isInside(int column, int row) const;
    bool isWall(int column, int row) const;
    bool isWalkable(int column, int row) const;
    char getTile(int column, int row) const;

    // Removes a dot or power pellet from the maze. The function returns the
    // number of points earned (0 when there was nothing to collect).
    // wasPowerPellet is set to true when the collected item was a big pellet.
    int collectItem(int column, int row, bool& wasPowerPellet);

    int getRemainingDots() const;
    int getCurrentLevel() const;

    sf::Vector2i getPlayerStart() const;
    std::vector<sf::Vector2i> getGhostStarts() const;
    std::vector<sf::Vector2i> getWalkableTiles() const;

    // Converts between tile coordinates and pixel coordinates.
    sf::Vector2f tileCenter(int column, int row) const;
    sf::Vector2i pixelToTile(sf::Vector2f pixel) const;

private:
    std::vector<std::string> grid;
    int currentLevel;
    int remainingDots;
    sf::Vector2i playerStart;
    std::vector<sf::Vector2i> ghostStarts;

    void findSpawns();
};
