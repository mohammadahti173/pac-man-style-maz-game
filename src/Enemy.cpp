#include "Enemy.h"

#include <cmath>
#include <cstdlib>
#include <vector>

#include "GraphicsUtils.h"
#include "Maze.h"

Enemy::Enemy()
{
    startTile = sf::Vector2i(10, 10);
    currentTile = sf::Vector2i(10, 10);
    nextTile = sf::Vector2i(10, 10);
    position = sf::Vector2f(0.0f, 0.0f);

    moving = false;
    active = true;
    progress = 0.0f;
    respawnTimer = 0.0f;
    animationTime = 0.0f;

    bodyColor = sf::Color::Red;
    personality = 0;

    direction = Direction::None;
    facing = Direction::Left;
}

void Enemy::setup(sf::Color newBodyColor, int personalityType)
{
    bodyColor = newBodyColor;
    personality = personalityType;
}

void Enemy::reset(sf::Vector2i newStartTile, const Maze& maze)
{
    startTile = newStartTile;
    currentTile = startTile;
    nextTile = startTile;

    moving = false;
    active = true;
    progress = 0.0f;
    respawnTimer = 0.0f;
    animationTime = 0.0f;

    direction = Direction::None;
    facing = Direction::Left;

    position = maze.tileCenter(currentTile.x, currentTile.y);
}

void Enemy::update(float deltaTime,
                   const Maze& maze,
                   sf::Vector2i playerTile,
                   Direction playerDirection,
                   bool frightened,
                   float speedFactor)
{
    animationTime += deltaTime;

    // A ghost that was eaten waits at its start position for a few seconds.
    if (!active)
    {
        respawnTimer -= deltaTime;

        if (respawnTimer <= 0.0f)
        {
            active = true;
            currentTile = startTile;
            nextTile = startTile;
            moving = false;
            progress = 0.0f;
            direction = Direction::None;
            facing = Direction::Left;
            position = maze.tileCenter(currentTile.x, currentTile.y);
        }

        return;
    }

    if (!moving)
    {
        chooseDirection(maze, playerTile, playerDirection, frightened);
    }

    if (moving)
    {
        float speed = Config::GHOST_SPEED * speedFactor;

        // Ghosts become slower while they are frightened.
        if (frightened)
        {
            speed *= 0.55f;
        }

        progress += speed * deltaTime;

        if (progress >= 1.0f)
        {
            currentTile = nextTile;
            progress = 0.0f;
            moving = false;
            chooseDirection(maze, playerTile, playerDirection, frightened);
        }
    }

    updatePosition();
}

void Enemy::chooseDirection(const Maze& maze,
                            sf::Vector2i playerTile,
                            Direction playerDirection,
                            bool frightened)
{
    Direction allDirections[4] =
    {
        Direction::Up,
        Direction::Left,
        Direction::Down,
        Direction::Right
    };

    std::vector<Direction> options;

    // While a ghost is still inside the ghost room it is allowed to turn
    // around. Without this exception a ghost could get trapped in the small
    // room when the player is below it.
    bool inGhostRoom = (maze.getTile(currentTile.x, currentTile.y) == 'G');

    // Ghosts normally do not turn around unless they hit a dead end.
    for (int i = 0; i < 4; i++)
    {
        Direction candidateDirection = allDirections[i];

        if (!inGhostRoom && candidateDirection == oppositeDirection(direction))
        {
            continue;
        }

        sf::Vector2i candidateTile = currentTile + directionStep(candidateDirection);

        if (maze.isWalkable(candidateTile.x, candidateTile.y))
        {
            options.push_back(candidateDirection);
        }
    }

    // If every option was blocked, the ghost may reverse.
    if (options.empty())
    {
        Direction back = oppositeDirection(direction);
        sf::Vector2i backTile = currentTile + directionStep(back);

        if (back != Direction::None && maze.isWalkable(backTile.x, backTile.y))
        {
            options.push_back(back);
        }
    }

    if (options.empty())
    {
        moving = false;
        return;
    }

    Direction chosen = options[0];

    if (frightened && !inGhostRoom)
    {
        // Frightened ghosts run around randomly.
        chosen = options[std::rand() % options.size()];
    }
    else
    {
        sf::Vector2i target = getTargetTile(playerTile, playerDirection);

        // A ghost that is still in the ghost room must walk toward the
        // opening above the room instead of chasing the player. Otherwise
        // the normal chase AI can keep it circling inside the room.
        if (inGhostRoom)
        {
            target = currentTile;

            while (target.y > 0 && maze.getTile(target.x, target.y) == 'G')
            {
                target.y--;
            }
        }

        int bestDistance = 1000000;

        for (Direction option : options)
        {
            sf::Vector2i candidateTile = currentTile + directionStep(option);

            int dx = candidateTile.x - target.x;
            int dy = candidateTile.y - target.y;
            int distance = std::abs(dx) + std::abs(dy);

            // A little randomness stops every ghost from following exactly
            // the same path.
            if (personality == 2)
            {
                distance += std::rand() % 8;
            }
            else if (personality == 3)
            {
                distance += std::rand() % 16;
            }

            if (distance < bestDistance)
            {
                bestDistance = distance;
                chosen = option;
            }
        }

        // Occasionally choose a random direction for a less robotic look.
        if (!inGhostRoom && std::rand() % 100 < 8)
        {
            chosen = options[std::rand() % options.size()];
        }
    }

    direction = chosen;
    facing = chosen;
    nextTile = currentTile + directionStep(chosen);
    progress = 0.0f;
    moving = true;
}

