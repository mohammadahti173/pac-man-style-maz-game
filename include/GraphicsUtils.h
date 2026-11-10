#pragma once

#include <SFML/Graphics.hpp>
#include <cmath>

#include "Constants.h"

// -----------------------------------------------------------------------------
// Small drawing helpers shared by the player, the enemies and the menus.
// Keeping these shapes in one place makes the rest of the classes easier to
// read and keeps the game looking consistent.
// -----------------------------------------------------------------------------

// Draws a classic Pac-Man shape: a yellow circle with a mouth cut out.
inline void drawPacManShape(sf::RenderWindow& window,
                            sf::Vector2f position,
                            float radius,
                            sf::Color color,
                            float rotationDegrees,
                            float mouthDegrees)
{
    // Keep the mouth inside a sensible range.
    if (mouthDegrees < 4.0f)  mouthDegrees = 4.0f;
    if (mouthDegrees > 60.0f) mouthDegrees = 60.0f;

    const int pointCount = 26;
    sf::ConvexShape body(pointCount);

    // Point 0 is the center of the circle.
    body.setPoint(0, sf::Vector2f(0.0f, 0.0f));

    // The other points follow the circle from one side of the mouth to the
    // other. This creates the round Pac-Man body.
    float startAngle = mouthDegrees;
    float endAngle   = 360.0f - mouthDegrees;

    for (int i = 0; i < pointCount - 1; i++)
    {
        float t = 0.0f;
        if (pointCount > 2)
        {
            t = static_cast<float>(i) / static_cast<float>(pointCount - 2);
        }

        float angle = startAngle + (endAngle - startAngle) * t;
        float radians = angle * 3.14159265f / 180.0f;

        body.setPoint(i + 1, sf::Vector2f(std::cos(radians) * radius,
                                          std::sin(radians) * radius));
    }

    body.setFillColor(color);
    body.setPosition(position);
    body.setRotation(sf::degrees(rotationDegrees));
    window.draw(body);
}

// Draws a colorful ghost using simple circles and rectangles.
inline void drawGhostShape(sf::RenderWindow& window,
                           sf::Vector2f position,
                           float radius,
                           sf::Color bodyColor,
                           Direction facing,
                           bool frightened,
                           bool flash)
{
    // In power mode the ghosts turn blue. Near the end of the power-up they
    // flash between blue and white as a warning.
    if (frightened)
    {
        if (flash)
        {
            bodyColor = sf::Color(245, 245, 255);
        }
        else
        {
            bodyColor = sf::Color(45, 80, 220);
        }
    }

    // Head (a full circle).
    sf::CircleShape head(radius, 28);
    head.setOrigin(sf::Vector2f(radius, radius));
    head.setPosition(sf::Vector2f(position.x, position.y - radius * 0.2f));
    head.setFillColor(bodyColor);
    window.draw(head);

    // Body (a rectangle below the head).
    sf::RectangleShape body(sf::Vector2f(radius * 2.0f, radius * 0.9f));
    body.setPosition(sf::Vector2f(position.x - radius, position.y - radius * 0.2f));
    body.setFillColor(bodyColor);
    window.draw(body);

    // Three little feet make the bottom look wavy.
    for (int i = 0; i < 3; i++)
    {
        float footRadius = radius * 0.32f;
        sf::CircleShape foot(footRadius, 14);
        foot.setOrigin(sf::Vector2f(footRadius, footRadius));
        foot.setPosition(sf::Vector2f(position.x - radius * 0.66f + i * radius * 0.66f,
                                      position.y + radius * 0.78f));
        foot.setFillColor(bodyColor);
        window.draw(foot);
    }

    // The eyes look toward the current movement direction.
    sf::Vector2f look(0.0f, 0.0f);
    if (facing == Direction::Left)  look = sf::Vector2f(-1.0f, 0.0f);
    if (facing == Direction::Right) look = sf::Vector2f(1.0f, 0.0f);
    if (facing == Direction::Up)    look = sf::Vector2f(0.0f, -1.0f);
    if (facing == Direction::Down)  look = sf::Vector2f(0.0f, 1.0f);

    float eyeY = position.y - radius * 0.38f;

    for (int side = 0; side < 2; side++)
    {
        float eyeX = position.x + (side == 0 ? -radius * 0.38f : radius * 0.38f);

        float whiteRadius = radius * 0.30f;
        sf::CircleShape whiteEye(whiteRadius, 18);
        whiteEye.setOrigin(sf::Vector2f(whiteRadius, whiteRadius));
        whiteEye.setPosition(sf::Vector2f(eyeX, eyeY));
        whiteEye.setFillColor(sf::Color::White);
        window.draw(whiteEye);

        float pupilRadius = radius * 0.14f;
        sf::CircleShape pupil(pupilRadius, 14);
        pupil.setOrigin(sf::Vector2f(pupilRadius, pupilRadius));
        pupil.setPosition(sf::Vector2f(eyeX + look.x * radius * 0.13f,
                                       eyeY + look.y * radius * 0.13f));
        pupil.setFillColor(sf::Color(10, 15, 45));
        window.draw(pupil);
    }

    // A simple mouth shows that the ghost is frightened.
    if (frightened)
    {
        sf::RectangleShape mouth(sf::Vector2f(radius * 0.75f, radius * 0.13f));
        mouth.setOrigin(sf::Vector2f(radius * 0.375f, radius * 0.065f));
        mouth.setPosition(sf::Vector2f(position.x, position.y + radius * 0.30f));
        mouth.setFillColor(sf::Color::White);
        window.draw(mouth);
    }
}

// A soft circle used behind important objects to create a neon glow.
inline void drawGlowCircle(sf::RenderWindow& window,
                           sf::Vector2f position,
                           float radius,
                           sf::Color color)
{
    sf::CircleShape glow(radius, 32);
    glow.setOrigin(sf::Vector2f(radius, radius));
    glow.setPosition(position);
    glow.setFillColor(color);
    window.draw(glow);
}
