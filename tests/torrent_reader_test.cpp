#include <gtest/gtest.h>
#include "torrent_reader.h"
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;

class TorrentReaderTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_data_dir = fs::path(__FILE__).parent_path() / "data";
    }

    fs::path test_data_dir;
};

// Test parsing simple integer (wrapped in dict as TorrentReader expects)
TEST_F(TorrentReaderTest, ParseSimpleInteger) {
    // Create a dict with an integer value
    auto temp_file = test_data_dir / "dict_with_int.torrent";
    std::ofstream out(temp_file);
    out << "d5:valuei42ee";
    out.close();
    
    TorrentReader reader(temp_file.string());
    const auto& root = reader.getRoot();
    
    ASSERT_TRUE(root.isDict());
    const auto& dict = root.asDict();
    ASSERT_TRUE(dict.find("value") != dict.end());
    EXPECT_EQ(dict.at("value").asInt(), 42);
    
    fs::remove(temp_file);
}

// Test parsing simple string (wrapped in dict)
TEST_F(TorrentReaderTest, ParseSimpleString) {
    auto temp_file = test_data_dir / "dict_with_string.torrent";
    std::ofstream out(temp_file);
    out << "d5:value5:helloe";
    out.close();
    
    TorrentReader reader(temp_file.string());
    const auto& root = reader.getRoot();
    
    ASSERT_TRUE(root.isDict());
    const auto& dict = root.asDict();
    ASSERT_TRUE(dict.find("value") != dict.end());
    EXPECT_EQ(dict.at("value").asString(), "hello");
    
    fs::remove(temp_file);
}

// Test parsing simple list (wrapped in dict)
TEST_F(TorrentReaderTest, ParseSimpleList) {
    auto temp_file = test_data_dir / "dict_with_list.torrent";
    std::ofstream out(temp_file);
    out << "d4:listli1ei2ei3eee";
    out.close();
    
    TorrentReader reader(temp_file.string());
    const auto& root = reader.getRoot();
    
    ASSERT_TRUE(root.isDict());
    const auto& dict = root.asDict();
    ASSERT_TRUE(dict.find("list") != dict.end());
    
    const auto& list = dict.at("list").asList();
    ASSERT_EQ(list.size(), 3);
    EXPECT_EQ(list[0].asInt(), 1);
    EXPECT_EQ(list[1].asInt(), 2);
    EXPECT_EQ(list[2].asInt(), 3);
    
    fs::remove(temp_file);
}

// Test parsing simple dictionary
TEST_F(TorrentReaderTest, ParseSimpleDictionary) {
    auto filepath = test_data_dir / "simple_dict.torrent";
    ASSERT_TRUE(fs::exists(filepath));
    
    TorrentReader reader(filepath.string());
    const auto& root = reader.getRoot();
    
    ASSERT_TRUE(root.isDict());
    const auto& dict = root.asDict();
    ASSERT_EQ(dict.size(), 1);
    ASSERT_TRUE(dict.find("key") != dict.end());
    EXPECT_EQ(dict.at("key").asString(), "value");
}

// Test parsing nested structure
TEST_F(TorrentReaderTest, ParseNestedStructure) {
    auto filepath = test_data_dir / "nested_struct.torrent";
    ASSERT_TRUE(fs::exists(filepath));
    
    TorrentReader reader(filepath.string());
    const auto& root = reader.getRoot();
    
    ASSERT_TRUE(root.isDict());
    const auto& dict = root.asDict();
    
    // Check "inner" key which contains a list
    ASSERT_TRUE(dict.find("inner") != dict.end());
    const auto& inner = dict.at("inner");
    ASSERT_TRUE(inner.isList());
    
    const auto& list = inner.asList();
    ASSERT_EQ(list.size(), 1);
    
    // Check nested dictionary in list
    ASSERT_TRUE(list[0].isDict());
    const auto& nested_dict = list[0].asDict();
    ASSERT_TRUE(nested_dict.find("key") != nested_dict.end());
    EXPECT_EQ(nested_dict.at("key").asString(), "value");
}

