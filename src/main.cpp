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
        
        //изменить входные данные метода putAnswers или конвертировать через цикл?
        //converter.putAnswers(server.search(converter.GetRequests()));

    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    
    return 0;
}