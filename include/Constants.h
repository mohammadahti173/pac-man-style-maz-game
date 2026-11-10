#pragma once

#include <SFML/Graphics.hpp>

// -----------------------------------------------------------------------------
// All important numbers for the game are collected in this one namespace.
// If you want a bigger window, a faster player, or a longer power-up time then
// this is the first place to look.
// -----------------------------------------------------------------------------
namespace Config
{
    const unsigned int WINDOW_WIDTH  = 900;
    const unsigned int WINDOW_HEIGHT = 780;

    const int MAZE_COLUMNS = 21;
    const int MAZE_ROWS    = 21;
    const float TILE_SIZE  = 30.0f;

    // The maze is drawn inside this rectangle of the window.
    const float MAZE_LEFT = 135.0f;
    const float MAZE_TOP  = 105.0f;

    const float PLAYER_RADIUS = 12.0f;
    const float GHOST_RADIUS  = 12.0f;

    const int START_LIVES   = 3;
    const int TOTAL_LEVELS  = 3;

    // Movement speed is measured in maze tiles per second.
    const float PLAYER_SPEED = 5.3f;
    const float GHOST_SPEED  = 4.2f;
}

// Four possible movement directions plus "None" for standing still.
enum class Direction
{
    None,
    Up,
    Down,
    Left,
    Right
};

// Every screen of the game is represented by one of these values.
enum class GameState
{
    MainMenu,
    Instructions,
    Settings,
    Playing,
    Paused,
    Dying,          // internal short animation after the player is caught
    LevelComplete,  // internal short animation between levels
    GameOver,
    Victory
};

// Returns the direction that points the opposite way.
inline Direction oppositeDirection(Direction direction)
{
    if (direction == Direction::Up)    return Direction::Down;
    if (direction == Direction::Down)  return Direction::Up;
    if (direction == Direction::Left)  return Direction::Right;
    if (direction == Direction::Right) return Direction::Left;
    return Direction::None;
}

// Returns the x/y step for one maze tile in the given direction.
inline sf::Vector2i directionStep(Direction direction)
{
    if (direction == Direction::Up)    return sf::Vector2i(0, -1);
    if (direction == Direction::Down)  return sf::Vector2i(0, 1);
    if (direction == Direction::Left)  return sf::Vector2i(-1, 0);
    if (direction == Direction::Right) return sf::Vector2i(1, 0);
    return sf::Vector2i(0, 0);
}

// SFML rotates clockwise. These angles make the Pac-Man mouth face the
// direction of travel.
inline float directionAngleDegrees(Direction direction)
{
    if (direction == Direction::Down)  return 90.0f;
    if (direction == Direction::Left)  return 180.0f;
    if (direction == Direction::Up)    return 270.0f;
    return 0.0f; // Right is the default
}

// Converts a maze column/row into the pixel position at the tile center.
inline sf::Vector2f tileCenterPosition(int column, int row)
{
    float x = Config::MAZE_LEFT + column * Config::TILE_SIZE + Config::TILE_SIZE * 0.5f;
    float y = Config::MAZE_TOP  + row    * Config::TILE_SIZE + Config::TILE_SIZE * 0.5f;
    return sf::Vector2f(x, y);
}

inline sf::Vector2f tileCenterPosition(sf::Vector2i tile)
{
    return tileCenterPosition(tile.x, tile.y);
}

// Converts a pixel position into the maze tile that contains it.
inline sf::Vector2i pixelToTilePosition(sf::Vector2f pixel)
{
    int column = static_cast<int>((pixel.x - Config::MAZE_LEFT) / Config::TILE_SIZE);
    int row    = static_cast<int>((pixel.y - Config::MAZE_TOP)  / Config::TILE_SIZE);
    return sf::Vector2i(column, row);
}

// Common colors used by several classes.
namespace Colors
{
    inline const sf::Color Background    {6, 8, 24};
    inline const sf::Color WallFill      {12, 32, 76};
    inline const sf::Color WallGlow      {0, 170, 255, 24};
    inline const sf::Color WallOutline   {0, 205, 255, 210};
    inline const sf::Color Dot           {255, 244, 200};
    inline const sf::Color DotGlow       {255, 220, 120, 35};
    inline const sf::Color PowerPellet   {130, 240, 255};
    inline const sf::Color PowerGlow     {80, 220, 255, 45};
    inline const sf::Color PlayerYellow  {255, 225, 45};
    inline const sf::Color Accent        {0, 235, 255};
    inline const sf::Color TextLight     {220, 235, 255};
    inline const sf::Color TextDim       {130, 160, 200};
    inline const sf::Color Danger        {255, 70, 120};
    inline const sf::Color Gold          {255, 210, 60};
}
