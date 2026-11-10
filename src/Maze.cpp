#include "Maze.h"

#include <cmath>

#include "GraphicsUtils.h"

// -----------------------------------------------------------------------------
// The three maze layouts. Every layout is exactly 21 characters wide and
// 21 rows tall.
//
//   #  wall
//   .  normal dot
//   o  power pellet
//   P  player start
//   G  ghost start
//   (space) empty walkable tile
//
// All three layouts were checked with a flood-fill test: every walkable tile
// can be reached from the player start.
// -----------------------------------------------------------------------------
static const std::vector<std::vector<std::string>> LEVEL_LAYOUTS =
{
    // ---------------------------- LEVEL 1 ----------------------------
    {
        "#####################",
        "#.........#.........#",
        "#.###.###.#.###.###.#",
        "#o###.###.#.###.###o#",
        "#...................#",
        "#.###.#.#####.#.###.#",
        "#.....#...#...#.....#",
        "#####.###.#.###.#####",
        "#.......#...#.......#",
        "#.#####.#...#.#####.#",
        "#.#...#.......#...#.#",
        "#.#.#.#.#...#.#.#.#.#",
        "#...#...#GGG#...#...#",
        "###.###.#GGG#.###.###",
        "#.......#GGG#.......#",
        "#.#####.#####.#####.#",
        "#o....#.......#....o#",
        "#####.#.#####.#.#####",
        "#.....P.......#.....#",
        "#.#################.#",
        "#####################"
    },

    // ---------------------------- LEVEL 2 ----------------------------
    {
        "#####################",
        "#.........#.........#",
        "#.#####.#.#.#.#####.#",
        "#o....#.#.#.#.#....o#",
        "#.###.#.#.#.#.#.###.#",
        "#.#...#...#...#...#.#",
        "#.#.###.#####.###.#.#",
        "#...#...........#...#",
        "###.#.###.#.###.#.###",
        "#...#.#.......#.#...#",
        "#.###.#.#...#.#.###.#",
        "#.....#.#GGG#.#.....#",
        "#.#####.#GGG#.#####.#",
        "#.......#GGG#.......#",
        "#####.#.#####.#.#####",
        "#.....#...#...#.....#",
        "#.###.###.#.###.###.#",
        "#o..#.....#.....#..o#",
        "#...#.#...P...#.#...#",
        "###.#.#########.#.###",
        "#####################"
    },

    // ---------------------------- LEVEL 3 ----------------------------
    {
        "#####################",
        "#.....#.......#.....#",
        "#.###.#.#####.#.###.#",
        "#o....#...#...#....o#",
        "#.#####.#.#.#.#####.#",
        "#.......#.#.#.......#",
        "###.###.#.#.#.###.###",
        "#...#...#...#...#...#",
        "#.#.#.#.#####.#.#.#.#",
        "#.#...#.......#...#.#",
        "#.#.###.#...#.###.#.#",
        "#.#.....#GGG#.....#.#",
        "#.#####.#GGG#.#####.#",
        "#.......#GGG#.......#",
        "#.#####.#####.#####.#",
        "#.#...#.......#...#.#",
        "#.#.#.#.#####.#.#.#.#",
        "#o..P..#.....#....o.#",
        "###.#####.#####.#####",
        "#...................#",
        "#####################"
    }
};

Maze::Maze()
{
    currentLevel = 1;
    remainingDots = 0;
    playerStart = sf::Vector2i(1, 1);
}

void Maze::loadLevel(int levelNumber)
{
    int index = levelNumber - 1;

    // Protect the game from an invalid level number.
    if (index < 0)
    {
        index = 0;
    }
    if (index >= static_cast<int>(LEVEL_LAYOUTS.size()))
    {
        index = static_cast<int>(LEVEL_LAYOUTS.size()) - 1;
    }

    grid = LEVEL_LAYOUTS[index];
    currentLevel = index + 1;

    findSpawns();

    // Count all collectible items in the new level.
    remainingDots = 0;
    for (int row = 0; row < Config::MAZE_ROWS; row++)
    {
        for (int column = 0; column < Config::MAZE_COLUMNS; column++)
        {
            char tile = grid[row][column];
            if (tile == '.' || tile == 'o')
            {
                remainingDots++;
            }
        }
    }
}

void Maze::findSpawns()
{
    ghostStarts.clear();
    playerStart = sf::Vector2i(1, 1);

    for (int row = 0; row < Config::MAZE_ROWS; row++)
    {
        for (int column = 0; column < Config::MAZE_COLUMNS; column++)
        {
            char tile = grid[row][column];

            if (tile == 'P')
            {
                playerStart = sf::Vector2i(column, row);
            }
            else if (tile == 'G')
            {
                ghostStarts.push_back(sf::Vector2i(column, row));
            }
        }
    }

    // Very small safety net, in case a future maze forgets a spawn point.
    if (ghostStarts.empty())
    {
        ghostStarts.push_back(sf::Vector2i(10, 10));
    }
}

bool Maze::isInside(int column, int row) const
{
    if (column < 0 || column >= Config::MAZE_COLUMNS)
    {
        return false;
    }
    if (row < 0 || row >= Config::MAZE_ROWS)
    {
        return false;
    }
    if (row >= static_cast<int>(grid.size()))
    {
        return false;
    }
    if (column >= static_cast<int>(grid[row].size()))
    {
        return false;
    }
    return true;
}

