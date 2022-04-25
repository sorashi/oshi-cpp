//
// Created by praza on 18.03.2022.
//

#ifndef OSHI_CPP_GRAMMAR_CPP_UTILITIES_H_
#define OSHI_CPP_GRAMMAR_CPP_UTILITIES_H_

#include <string>
#include <vector>
#include "zlib.h"
#define CHUNK 16384

#ifdef DEBUG
#define D(x) x
#else
#define D(x)
#endif

class Utilities {
 public:
  /// Returns true if the \p s is whitespace only or it's an empty string
  /// \param s The string to check
  static bool StringIsWhitespaceOrEmpty(const std::string &s);

  static int InflateFile(FILE *source, FILE *dest);

  static void XmlEntityToEntityNameInPlace(std::string &xml_entity);

  template<class T>
  static std::ostream &Join(std::vector<T> vector, std::string delimiter, std::ostream &os) {
	bool first = true;
	for (auto &item : vector) {
	  if (first) first = false;
	  else os << delimiter;
	  os << item;
	}
	return os;
  }

  static bool AreStringsEqualCaseInsensitive(const std::string &a, const std::string &b);
  /// Converts space/tab separated globs a b c into a single glob @(a|b|c) in-place.
  /// Beware that for now, the spaces/tabs are simply replaced by |.
  /// \param space_separated_glob The space or tab separated glob patterns a b c ...
  /// \return A reference to the edited \p space_separated_glob
  static std::string &SpaceSeparatedGlobsIntoSingleGlobInPlace(std::string &space_separated_glob);
};
#endif //OSHI_CPP_GRAMMAR_CPP_UTILITIES_H_
