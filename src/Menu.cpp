#include "Menu.h"

#include <cmath>
#include <iomanip>
#include <sstream>

#include "GraphicsUtils.h"

namespace
{
    // Centers a text object horizontally and places its top at topY.
    void centerText(sf::Text& text, float centerX, float topY)
    {
        sf::FloatRect bounds = text.getLocalBounds();
        text.setOrigin(sf::Vector2f(bounds.position.x + bounds.size.x * 0.5f,
                                    bounds.position.y));
        text.setPosition(sf::Vector2f(centerX, topY));
    }

    // Centers a text object in the middle of a rectangle.
    void centerTextInRect(sf::Text& text, const sf::FloatRect& rectangle)
    {
        sf::FloatRect bounds = text.getLocalBounds();
        text.setOrigin(sf::Vector2f(bounds.position.x + bounds.size.x * 0.5f,
                                    bounds.position.y + bounds.size.y * 0.5f));
        text.setPosition(sf::Vector2f(rectangle.position.x + rectangle.size.x * 0.5f,
                                      rectangle.position.y + rectangle.size.y * 0.5f));
    }

    std::string paddedScore(int score)
    {
        std::ostringstream stream;
        stream << std::setw(6) << std::setfill('0') << score;
        return stream.str();
    }

    const char* difficultyName(int difficulty)
    {
        if (difficulty == 0) return "CHILL";
        if (difficulty == 2) return "HARD";
        return "CLASSIC";
    }
}

Menu::Menu()
{
    selectedIndex = 0;
    currentState = GameState::MainMenu;
}

void Menu::resetSelection()
{
    selectedIndex = 0;
    buttons.clear();
    currentState = GameState::MainMenu;
}

void Menu::moveSelection(int change)
{
    if (buttons.empty())
    {
        return;
    }

    int count = static_cast<int>(buttons.size());
    selectedIndex = selectedIndex + change;

    // Wrap around the list.
    if (selectedIndex < 0)
    {
        selectedIndex = count - 1;
    }
    if (selectedIndex >= count)
    {
        selectedIndex = 0;
    }
}

MenuAction Menu::activateSelection() const
{
    if (selectedIndex >= 0 && selectedIndex < static_cast<int>(buttons.size()))
    {
        return buttons[selectedIndex].action;
    }
    return MenuAction::None;
}

MenuAction Menu::handleClick(sf::Vector2f mousePosition, GameState state) const
{
    if (currentState != state)
    {
        return MenuAction::None;
    }

    for (const Button& button : buttons)
    {
        if (button.bounds.contains(mousePosition))
        {
            return button.action;
        }
    }

    return MenuAction::None;
}

void Menu::clearButtons(GameState state)
{
    buttons.clear();
    currentState = state;
}

void Menu::addButton(sf::Vector2f position,
                     sf::Vector2f size,
                     MenuAction action,
                     const std::string& label)
{
    Button button;
    button.bounds = sf::FloatRect(position, size);
    button.action = action;
    button.label = label;
    buttons.push_back(button);
}

void Menu::drawButton(sf::RenderWindow& window, const sf::Font& font, int index)
{
    if (index < 0 || index >= static_cast<int>(buttons.size()))
    {
        return;
    }

    const Button& button = buttons[index];

    sf::Vector2i mousePixel = sf::Mouse::getPosition(window);
    sf::Vector2f mousePosition(static_cast<float>(mousePixel.x),
                               static_cast<float>(mousePixel.y));

    bool hovered = button.bounds.contains(mousePosition);
    bool highlighted = hovered || (index == selectedIndex);

    if (highlighted)
    {
        sf::RectangleShape glow(sf::Vector2f(button.bounds.size.x + 10.0f,
                                             button.bounds.size.y + 10.0f));
        glow.setPosition(sf::Vector2f(button.bounds.position.x - 5.0f,
                                      button.bounds.position.y - 5.0f));
        glow.setFillColor(sf::Color(0, 200, 255, 26));
        window.draw(glow);
    }

    sf::RectangleShape rectangle(button.bounds.size);
    rectangle.setPosition(button.bounds.position);
    rectangle.setOutlineThickness(2.0f);

    if (highlighted)
    {
        rectangle.setFillColor(sf::Color(0, 55, 90, 235));
        rectangle.setOutlineColor(Colors::Accent);
    }
    else
    {
        rectangle.setFillColor(sf::Color(10, 20, 45, 215));
        rectangle.setOutlineColor(sf::Color(0, 120, 180, 170));
    }

    window.draw(rectangle);

    sf::Text text(font, button.label, 18);
    text.setFillColor(highlighted ? sf::Color::White : Colors::TextLight);
    centerTextInRect(text, button.bounds);
    window.draw(text);
}

