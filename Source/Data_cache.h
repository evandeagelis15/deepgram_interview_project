#ifndef DEEPGRAM_PROJECT_DATA_CACHE_H
#define DEEPGRAM_PROJECT_DATA_CACHE_H

#include <sstream>
#include <unordered_map>
#include <vector>

///@brief Manages stored file records and provides functions to query them
class Data_cache {
public:
    Data_cache() = default;

    ///@brief Add new file
    ///@param name name of file
    ///@param wav_data raw wav bytes
    auto store_file(std::string name, const std::vector<uint8_t> &wav_data) -> void;

    ///@brief get raw wav bytes from name of file
    ///@param name name of file
    ///@return vector of bytes
    [[nodiscard]] auto get_wav_data(const std::string &name) -> std::vector<uint8_t>;

    ///@brief get all info of file
    ///@param name name of file
    ///@return json string of file info
    [[nodiscard]] auto get_info(const std::string &name) -> std::string;

    ///@brief get list of all files matching queries
    ///@param query_params pair of query param and value
    ///@return vector of file names
    [[nodiscard]] auto get_list(
        const std::vector<std::pair<std::string, std::string> > &query_params) -> std::vector<std::string>;

    ///@brief get number of stored files
    ///@return number of stored files
    [[nodiscard]] auto get_size() const -> int;

private:
    ///@brief Validate string to int conversion
    ///@param str string to be converted
    ///@param out_int converted int
    ///@return true if successful
    static auto try_parse_int(const std::string &str, int &out_int) -> bool;

    ///@brief Basic structure to hold file information
    struct Stored_file
    {
        std::string name{};

        std::vector<uint8_t> raw_wav_data{};

        int duration{};

        int size{};

        ///@brief write file struct to json string
        [[nodiscard]] auto to_string() const -> std::string
        {
            std::ostringstream json;

            json << "{"
                 << "\"name\":\"" << name << "\","
                 << "\"duration\":" << duration << ","
                 << "\"size\": \"" << size
                 << "KB\"}";

            return json.str();
        };
    };

    ///@brief vector of stored files
    std::vector<Stored_file> file_vector_{};

    ///@brief map of file name to vector position
    std::unordered_map<std::string, size_t> lookup_table_{};
};

#endif //DEEPGRAM_PROJECT_DATA_CACHE_H
