#include <gtest/gtest.h>
//
#include <fstream>
#include <filesystem>
#include "converterJSON.h"
//
#include <vector>
#include <string>
#include "invertedIndex.h"
//
#include <searchServer.h>

void CreateTestFile(const std::string& filename, const std::string& content) {
    std::ofstream out(filename);
    out << content;
    out.close();
}

// Удаления файлов после тестов
void RemoveTestFile(const std::string& filename) {
    std::remove(filename.c_str());
}

// ====== РАЗДЕЛ КЛАССА ConverterJSON ======
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

// ====== РАЗДЕЛ КЛАССА InvertedIndex ======

void TestInvertedIndexFunctionality(
    const std::vector<std::string>& docs,
    const std::vector<std::string>& requests,
    const std::vector<std::vector<Entry>>& expected
) {
    std::vector<std::vector<Entry>> result;
    InvertedIndex idx;
    
    idx.updateDocumentBase(docs);
    
    for (const auto& request : requests) {
        std::vector<Entry> word_count = idx.getWordCount(request);
        result.push_back(word_count);
    }
    
    ASSERT_EQ(result, expected);
}

// --- ТЕСТ 1: Базовая проверка на двух простых предложениях ---
TEST(TestCaseInvertedIndex,TESTBASIC)
{
    const std::vector<std::string> docs { 
        "london is the capital of great britain",
        "big ben is the nickname for the Great bell of the striking clock"
    };

    const std::vector<std::string> requests = {"london", "the"};
    
    const std::vector<std::vector<Entry>> expected = {
        {
            {0, 1}
        },
        {
            {0, 1}, {1, 3}
        }
    };
    
    TestInvertedIndexFunctionality(docs, requests, expected);
}

// --- ТЕСТ 2: Проверка на множественные повторения слов ---
TEST(TestCaseInvertedIndex, TestBasic2) {
    const std::vector<std::string> docs = {
        "milk milk milk milk water waterwater",
        "milk water water",
        "milk milk milk milk milk water water water waterwater",
        "americano cappuccino"
    };
    
    const std::vector<std::string> requests = {"milk", "water", "cappuccino"};
    
    const std::vector<std::vector<Entry>> expected = {
        {
            {0, 4}, {1, 1}, {2, 5}
        },
        {
            {0, 1}, {1, 2}, {2, 3} 
        },
        {
            {3, 1}
    }};
    
    TestInvertedIndexFunctionality(docs, requests, expected);
}

// --- ТЕСТ 3: Проверка отсутствующих слов (Негативный тест) ---
TEST(TestCaseInvertedIndex, TestInvertedIndexMissingWord) {
    const std::vector<std::string> docs = {
        "abcdefghijkl",
        "statement"
    };
    const std::vector<std::string> requests = {"m", "statement"};
    
    const std::vector<std::vector<Entry>> expected = {
        {
            // Для слова "m" ожидается абсолютно пустой вектор, так как его нет в базе
        },
        {
            {1, 1} // "statement" есть в doc_id 1 в количестве 1 штуки
        }
    };
    
    TestInvertedIndexFunctionality(docs, requests, expected);
}

TEST(TestCaseSearchServer, TestSimple) { 
    const std::vector<std::string> docs = {
        "milk milk milk milk water water water",
        "milk water water",
        "milk milk milk milk milk water water water water water",
        "americano cappuccino"
    };
    const std::vector<std::string> request = {"milk water", "sugar"};
    const std::vector<std::vector<RelativeIndex>> expected = {
        {
            {2, 1},
            {0, 0.7},
            {1, 0.3}
        },
        {
        }
    };

    InvertedIndex idx;
    idx.updateDocumentBase(docs);
    SearchServer srv(idx);
    std::vector<std::vector<RelativeIndex>> result = srv.search(request);
    ASSERT_EQ(result, expected);
}

TEST(TestCaseSearchServer, TestTop5) { 
    const std::vector<std::string> docs = {
        "london is the capital of great britain",
        "paris is the capital of france",
        "berlin is the capital of germany",
        "rome is the capital of italy",
        "madrid is the capital of spain",
        "lisboa is the capital of portugal",
        "bern is the capital of switzerland",
        "moscow is the capital of russia",
        "kiev is the capital of ukraine",
        "minsk is the capital of belarus",
        "astana is the capital of kazakhstan",
        "beijing is the capital of china",
        "tokyo is the capital of japan",
        "bangkok is the capital of thailand",
        "welcome to moscow the capital of russia the third rome",
        "amsterdam is the capital of netherlands",
        "helsinki is the capital of finland",
        "oslo is the capital of norway",
        "stockholm is the capital ofsweden",
        "riga is the capital of latvia",
        "tallinn is the capital of estonia",
        "warsaw is the capital of poland",
    };
    const std::vector<std::string> request ={"moscow is the capital ofrussia"};
    const std::vector<std::vector<RelativeIndex>> expected ={
        {
            {7,1},
            {14,1},
            {0,0.666666687},
            {1,0.666666687},
            {2,0.666666687}
        }
    };
    InvertedIndex idx;
    idx.updateDocumentBase(docs);
    SearchServer srv(idx);
    std::vector<std::vector<RelativeIndex>> result = srv.search(request);
    ASSERT_EQ(result,expected);
}
//ASSERT_... (Критическая проверка)
//EXPECT_... (Мягкая проверка)
//..._EQ (Проверки на равенство)
//..._TRUE(Проверки на истину)
//..._FALSE(Проверки на ложь)
//..._FLOAT_EQ(Проверки на равенство ДЛЯ FLOAT и DOUBLE)
//..._THROW(Проверка на генерацию исключений)