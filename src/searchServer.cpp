#include <searchServer.h>
#include <converterJSON.h>
#include <sstream>
#include <set>
#include <algorithm>

std::vector<std::vector<RelativeIndex>>SearchServer::search (const std::vector<std::string>& queries_input) {
    std::vector<std::vector<RelativeIndex>> result;
    

    for(auto &query : queries_input)
    {
        std::stringstream ss(query);
        std::set<std::string> unique_set;
        std::string words;

        while(ss >> words)
        {
            unique_set.insert(words);
        }

        std::vector<std::string> unique_words(unique_set.begin(), unique_set.end());

        if (unique_words.empty()) {
            result.push_back({});
            continue;
        }

        std::sort(unique_words.begin(), unique_words.end(),[this](const std::string& a, const std::string& b){

            size_t count_a = 0;

            for(auto& entry: _index.getWordCount(a))
            {
                count_a += entry.count;
            }

            size_t count_b = 0;

            for(auto& entry: _index.getWordCount(b))
            {
                count_b += entry.count;
            }

            return count_a < count_b;
        });

        std::map<size_t, size_t> absolute_relevance;
        
        auto current_word_entries = _index.getWordCount(unique_words[0]);

        for(const auto& entry : current_word_entries)
        {
            absolute_relevance[entry.doc_id] = entry.count;
        }

        for(size_t i = 1; i < unique_words.size(); ++i)
        {
            if(absolute_relevance.empty())
            {
                break;
            }

            auto first_word_entries = _index.getWordCount(unique_words[i]);

            std::map<size_t, size_t> next_relevance;

            for(const auto& entry : first_word_entries)
            {
                if(absolute_relevance.count(entry.doc_id) > 0)
                {
                    next_relevance[entry.doc_id] = absolute_relevance[entry.doc_id] + entry.count;
                }
            }

            absolute_relevance = next_relevance;
        }

        if(absolute_relevance.empty())
        {
            result.push_back({});
            continue;
        }

        size_t max_abs_relevance = 0;

        for(const auto& [doc_id, abs_rank] : absolute_relevance)
        {
            if(abs_rank > max_abs_relevance)
            {
                max_abs_relevance = abs_rank;
            }
        }

        std::vector<RelativeIndex> query_result;
        for(const auto& [doc_id, abs_rank] :absolute_relevance)
        {
            RelativeIndex rel_index;
            rel_index.doc_id = doc_id;
            rel_index.rank = static_cast<float>(abs_rank)/static_cast<float>(max_abs_relevance);

            query_result.push_back(rel_index);
        }

        std::sort(query_result.begin(), query_result.end(),[](const RelativeIndex& a, const RelativeIndex& b){
            if(a.rank != b.rank)
            {
                return a.rank > b.rank;
            }

            return a.doc_id < b.doc_id;
        });

        ConverterJSON converter;
        size_t max_responses = converter.GetResponsesLimit();

        if(query_result.size() > max_responses)
        {
            query_result.resize(max_responses);
        }

        result.push_back(query_result);
    }
    return result;
}