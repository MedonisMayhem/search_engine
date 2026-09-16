#include <iostream>
#include <converterJSON.h>
#include <invertedIndex.h>
#include <searchServer.h>

int main()
{
    try
    {
        ConverterJSON converter;

        InvertedIndex index;

        index.updateDocumentBase(converter.GetTextDocuments());

        SearchServer server(index);
        
        //конвертировавание результат из метода search для ввода в метод putAnswers
        std::vector<std::vector<RelativeIndex>> search_result = server.search(converter.GetRequests());
        std::vector<std::vector<std::pair<int,float>>> answers;
        
        for(const auto& query_result : search_result)
        {
            std::vector<std::pair<int,float>> answer;
            for(const auto& item : query_result)
            {
                answer.push_back({static_cast<int>(item.doc_id), item.rank});
            }
            answers.push_back(answer);
        }
        converter.putAnswers(answers);
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    
    return 0;
}
