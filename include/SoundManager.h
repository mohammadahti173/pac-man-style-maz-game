#pragma once

#include <SFML/Audio.hpp>
#include <optional>

// -----------------------------------------------------------------------------
// Loads the small arcade sound effects and plays them when something happens.
// If an audio file is missing the game still works; the sound is just skipped.
// -----------------------------------------------------------------------------
class SoundManager
{
public:
    SoundManager();

    void setEnabled(bool newValue);
    bool isEnabled() const;

    void playEat();
    void playPower();
    void playGhost();
    void playDeath();
    void playLevel();
    void playClick();

private:
    bool enabled;

    sf::SoundBuffer eatBuffer;
    sf::SoundBuffer powerBuffer;
    sf::SoundBuffer ghostBuffer;
    sf::SoundBuffer deathBuffer;
    sf::SoundBuffer levelBuffer;
    sf::SoundBuffer clickBuffer;

    std::optional<sf::Sound> eatSound;
    std::optional<sf::Sound> powerSound;
    std::optional<sf::Sound> ghostSound;
    std::optional<sf::Sound> deathSound;
    std::optional<sf::Sound> levelSound;
    std::optional<sf::Sound> clickSound;

    void play(std::optional<sf::Sound>& sound);
};
