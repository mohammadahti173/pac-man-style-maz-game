#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

#include "Constants.h"
#include "Enemy.h"
#include "Maze.h"
#include "Menu.h"
#include "Player.h"
#include "SoundManager.h"

// -----------------------------------------------------------------------------
// The Game class owns the window and every important game value: score, lives,
// current level, power-up timer and the current screen. It also contains the
// main loop.
// -----------------------------------------------------------------------------
class Game
{
public:
    Game();
    void run();

private:
    struct Star
    {
        sf::Vector2f position;
        float speed;
        float phase;
        float radius;
    };

    sf::RenderWindow window;
    sf::View view;
    sf::Font titleFont;
    sf::Font textFont;
    bool fullscreen;

    Maze maze;
    Player player;
    std::vector<Enemy> enemies;
    Menu menu;
    SoundManager sounds;

    GameState state;

    int level;
    int score;
    int highScore;
    int lives;
    int difficulty;          // 0 = Chill, 1 = Classic, 2 = Hard
    bool soundEnabled;

    float powerTimer;
    float invulnerableTimer;
    int ghostsEatenInPower;
    float stateTimer;
    float gameTime;

    bool bonusActive;
    sf::Vector2i bonusTile;
    float bonusTimer;
    int dotsEaten;
    int nextBonusAt;

    std::vector<Star> stars;

    void processEvents();
    void applyView();
    void toggleFullscreen();
    void handleKeyPress(sf::Keyboard::Key key);
    void handleMenuAction(MenuAction action);
    void changeState(GameState newState);

    void update(float deltaTime);
    void updatePlaying(float deltaTime);
    void startNewGame();
    void loadLevel(int levelNumber);
    void resetActors();
    void loseLife();
    void finishLevel();
    void spawnBonus();
    void addScore(int points);

    void draw();
    void drawBackground();
    void drawPlaying();
    void drawHUD();
    void drawBonus();
    void drawPowerEffect();
    void drawDeathOverlay();
    void drawLevelCompleteOverlay();

    float getGhostSpeedFactor() const;
    float getPowerDuration() const;

    void loadHighScore();
    void saveHighScore();
    void loadFonts();
    void createStars();
    std::string formatScore(int value) const;
};
