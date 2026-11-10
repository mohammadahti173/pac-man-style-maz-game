#include "SoundManager.h"

#include "AssetPaths.h"

SoundManager::SoundManager()
{
    enabled = true;

    // Load each sound only if its file can be found.
    if (eatBuffer.loadFromFile(findAssetPath("assets/sounds/eat.wav")))
    {
        eatSound.emplace(eatBuffer);
        eatSound->setVolume(55.0f);
    }

    if (powerBuffer.loadFromFile(findAssetPath("assets/sounds/power.wav")))
    {
        powerSound.emplace(powerBuffer);
        powerSound->setVolume(65.0f);
    }

    if (ghostBuffer.loadFromFile(findAssetPath("assets/sounds/ghost.wav")))
    {
        ghostSound.emplace(ghostBuffer);
        ghostSound->setVolume(65.0f);
    }

    if (deathBuffer.loadFromFile(findAssetPath("assets/sounds/death.wav")))
    {
        deathSound.emplace(deathBuffer);
        deathSound->setVolume(65.0f);
    }

    if (levelBuffer.loadFromFile(findAssetPath("assets/sounds/level.wav")))
    {
        levelSound.emplace(levelBuffer);
        levelSound->setVolume(65.0f);
    }

    if (clickBuffer.loadFromFile(findAssetPath("assets/sounds/click.wav")))
    {
        clickSound.emplace(clickBuffer);
        clickSound->setVolume(45.0f);
    }
}

void SoundManager::setEnabled(bool newValue)
{
    enabled = newValue;
}

bool SoundManager::isEnabled() const
{
    return enabled;
}

void SoundManager::play(std::optional<sf::Sound>& sound)
{
    if (!enabled)
    {
        return;
    }

    if (sound.has_value())
    {
        sound->play();
    }
}

void SoundManager::playEat()
{
    play(eatSound);
}

void SoundManager::playPower()
{
    play(powerSound);
}

void SoundManager::playGhost()
{
    play(ghostSound);
}

void SoundManager::playDeath()
{
    play(deathSound);
}

void SoundManager::playLevel()
{
    play(levelSound);
}

void SoundManager::playClick()
{
    play(clickSound);
}