void Menu::drawTitle(sf::RenderWindow& window,
                     const sf::Font& font,
                     const std::string& text,
                     unsigned int characterSize,
                     float y,
                     sf::Color color)
{
    // A translucent copy with a thick outline creates a neon glow.
    sf::Text glow(font, text, characterSize);
    glow.setFillColor(sf::Color(0, 190, 255, 35));
    glow.setOutlineThickness(static_cast<float>(characterSize) * 0.12f);
    glow.setOutlineColor(sf::Color(0, 190, 255, 25));
    centerText(glow, Config::WINDOW_WIDTH * 0.5f, y);
    window.draw(glow);

    sf::Text mainText(font, text, characterSize);
    mainText.setFillColor(color);
    mainText.setOutlineThickness(2.0f);
    mainText.setOutlineColor(sf::Color(10, 15, 30));
    centerText(mainText, Config::WINDOW_WIDTH * 0.5f, y);
    window.draw(mainText);
}

void Menu::drawCenteredText(sf::RenderWindow& window,
                            const sf::Font& font,
                            const std::string& text,
                            unsigned int characterSize,
                            float y,
                            sf::Color color)
{
    sf::Text drawText(font, text, characterSize);
    drawText.setFillColor(color);
    centerText(drawText, Config::WINDOW_WIDTH * 0.5f, y);
    window.draw(drawText);
}

void Menu::drawLeftText(sf::RenderWindow& window,
                        const sf::Font& font,
                        const std::string& text,
                        unsigned int characterSize,
                        float x,
                        float y,
                        sf::Color color)
{
    sf::Text drawText(font, text, characterSize);
    drawText.setFillColor(color);
    drawText.setPosition(sf::Vector2f(x, y));
    window.draw(drawText);
}

void Menu::drawPanel(sf::RenderWindow& window, sf::FloatRect area)
{
    sf::RectangleShape glow(sf::Vector2f(area.size.x + 12.0f, area.size.y + 12.0f));
    glow.setPosition(sf::Vector2f(area.position.x - 6.0f, area.position.y - 6.0f));
    glow.setFillColor(sf::Color(0, 170, 255, 18));
    window.draw(glow);

    sf::RectangleShape panel(area.size);
    panel.setPosition(area.position);
    panel.setFillColor(sf::Color(8, 14, 38, 225));
    panel.setOutlineThickness(2.0f);
    panel.setOutlineColor(sf::Color(0, 150, 220, 120));
    window.draw(panel);
}

void Menu::drawGhostRow(sf::RenderWindow& window, float time, float y)
{
    sf::Color colors[4] =
    {
        sf::Color(255, 70, 70),
        sf::Color(255, 120, 200),
        sf::Color(60, 220, 255),
        sf::Color(255, 160, 50)
    };

    for (int i = 0; i < 4; i++)
    {
        float x = 360.0f + i * 70.0f;
        float bob = std::sin(time * 2.5f + i * 0.7f) * 5.0f;
        drawGhostShape(window,
                       sf::Vector2f(x, y + bob),
                       16.0f,
                       colors[i],
                       Direction::Left,
                       false,
                       false);
    }
}

