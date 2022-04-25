//
// Created by praza on 26.03.2022.
//

#include "GrammarFormGuesser.h"

GuessResult GrammarFormGuesser::Guess(const std::string &s) const {
  // empty triple, matching all part-of-speech tags and applying to any role
  GrammarTriple gt{s, "*", ""};
  GuessResult result = GuessInternal(gt, {});
  // insert the original query for printing to stdout
  result.original_query = s;
  return result;
}
GuessResultInternal GrammarFormGuesser::GuessInternal(const GrammarTriple &gt,
													  std::vector<const GrammarRule *> applied_rules) const {
  // lookup in the dictionary
  const DictionaryEntry *found = dic.Query(gt.form);
  // if found, return immediately
  if (found != nullptr) {
	return GuessResultInternal{true, applied_rules, found};
  }
  // otherwise, apply applicable grammar rules and search recursively
  GuessResultInternal best_result{false, {}, nullptr};
  for (auto &rule : gr.rules) {
	if (!rule.IsApplicable(gt)) continue;
	auto new_triple = rule.Apply(gt);

	applied_rules.push_back(&rule);
	// applied_rules are copied into the recursive call
	auto result = GuessInternal(new_triple, applied_rules);
	applied_rules.pop_back();

	// take the shortest path to success
	if (result.success && (!best_result.success || (best_result.rules.size() > result.rules.size()))) {
	  best_result = result;
	}
  }
  if (best_result.success) return best_result;
  // no result in any branch, return unsuccessful
  return GuessResultInternal{false, applied_rules, nullptr};
}
