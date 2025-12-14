#pragma once

#include <filesystem>
#include <vector>
#include <string>
#include <functional>

class AssetSearch
{
public:
    AssetSearch();

    void SetQuery(const std::string& query);

    const std::string& GetQuery() const { return searchQuery; }

    // Check if search active
    bool IsActive() const { return !searchQuery.empty(); }

    // Returns true if asset matches search criteria
    bool Matches(const std::filesystem::path& assetPath) const;

    // Returns matching asset paths
    std::vector<std::filesystem::path> SearchInDirectory(
        const std::filesystem::path& directory,
        bool recursive = true
    ) const;

    void Clear();

private:
    std::string searchQuery;
    bool ContainsQuery(const std::string& text) const;
};
