//
// Created by praza on 26.03.2022.
//

#ifndef OSHI_CPP__GRAMMARFORMGUESSER_H_
#define OSHI_CPP__GRAMMARFORMGUESSER_H_
#include "Grammar.h"
#include "Dictionary.h"

/// An instance of this class is invalid if the lifetime of the GrammarFormGuesser that generated it is shorter.
class GuessResultInternal {
 public:
  bool success = false;
  std::vector<const GrammarRule *> rules;
  const DictionaryEntry *entry;
  GuessResultInternal(const GuessResultInternal &other) = default;
  GuessResultInternal(bool success, std::vector<const GrammarRule *> rules, const DictionaryEntry *entry) : success(
	  success), rules(rules), entry(entry) {}
};

/// This class owns its data in contrast with GuessResultInternal
class GuessResult {
 public:
  bool success;
  std::vector<GrammarRule> rules;
  DictionaryEntry entry;
  std::string original_query;
  GuessResult(const GuessResultInternal &guess) : success(guess.success) {
	for (auto rule : guess.rules) rules.push_back(*rule);
	if (guess.entry != nullptr) entry = *guess.entry;
  }
  friend std::ostream &operator<<(std::ostream &os, const GuessResult &gr) {
	std::string form = gr.original_query;
	size_t tabs = 0;
	for (auto it = gr.rules.begin(); it != gr.rules.end(); ++it) {
	  const GrammarRule &current = *it;
	  for (size_t i = 0; i < tabs; ++i) os << "  ";
	  os << form << " is " << current.rule << " for ";
	  form = current.ApplyToForm(form);
	  os << form << std::endl;
	  ++tabs;
	}
	os << gr.entry;
	return os;
  }
};

/// Uses Grammar and Dictionary to produce a GuessResult
class GrammarFormGuesser {
  const Grammar gr;
  const Dictionary dic;
  /// Internal recursive search for grammar form called by the public-facing Guess method
  GuessResultInternal GuessInternal(const GrammarTriple &gt, std::vector<const GrammarRule *> applied_rules) const;
 public:
  /// \param gr Grammar rules to consider
  /// \param dic Dictionary to look in
  GrammarFormGuesser(Grammar &&gr, Dictionary &&dic) : gr(gr), dic(dic) {}
  GuessResult Guess(const std::string &s) const;
};

#endif //OSHI_CPP__GRAMMARFORMGUESSER_H_
