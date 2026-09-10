#pragma once

#include <string>
#include <vector>
#include <map>
#include <iostream>

struct Entry {
	size_t doc_id, count;

	bool operator == (const Entry& other) const {
		return(doc_id == other.doc_id && count == other.count);
	}
};

class InvertedIndex {
public:
	InvertedIndex()=default;
	/*
	* Обновить или заполнить базу документов, по которой
	будем совершать поиск
	* @paramtexts_input содержимое документов
	*/
	void updateDocumentBase(std::vector<std::string>input_docs);
    /*
	* Метод определяет количество вхождений слова word в загруженной базе документов
	* @paramword слово,частоту вхождений которого необходимо определить
	* @return возвращает подготовленный список с частотой слов
	*/
	std::vector<Entry> GetWordCount(const std::string& word);
private:
	std::vector<std::string> docs; // Список содержимого документов (индекс вектора = doc_id)
	std::map<std::string, std::vector<Entry>> freq_dictionary; // Частотный словарь
};