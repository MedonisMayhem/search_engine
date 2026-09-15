#include <invertedIndex.h>
#include <thread>
#include <sstream>

void InvertedIndex::updateDocumentBase(std::vector<std::string> input_docs) {
    this->freq_dictionary.clear();
    this->docs = input_docs;

    size_t docs_count = docs.size();
    if(docs_count == 0) return;
    
    std::vector<std::map<std::string, size_t>>  local_counts(docs_count);
    std::vector<std::thread> threads;

    for(size_t i = 0; i < docs_count; ++i)
    {
        size_t doc_id = i;
        const std::string text = docs[i];

        threads.push_back(std::thread([doc_id,text, &local_counts](){
            std::stringstream ss(text);
            std::string word;

            while(ss >> word)
            {
                local_counts[doc_id][word]++;
            }
        }));
    }


    for(auto& thread : threads)
    {
        if(thread.joinable()) thread.join();
    }

    for(size_t doc_id = 0; doc_id < docs_count; ++doc_id)
    {
        for (const auto& [word, count] : local_counts[doc_id])
        {
            freq_dictionary[word].push_back({doc_id, count});
        }
    }
}

std::vector<Entry> InvertedIndex::getWordCount(const std::string& word) {
    auto it = freq_dictionary.find(word);

    if(it == freq_dictionary.end())
    {
        return std::vector<Entry>();
    }

    return it->second;
}