void Menu::drawPacManDecor(sf::RenderWindow& window, float time)
{
    float bob = std::sin(time * 2.5f) * 5.0f;
    float mouth = 18.0f + 12.0f * (0.5f + 0.5f * std::sin(time * 9.0f));

    drawGlowCircle(window, sf::Vector2f(270.0f, 232.0f + bob),
                   22.0f, sf::Color(255, 220, 40, 30));

    drawPacManShape(window,
                    sf::Vector2f(270.0f, 232.0f + bob),
                    18.0f,
                    Colors::PlayerYellow,
                    0.0f,
                    mouth);
}

void Menu::drawMainMenu(sf::RenderWindow& window,
                        const sf::Font& titleFont,
                        const sf::Font& textFont,
                        int highScore,
                        float time)
{
    clearButtons(GameState::MainMenu);

    drawTitle(window, titleFont, "PAC-MAN", 58, 72.0f, Colors::Gold);
    drawCenteredText(window, titleFont, "NEON MAZE", 16, 150.0f, Colors::Accent);

    drawPacManDecor(window, time);
    drawGhostRow(window, time, 232.0f);

    float buttonX = 285.0f;
    float buttonWidth = 330.0f;
    float buttonHeight = 54.0f;

    addButton(sf::Vector2f(buttonX, 310.0f),
              sf::Vector2f(buttonWidth, buttonHeight),
              MenuAction::Play,
              "PLAY GAME");

    addButton(sf::Vector2f(buttonX, 376.0f),
              sf::Vector2f(buttonWidth, buttonHeight),
              MenuAction::Instructions,
              "INSTRUCTIONS");

    addButton(sf::Vector2f(buttonX, 442.0f),
              sf::Vector2f(buttonWidth, buttonHeight),
              MenuAction::Settings,
              "SETTINGS");

    addButton(sf::Vector2f(buttonX, 508.0f),
              sf::Vector2f(buttonWidth, buttonHeight),
              MenuAction::Exit,
              "EXIT");

    for (int i = 0; i < static_cast<int>(buttons.size()); i++)
    {
        drawButton(window, textFont, i);
    }

    drawCenteredText(window, titleFont,
                     "HIGH SCORE " + paddedScore(highScore),
                     16, 602.0f, Colors::TextLight);

    drawCenteredText(window, textFont,
                     "W / S or ARROWS + ENTER   |   MOUSE SUPPORTED",
                     14, 648.0f, Colors::TextDim);

    drawCenteredText(window, textFont,
                     "C++ / SFML 3 ARCADE PROJECT",
                     12, 724.0f, sf::Color(65, 90, 130));
}

void Menu::drawInstructions(sf::RenderWindow& window,
                            const sf::Font& titleFont,
                            const sf::Font& textFont,
                            float time)
{
    (void)time;
    clearButtons(GameState::Instructions);

    drawTitle(window, titleFont, "HOW TO PLAY", 30, 42.0f, Colors::Accent);
    drawPanel(window, sf::FloatRect(sf::Vector2f(80.0f, 108.0f),
                                    sf::Vector2f(740.0f, 468.0f)));

    float x = 120.0f;
    float y = 140.0f;
    float lineHeight = 34.0f;

    std::vector<std::string> lines =
    {
        "MOVE          W A S D  or  ARROW KEYS",
        "PAUSE / RESUME          P  or  ESC",
        "",
        "EAT EVERY DOT TO CLEAR THE MAZE.",
        "BIG GLOWING PELLETS START POWER MODE.",
        "IN POWER MODE, EAT GHOSTS FOR 200, 400, 800, 1600.",
        "COLLECT THE BONUS FRUIT FOR EXTRA POINTS.",
        "YOU HAVE 3 LIVES. ONE GHOST TOUCH = ONE LIFE.",
        "TURN EARLY: PAC-MAN TURNS AT THE NEXT TILE CENTER.",
        "CLEAR ALL 3 MAZES TO WIN THE GAME."
    };

    for (const std::string& line : lines)
    {
        if (!line.empty())
        {
            drawLeftText(window, textFont, line, 16, x, y, Colors::TextLight);
        }
        y += lineHeight;
    }

    drawCenteredText(window, textFont,
                     "TIP: YOU CAN ALSO CLICK THE MENU BUTTONS WITH THE MOUSE.",
                     13, 596.0f, Colors::TextDim);

    addButton(sf::Vector2f(330.0f, 628.0f),
              sf::Vector2f(240.0f, 52.0f),
              MenuAction::Back,
              "BACK");

    for (int i = 0; i < static_cast<int>(buttons.size()); i++)
    {
        drawButton(window, textFont, i);
    }

    drawCenteredText(window, textFont, "ESC TO RETURN", 13, 710.0f, Colors::TextDim);
}

