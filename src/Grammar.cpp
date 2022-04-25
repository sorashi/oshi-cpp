//
// Created by praza on 17.03.2022.
//

#include "Grammar.h"
#include "glob-cpp/glob.h"
#include <algorithm>

// the regex uses \S for "non-whitespace" characters, because the rule parts are separated by whitespace
// and the parts may contain Japanese characters, which are faster and easier to match by \S than some
// regex Unicode pattern
const std::regex GrammarRule::rule_regex_ = std::regex(
	"^(\\S+)\\s*(\\S*)\\s+〜(\\S*)\\s*(\\S*)\\s+for\\s+(\\S*)\\s+〜(\\S*) +((?:[ \t]*\\S+)+)\\s*$");

void Grammar::LoadGrammarRules() {
  auto grammar_file = std::ifstream(grammar_file_path_);
  for (std::string line; getline(grammar_file, line);) {
	// ignore comments and whitespace-only lines
	if (line.starts_with('#') || Utilities::StringIsWhitespaceOrEmpty(line))
	  continue;
	// parse the rest
	std::vector<GrammarRule> parsed_rules = GrammarRule::Parse(line);
	rules_.insert(rules_.end(), parsed_rules.begin(), parsed_rules.end());
  }
  D(std::cerr << "Loaded " << rules_.size() << " grammar rules." << std::endl);
}
bool GrammarRule::ExpandRule(const GrammarRule &rule, std::vector<GrammarRule> &rules) {
  size_t pattern_katakana_position = std::string::npos;
  size_t target_pattern_katakana_position = std::string::npos;
  std::string pattern_which_katakana, target_pattern_which_katakana;
  for (const auto &vowel : katakana_vowels) {
	size_t pattern_katakana_position_internal = rule.pattern.find(vowel);
	size_t target_pattern_katakana_position_internal = rule.target_pattern.find(vowel);
	if (pattern_katakana_position_internal != std::string::npos) {
	  if (pattern_katakana_position != std::string::npos) return false;
	  pattern_katakana_position = pattern_katakana_position_internal;
	  pattern_which_katakana = vowel;
	}
	if (target_pattern_katakana_position_internal != std::string::npos) {
	  if (target_pattern_katakana_position != std::string::npos)
		return false;
	  target_pattern_katakana_position = target_pattern_katakana_position_internal;
	  target_pattern_which_katakana = vowel;
	}
  }
  // katakana not included
  if (pattern_katakana_position == std::string::npos && target_pattern_katakana_position == std::string::npos) {
	rules.push_back(rule);
	return true;
  }
  // one has katakana, but not the other
  if (pattern_katakana_position == std::string::npos || target_pattern_katakana_position == std::string::npos)
	return false;
  for (int i = 0; i < SOUND_CHANGE_ARRAY_SIZE; ++i) {
	// copy
	GrammarRule new_rule = rule;
	new_rule.pattern.replace(pattern_katakana_position,
							 pattern_which_katakana.size(),
							 sound_change.at(pattern_which_katakana)[i]);
	new_rule.target_pattern.replace(target_pattern_katakana_position,
									target_pattern_which_katakana.size(),
									sound_change.at(target_pattern_which_katakana)[i]);
	rules.push_back(std::move(new_rule));
  }
  return true;
}
std::vector<GrammarRule> GrammarRule::Parse(const std::string &from) {
  std::smatch sm;
  if (!std::regex_match(from, sm, GrammarRule::rule_regex_))
	throw std::runtime_error("Could not parse grammar rule: " + from);
  std::string pos_globs(sm.str(7));
  Utilities::SpaceSeparatedGlobsIntoSingleGlobInPlace(pos_globs);
  GrammarRule
	  temp{std::move(sm.str(1)), std::move(sm.str(2)), std::move(sm.str(3)), std::move(sm.str(4)), std::move(sm.str(5)),
		   std::move(sm.str(6)), std::move(pos_globs)};
  std::vector<GrammarRule> rules;
  if (!ExpandRule(temp, rules)) throw new std::runtime_error("Invalid rule: " + from);
  return rules;
}
std::ostream &operator<<(std::ostream &os, const GrammarRule &gr) {
  os << "GrammarRule[";
  os << "rule: " << gr.rule << ", ";
  if (!gr.role.empty()) os << "role: " << gr.role << ", ";
  os << "pattern: ~" << gr.pattern << ", ";
  if (!gr.pos.empty()) os << "pos: " << gr.pos << ", ";
  os << "target: " << gr.target << ", ";
  os << "target_pattern: ~" << gr.target_pattern;
  if (!gr.pos_globs.empty()) os << ", pos_globs: " << gr.pos_globs;
  os << "]";
  return os;
}
GrammarTriple GrammarRule::Apply(const GrammarTriple &grammar_triple) const {
  GrammarTriple result;
  result.role = this->target;
  result.form = ApplyToForm(grammar_triple.form);
  result.glob = this->pos_globs;
  return result;
}
std::string GrammarRule::ApplyToForm(const std::string &s) const {
  auto pattern_location = s.rfind(this->pattern);
  if (pattern_location == std::string::npos || pattern_location + this->pattern.size() != s.size())
	throw std::logic_error("Pattern is not applicable");
  return s.substr(0, pattern_location) + this->target_pattern;
}
bool GrammarRule::IsApplicable(const GrammarTriple &grammar_triple) const {
  // If grammar_triple.role is empty, it matches anything, so we skip the following check.
  if (!grammar_triple.role.empty()) {
	// The grammar_triple role must match GrammarRule->rule if GrammarRule->role is empty. Otherwise,
	// it must match the non-empty GrammarRole->role.
	if (grammar_triple.role != this->rule
		&& (this->role.empty() || this->role != grammar_triple.role)) {
	  return false;
	}
  }
  // The GrammarRule->pattern must be found at the END of grammar_triple.form
  auto pattern_location = grammar_triple.form.rfind(this->pattern);
  if (pattern_location == std::string::npos
	  || pattern_location + this->pattern.size() != grammar_triple.form.size())
	return false;
  // The grammar_triple glob must match GrammarRule->pos if GrammarRule->pos is non-empty.
  if (!this->pos.empty()) {
	glob::glob g(grammar_triple.glob);
	if (!glob::glob_match(this->pos, g)) return false;
  }
  return true;
}
bool GrammarTriple::operator==(const GrammarTriple &other) const {
  return form == other.form && glob == other.glob && role == other.role;
}
