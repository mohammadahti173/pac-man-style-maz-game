#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

#include "Constants.h"

// The menu returns one of these actions to the Game class.
enum class MenuAction
{
    None,
    Play,
    Instructions,
    Settings,
    Exit,
    Back,
    Resume,
    Restart,
    MainMenu,
    ToggleSound,
    ChangeDifficulty
};

// -----------------------------------------------------------------------------
// The Menu class draws every non-gameplay screen and keeps small clickable
// rectangles for its buttons. The Game class decides what each action means.
// -----------------------------------------------------------------------------
class Menu
{
public:
    Menu();

    void resetSelection();
    void moveSelection(int change);
    MenuAction activateSelection() const;
    MenuAction handleClick(sf::Vector2f mousePosition, GameState state) const;

    void drawMainMenu(sf::RenderWindow& window,
                      const sf::Font& titleFont,
                      const sf::Font& textFont,
                      int highScore,
                      float time);

    void drawInstructions(sf::RenderWindow& window,
                          const sf::Font& titleFont,
                          const sf::Font& textFont,
                          float time);

    void drawSettings(sf::RenderWindow& window,
                      const sf::Font& titleFont,
                      const sf::Font& textFont,
                      bool soundEnabled,
                      int difficulty,
                      float time);

    void drawPause(sf::RenderWindow& window,
                   const sf::Font& titleFont,
                   const sf::Font& textFont,
                   int score,
                   int level,
                   float time);

    void drawGameOver(sf::RenderWindow& window,
                      const sf::Font& titleFont,
                      const sf::Font& textFont,
                      int score,
                      int highScore,
                      int level,
                      float time);

    void drawVictory(sf::RenderWindow& window,
                     const sf::Font& titleFont,
                     const sf::Font& textFont,
                     int score,
                     int highScore,
                     float time);

private:
    struct Button
    {
        sf::FloatRect bounds;
        MenuAction action;
        std::string label;
    };

    std::vector<Button> buttons;
    int selectedIndex;
    GameState currentState;

    void clearButtons(GameState state);
    void addButton(sf::Vector2f position,
                   sf::Vector2f size,
                   MenuAction action,
                   const std::string& label);
    void drawButton(sf::RenderWindow& window, const sf::Font& font, int index);

    void drawTitle(sf::RenderWindow& window,
                   const sf::Font& font,
                   const std::string& text,
                   unsigned int characterSize,
                   float y,
                   sf::Color color);

    void drawCenteredText(sf::RenderWindow& window,
                          const sf::Font& font,
                          const std::string& text,
                          unsigned int characterSize,
                          float y,
                          sf::Color color);

    void drawLeftText(sf::RenderWindow& window,
                      const sf::Font& font,
                      const std::string& text,
                      unsigned int characterSize,
                      float x,
                      float y,
                      sf::Color color);

    void drawPanel(sf::RenderWindow& window, sf::FloatRect area);
    void drawGhostRow(sf::RenderWindow& window, float time, float y);
    void drawPacManDecor(sf::RenderWindow& window, float time);
};