bool Maze::isWall(int column, int row) const
{
    if (!isInside(column, row))
    {
        return true; // Outside the maze always counts as a wall.
    }
    return grid[row][column] == '#';
}

bool Maze::isWalkable(int column, int row) const
{
    return isInside(column, row) && !isWall(column, row);
}

char Maze::getTile(int column, int row) const
{
    if (!isInside(column, row))
    {
        return '#';
    }
    return grid[row][column];
}

int Maze::collectItem(int column, int row, bool& wasPowerPellet)
{
    wasPowerPellet = false;

    if (!isInside(column, row))
    {
        return 0;
    }

    char tile = grid[row][column];

    if (tile == '.')
    {
        grid[row][column] = ' ';
        remainingDots--;
        return 10;
    }

    if (tile == 'o')
    {
        grid[row][column] = ' ';
        remainingDots--;
        wasPowerPellet = true;
        return 50;
    }

    return 0;
}

int Maze::getRemainingDots() const
{
    return remainingDots;
}

int Maze::getCurrentLevel() const
{
    return currentLevel;
}

sf::Vector2i Maze::getPlayerStart() const
{
    return playerStart;
}

std::vector<sf::Vector2i> Maze::getGhostStarts() const
{
    return ghostStarts;
}

std::vector<sf::Vector2i> Maze::getWalkableTiles() const
{
    std::vector<sf::Vector2i> tiles;

    for (int row = 0; row < Config::MAZE_ROWS; row++)
    {
        for (int column = 0; column < Config::MAZE_COLUMNS; column++)
        {
            if (!isWall(column, row))
            {
                tiles.push_back(sf::Vector2i(column, row));
            }
        }
    }

    return tiles;
}

sf::Vector2f Maze::tileCenter(int column, int row) const
{
    return tileCenterPosition(column, row);
}

sf::Vector2i Maze::pixelToTile(sf::Vector2f pixel) const
{
    return pixelToTilePosition(pixel);
}

void Maze::draw(sf::RenderWindow& window, float time)
{
    for (int row = 0; row < Config::MAZE_ROWS; row++)
    {
        for (int column = 0; column < Config::MAZE_COLUMNS; column++)
        {
            char tile = grid[row][column];

            float x = Config::MAZE_LEFT + column * Config::TILE_SIZE;
            float y = Config::MAZE_TOP  + row    * Config::TILE_SIZE;

            if (tile == '#')
            {
                // Outer glow.
                sf::RectangleShape glow(sf::Vector2f(Config::TILE_SIZE + 4.0f,
                                                     Config::TILE_SIZE + 4.0f));
                glow.setPosition(sf::Vector2f(x - 2.0f, y - 2.0f));
                glow.setFillColor(Colors::WallGlow);
                window.draw(glow);

                // Dark blue wall block with a bright neon outline.
                sf::RectangleShape block(sf::Vector2f(Config::TILE_SIZE - 4.0f,
                                                      Config::TILE_SIZE - 4.0f));
                block.setPosition(sf::Vector2f(x + 2.0f, y + 2.0f));
                block.setFillColor(Colors::WallFill);
                block.setOutlineThickness(1.5f);
                block.setOutlineColor(Colors::WallOutline);
                window.draw(block);

                // Small inner highlight makes the wall look less flat.
                sf::RectangleShape inner(sf::Vector2f(Config::TILE_SIZE - 12.0f,
                                                      Config::TILE_SIZE - 12.0f));
                inner.setPosition(sf::Vector2f(x + 6.0f, y + 6.0f));
                inner.setFillColor(sf::Color(20, 55, 115, 120));
                window.draw(inner);
            }
            else if (tile == '.')
            {
                sf::Vector2f center(x + Config::TILE_SIZE * 0.5f,
                                    y + Config::TILE_SIZE * 0.5f);

                drawGlowCircle(window, center, 5.5f, Colors::DotGlow);

                sf::CircleShape dot(2.8f, 14);
                dot.setOrigin(sf::Vector2f(2.8f, 2.8f));
                dot.setPosition(center);
                dot.setFillColor(Colors::Dot);
                window.draw(dot);
            }
            else if (tile == 'o')
            {
                sf::Vector2f center(x + Config::TILE_SIZE * 0.5f,
                                    y + Config::TILE_SIZE * 0.5f);

                // The pellet pulses to show that it is special.
                float pulse = 1.0f + 0.18f * std::sin(time * 5.0f);
                float glowRadius = 14.0f * pulse;
                float coreRadius = 6.5f * pulse;

                drawGlowCircle(window, center, glowRadius, Colors::PowerGlow);

                sf::CircleShape pellet(coreRadius, 24);
                pellet.setOrigin(sf::Vector2f(coreRadius, coreRadius));
                pellet.setPosition(center);
                pellet.setFillColor(Colors::PowerPellet);
                pellet.setOutlineThickness(1.5f);
                pellet.setOutlineColor(sf::Color(220, 255, 255, 220));
                window.draw(pellet);
            }
        }
    }
}