void Menu::drawSettings(sf::RenderWindow& window,
                        const sf::Font& titleFont,
                        const sf::Font& textFont,
                        bool soundEnabled,
                        int difficulty,
                        float time)
{
    (void)time;
    clearButtons(GameState::Settings);

    drawTitle(window, titleFont, "SETTINGS", 34, 70.0f, Colors::Accent);
    drawPanel(window, sf::FloatRect(sf::Vector2f(140.0f, 155.0f),
                                    sf::Vector2f(620.0f, 365.0f)));

    std::string soundLabel = "SOUND: ";
    soundLabel += (soundEnabled ? "ON" : "OFF");

    std::string difficultyLabel = "DIFFICULTY: ";
    difficultyLabel += difficultyName(difficulty);

    addButton(sf::Vector2f(250.0f, 225.0f),
              sf::Vector2f(400.0f, 54.0f),
              MenuAction::ToggleSound,
              soundLabel);

    addButton(sf::Vector2f(250.0f, 335.0f),
              sf::Vector2f(400.0f, 54.0f),
              MenuAction::ChangeDifficulty,
              difficultyLabel);

    addButton(sf::Vector2f(310.0f, 445.0f),
              sf::Vector2f(280.0f, 52.0f),
              MenuAction::Back,
              "BACK");

    for (int i = 0; i < static_cast<int>(buttons.size()); i++)
    {
        drawButton(window, textFont, i);
    }

    drawCenteredText(window, textFont,
                     "Turn the arcade sound effects on or off.",
                     14, 298.0f, Colors::TextDim);

    drawCenteredText(window, textFont,
                     "Chill, Classic and Hard change ghost speed and power-up time.",
                     14, 408.0f, Colors::TextDim);

    drawCenteredText(window, textFont,
                     "UP / DOWN + ENTER or CLICK",
                     13, 552.0f, sf::Color(80, 110, 150));
}

void Menu::drawPause(sf::RenderWindow& window,
                     const sf::Font& titleFont,
                     const sf::Font& textFont,
                     int score,
                     int level,
                     float time)
{
    (void)time;
    clearButtons(GameState::Paused);

    sf::RectangleShape overlay(sf::Vector2f(static_cast<float>(Config::WINDOW_WIDTH),
                                            static_cast<float>(Config::WINDOW_HEIGHT)));
    overlay.setFillColor(sf::Color(2, 4, 18, 185));
    window.draw(overlay);

    drawTitle(window, titleFont, "PAUSED", 40, 130.0f, Colors::Accent);

    drawCenteredText(window, titleFont,
                     "SCORE " + paddedScore(score) + "   LEVEL " + std::to_string(level),
                     17, 215.0f, Colors::TextLight);

    float buttonX = 285.0f;
    float buttonWidth = 330.0f;
    float buttonHeight = 54.0f;

    addButton(sf::Vector2f(buttonX, 300.0f),
              sf::Vector2f(buttonWidth, buttonHeight),
              MenuAction::Resume,
              "RESUME");

    addButton(sf::Vector2f(buttonX, 376.0f),
              sf::Vector2f(buttonWidth, buttonHeight),
              MenuAction::Restart,
              "RESTART GAME");

    addButton(sf::Vector2f(buttonX, 452.0f),
              sf::Vector2f(buttonWidth, buttonHeight),
              MenuAction::MainMenu,
              "MAIN MENU");

    for (int i = 0; i < static_cast<int>(buttons.size()); i++)
    {
        drawButton(window, textFont, i);
    }

    drawCenteredText(window, textFont,
                     "P / ESC TO RESUME     R TO RESTART     M FOR MENU",
                     13, 580.0f, Colors::TextDim);
}

