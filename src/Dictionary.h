//
// Created by praza on 21.03.2022.
//

#ifndef OSHI_CPP__DICTIONARY_H_
#define OSHI_CPP__DICTIONARY_H_

#include "Utilities.h"
#include "pugixml.hpp"
#include <iostream>
#include <vector>
#include <unordered_map>

#define JMDICT_GZ "JMdict_e.gz"
#define JMDICT_XML "JMdict_e.xml"

/// A class representing individual possible senses of a single dictionary entry
class DictionaryEntrySense {
 public:
  /// POS tags
  std::vector<std::string> part_of_speech;
  /// that is "translations"
  std::vector<std::string> glosses;
  friend std::ostream &operator<<(std::ostream &os, DictionaryEntrySense &sense);
};

class DictionaryEntry {
 public:
  /// Possible readings (kana) of the entry
  std::vector<std::string> readings;
  /// Possible writings (kanji+kana) of the entry
  std::vector<std::string> writings;
  std::vector<DictionaryEntrySense> senses;

  friend std::ostream &operator<<(std::ostream &os, const DictionaryEntry &entry);
};

class Dictionary {
 private:
  std::vector<DictionaryEntry> entries;
  std::unordered_map<std::string, DictionaryEntry *> entry_map;
  void PrepareLookupMap();
 public:
  /// Find a dictionary entry corresponding exactly to \p query
  /// \return nullptr if nothing found, otherwise first DictionaryEntry matching by writing
  const DictionaryEntry *Query(const std::string &query) const;
  /// Decompresses the dictionary into XML
  /// \return true if succeeded
  static bool InflateDictionary();
  /// Load dictionary data from parsed XML document
  void LoadDictionary(pugi::xml_document &doc);
};

#endif //OSHI_CPP__DICTIONARY_H_
