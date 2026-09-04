#include <converterJSON.h>

using json = nlohmann::json;

std::vector<std::string> ConverterJSON::GetTextDocuments()
{
    std::vector<std::string> text_documents;
    const std::string CURRENT_VERSION = "0.1";

    std::ifstream inFile("config.json");

    //checking for the presence of config.json
    if(!inFile.is_open())
    {
        throw std::runtime_error("config file is missing");
    }

    json configJson;

    //reading config.json
    try
    {
        inFile >> configJson;
    }
    catch(const json::parse_error& e)
    {
        throw std::runtime_error("config.json parsing error: " + std::string(e.what()));
    }
    
    //Checking the root “config”
    if(!configJson.contains("config") || configJson["config"].is_null())
    {
        throw std::runtime_error("config file is empty");
    }

    //name check and starting
    if(configJson["config"].contains("name") && configJson["config"]["name"].is_string())
    {
        std::string engineName = configJson["config"]["name"].get<std::string>();
        std::cout << "Starting " << engineName << std::endl;
    }
    else
    {
        std::cout << "Starting UnknownSearchEngine" << std::endl;//requires verification for necessity
    }

    //version check   
    if(configJson["config"].contains("version") && configJson["config"]["version"].is_string())
    {
        std::string engineVersion = configJson["config"]["version"].get<std::string>();
        if (engineVersion != CURRENT_VERSION) {
            throw std::runtime_error("config.json has incorrect file version");
        }
    }
    else
    {
        throw std::runtime_error("config.json is missing version field");
    }

    //Checking and reading the root “files”
    if(configJson.contains("files") && configJson["files"].is_array())
    {
        for(const auto& fileEntry: configJson["files"])
        {
            auto filePath = fileEntry.get<std::string>();
            std::ifstream docfile(filePath);

            if(!docfile.is_open())
            {
                std::cerr << "Error: file " << filePath << " does not exist." << std::endl;
                text_documents.push_back("");
                continue;
            }

            std::string text((std::istreambuf_iterator<char>(docfile)),
                              std::istreambuf_iterator<char>());
            text_documents.push_back(text);
        }
    }

    return text_documents;
}
	/**
	* Метод считывает поле max_responses для определения предельного
	* количества ответов на один запрос
	* @return
	*/
int ConverterJSON::GetResponsesLimit()
{
    std::ifstream inFile("config.json");
    if (!inFile.is_open()) return 5;
    
    json configJSON;

    try
    {
        inFile >> configJSON;

        json jsonSelection = configJSON.value("config",json::object());

        return jsonSelection.value("max_responses", 5);
    }
    catch(...)
    {
        return 5;
    } 
}
	/**
	* Метод получения запросов из файла requests.json
	* @return возвращает список запросов из файла requests.json
	*/
std::vector<std::string>ConverterJSON::GetRequests()
{
    std::vector<std::string> requests;
    std::ifstream inFile("requests.json");
    if (!inFile.is_open()) return requests;

    json requestsJSON;

    try
    {
        inFile >> requestsJSON;

        if(requestsJSON.contains("requests") && requestsJSON["requests"].is_array())
        {
            for(const auto & request : requestsJSON["requests"])
            {
                requests.push_back(request.get<std::string>());
            }
        }  
    }
    catch(const std::exception& e)
    {
        std::cerr << "Parsing error: " << e.what() << '\n';
    }
    return requests;
}
	/**
	* Положить в файл answers.json результаты поисковых запросов
	*/
void ConverterJSON::putAnswers(std::vector<std::vector<std::pair<int,float>>>answers)
{
    std::ofstream outFile("answers.json",std::ios::trunc);
    
    if(!outFile.is_open()) return;
    
    json rootJSON;

    rootJSON["answers"] = json::object();

    for (size_t i = 0; i < answers.size(); ++i)
    {
        std::ostringstream oss;
        
        oss << "request" << std::setfill('0') << std::setw(3) << (i + 1);

        std::string requestName = oss.str(); 

        json requestEntry;
        if(answers[i].empty())
        {
            requestEntry["result"] = "false";
        }
        else
        {
            requestEntry["result"] = "true";
            json relevanceArray = json::array();

            for(const auto& pair : answers[i])
            {
                json item;
                item["docid"] = pair.first;
                item["rank"] = pair.second;

                relevanceArray.push_back(item);
            }

            requestEntry["relevance"] = relevanceArray;
        }

        rootJSON["answers"][requestName] = requestEntry;
    }
    outFile << rootJSON.dump(4);
}