void Menu::drawGameOver(sf::RenderWindow& window,
                        const sf::Font& titleFont,
                        const sf::Font& textFont,
                        int score,
                        int highScore,
                        int level,
                        float time)
{
    (void)time;
    clearButtons(GameState::GameOver);

    sf::RectangleShape overlay(sf::Vector2f(static_cast<float>(Config::WINDOW_WIDTH),
                                            static_cast<float>(Config::WINDOW_HEIGHT)));
    overlay.setFillColor(sf::Color(12, 0, 8, 200));
    window.draw(overlay);

    drawTitle(window, titleFont, "GAME OVER", 40, 110.0f, Colors::Danger);

    drawCenteredText(window, titleFont,
                     "SCORE " + paddedScore(score),
                     20, 220.0f, Colors::TextLight);

    drawCenteredText(window, titleFont,
                     "HIGH SCORE " + paddedScore(highScore),
                     15, 262.0f, Colors::Gold);

    drawCenteredText(window, textFont,
                     "LEVEL REACHED: " + std::to_string(level),
                     16, 304.0f, Colors::TextDim);

    addButton(sf::Vector2f(285.0f, 365.0f),
              sf::Vector2f(330.0f, 56.0f),
              MenuAction::Play,
              "PLAY AGAIN");

    addButton(sf::Vector2f(285.0f, 445.0f),
              sf::Vector2f(330.0f, 56.0f),
              MenuAction::MainMenu,
              "MAIN MENU");

    for (int i = 0; i < static_cast<int>(buttons.size()); i++)
    {
        drawButton(window, textFont, i);
    }

    drawCenteredText(window, textFont,
                     "R TO RESTART     M FOR MENU",
                     13, 545.0f, Colors::TextDim);
}

void Menu::drawVictory(sf::RenderWindow& window,
                       const sf::Font& titleFont,
                       const sf::Font& textFont,
                       int score,
                       int highScore,
                       float time)
{
    clearButtons(GameState::Victory);

    sf::RectangleShape overlay(sf::Vector2f(static_cast<float>(Config::WINDOW_WIDTH),
                                            static_cast<float>(Config::WINDOW_HEIGHT)));
    overlay.setFillColor(sf::Color(3, 8, 20, 185));
    window.draw(overlay);

    drawGhostRow(window, time, 170.0f);
    drawTitle(window, titleFont, "YOU WIN!", 46, 90.0f, Colors::Gold);
    drawCenteredText(window, titleFont, "ALL 3 MAZES CLEARED", 16, 230.0f, Colors::Accent);

    drawCenteredText(window, titleFont,
                     "FINAL SCORE " + paddedScore(score),
                     20, 300.0f, Colors::TextLight);

    drawCenteredText(window, titleFont,
                     "HIGH SCORE " + paddedScore(highScore),
                     15, 344.0f, Colors::Gold);

    addButton(sf::Vector2f(285.0f, 405.0f),
              sf::Vector2f(330.0f, 56.0f),
              MenuAction::Play,
              "PLAY AGAIN");

    addButton(sf::Vector2f(285.0f, 485.0f),
              sf::Vector2f(330.0f, 56.0f),
              MenuAction::MainMenu,
              "MAIN MENU");

    for (int i = 0; i < static_cast<int>(buttons.size()); i++)
    {
        drawButton(window, textFont, i);
    }

    drawCenteredText(window, textFont,
                     "THANK YOU FOR PLAYING!",
                     15, 590.0f, Colors::TextDim);
}
