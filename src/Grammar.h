//
// Created by praza on 17.03.2022.
//

#ifndef OSHI_CPP_GRAMMAR_H
#define OSHI_CPP_GRAMMAR_H

#include <regex>
#include <fstream>
#include <iostream>
#include "Utilities.h"
#include <unordered_map>
#include <array>

#define SOUND_CHANGE_ARRAY_SIZE 9
#define KATAKANA_VOWEL_COUNT 5
const std::string katakana_vowels[KATAKANA_VOWEL_COUNT]{"ア", "イ", "ウ", "エ", "オ"};
const std::unordered_map<std::string, std::array<std::string, SOUND_CHANGE_ARRAY_SIZE>> sound_change{
	{"ア", {"さ", "か", "が", "ま", "ば", "な", "ら", "わ", "た"}},
	{"イ", {"し", "き", "ぎ", "み", "び", "に", "り", "い", "ち"}},
	{"ウ", {"す", "く", "ぐ", "む", "ぶ", "ぬ", "る", "う", "つ"}},
	{"エ", {"せ", "け", "げ", "め", "べ", "ね", "れ", "え", "て"}},
	{"オ", {"そ", "こ", "ご", "も", "ぼ", "の", "ろ", "お", "と"}}
};

class GrammarTriple {
 public:
  /// The word represented by this triple
  std::string form;
  /// A glob for part-of-speech this triple may be representing
  std::string glob;
  /// Name of the grammatical role this triple is representing
  std::string role;
  bool operator==(const GrammarTriple &other) const;
  bool operator!=(const GrammarTriple &other) const { return !(*this == other); }
  friend std::ostream &operator<<(std::ostream &os, const GrammarTriple &grammar_triple) {
	os << "(" << grammar_triple.form << ", "
	   << grammar_triple.glob << ", "
	   << grammar_triple.role << ")";
	return os;
  }
};

class GrammarRule {
 private:
  /// Regex for matching grammar rules
  /// Capturing groups [optional]: RULE [ROLE] PATTERN [POS] TARGET PATTERN POS-GLOB...
  static const std::regex rule_regex_;
  /// Expands a \p rule containing katakana sound representations into 9 GrammarRules and inserts them into \p rules
  /// (see header of grammar.rules file).
  /// If the rule cannot be expanded it is inserted into \p rules.
  /// \param rule The rule to expand
  /// \param rules The list to add the result to
  /// \return False the \p rule isn't a correct GrammarRule
  static bool ExpandRule(const GrammarRule &rule, std::vector<GrammarRule> &rules);
 public:
  /// english name of the rule
  std::string rule;
  /// optional English name for the rule used for reference by other rules
  std::string role;
  /// the pattern to match
  std::string pattern;
  /// optional part-of-speech tag
  std::string pos;
  /// reference to another define rule (to role)
  std::string target;
  /// the target replacement pattern
  std::string target_pattern;
  /// when POS is omitted, the rule is applicable for all of these part-of-speech tag globs
  std::string pos_globs;

  GrammarRule(std::string &&rule, std::string &&role, std::string &&pattern,
			  std::string &&pos, std::string &&target, std::string &&target_pattern,
			  std::string &&pos_globs)
	  : rule(rule), role(role), pattern(pattern), pos(pos), target(target),
		target_pattern(target_pattern), pos_globs(pos_globs) {}

  static std::vector<GrammarRule> Parse(const std::string &from);
  friend std::ostream &operator<<(std::ostream &os, const GrammarRule &gr);
  /// Apply this GrammarRule to \p s, return the resulting string
  /// \param s The grammar form to apply this GrammarRule to
  /// \return The new string
  std::string ApplyToForm(const std::string &s) const;
  /// Applies this GrammarRule to a \p grammar_triple, transforming it into a new GrammarTriple, which it returns.
  GrammarTriple Apply(const GrammarTriple &grammar_triple) const;
  /// Returns whether the current GrammarRule is applicable to \p grammar_triple
  bool IsApplicable(const GrammarTriple &grammar_triple) const;
};

class Grammar {
 private:
  const std::string grammar_file_path_ = "grammar.rules";
  std::vector<GrammarRule> rules_;
 public:
  /// Loads grammar rules from the default path
  void LoadGrammarRules();
  const std::vector<GrammarRule> &rules = rules_;
};

#endif //OSHI_CPP_GRAMMAR_H
