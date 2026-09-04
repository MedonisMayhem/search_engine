#include <gtest/gtest.h>
#include <fstream>
#include <filesystem>
#include "converterJSON.h"

void CreateTestFile(const std::string& filename, const std::string& content) {
    std::ofstream out(filename);
    out << content;
    out.close();
}

// Удаления файлов после тестов
void RemoveTestFile(const std::string& filename) {
    std::remove(filename.c_str());
}


// --- ТЕСТЫ ДЛЯ МЕТОДА GetTextDocuments ---


TEST(ConverterJSONTest, GetTextDocuments_ThrowsIfVersionIncorrect) {
    CreateTestFile("config.json", R"({
        "config": {
            "name": "SkillboxSearchEngine",
            "version": "1.0" 
        }
    })");

    ConverterJSON converter;
    
    // Ждем, что метод выбросит std::runtime_error из-за несовпадения версий
    EXPECT_THROW(converter.GetTextDocuments(), std::runtime_error);

    RemoveTestFile("config.json");
}

//без версии
TEST(ConverterJSONTest, GetTextDocuments_ThrowsIfVersionMissing) {
    CreateTestFile("config.json", R"({
        "config": {
            "name": "SkillboxSearchEngine"
            
        }
    })");

    ConverterJSON converter;
    
    // Код должен выдать исключение "config.json is missing version field"
    EXPECT_THROW(converter.GetTextDocuments(), std::runtime_error);

    RemoveTestFile("config.json");
}

// --- ТЕСТЫ ДЛЯ МЕТОДА GetRequests ---

TEST(ConverterJSONTest, GetRequests_ValidFile) {
    CreateTestFile("requests.json", R"({
        "requests": ["milk", "sugar water"]
    })");

    ConverterJSON converter;
    auto requests = converter.GetRequests();

    ASSERT_EQ(requests.size(), 2);
    EXPECT_EQ(requests[0], "milk");
    EXPECT_EQ(requests[1], "sugar water");

    RemoveTestFile("requests.json");
}

TEST(ConverterJSONTest, GetRequests_MissingFile) {
    RemoveTestFile("requests.json");

    ConverterJSON converter;
    auto requests = converter.GetRequests();

    EXPECT_TRUE(requests.empty());
}

// --- ТЕСТЫ ДЛЯ МЕТОДА GetResponsesLimit ---

TEST(ConverterJSONTest, GetResponsesLimit_ValidValue) {
    CreateTestFile("config.json", R"({
        "config": {
            "name": "SearchEngine",
            "version": "0.1",
            "max_responses": 10
        }
    })");

    ConverterJSON converter;
    EXPECT_EQ(converter.GetResponsesLimit(), 10);

    RemoveTestFile("config.json");
}

// Неверная версия


TEST(ConverterJSONTest, GetResponsesLimit_DefaultValue) {
    RemoveTestFile("config.json");

    ConverterJSON converter;
    EXPECT_EQ(converter.GetResponsesLimit(), 5);
}

// --- ТЕСТЫ ДЛЯ МЕТОДА putAnswers ---

TEST(ConverterJSONTest, PutAnswers_HandlesMixOfResults) {
    ConverterJSON converter;

    std::vector<std::vector<std::pair<int, float>>> mockAnswers = {
        { {0, 0.95f}, {3, 0.42f} },
        { }
    };

    converter.putAnswers(mockAnswers);

    std::ifstream inFile("answers.json");
    ASSERT_TRUE(inFile.is_open()) << "Файл answers.json не был создан методом putAnswers!";

    nlohmann::json resultJSON;
    inFile >> resultJSON;
    inFile.close();

    //Проверка структуры JSON
    ASSERT_TRUE(resultJSON.contains("answers"));
    
    // Проверка request001 (успешный)
    ASSERT_TRUE(resultJSON["answers"].contains("request001"));
    EXPECT_EQ(resultJSON["answers"]["request001"]["result"], "true");
    ASSERT_TRUE(resultJSON["answers"]["request001"].contains("relevance"));
    EXPECT_EQ(resultJSON["answers"]["request001"]["relevance"][0]["docid"], 0);
    EXPECT_FLOAT_EQ(resultJSON["answers"]["request001"]["relevance"][0]["rank"].get<float>(), 0.95f);

    // Проверка request002 (пустой)
    ASSERT_TRUE(resultJSON["answers"].contains("request002"));
    EXPECT_EQ(resultJSON["answers"]["request002"]["result"], "false");
    EXPECT_FALSE(resultJSON["answers"]["request002"].contains("relevance"));

    RemoveTestFile("answers.json");
}

// --- ТЕСТЫ НА ОШИБКИ В GetTextDocuments (Исключения) ---

TEST(ConverterJSONTest, GetTextDocuments_ThrowsIfConfigMissing) {
    RemoveTestFile("config.json");
    ConverterJSON converter;
    
    // Метод должен выбросить std::runtime_error("config file is missing")
    EXPECT_THROW(converter.GetTextDocuments(), std::runtime_error);
}


//ASSERT_... (Критическая проверка)
//EXPECT_... (Мягкая проверка)
//..._EQ (Проверки на равенство)
//..._TRUE(Проверки на истину)
//..._FALSE(Проверки на ложь)
//..._FLOAT_EQ(Проверки на равенство ДЛЯ FLOAT и DOUBLE)
//..._THROW(Проверка на генерацию исключений)