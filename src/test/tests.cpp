#include <gtest/gtest.h>
#include "Grammar.h"
#include <vector>

TEST(TestUtilities, StringIsWhitespaceOrEmpty) {
  EXPECT_TRUE(Utilities::StringIsWhitespaceOrEmpty("    "));
  EXPECT_TRUE(Utilities::StringIsWhitespaceOrEmpty(""));
  EXPECT_TRUE(Utilities::StringIsWhitespaceOrEmpty("\t"));
  EXPECT_TRUE(Utilities::StringIsWhitespaceOrEmpty("\n\n"));
  EXPECT_FALSE(Utilities::StringIsWhitespaceOrEmpty("   a   "));
}

TEST(TestUtilities, XmlEntityToEntityNameInPlace) {
  std::string entity = "&v5k;";
  Utilities::XmlEntityToEntityNameInPlace(entity);
  EXPECT_EQ("v5k", entity);
}

TEST(TestUtilities, Join_MultipleStrings) {
  std::vector<std::string> strings{"a", "b", "c"};
  std::stringstream ss;

  Utilities::Join(strings, ",", ss);

  EXPECT_EQ(ss.str(), "a,b,c");

  std::vector<int> single_int{0};
}

TEST(TestUtilities, Join_MultipleInts) {
  std::vector<int> multiple_ints{9, 8, 7, 6};
  std::stringstream ss;

  Utilities::Join(multiple_ints, "", ss);

  EXPECT_EQ(ss.str(), "9876");
}

TEST(TestUtilities, Join_NoInts) {
  std::vector<int> no_ints;
  std::stringstream ss;

  Utilities::Join(no_ints, "    ", ss);

  EXPECT_EQ(ss.str(), "");
}

TEST(TestUtilities, AreStringsEqualCaseInsensitive) {
  std::string a = "!ABC dEf+";
  std::string b = "!abC Def+";
  EXPECT_TRUE(Utilities::AreStringsEqualCaseInsensitive(a, b));

  a = "!ABx dEf+";
  b = "!abC Def+";
  EXPECT_FALSE(Utilities::AreStringsEqualCaseInsensitive(a, b));

  a = "a";
  b = "";
  EXPECT_FALSE(Utilities::AreStringsEqualCaseInsensitive(a, b));
}

TEST(TestUtilities, SpaceSeparatedGlobsIntoSingleGlobInPlace) {
  std::string space_separated_globs = "a b c";
  Utilities::SpaceSeparatedGlobsIntoSingleGlobInPlace(space_separated_globs);
  EXPECT_EQ("@(a|b|c)", space_separated_globs);

  space_separated_globs = "a\tb c";
  Utilities::SpaceSeparatedGlobsIntoSingleGlobInPlace(space_separated_globs);
  EXPECT_EQ("@(a|b|c)", space_separated_globs);
}

TEST(TestGrammar, GrammarRule_ApplyToForm) {
  GrammarRule gr = GrammarRule::Parse("て-form 〜て for past 〜た v[15]* vk vs-*")[0];
  EXPECT_EQ("書いた", gr.ApplyToForm("書いて"));

  gr = GrammarRule::Parse("continuous plain 〜いる v1 for て-form 〜 v[15]* vk vs-*")[0];
  EXPECT_EQ("書いて", gr.ApplyToForm("書いている"));

  EXPECT_ANY_THROW(gr.ApplyToForm("書いて"));

  gr = GrammarRule::Parse("past 〜た for plain 〜る v1*")[0];
  EXPECT_ANY_THROW(gr.ApplyToForm("良くない"));
}

TEST(TestGrammar, GrammarRule_IsApplicable) {
  GrammarRule gr = GrammarRule::Parse("colloquial plain 〜る for continuous 〜いる v1")[0];
  GrammarTriple grammar_triple{"書いてる", "@(v1*)", "plain"};
  EXPECT_TRUE(gr.IsApplicable(grammar_triple));

  gr = GrammarRule::Parse("continuous plain 〜いる v1 for て-form 〜 v[15]* vk vs-*")[0];
  grammar_triple = GrammarTriple{"書いている", "@(v1)", "continuous"};
  EXPECT_TRUE(gr.IsApplicable(grammar_triple));

  grammar_triple = GrammarTriple{"書いた", "@(v[15]*|vk|vs-*)", "past"};
  EXPECT_FALSE(gr.IsApplicable(grammar_triple));

  gr = GrammarRule::Parse("past 〜た for plain 〜る v1*")[0];
  grammar_triple = GrammarTriple{"良くない", "@(adj-i)", "plain"};
  EXPECT_FALSE(gr.IsApplicable(grammar_triple));
}

TEST(TestGrammar, GrammarRule_IsApplicable_ToEmptyTriple) {
  GrammarRule gr = GrammarRule::Parse("past 〜た for plain 〜る v1*")[0];
  GrammarTriple grammar_triple = GrammarTriple{"書いてた", "*", ""};
  EXPECT_TRUE(gr.IsApplicable(grammar_triple));

  gr = GrammarRule::Parse("past 〜かった for negative 〜い *")[0];
  grammar_triple = GrammarTriple{"良くなかった", "*", ""};
  EXPECT_TRUE(gr.IsApplicable(grammar_triple));
}

TEST(TestGrammar, GrammarRule_Apply) {
  GrammarRule gr = GrammarRule::Parse("colloquial plain 〜る for continuous 〜いる v1")[0];
  GrammarTriple grammar_triple{"書いてる", "@(v1*)", "plain"};
  GrammarTriple final_triple{"書いている", "@(v1)", "continuous"};
  EXPECT_EQ(final_triple, gr.Apply(grammar_triple));

  gr = GrammarRule::Parse("past 〜かった for plain 〜い adj-i")[0];
  grammar_triple = GrammarTriple{"良くなかった", "*", ""};
  final_triple = GrammarTriple{"良くない", "@(adj-i)", "plain"};
  EXPECT_EQ(final_triple, gr.Apply(grammar_triple));
}