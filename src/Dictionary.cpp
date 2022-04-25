//
// Created by praza on 21.03.2022.
//

#include "Dictionary.h"
bool Dictionary::InflateDictionary() {
  FILE *jmdict_gz = fopen(JMDICT_GZ, "rb");
  if (!jmdict_gz) return false;
  FILE *jmdict = fopen(JMDICT_XML, "wb");
  if (!jmdict) return false;
  auto inflation_err = Utilities::InflateFile(jmdict_gz, jmdict);
  fclose(jmdict);
  fclose(jmdict_gz);
  return inflation_err == 0;
}
void Dictionary::LoadDictionary(pugi::xml_document &doc) {
  auto root = doc.child("JMdict");
  for (auto xml_entry : root.children("entry")) {
	DictionaryEntry entry;
	for (auto r_ele : xml_entry.children("r_ele")) entry.readings.emplace_back(r_ele.child_value("reb"));
	for (auto k_ele : xml_entry.children("k_ele")) entry.writings.emplace_back(k_ele.child_value("keb"));

	for (auto xml_sense : xml_entry.children("sense")) {
	  DictionaryEntrySense sense;
	  for (auto pos : xml_sense.children("pos")) {
		std::string pos_string(pos.child_value());
		// POS tags are XML entities like &v5k;, but pugixml (thankfully) does not expand these,
		// because pugixml does not support DTD. So we use the entity name.
		Utilities::XmlEntityToEntityNameInPlace(pos_string);
		sense.part_of_speech.push_back(std::move(pos_string));
	  }
	  if (sense.part_of_speech.empty() && !entry.senses.empty())
		// copy the previous pos
		sense.part_of_speech = entry.senses[entry.senses.size() - 1].part_of_speech;
	  for (auto gloss : xml_sense.children("gloss")) sense.glosses.emplace_back(gloss.child_value());
	  entry.senses.push_back(std::move(sense));
	}
	entries.push_back(std::move(entry));
  }

  PrepareLookupMap();
}
void Dictionary::PrepareLookupMap() {
  for (auto &entry : entries) {
	for (auto &writing : entry.writings) entry_map.insert(std::make_pair(writing, &entry));
  }
}
const DictionaryEntry *Dictionary::Query(const std::string &query) const {
  auto found = entry_map.find(query);
  if (found == entry_map.end()) return nullptr;
  return found->second;
}
std::ostream &operator<<(std::ostream &os, DictionaryEntrySense &sense) {
  os << "(";
  Utilities::Join(sense.part_of_speech, " ", os);
  os << ") ";
  Utilities::Join(sense.glosses, ", ", os);
  return os;
}
std::ostream &operator<<(std::ostream &os, const DictionaryEntry &entry) {
  Utilities::Join(entry.writings, " ", os);
  if (!entry.writings.empty()) os << " ";
  os << "[";
  Utilities::Join(entry.readings, " ", os);
  os << "]: ";
  Utilities::Join(entry.senses, "; ", os);
  return os;
}
