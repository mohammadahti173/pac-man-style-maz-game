#pragma once

#include <filesystem>
#include <string>
#include <vector>

// The game can be started from different working directories (for example from
// the project folder or from a build folder). This helper checks a few common
// places and returns the first path that really exists.
inline std::string findAssetPath(const std::string& relativePath)
{
    std::vector<std::string> candidates;
    candidates.push_back(relativePath);
    candidates.push_back("../" + relativePath);
    candidates.push_back("../../" + relativePath);

    for (const std::string& path : candidates)
    {
        if (std::filesystem::exists(path))
        {
            return path;
        }
    }

    // If nothing was found we return the original name. SFML will simply fail
    // to load it and the game will use its fallback behavior.
    return relativePath;
}