// Test valid torrent structure
TEST_F(TorrentReaderTest, ValidTorrentStructure) {
    auto filepath = test_data_dir / "valid_torrent.torrent";
    ASSERT_TRUE(fs::exists(filepath));
    
    TorrentReader reader(filepath.string());
    
    EXPECT_TRUE(reader.isValidTorrent());
    
    const auto& root = reader.getRoot();
    ASSERT_TRUE(root.isDict());
    
    const auto& dict = root.asDict();
    ASSERT_TRUE(dict.find("info") != dict.end());
    
    const auto& info = dict.at("info");
    ASSERT_TRUE(info.isDict());
    
    const auto& info_dict = info.asDict();
    ASSERT_TRUE(info_dict.find("name") != info_dict.end());
    EXPECT_EQ(info_dict.at("name").asString(), "test.file");
    
    ASSERT_TRUE(info_dict.find("length") != info_dict.end());
    EXPECT_EQ(info_dict.at("length").asInt(), 1000);
}

// Test DictView field() method
TEST_F(TorrentReaderTest, DictViewField) {
    auto filepath = test_data_dir / "valid_torrent.torrent";
    ASSERT_TRUE(fs::exists(filepath));
    
    TorrentReader reader(filepath.string());
    
    DictView view = reader.field();
    
    int count = 0;
    for (const auto& entry : view) {
        count++;
        EXPECT_EQ(entry.key(), "info");
        EXPECT_TRUE(entry.value().isDict());
    }
    EXPECT_EQ(count, 1);
}

// Test error: empty file
TEST_F(TorrentReaderTest, ErrorEmptyFile) {
    auto filepath = test_data_dir / "empty.torrent";
    ASSERT_TRUE(fs::exists(filepath));
    
    EXPECT_THROW({
        TorrentReader reader(filepath.string());
    }, std::runtime_error);
}

// Test error: invalid bencode
TEST_F(TorrentReaderTest, ErrorInvalidBencode) {
    auto filepath = test_data_dir / "invalid.torrent";
    ASSERT_TRUE(fs::exists(filepath));
    
    EXPECT_THROW({
        TorrentReader reader(filepath.string());
    }, std::runtime_error);
}

// Test error: non-existent file
TEST_F(TorrentReaderTest, ErrorNonExistentFile) {
    EXPECT_THROW({
        TorrentReader reader("non_existent_file.torrent");
    }, std::runtime_error);
}

// Test error: file not starting with 'd'
TEST_F(TorrentReaderTest, ErrorNotStartingWithDict) {
    auto filepath = test_data_dir / "simple_int.torrent";
    ASSERT_TRUE(fs::exists(filepath));
    
    EXPECT_THROW({
        TorrentReader reader(filepath.string());
    }, std::runtime_error);
}

// Test TorrentValue type checking
TEST_F(TorrentReaderTest, TorrentValueTypeChecking) {
    auto filepath = test_data_dir / "simple_dict.torrent";
    TorrentReader reader(filepath.string());
    const auto& root = reader.getRoot();
    
    EXPECT_FALSE(root.isInt());
    EXPECT_FALSE(root.isString());
    EXPECT_FALSE(root.isList());
    EXPECT_TRUE(root.isDict());
}

// Test negative integers
TEST_F(TorrentReaderTest, ParseNegativeInteger) {
    // Create a temporary file with negative integer
    auto temp_file = test_data_dir / "temp_negative.torrent";
    std::ofstream out(temp_file);
    out << "d5:valuei-42ee";
    out.close();
    
    TorrentReader reader(temp_file.string());
    const auto& root = reader.getRoot();
    
    ASSERT_TRUE(root.isDict());
    const auto& dict = root.asDict();
    ASSERT_TRUE(dict.find("value") != dict.end());
    EXPECT_EQ(dict.at("value").asInt(), -42);
    
    fs::remove(temp_file);
}

// Test large integer
TEST_F(TorrentReaderTest, ParseLargeInteger) {
    auto temp_file = test_data_dir / "temp_large.torrent";
    std::ofstream out(temp_file);
    out << "d5:valuei9223372036854775807ee";  // Max long long
    out.close();
    
    TorrentReader reader(temp_file.string());
    const auto& root = reader.getRoot();
    
    ASSERT_TRUE(root.isDict());
    const auto& dict = root.asDict();
    ASSERT_TRUE(dict.find("value") != dict.end());
    EXPECT_EQ(dict.at("value").asInt(), 9223372036854775807LL);
    
    fs::remove(temp_file);
}

