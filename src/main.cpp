#include <iostream>
#include "Grammar.h"
#include "pugixml.hpp"
#include <filesystem>
#include "Dictionary.h"
#include "GrammarFormGuesser.h"

/// Decides whether the \p s is an exit command for a prompt (e/q/exit/quit, case insensitive)
bool IsExitCommand(const std::string &s) {
  if (s.empty()) return false;
  auto first_char_lower = std::tolower(s[0]);
  if (s.size() == 1 && first_char_lower == 'e' || first_char_lower == 'q') return true;
  if (Utilities::AreStringsEqualCaseInsensitive(s, "exit")) return true;
  if (Utilities::AreStringsEqualCaseInsensitive(s, "quit")) return true;
  return false;
}

bool Prompt(const GrammarFormGuesser &guesser) {
  std::cout << "> ";
  std::cout.flush();
  std::string input;
  std::getline(std::cin, input);

  // handle cin errors
  if (std::cin.bad()) {
	std::cerr << "Error while reading stdin" << std::endl;
	return false;
  }
  if (std::cin.eof()) {
	std::cerr << "EOF exiting." << std::endl;
	return false;
  }

  if (IsExitCommand(input)) return false;
  auto result = guesser.Guess(input);
  if (result.success) std::cout << result << std::endl;
  else std::cout << "No result :(" << std::endl;
  return true;
}

int main() {
  Grammar gr;
  gr.LoadGrammarRules();

  // Make sure JMDICT_XML exists, otherwise try extracting JMDICT_GZ
  if (!std::filesystem::exists(JMDICT_XML)) {
	std::cout << "Decompressing dictionary..." << std::endl;
	if (!Dictionary::InflateDictionary()) {
	  std::cerr << "An error occurred while decompressing the dictionary file. Make sure the file " << JMDICT_GZ
				<< " exists in the current directory." << std::endl;
	  return 1;
	}
  }

  Dictionary dic;
  {
	// Parse JMDICT_XML into pugi::xml_document
	std::unique_ptr<pugi::xml_document> doc = std::make_unique<pugi::xml_document>(pugi::xml_document());
	std::cout << "Parsing dictionary..." << std::endl;
	pugi::xml_parse_result result = doc->load_file(JMDICT_XML);
	if (result.status != pugi::status_ok) {
	  std::cerr << "An error occurred during parsing of the dictionary file " << JMDICT_XML << ". Error description: "
				<< result.description() << std::endl;
	  return 1;
	}
	// move the XML document into a Dictionary class instance
	dic.LoadDictionary(*doc);
  }
  bool loop = true;
  GrammarFormGuesser guesser(std::move(gr), std::move(dic));
  while (loop) {
	loop = Prompt(guesser);
  }
  return 0;
}
