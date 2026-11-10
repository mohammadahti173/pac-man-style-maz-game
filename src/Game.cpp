#include "Game.h"

#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "AssetPaths.h"
#include "GraphicsUtils.h"

namespace
{
    void drawTextAt(sf::RenderWindow& window,
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

    void drawTextCentered(sf::RenderWindow& window,
                          const sf::Font& font,
                          const std::string& text,
                          unsigned int characterSize,
                          float centerX,
                          float y,
                          sf::Color color)
    {
        sf::Text drawText(font, text, characterSize);
        drawText.setFillColor(color);

        sf::FloatRect bounds = drawText.getLocalBounds();
        drawText.setOrigin(sf::Vector2f(bounds.position.x + bounds.size.x * 0.5f,
                                        bounds.position.y));
        drawText.setPosition(sf::Vector2f(centerX, y));
        window.draw(drawText);
    }

    void drawTextRight(sf::RenderWindow& window,
                       const sf::Font& font,
                       const std::string& text,
                       unsigned int characterSize,
                       float rightX,
                       float y,
                       sf::Color color)
    {
        sf::Text drawText(font, text, characterSize);
        drawText.setFillColor(color);

        sf::FloatRect bounds = drawText.getLocalBounds();
        drawText.setOrigin(sf::Vector2f(bounds.position.x + bounds.size.x,
                                        bounds.position.y));
        drawText.setPosition(sf::Vector2f(rightX, y));
        window.draw(drawText);
    }
}

Game::Game()
    : window(sf::VideoMode::getDesktopMode(),
             "NEON PAC-MAN MAZE",
             sf::Style::Default,
             sf::State::Fullscreen),
      fullscreen(true)
{
    window.setVerticalSyncEnabled(true);
    window.setKeyRepeatEnabled(false);

    applyView();

    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    loadFonts();
    loadHighScore();
    createStars();

    state = GameState::MainMenu;
    level = 1;
    score = 0;
    lives = Config::START_LIVES;
    difficulty = 1;
    soundEnabled = true;

    powerTimer = 0.0f;
    invulnerableTimer = 0.0f;
    ghostsEatenInPower = 0;
    stateTimer = 0.0f;
    gameTime = 0.0f;

    bonusActive = false;
    bonusTile = sf::Vector2i(0, 0);
    bonusTimer = 0.0f;
    dotsEaten = 0;
    nextBonusAt = 40;

    sounds.setEnabled(soundEnabled);

    menu.resetSelection();
}

void Game::run()
{
    sf::Clock clock;

    while (window.isOpen())
    {
        float deltaTime = clock.restart().asSeconds();

        // Protect the game from huge time jumps (for example after dragging the
        // window). This keeps movement stable.
        if (deltaTime > 0.05f)
        {
            deltaTime = 0.05f;
        }

        gameTime += deltaTime;

        processEvents();
        update(deltaTime);
        draw();
    }

    saveHighScore();
}

void Game::applyView()
{
    sf::Vector2u size = window.getSize();

    if (size.x == 0 || size.y == 0)
    {
        return;
    }

    float windowRatio = static_cast<float>(size.x) / static_cast<float>(size.y);
    float viewRatio = static_cast<float>(Config::WINDOW_WIDTH) /
                      static_cast<float>(Config::WINDOW_HEIGHT);

    float viewportWidth = 1.0f;
    float viewportHeight = 1.0f;

    // Letterbox: keep the game's aspect ratio and center it on the screen.
    if (windowRatio > viewRatio)
    {
        viewportWidth = viewRatio / windowRatio;
    }
    else
    {
        viewportHeight = windowRatio / viewRatio;
    }

    view.setSize(sf::Vector2f(static_cast<float>(Config::WINDOW_WIDTH),
                              static_cast<float>(Config::WINDOW_HEIGHT)));
    view.setCenter(sf::Vector2f(static_cast<float>(Config::WINDOW_WIDTH) * 0.5f,
                                static_cast<float>(Config::WINDOW_HEIGHT) * 0.5f));
    view.setViewport(sf::FloatRect(
        sf::Vector2f((1.0f - viewportWidth) * 0.5f, (1.0f - viewportHeight) * 0.5f),
        sf::Vector2f(viewportWidth, viewportHeight)));

    window.setView(view);
}

void Game::toggleFullscreen()
{
    fullscreen = !fullscreen;

    if (fullscreen)
    {
        window.create(sf::VideoMode::getDesktopMode(),
                      "NEON PAC-MAN MAZE",
                      sf::Style::Default,
                      sf::State::Fullscreen);
    }
    else
    {
        window.create(sf::VideoMode({Config::WINDOW_WIDTH, Config::WINDOW_HEIGHT}),
                      "NEON PAC-MAN MAZE",
                      sf::Style::Titlebar | sf::Style::Close | sf::Style::Resize,
                      sf::State::Windowed);
    }

    window.setVerticalSyncEnabled(true);
    window.setKeyRepeatEnabled(false);

    applyView();
}

void Game::processEvents()
{
    while (const std::optional<sf::Event> event = window.pollEvent())
    {
        if (event->is<sf::Event::Closed>())
        {
            saveHighScore();
            window.close();
        }
        else if (const auto* key = event->getIf<sf::Event::KeyPressed>())
        {
            handleKeyPress(key->code);
        }
        else if (const auto* mouse = event->getIf<sf::Event::MouseButtonPressed>())
        {
            if (mouse->button == sf::Mouse::Button::Left)
            {
                sf::Vector2f mousePosition = window.mapPixelToCoords(mouse->position);

                MenuAction action = menu.handleClick(mousePosition, state);

                if (action != MenuAction::None)
                {
                    sounds.playClick();
                    handleMenuAction(action);
                }
            }
        }
        else if (event->is<sf::Event::Resized>())
        {
            applyView();
        }
    }
}

void Game::handleKeyPress(sf::Keyboard::Key key)
{
    // F11 switches between fullscreen and a resizable window at any time.
    if (key == sf::Keyboard::Key::F11)
    {
        toggleFullscreen();
        return;
    }

    if (state == GameState::MainMenu)
    {
        if (key == sf::Keyboard::Key::W || key == sf::Keyboard::Key::Up)
        {
            menu.moveSelection(-1);
        }
        else if (key == sf::Keyboard::Key::S || key == sf::Keyboard::Key::Down)
        {
            menu.moveSelection(1);
        }
        else if (key == sf::Keyboard::Key::Enter || key == sf::Keyboard::Key::Space)
        {
            MenuAction action = menu.activateSelection();
            if (action != MenuAction::None)
            {
                sounds.playClick();
                handleMenuAction(action);
            }
        }
        else if (key == sf::Keyboard::Key::Escape)
        {
            saveHighScore();
            window.close();
        }
        return;
    }

    if (state == GameState::Instructions)
    {
        if (key == sf::Keyboard::Key::Escape || key == sf::Keyboard::Key::Backspace)
        {
            changeState(GameState::MainMenu);
        }
        else if (key == sf::Keyboard::Key::Enter || key == sf::Keyboard::Key::Space)
        {
            MenuAction action = menu.activateSelection();
            if (action != MenuAction::None)
            {
                sounds.playClick();
                handleMenuAction(action);
            }
        }
        return;
    }

    if (state == GameState::Settings)
    {
        if (key == sf::Keyboard::Key::W || key == sf::Keyboard::Key::Up)
        {
            menu.moveSelection(-1);
        }
        else if (key == sf::Keyboard::Key::S || key == sf::Keyboard::Key::Down)
        {
            menu.moveSelection(1);
        }
        else if (key == sf::Keyboard::Key::Enter || key == sf::Keyboard::Key::Space)
        {
            MenuAction action = menu.activateSelection();
            if (action != MenuAction::None)
            {
                sounds.playClick();
                handleMenuAction(action);
            }
        }
        else if (key == sf::Keyboard::Key::Escape)
        {
            changeState(GameState::MainMenu);
        }
        return;
    }

    if (state == GameState::Playing)
    {
        if (key == sf::Keyboard::Key::W || key == sf::Keyboard::Key::Up)
        {
            player.setDesiredDirection(Direction::Up);
        }
        else if (key == sf::Keyboard::Key::S || key == sf::Keyboard::Key::Down)
        {
            player.setDesiredDirection(Direction::Down);
        }
        else if (key == sf::Keyboard::Key::A || key == sf::Keyboard::Key::Left)
        {
            player.setDesiredDirection(Direction::Left);
        }
        else if (key == sf::Keyboard::Key::D || key == sf::Keyboard::Key::Right)
        {
            player.setDesiredDirection(Direction::Right);
        }
        else if (key == sf::Keyboard::Key::P || key == sf::Keyboard::Key::Escape)
        {
            sounds.playClick();
            changeState(GameState::Paused);
        }
        return;
    }

    if (state == GameState::Paused)
    {
        if (key == sf::Keyboard::Key::P || key == sf::Keyboard::Key::Escape)
        {
            sounds.playClick();
            changeState(GameState::Playing);
        }
        else if (key == sf::Keyboard::Key::W || key == sf::Keyboard::Key::Up)
        {
            menu.moveSelection(-1);
        }
        else if (key == sf::Keyboard::Key::S || key == sf::Keyboard::Key::Down)
        {
            menu.moveSelection(1);
        }
        else if (key == sf::Keyboard::Key::Enter || key == sf::Keyboard::Key::Space)
        {
            MenuAction action = menu.activateSelection();
            if (action != MenuAction::None)
            {
                sounds.playClick();
                handleMenuAction(action);
            }
        }
        else if (key == sf::Keyboard::Key::R)
        {
            startNewGame();
        }
        else if (key == sf::Keyboard::Key::M)
        {
            saveHighScore();
            changeState(GameState::MainMenu);
        }
        return;
    }

    if (state == GameState::GameOver || state == GameState::Victory)
    {
        if (key == sf::Keyboard::Key::W || key == sf::Keyboard::Key::Up)
        {
            menu.moveSelection(-1);
        }
        else if (key == sf::Keyboard::Key::S || key == sf::Keyboard::Key::Down)
        {
            menu.moveSelection(1);
        }
        else if (key == sf::Keyboard::Key::Enter || key == sf::Keyboard::Key::Space)
        {
            MenuAction action = menu.activateSelection();
            if (action != MenuAction::None)
            {
                sounds.playClick();
                handleMenuAction(action);
            }
        }
        else if (key == sf::Keyboard::Key::R)
        {
            startNewGame();
        }
        else if (key == sf::Keyboard::Key::M || key == sf::Keyboard::Key::Escape)
        {
            saveHighScore();
            changeState(GameState::MainMenu);
        }
        return;
    }
}

void Game::handleMenuAction(MenuAction action)
{
    if (action == MenuAction::Play)
    {
        startNewGame();
    }
    else if (action == MenuAction::Instructions)
    {
        changeState(GameState::Instructions);
    }
    else if (action == MenuAction::Settings)
    {
        changeState(GameState::Settings);
    }
    else if (action == MenuAction::Exit)
    {
        saveHighScore();
        window.close();
    }
    else if (action == MenuAction::Back)
    {
        changeState(GameState::MainMenu);
    }
    else if (action == MenuAction::Resume)
    {
        changeState(GameState::Playing);
    }
    else if (action == MenuAction::Restart)
    {
        startNewGame();
    }
    else if (action == MenuAction::MainMenu)
    {
        saveHighScore();
        changeState(GameState::MainMenu);
    }
    else if (action == MenuAction::ToggleSound)
    {
        soundEnabled = !soundEnabled;
        sounds.setEnabled(soundEnabled);

        // Give a short confirmation when the sound is switched back on.
        if (soundEnabled)
        {
            sounds.playClick();
        }
    }
    else if (action == MenuAction::ChangeDifficulty)
    {
        difficulty = difficulty + 1;
        if (difficulty > 2)
        {
            difficulty = 0;
        }
    }
}

void Game::changeState(GameState newState)
{
    state = newState;
    stateTimer = 0.0f;
    menu.resetSelection();
}

void Game::update(float deltaTime)
{
    if (state == GameState::Playing)
    {
        updatePlaying(deltaTime);
    }
    else if (state == GameState::Dying)
    {
        stateTimer -= deltaTime;

        if (stateTimer <= 0.0f)
        {
            lives--;

            if (lives <= 0)
            {
                lives = 0;
                saveHighScore();
                changeState(GameState::GameOver);
            }
            else
            {
                resetActors();
                invulnerableTimer = 2.2f;
                changeState(GameState::Playing);
            }
        }
    }
    else if (state == GameState::LevelComplete)
    {
        stateTimer -= deltaTime;

        if (stateTimer <= 0.0f)
        {
            if (level >= Config::TOTAL_LEVELS)
            {
                saveHighScore();
                changeState(GameState::Victory);
            }
            else
            {
                loadLevel(level + 1);
                changeState(GameState::Playing);
            }
        }
    }
}

void Game::updatePlaying(float deltaTime)
{
    if (invulnerableTimer > 0.0f)
    {
        invulnerableTimer -= deltaTime;
        if (invulnerableTimer < 0.0f)
        {
            invulnerableTimer = 0.0f;
        }
    }

    if (powerTimer > 0.0f)
    {
        powerTimer -= deltaTime;

        if (powerTimer <= 0.0f)
        {
            powerTimer = 0.0f;
            ghostsEatenInPower = 0;
        }
    }

    if (bonusActive)
    {
        bonusTimer -= deltaTime;

        if (bonusTimer <= 0.0f)
        {
            bonusActive = false;
        }
    }

    player.update(deltaTime, maze);

    // Use the tile under the center of Pac-Man for collecting. This is safer
    // than currentTile, because an instant reversal can swap the two tiles.
    sf::Vector2i playerTile = maze.pixelToTile(player.getPosition());
    bool wasPowerPellet = false;
    int collectedPoints = maze.collectItem(playerTile.x, playerTile.y, wasPowerPellet);

    if (collectedPoints > 0)
    {
        addScore(collectedPoints);
        dotsEaten++;

        if (wasPowerPellet)
        {
            powerTimer = getPowerDuration();
            ghostsEatenInPower = 0;
            sounds.playPower();
        }
        else
        {
            sounds.playEat();
        }

        if (!bonusActive && dotsEaten >= nextBonusAt)
        {
            spawnBonus();
        }
    }

    // The level ends immediately when the last dot is eaten.
    if (maze.getRemainingDots() <= 0)
    {
        finishLevel();
        return;
    }

    // Check the bonus fruit.
    if (bonusActive)
    {
        sf::Vector2f bonusPosition = tileCenterPosition(bonusTile);
        float dx = player.getPosition().x - bonusPosition.x;
        float dy = player.getPosition().y - bonusPosition.y;
        float distance = std::sqrt(dx * dx + dy * dy);

        if (distance < 20.0f)
        {
            addScore(100 * level);
            bonusActive = false;
            sounds.playPower();
        }
    }

    bool frightened = powerTimer > 0.0f;
    float ghostSpeedFactor = getGhostSpeedFactor();

    for (Enemy& enemy : enemies)
    {
        enemy.update(deltaTime,
                     maze,
                     player.getCurrentTile(),
                     player.getFacingDirection(),
                     frightened,
                     ghostSpeedFactor);
    }

    // Check collisions between Pac-Man and the ghosts.
    for (Enemy& enemy : enemies)
    {
        if (!enemy.isActive())
        {
            continue;
        }

        float dx = player.getPosition().x - enemy.getPosition().x;
        float dy = player.getPosition().y - enemy.getPosition().y;
        float distance = std::sqrt(dx * dx + dy * dy);
        float collisionDistance = (player.getRadius() + enemy.getRadius()) * 0.82f;

        if (distance < collisionDistance)
        {
            if (frightened)
            {
                enemy.eat();

                int ghostPoints = 200 * (1 << ghostsEatenInPower);
                ghostsEatenInPower++;

                if (ghostsEatenInPower > 4)
                {
                    ghostsEatenInPower = 4;
                }

                addScore(ghostPoints);
                sounds.playGhost();
            }
            else if (invulnerableTimer <= 0.0f)
            {
                loseLife();
                return;
            }
        }
    }
}

void Game::startNewGame()
{
    saveHighScore();

    score = 0;
    lives = Config::START_LIVES;
    level = 1;
    powerTimer = 0.0f;
    invulnerableTimer = 0.0f;
    ghostsEatenInPower = 0;
    bonusActive = false;
    bonusTimer = 0.0f;
    dotsEaten = 0;
    nextBonusAt = 40;

    loadLevel(1);
    changeState(GameState::Playing);
}

void Game::loadLevel(int levelNumber)
{
    level = levelNumber;
    maze.loadLevel(level);

    player.reset(maze.getPlayerStart(), maze);

    enemies.clear();

    std::vector<sf::Vector2i> ghostStarts = maze.getGhostStarts();

    // These indices spread the ghosts around the little ghost room. The room
    // contains nine G tiles, so the values are always valid.
    int spawnOrder[4] = {4, 1, 7, 3};

    sf::Color ghostColors[4] =
    {
        sf::Color(255, 70, 70),    // red chaser
        sf::Color(255, 120, 200),  // pink ambusher
        sf::Color(60, 220, 255),   // cyan unpredictable ghost
        sf::Color(255, 160, 50)    // orange wanderer
    };

    int ghostCount = 4;
    if (level == 1)
    {
        ghostCount = 3; // The first level is a little easier.
    }

    for (int i = 0; i < ghostCount; i++)
    {
        Enemy enemy;
        enemy.setup(ghostColors[i], i);

        int startIndex = spawnOrder[i];
        if (startIndex >= static_cast<int>(ghostStarts.size()))
        {
            startIndex = i % static_cast<int>(ghostStarts.size());
        }

        enemy.reset(ghostStarts[startIndex], maze);
        enemies.push_back(enemy);
    }

    powerTimer = 0.0f;
    invulnerableTimer = 0.0f;
    ghostsEatenInPower = 0;

    bonusActive = false;
    bonusTimer = 0.0f;
    dotsEaten = 0;
    nextBonusAt = 40;
}

void Game::resetActors()
{
    player.reset(maze.getPlayerStart(), maze);

    for (Enemy& enemy : enemies)
    {
        enemy.reset(enemy.getStartTile(), maze);
    }

    powerTimer = 0.0f;
    invulnerableTimer = 2.0f;
    ghostsEatenInPower = 0;
    bonusActive = false;
    bonusTimer = 0.0f;
}

void Game::loseLife()
{
    if (state == GameState::Dying)
    {
        return;
    }

    sounds.playDeath();
    changeState(GameState::Dying);
    stateTimer = 1.5f;
}

void Game::finishLevel()
{
    addScore(500 * level);
    sounds.playLevel();
    saveHighScore();

    changeState(GameState::LevelComplete);
    stateTimer = 2.2f;
}

void Game::spawnBonus()
{
    std::vector<sf::Vector2i> tiles = maze.getWalkableTiles();

    if (tiles.empty())
    {
        return;
    }

    // Try a few random tiles and avoid the player's tile and the ghost room.
    for (int attempt = 0; attempt < 40; attempt++)
    {
        int index = std::rand() % static_cast<int>(tiles.size());
        sf::Vector2i candidate = tiles[index];

        if (candidate == player.getCurrentTile())
        {
            continue;
        }

        if (maze.getTile(candidate.x, candidate.y) == 'G')
        {
            continue;
        }

        bonusTile = candidate;
        bonusActive = true;
        bonusTimer = 8.0f;
        nextBonusAt += 55;
        return;
    }
}

void Game::addScore(int points)
{
    score += points;

    if (score > highScore)
    {
        highScore = score;
    }
}

float Game::getGhostSpeedFactor() const
{
    float difficultyFactor = 1.0f;

    if (difficulty == 0)
    {
        difficultyFactor = 0.85f;
    }
    else if (difficulty == 2)
    {
        difficultyFactor = 1.10f;
    }

    float levelFactor = 1.0f + (level - 1) * 0.06f;

    return difficultyFactor * levelFactor;
}

float Game::getPowerDuration() const
{
    float durations[3] = {8.0f, 7.0f, 6.0f};
    float duration = durations[difficulty];

    // Higher levels make the power-up slightly shorter.
    duration -= (level - 1) * 0.5f;

    if (duration < 4.0f)
    {
        duration = 4.0f;
    }

    return duration;
}

void Game::draw()
{
    window.clear(Colors::Background);
    drawBackground();

    if (state == GameState::MainMenu)
    {
        menu.drawMainMenu(window, titleFont, textFont, highScore, gameTime);
    }
    else if (state == GameState::Instructions)
    {
        menu.drawInstructions(window, titleFont, textFont, gameTime);
    }
    else if (state == GameState::Settings)
    {
        menu.drawSettings(window, titleFont, textFont, soundEnabled, difficulty, gameTime);
    }
    else if (state == GameState::Playing)
    {
        drawPlaying();
        drawHUD();
    }
    else if (state == GameState::Paused)
    {
        drawPlaying();
        drawHUD();
        menu.drawPause(window, titleFont, textFont, score, level, gameTime);
    }
    else if (state == GameState::Dying)
    {
        drawPlaying();
        drawHUD();
        drawDeathOverlay();
    }
    else if (state == GameState::LevelComplete)
    {
        drawPlaying();
        drawHUD();
        drawLevelCompleteOverlay();
    }
    else if (state == GameState::GameOver)
    {
        drawPlaying();
        drawHUD();
        menu.drawGameOver(window, titleFont, textFont,
                          score, highScore, level, gameTime);
    }
    else if (state == GameState::Victory)
    {
        drawPlaying();
        drawHUD();
        menu.drawVictory(window, titleFont, textFont, score, highScore, gameTime);
    }

    window.display();
}

void Game::drawBackground()
{
    // A few large, very transparent circles create a soft glow behind the
    // maze. They are drawn from largest to smallest.
    for (int i = 0; i < 6; i++)
    {
        float radius = 460.0f - i * 65.0f;
        drawGlowCircle(window,
                       sf::Vector2f(Config::WINDOW_WIDTH * 0.5f, 430.0f),
                       radius,
                       sf::Color(10, 30, 85, 9));
    }

    // Slowly drifting stars.
    for (const Star& star : stars)
    {
        float y = star.position.y + gameTime * star.speed;
        y = std::fmod(y, static_cast<float>(Config::WINDOW_HEIGHT));

        float brightness = 0.5f + 0.5f * std::sin(gameTime * 1.7f + star.phase);
        std::uint8_t alpha = static_cast<std::uint8_t>(35 + brightness * 70.0f);

        sf::CircleShape starShape(star.radius, 8);
        starShape.setOrigin(sf::Vector2f(star.radius, star.radius));
        starShape.setPosition(sf::Vector2f(star.position.x, y));
        starShape.setFillColor(sf::Color(160, 215, 255, alpha));
        window.draw(starShape);
    }
}

void Game::drawPlaying()
{
    maze.draw(window, gameTime);
    drawBonus();

    bool frightened = powerTimer > 0.0f;
    bool flash = frightened && powerTimer < 2.0f &&
                 (static_cast<int>(powerTimer * 8.0f) % 2 == 0);

    for (Enemy& enemy : enemies)
    {
        enemy.draw(window, gameTime, frightened, flash);
    }

    bool blink = (invulnerableTimer > 0.0f) || (state == GameState::Dying);
    player.draw(window, gameTime, powerTimer > 0.0f, blink);

    drawPowerEffect();
}

void Game::drawBonus()
{
    if (!bonusActive)
    {
        return;
    }

    float pulse = 1.0f + 0.12f * std::sin(gameTime * 6.0f);
    sf::Vector2f center = tileCenterPosition(bonusTile);

    drawGlowCircle(window, center, 18.0f * pulse, sf::Color(255, 80, 120, 55));

    // Two cherries and a small green stem.
    sf::CircleShape leftCherry(6.0f, 20);
    leftCherry.setOrigin(sf::Vector2f(6.0f, 6.0f));
    leftCherry.setPosition(sf::Vector2f(center.x - 5.0f, center.y + 3.0f));
    leftCherry.setFillColor(sf::Color(240, 40, 80));
    window.draw(leftCherry);

    sf::CircleShape rightCherry(6.0f, 20);
    rightCherry.setOrigin(sf::Vector2f(6.0f, 6.0f));
    rightCherry.setPosition(sf::Vector2f(center.x + 5.0f, center.y + 5.0f));
    rightCherry.setFillColor(sf::Color(255, 70, 110));
    window.draw(rightCherry);

    sf::RectangleShape stem(sf::Vector2f(2.0f, 11.0f));
    stem.setOrigin(sf::Vector2f(1.0f, 11.0f));
    stem.setPosition(sf::Vector2f(center.x, center.y - 2.0f));
    stem.setRotation(sf::degrees(25.0f));
    stem.setFillColor(sf::Color(80, 220, 120));
    window.draw(stem);

    sf::CircleShape leaf(3.0f, 12);
    leaf.setOrigin(sf::Vector2f(3.0f, 3.0f));
    leaf.setPosition(sf::Vector2f(center.x + 4.0f, center.y - 11.0f));
    leaf.setFillColor(sf::Color(80, 220, 120));
    window.draw(leaf);
}

void Game::drawPowerEffect()
{
    if (powerTimer <= 0.0f)
    {
        return;
    }

    float pulse = 0.5f + 0.5f * std::sin(gameTime * 8.0f);

    sf::RectangleShape tint(sf::Vector2f(Config::MAZE_COLUMNS * Config::TILE_SIZE,
                                         Config::MAZE_ROWS * Config::TILE_SIZE));
    tint.setPosition(sf::Vector2f(Config::MAZE_LEFT, Config::MAZE_TOP));
    tint.setFillColor(sf::Color(40, 220, 255,
                                static_cast<std::uint8_t>(7 + pulse * 8.0f)));
    window.draw(tint);

    sf::RectangleShape border(sf::Vector2f(Config::MAZE_COLUMNS * Config::TILE_SIZE + 8.0f,
                                           Config::MAZE_ROWS * Config::TILE_SIZE + 8.0f));
    border.setPosition(sf::Vector2f(Config::MAZE_LEFT - 4.0f, Config::MAZE_TOP - 4.0f));
    border.setFillColor(sf::Color::Transparent);
    border.setOutlineThickness(3.0f);
    border.setOutlineColor(sf::Color(100, 240, 255,
                                     static_cast<std::uint8_t>(90 + pulse * 100.0f)));
    window.draw(border);
}

void Game::drawHUD()
{
    sf::RectangleShape topBar(sf::Vector2f(static_cast<float>(Config::WINDOW_WIDTH), 92.0f));
    topBar.setPosition(sf::Vector2f(0.0f, 0.0f));
    topBar.setFillColor(sf::Color(7, 11, 32, 238));
    window.draw(topBar);

    sf::RectangleShape topLine(sf::Vector2f(static_cast<float>(Config::WINDOW_WIDTH), 2.0f));
    topLine.setPosition(sf::Vector2f(0.0f, 90.0f));
    topLine.setFillColor(sf::Color(0, 190, 255, 120));
    window.draw(topLine);

    // Score on the left.
    drawTextAt(window, textFont, "SCORE", 12, 40.0f, 14.0f, Colors::Accent);
    drawTextAt(window, titleFont, formatScore(score), 16, 40.0f, 38.0f, Colors::TextLight);

    // High score in the middle.
    drawTextCentered(window, textFont, "HIGH SCORE", 12,
                     Config::WINDOW_WIDTH * 0.5f, 14.0f, Colors::Accent);
    drawTextCentered(window, titleFont, formatScore(highScore), 16,
                     Config::WINDOW_WIDTH * 0.5f, 38.0f, Colors::Gold);

    // Level on the right.
    drawTextRight(window, textFont, "LEVEL", 12,
                  Config::WINDOW_WIDTH - 40.0f, 14.0f, Colors::Accent);
    drawTextRight(window, titleFont, std::to_string(level), 16,
                  Config::WINDOW_WIDTH - 40.0f, 38.0f, Colors::TextLight);

    // Lives and small Pac-Man icons at the bottom left.
    drawTextAt(window, textFont, "LIVES", 11, 40.0f, 738.0f, Colors::Accent);

    for (int i = 0; i < lives; i++)
    {
        drawPacManShape(window,
                        sf::Vector2f(108.0f + i * 25.0f, 757.0f),
                        8.0f,
                        Colors::PlayerYellow,
                        180.0f,
                        28.0f);
    }

    // Power-up bar in the bottom middle.
    if (powerTimer > 0.0f)
    {
        drawTextCentered(window, textFont, "POWER MODE", 11,
                         Config::WINDOW_WIDTH * 0.5f, 736.0f, Colors::Accent);

        float fullWidth = 240.0f;
        float ratio = powerTimer / getPowerDuration();
        if (ratio < 0.0f) ratio = 0.0f;
        if (ratio > 1.0f) ratio = 1.0f;

        sf::RectangleShape background(sf::Vector2f(fullWidth, 9.0f));
        background.setPosition(sf::Vector2f(Config::WINDOW_WIDTH * 0.5f - fullWidth * 0.5f,
                                            754.0f));
        background.setFillColor(sf::Color(10, 24, 45));
        background.setOutlineThickness(1.0f);
        background.setOutlineColor(sf::Color(0, 150, 210));
        window.draw(background);

        sf::RectangleShape fill(sf::Vector2f(fullWidth * ratio, 9.0f));
        fill.setPosition(background.getPosition());
        fill.setFillColor(sf::Color(80, 235, 255));
        window.draw(fill);
    }

    // Dots left at the bottom right.
    drawTextRight(window, textFont, "DOTS LEFT", 11,
                  Config::WINDOW_WIDTH - 40.0f, 738.0f, Colors::Accent);
    drawTextRight(window, titleFont, std::to_string(maze.getRemainingDots()), 14,
                  Config::WINDOW_WIDTH - 40.0f, 756.0f, Colors::TextLight);
}

void Game::drawDeathOverlay()
{
    float pulse = 0.5f + 0.5f * std::sin(gameTime * 12.0f);

    sf::RectangleShape overlay(sf::Vector2f(static_cast<float>(Config::WINDOW_WIDTH),
                                            static_cast<float>(Config::WINDOW_HEIGHT)));
    overlay.setFillColor(sf::Color(130, 0, 25,
                                   static_cast<std::uint8_t>(35 + pulse * 35.0f)));
    window.draw(overlay);

    drawTextCentered(window, titleFont, "LIFE LOST", 30,
                     Config::WINDOW_WIDTH * 0.5f, 320.0f, Colors::Danger);

    drawTextCentered(window, textFont, "GET READY...", 15,
                     Config::WINDOW_WIDTH * 0.5f, 372.0f, Colors::TextLight);
}

void Game::drawLevelCompleteOverlay()
{
    sf::RectangleShape overlay(sf::Vector2f(static_cast<float>(Config::WINDOW_WIDTH),
                                            static_cast<float>(Config::WINDOW_HEIGHT)));
    overlay.setFillColor(sf::Color(2, 4, 18, 175));
    window.draw(overlay);

    drawTextCentered(window, titleFont,
                     "LEVEL " + std::to_string(level) + " CLEARED",
                     27, Config::WINDOW_WIDTH * 0.5f, 300.0f, Colors::Accent);

    drawTextCentered(window, textFont,
                     "BONUS +" + std::to_string(500 * level) + " POINTS",
                     18, Config::WINDOW_WIDTH * 0.5f, 352.0f, Colors::Gold);

    if (level >= Config::TOTAL_LEVELS)
    {
        drawTextCentered(window, textFont, "ALL MAZES CLEARED!", 14,
                         Config::WINDOW_WIDTH * 0.5f, 404.0f, Colors::TextLight);
    }
    else
    {
        drawTextCentered(window, textFont, "NEXT MAZE INCOMING...", 14,
                         Config::WINDOW_WIDTH * 0.5f, 404.0f, Colors::TextLight);
    }
}

void Game::loadHighScore()
{
    highScore = 0;

    std::ifstream file("highscore.txt");
    if (file.is_open())
    {
        int value = 0;
        if (file >> value)
        {
            highScore = value;
        }
    }

    if (highScore < 0)
    {
        highScore = 0;
    }
}

void Game::saveHighScore()
{
    std::ofstream file("highscore.txt");
    if (file.is_open())
    {
        file << highScore << "\n";
    }
}

void Game::loadFonts()
{
    std::string titlePath = findAssetPath("assets/fonts/PressStart2P-Regular.ttf");
    if (!titleFont.openFromFile(titlePath))
    {
        if (!titleFont.openFromFile("C:/Windows/Fonts/consola.ttf"))
        {
            (void)titleFont.openFromFile("C:/Windows/Fonts/arial.ttf");
        }
    }

    std::string textPath = findAssetPath("assets/fonts/Orbitron.ttf");
    if (!textFont.openFromFile(textPath))
    {
        if (!textFont.openFromFile("C:/Windows/Fonts/consola.ttf"))
        {
            (void)textFont.openFromFile("C:/Windows/Fonts/arial.ttf");
        }
    }
}

void Game::createStars()
{
    stars.clear();

    for (int i = 0; i < 90; i++)
    {
        Star star;
        star.position = sf::Vector2f(static_cast<float>(std::rand() % Config::WINDOW_WIDTH),
                                     static_cast<float>(std::rand() % Config::WINDOW_HEIGHT));
        star.speed = 3.0f + static_cast<float>(std::rand() % 18);
        star.phase = static_cast<float>(std::rand() % 628) / 100.0f;
        star.radius = 0.8f + static_cast<float>(std::rand() % 3) * 0.45f;
        stars.push_back(star);
    }
}

std::string Game::formatScore(int value) const
{
    std::ostringstream stream;
    stream << std::setw(6) << std::setfill('0') << value;
    return stream.str();
}
