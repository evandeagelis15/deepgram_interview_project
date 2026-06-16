#define BOOST_TEST_DYN_LINK
//#define BOOST_TEST_MAIN - don't need to repeat this define in more than one cpp file
#define BOOST_TEST_MODULE DataCacheTests
#include <boost/test/included/unit_test.hpp>
#include <vector>
#include <string>
#include <ranges>

#include "Data_cache.h"

// --- Test Fixture for common setup ---
struct CacheFixture {
    Data_cache cache;
    std::vector<uint8_t> dummy_data1{0x01, 0x02, 0x03};
    std::vector<uint8_t> dummy_data2{0x04, 0x05};

    CacheFixture() = default;
};

BOOST_FIXTURE_TEST_SUITE(DataCacheTestSuite, CacheFixture)

// 1. Tests for store_file and get_size
BOOST_AUTO_TEST_CASE(test_store_file_and_size)
{
    BOOST_TEST(cache.get_size() == 0);

    cache.store_file("test1.wav", dummy_data1);
    BOOST_TEST(cache.get_size() == 1);

    // Duplicate insertion: should be ignored, size must stay 1
    cache.store_file("test1.wav", dummy_data2);
    BOOST_TEST(cache.get_size() == 1);

    cache.store_file("test2.wav", dummy_data2);
    BOOST_TEST(cache.get_size() == 2);
}

// 2. Tests for get_wav_data
BOOST_AUTO_TEST_CASE(test_get_wav_data)
{
    cache.store_file("test1.wav", dummy_data1);

    // Verify correct data recovery
    std::vector<uint8_t> retrieved = cache.get_wav_data("test1.wav");
    BOOST_CHECK_EQUAL_COLLECTIONS(retrieved.begin(), retrieved.end(),
                                  dummy_data1.begin(), dummy_data1.end());

    // Verify handling of non-existent files (should return empty vector)
    std::vector<uint8_t> missing = cache.get_wav_data("ghost.wav");
    BOOST_TEST(missing.empty());
}

// 3. Tests for get_info
BOOST_AUTO_TEST_CASE(test_get_info)
{
    cache.store_file("test1.wav", dummy_data1);

    // Verify info string generation
    std::string info = cache.get_info("test1.wav");
    BOOST_TEST(info == "{\"name\":\"test1.wav\",\"duration\":1,\"size\": \"0KB\"}");

    // Verify missing info behavior
    std::string missing_info = cache.get_info("ghost.wav");
    BOOST_TEST(missing_info.empty());
}

// 4. Tests for get_list filtering logic
BOOST_AUTO_TEST_CASE(test_get_list_filtering)
{
    cache.store_file("alpha.wav", dummy_data1); // default duration=1, size=30
    cache.store_file("beta.wav", dummy_data2);  // default duration=1, size=30

    // Test: Empty queries should return all files
    auto all_files = cache.get_list({});
    BOOST_TEST(all_files.size() == 2);

    // Test: Filtering exactly by name
    auto name_filter = cache.get_list({{"name", "alpha.wav"}});
    BOOST_REQUIRE_EQUAL(name_filter.size(), 1);
    BOOST_TEST(name_filter[0] == "alpha.wav");

    // Test: Max constraints matching criteria (duration <= 5)
    auto max_duration_filter = cache.get_list({{"max_duration", "5"}});
    BOOST_TEST(max_duration_filter.size() == 2);

    // Test: Max constraints failing criteria (duration <= 0)
    auto max_duration_fail = cache.get_list({{"max_duration", "0"}});
    BOOST_TEST(max_duration_fail.empty());

    // Test: Min constraints matching criteria (size >= 0)
    auto min_size_filter = cache.get_list({{"min_size", "0"}});
    BOOST_TEST(min_size_filter.size() == 2);

}

// 5. Edge cases: Bad inputs and invalid integer strings
BOOST_AUTO_TEST_CASE(test_invalid_query_parameters)
{
    cache.store_file("alpha.wav", dummy_data1);

    // Garbage integer parsing should bypass the constraint silently
    auto garbage_filter = cache.get_list({{"max_size", "abc"}});
    BOOST_TEST(garbage_filter.size() == 1); // Passes filter because 'parsed' flag resolves false

    // Overflow integer parsing handles gracefully via try_parse_int catch blocks
    auto overflow_filter = cache.get_list({{"min_duration", "999999999999999999999"}});
    BOOST_TEST(overflow_filter.size() == 1); // Passes filter because parsing failed
}

BOOST_AUTO_TEST_SUITE_END()