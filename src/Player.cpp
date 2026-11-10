#include "Player.h"

#include <cmath>

#include "GraphicsUtils.h"
#include "Maze.h"

Player::Player()
{
    currentTile = sf::Vector2i(1, 1);
    nextTile = sf::Vector2i(1, 1);
    position = sf::Vector2f(0.0f, 0.0f);
    moving = false;
    progress = 0.0f;
    animationTime = 0.0f;
    direction = Direction::None;
    desiredDirection = Direction::None;
    facing = Direction::Right;
}

void Player::reset(sf::Vector2i startTile, const Maze& maze)
{
    currentTile = startTile;
    nextTile = startTile;
    moving = false;
    progress = 0.0f;
    animationTime = 0.0f;
    direction = Direction::None;
    desiredDirection = Direction::None;
    facing = Direction::Right;
    position = maze.tileCenter(currentTile.x, currentTile.y);
}

void Player::setDesiredDirection(Direction newDirection)
{
    desiredDirection = newDirection;
}

void Player::update(float deltaTime, const Maze& maze)
{
    animationTime += deltaTime;

    // An immediate reversal is allowed even while the player is between two
    // tiles. This makes the controls feel responsive.
    if (moving && desiredDirection != Direction::None &&
        desiredDirection == oppositeDirection(direction))
    {
        sf::Vector2i oldTile = currentTile;
        currentTile = nextTile;
        nextTile = oldTile;
        progress = 1.0f - progress;
        direction = desiredDirection;
        facing = direction;
    }

    if (!moving)
    {
        tryToStartMoving(maze);
    }

    if (moving)
    {
        // progress is measured in tiles, so speed is "tiles per second".
        progress += Config::PLAYER_SPEED * deltaTime;

        if (progress >= 1.0f)
        {
            // The player has reached the center of the next tile.
            currentTile = nextTile;
            progress = 0.0f;
            moving = false;
            tryToStartMoving(maze);
        }
    }

    updatePosition();
}

void Player::tryToStartMoving(const Maze& maze)
{
    // First choice: the direction requested by the player.
    if (desiredDirection != Direction::None)
    {
        sf::Vector2i candidate = currentTile + directionStep(desiredDirection);

        if (maze.isWalkable(candidate.x, candidate.y))
        {
            direction = desiredDirection;
            facing = direction;
            nextTile = candidate;
            progress = 0.0f;
            moving = true;
            return;
        }
    }

    // Second choice: keep moving in the current direction.
    if (direction != Direction::None)
    {
        sf::Vector2i candidate = currentTile + directionStep(direction);

        if (maze.isWalkable(candidate.x, candidate.y))
        {
            nextTile = candidate;
            progress = 0.0f;
            moving = true;
            return;
        }
    }

    // No legal move: stand still.
    moving = false;
}

void Player::updatePosition()
{
    if (!moving)
    {
        position = tileCenterPosition(currentTile);
        return;
    }

    sf::Vector2f from = tileCenterPosition(currentTile);
    sf::Vector2f to = tileCenterPosition(nextTile);

    // Linear interpolation gives smooth movement between tile centers.
    position.x = from.x + (to.x - from.x) * progress;
    position.y = from.y + (to.y - from.y) * progress;
}

sf::Vector2f Player::getPosition() const
{
    return position;
}

sf::Vector2i Player::getCurrentTile() const
{
    return currentTile;
}

Direction Player::getFacingDirection() const
{
    return facing;
}

float Player::getRadius() const
{
    return Config::PLAYER_RADIUS;
}

bool Player::isMoving() const
{
    return moving;
}

void Player::draw(sf::RenderWindow& window,
                  float time,
                  bool powerActive,
                  bool invulnerable)
{
    sf::Color bodyColor = Colors::PlayerYellow;

    // During the short invulnerability period after losing a life, Pac-Man
    // blinks to show that he cannot be hit right now.
    if (invulnerable && (static_cast<int>(time * 12.0f) % 2 == 0))
    {
        bodyColor.a = 90;
    }

    // Yellow glow behind the character.
    drawGlowCircle(window, position, 18.0f, sf::Color(255, 220, 40, 28));

    // Extra cyan aura while a power pellet is active.
    if (powerActive)
    {
        float pulse = 1.0f + 0.25f * std::sin(time * 8.0f);
        drawGlowCircle(window, position, 24.0f * pulse,
                       sf::Color(80, 230, 255, 55));

        sf::CircleShape ring(Config::PLAYER_RADIUS + 6.0f, 32);
        ring.setOrigin(sf::Vector2f(Config::PLAYER_RADIUS + 6.0f,
                                    Config::PLAYER_RADIUS + 6.0f));
        ring.setPosition(position);
        ring.setFillColor(sf::Color::Transparent);
        ring.setOutlineThickness(1.5f);
        ring.setOutlineColor(sf::Color(120, 240, 255, 180));
        window.draw(ring);
    }

    // The mouth opens and closes while the player is moving.
    float mouthDegrees = 10.0f;
    if (moving)
    {
        mouthDegrees = 12.0f + 18.0f * (0.5f + 0.5f * std::sin(animationTime * 14.0f));
    }
    else
    {
        mouthDegrees = 10.0f + 5.0f * (0.5f + 0.5f * std::sin(animationTime * 3.0f));
    }

    drawPacManShape(window,
                    position,
                    Config::PLAYER_RADIUS,
                    bodyColor,
                    directionAngleDegrees(facing),
                    mouthDegrees);
}