sf::Vector2i Enemy::getTargetTile(sf::Vector2i playerTile,
                                  Direction playerDirection) const
{
    // Red ghost: direct chase.
    if (personality == 0)
    {
        return playerTile;
    }

    // Pink ghost: aims a few tiles in front of Pac-Man.
    if (personality == 1)
    {
        sf::Vector2i step = directionStep(playerDirection);
        return sf::Vector2i(playerTile.x + step.x * 3,
                            playerTile.y + step.y * 3);
    }

    // Cyan ghost: direct chase with an unpredictable offset.
    if (personality == 2)
    {
        int offsetX = (std::rand() % 5) - 2;
        int offsetY = (std::rand() % 5) - 2;
        return sf::Vector2i(playerTile.x + offsetX,
                            playerTile.y + offsetY);
    }

    // Orange ghost: chases from far away, wanders when close.
    int dx = playerTile.x - currentTile.x;
    int dy = playerTile.y - currentTile.y;
    int distance = std::abs(dx) + std::abs(dy);

    if (distance > 7)
    {
        return playerTile;
    }

    if (startTile.x < Config::MAZE_COLUMNS / 2)
    {
        return sf::Vector2i(1, 1);
    }

    return sf::Vector2i(Config::MAZE_COLUMNS - 2, 1);
}

void Enemy::updatePosition()
{
    if (!moving)
    {
        position = tileCenterPosition(currentTile);
        return;
    }

    sf::Vector2f from = tileCenterPosition(currentTile);
    sf::Vector2f to = tileCenterPosition(nextTile);

    position.x = from.x + (to.x - from.x) * progress;
    position.y = from.y + (to.y - from.y) * progress;
}

void Enemy::eat()
{
    active = false;
    moving = false;
    progress = 0.0f;
    respawnTimer = 3.0f;
}

void Enemy::draw(sf::RenderWindow& window, float time, bool frightened, bool flash)
{
    if (!active)
    {
        return;
    }

    // Soft glow behind the ghost.
    sf::Color glowColor = bodyColor;
    glowColor.a = 32;

    if (frightened)
    {
        glowColor = sf::Color(60, 100, 255, 38);
    }

    float pulse = 1.0f + 0.08f * std::sin(time * 4.0f + currentTile.x);
    drawGlowCircle(window, position, 18.0f * pulse, glowColor);

    drawGhostShape(window,
                   position,
                   Config::GHOST_RADIUS,
                   bodyColor,
                   facing,
                   frightened,
                   flash);
}

bool Enemy::isActive() const
{
    return active;
}

sf::Vector2f Enemy::getPosition() const
{
    return position;
}

sf::Vector2i Enemy::getCurrentTile() const
{
    return currentTile;
}

sf::Vector2i Enemy::getStartTile() const
{
    return startTile;
}

float Enemy::getRadius() const
{
    return Config::GHOST_RADIUS;
}
