
#include "Data_cache.h"

#include <iostream>
#include <ostream>
#include <ranges>
#include <utility>
#include <__ranges/transform_view.h>

auto Data_cache::store_file(std::string name, const std::vector<uint8_t> &wav_data) -> void
{
    // TODO: Determine duration
    const auto size = static_cast<int>(wav_data.size())/1000;
    const auto new_file = Stored_file{.name = std::move(name), .raw_wav_data = wav_data, .duration = 1, .size = size};
    if (not lookup_table_.contains(name))
    {
        file_vector_.push_back(new_file);
        lookup_table_[new_file.name] = file_vector_.size() - 1;
    }
}

auto Data_cache::get_wav_data(const std::string &name) -> std::vector<uint8_t>
{
    auto out_data = std::vector<uint8_t>{};
    if (lookup_table_.contains(name))
    {
        out_data = file_vector_.at(lookup_table_[name]).raw_wav_data;
    }
    return out_data;
}

auto Data_cache::get_info(const std::string &name) -> std::string
{
    if (not lookup_table_.contains(name))
    {
        std::cerr << "No record for " << name << std::endl;
        return "";
    }
    return file_vector_.at(lookup_table_[name]).to_string();
}

auto Data_cache::get_list(
    const std::vector<std::pair<std::string, std::string> > &query_params) -> std::vector<std::string>
{
    auto matches_all_filters = [&](const Stored_file &file) -> bool {
        for (const auto &[key, value]: query_params)
        {
            auto lookup_key = std::string_view{key};

            if (lookup_key == "name")
            {
                if (file.name != value)
                {
                    return false; // Keep only if name matches
                }
            }
            else if (lookup_key.ends_with("duration"))
            {
                auto lookup = 0;
                // make sure value is good
                auto parsed = try_parse_int(value, lookup);

                // exit if it doesn't meet criteria
                if (lookup_key.starts_with("max") and parsed and file.duration > lookup)
                {
                    return false;
                }
                if (lookup_key.starts_with("min") and parsed and file.duration < lookup)
                {
                    return false;
                }
                if (not lookup_key.starts_with("max") and not lookup_key.starts_with("min") and file.duration != lookup)
                {
                    return false;
                }
            }
            else if (lookup_key.ends_with("size"))
            {
                auto lookup = 0;
                auto parsed = try_parse_int(value, lookup);

                if (lookup_key.starts_with("max") and parsed and file.size > lookup)
                {
                    return false;
                }
                if (lookup_key.starts_with("min") and parsed and file.size < lookup)
                {
                    return false;
                }
                if (not lookup_key.starts_with("max") and not lookup_key.starts_with("min") and file.size != lookup)
                {
                    return false;
                }
            }
        }
        return true; // File passed all active filters
    };

    auto filtered_view = file_vector_
                         | std::views::filter(matches_all_filters)
                         | std::ranges::views::transform([](const Stored_file &file) {
                             return file.name;
                         });


    auto out_data = std::vector<std::string>{};
    std::ranges::copy(filtered_view, std::back_inserter(out_data));

    return out_data;
}

auto Data_cache::get_size() const -> int
{
    return static_cast<int>(file_vector_.size());
}

auto Data_cache::try_parse_int(const std::string &str, int &out_int) -> bool
{
    try
    {
        size_t pos = 0;
        // Parse the string and store the index of the first unconverted character
        out_int = std::stoi(str, &pos);

        // Ensure the entire string was consumed (no trailing garbage characters)
        return pos == str.length();
    }
    catch (const std::invalid_argument &)
    {
        // Thrown if no conversion could be performed at all (e.g., "abc")
        return false;
    }
    catch (const std::out_of_range &)
    {
        // Thrown if the parsed value is too large or small for a standard 32-bit int
        return false;
    }
}

