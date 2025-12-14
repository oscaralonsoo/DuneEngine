#include "AssetSearch.h"
#include <algorithm>
#include <filesystem>

AssetSearch::AssetSearch()
    : searchQuery("")
{
}

void AssetSearch::SetQuery(const std::string& query)
{
    // Trim whitespace convert to lowercase
    searchQuery = query;
    // Remove whitespace
    searchQuery.erase(searchQuery.begin(), std::find_if(searchQuery.begin(), searchQuery.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
    searchQuery.erase(std::find_if(searchQuery.rbegin(), searchQuery.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), searchQuery.end());

    // Convert to lowercase
    std::transform(searchQuery.begin(), searchQuery.end(), searchQuery.begin(), ::tolower);
}

bool AssetSearch::Matches(const std::filesystem::path& assetPath) const
{
    if (!IsActive())
        return true;

    std::string filename = assetPath.filename().string();
    // Convert filename to lowercase
    std::transform(filename.begin(), filename.end(), filename.begin(), ::tolower);

    return ContainsQuery(filename);
}

std::vector<std::filesystem::path> AssetSearch::SearchInDirectory(
    const std::filesystem::path& directory,
    bool recursive) const
{
    std::vector<std::filesystem::path> results;
    if (!std::filesystem::exists(directory) || !std::filesystem::is_directory(directory))
        return results;

    try
    {
        if (recursive)
        {
            for (const auto& entry : std::filesystem::recursive_directory_iterator(directory))
            {
                if (Matches(entry.path()))
                {
                    results.push_back(entry.path());
                }
            }
        }
        else
        {
            for (const auto& entry : std::filesystem::directory_iterator(directory))
            {
                if (Matches(entry.path()))
                {
                    results.push_back(entry.path());
                }
            }
        }
    }
    catch (const std::filesystem::filesystem_error&)
    {
    }
    return results;
}

void AssetSearch::Clear()
{
    searchQuery.clear();
}

bool AssetSearch::ContainsQuery(const std::string& text) const
{
    if (searchQuery.empty())
        return true;
        
    return text.find(searchQuery) != std::string::npos;
}