// Test empty string
TEST_F(TorrentReaderTest, ParseEmptyString) {
    auto temp_file = test_data_dir / "temp_empty_str.torrent";
    std::ofstream out(temp_file);
    out << "d3:str0:e";
    out.close();
    
    TorrentReader reader(temp_file.string());
    const auto& root = reader.getRoot();
    
    ASSERT_TRUE(root.isDict());
    const auto& dict = root.asDict();
    ASSERT_TRUE(dict.find("str") != dict.end());
    EXPECT_EQ(dict.at("str").asString(), "");
    
    fs::remove(temp_file);
}

// Test empty list
TEST_F(TorrentReaderTest, ParseEmptyList) {
    auto temp_file = test_data_dir / "temp_empty_list.torrent";
    std::ofstream out(temp_file);
    out << "d4:listlee";
    out.close();
    
    TorrentReader reader(temp_file.string());
    const auto& root = reader.getRoot();
    
    ASSERT_TRUE(root.isDict());
    const auto& dict = root.asDict();
    ASSERT_TRUE(dict.find("list") != dict.end());
    EXPECT_EQ(dict.at("list").asList().size(), 0);
    
    fs::remove(temp_file);
}

// Test empty dictionary
TEST_F(TorrentReaderTest, ParseEmptyDictionary) {
    auto temp_file = test_data_dir / "temp_empty_dict.torrent";
    std::ofstream out(temp_file);
    out << "de";
    out.close();
    
    TorrentReader reader(temp_file.string());
    const auto& root = reader.getRoot();
    
    ASSERT_TRUE(root.isDict());
    EXPECT_EQ(root.asDict().size(), 0);
    
    fs::remove(temp_file);
}

// Test invalid torrent (no info key)
TEST_F(TorrentReaderTest, InvalidTorrentNoInfo) {
    auto filepath = test_data_dir / "simple_dict.torrent";
    TorrentReader reader(filepath.string());
    
    EXPECT_FALSE(reader.isValidTorrent());
}

// Test binary string data
TEST_F(TorrentReaderTest, ParseBinaryString) {
    auto temp_file = test_data_dir / "temp_binary.torrent";
    std::ofstream out(temp_file, std::ios::binary);
    out << "d4:data5:";
    out.write("\x00\x01\x02\x03\x04", 5);
    out << "e";
    out.close();
    
    TorrentReader reader(temp_file.string());
    const auto& root = reader.getRoot();
    
    ASSERT_TRUE(root.isDict());
    const auto& dict = root.asDict();
    ASSERT_TRUE(dict.find("data") != dict.end());
    
    const auto& str = dict.at("data").asString();
    EXPECT_EQ(str.size(), 5);
    EXPECT_EQ(static_cast<unsigned char>(str[0]), 0x00);
    EXPECT_EQ(static_cast<unsigned char>(str[1]), 0x01);
    EXPECT_EQ(static_cast<unsigned char>(str[2]), 0x02);
    
    fs::remove(temp_file);
}

// Test mixed type list
TEST_F(TorrentReaderTest, ParseMixedTypeList) {
    auto temp_file = test_data_dir / "temp_mixed_list.torrent";
    std::ofstream out(temp_file);
    out << "d4:listli42e5:helloee";
    out.close();
    
    TorrentReader reader(temp_file.string());
    const auto& root = reader.getRoot();
    
    ASSERT_TRUE(root.isDict());
    const auto& dict = root.asDict();
    ASSERT_TRUE(dict.find("list") != dict.end());
    
    const auto& list = dict.at("list").asList();
    ASSERT_EQ(list.size(), 2);
    
    EXPECT_TRUE(list[0].isInt());
    EXPECT_EQ(list[0].asInt(), 42);
    
    EXPECT_TRUE(list[1].isString());
    EXPECT_EQ(list[1].asString(), "hello");
    
    fs::remove(temp_file);
}
