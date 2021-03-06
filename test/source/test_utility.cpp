#include <algorithm>
#include <cassert>
#include <cstring>
#include <fstream>
#include <map>
#include <string>
#include <vector>

#include "gmock/gmock.h"
#include "gmock/gmock-matchers.h"

#include "perm.hpp"
#include "perm_group.hpp"
#include "perm_set.hpp"
#include "test_utility.hpp"

using namespace mpsym;
using namespace mpsym::internal;

template <typename P>
static testing::AssertionResult _perm_equal(
  std::vector<unsigned> const &expected, P const &perm)
{
  bool success = true;

  if (perm.degree() != expected.size()) {
    return testing::AssertionFailure()
      << "Permutation has incorrect degree (expected " << expected.size()
      << " but got " << perm.degree();
  }

  std::stringstream err;
  err << "Permutation differs:\n";

  for (unsigned i = 0u; i < perm.degree(); ++i) {
    if (perm[i] != expected[i]) {
      success = false;
      err << "@ index " << i << ":"
          << " expected " << expected[i]
          << " but got " << perm[i] << '\n';
    }
  }

  if (!success)
    return testing::AssertionFailure() << err.str();
  else
    return testing::AssertionSuccess();
}

testing::AssertionResult perm_equal(std::vector<unsigned> const &expected,
  Perm const &perm)
{
  return _perm_equal<Perm>(expected, perm);
}

testing::AssertionResult perm_group_equal(PermGroup const &expected,
                                          PermGroup const &actual)
{
  PermSet expected_elements;
  for (Perm const &perm : expected)
    expected_elements.insert(perm);

  return perm_group_equal(expected_elements, actual);
}

testing::AssertionResult perm_group_equal(PermSet expected_elements,
                                          PermGroup const &actual)
{
  bool expected_has_id = false;
  for (Perm const &perm : expected_elements) {
    if (perm.id()) {
      expected_has_id = true;
      break;
    }
  }

  if (!expected_has_id)
    expected_elements.emplace(actual.degree());

  std::vector<Perm> actual_elements;
  for (Perm const &perm : actual)
    actual_elements.push_back(perm);

  testing::Matcher<std::vector<Perm> const &> matcher(
    testing::UnorderedElementsAreArray(expected_elements.begin(),
                                       expected_elements.end()));

  testing::StringMatchResultListener listener;
  if (matcher.MatchAndExplain(actual_elements, &listener))
    return testing::AssertionSuccess();

  std::stringstream ss;
  ss << "\nShould be: ";
  matcher.DescribeTo(&ss);

  ss << "\nBut is: { ";
  for (auto i = 0u; i < actual_elements.size(); ++i) {
    ss << actual_elements[i]
       << (i == actual_elements.size() - 1u ? "" : ", ");
  }
  ss << " },\n";
  ss << listener.str();

  std::string msg(ss.str());

  int const indent = 4;

  size_t i = 0u;
  for (;;) {
    if ((i = msg.find('\n', i)) == std::string::npos)
      break;

    msg.insert(i + 1, std::string(indent, ' '));
    i += indent + 1;
  }

  return testing::AssertionFailure() << msg;
}

PermGroup verified_perm_group(VerifiedGroup group)
{
  struct PermutationGroupDescription {
    PermutationGroupDescription() {}

    PermutationGroupDescription(PermSet generators,
                                PermSet elements)
     : degree(generators.degree()),
       generators(generators),
       elements(elements) {}

    unsigned degree;
    PermSet generators;
    PermSet elements;
    bool verified = false;
  };

  static std::map<VerifiedGroup, PermutationGroupDescription> verified_groups {
    {A3,
      PermutationGroupDescription(
        {Perm(3, {{0, 1, 2}})},
        {Perm(3),
         Perm(3, {{0, 1, 2}}),
         Perm(3, {{0, 2, 1}})})
    },
    {A4,
      PermutationGroupDescription(
        {Perm(4, {{0, 1, 2}}),
         Perm(4, {{1, 2, 3}})},
        {Perm(4),
         Perm(4, {{0, 1, 2}}),
         Perm(4, {{0, 1, 3}}),
         Perm(4, {{0, 1}, {2, 3}}),
         Perm(4, {{0, 2, 1}}),
         Perm(4, {{0, 2, 3}}),
         Perm(4, {{0, 2}, {1, 3}}),
         Perm(4, {{0, 3, 1}}),
         Perm(4, {{0, 3, 2}}),
         Perm(4, {{0, 3}, {1, 2}}),
         Perm(4, {{1, 2, 3}}),
         Perm(4, {{1, 3, 2}})})
    },
    {A5,
      PermutationGroupDescription(
        {Perm(5, {{0, 1, 2, 3, 4}}),
         Perm(5, {{2, 3, 4}})},
        {Perm(5),
         Perm(5, {{0, 1, 2, 3, 4}}),
         Perm(5, {{0, 1, 2, 4, 3}}),
         Perm(5, {{0, 1, 2}}),
         Perm(5, {{0, 1, 3, 2, 4}}),
         Perm(5, {{0, 1, 3, 4, 2}}),
         Perm(5, {{0, 1, 3}}),
         Perm(5, {{0, 1, 4, 2, 3}}),
         Perm(5, {{0, 1, 4, 3, 2}}),
         Perm(5, {{0, 1, 4}}),
         Perm(5, {{0, 1}, {2, 3}}),
         Perm(5, {{0, 1}, {2, 4}}),
         Perm(5, {{0, 1}, {3 ,4}}),
         Perm(5, {{0, 2, 1, 3, 4}}),
         Perm(5, {{0, 2, 1, 4, 3}}),
         Perm(5, {{0, 2, 1}}),
         Perm(5, {{0, 2, 3, 1, 4}}),
         Perm(5, {{0, 2, 3, 4, 1}}),
         Perm(5, {{0, 2, 3}}),
         Perm(5, {{0, 2, 4, 1, 3}}),
         Perm(5, {{0, 2, 4, 3, 1}}),
         Perm(5, {{0, 2, 4}}),
         Perm(5, {{0, 2}, {1, 3}}),
         Perm(5, {{0, 2}, {1, 4}}),
         Perm(5, {{0, 2}, {3, 4}}),
         Perm(5, {{0, 3, 1, 2, 4}}),
         Perm(5, {{0, 3, 1, 4, 2}}),
         Perm(5, {{0, 3, 1}}),
         Perm(5, {{0, 3, 2, 1, 4}}),
         Perm(5, {{0, 3, 2, 4, 1}}),
         Perm(5, {{0, 3, 2}}),
         Perm(5, {{0, 3, 4, 1, 2}}),
         Perm(5, {{0, 3, 4, 2, 1}}),
         Perm(5, {{0, 3, 4}}),
         Perm(5, {{0, 3}, {1, 2}}),
         Perm(5, {{0, 3}, {1, 4}}),
         Perm(5, {{0, 3}, {2, 4}}),
         Perm(5, {{0, 4, 1, 2, 3}}),
         Perm(5, {{0, 4, 1, 3, 2}}),
         Perm(5, {{0, 4, 1}}),
         Perm(5, {{0, 4, 2, 1, 3}}),
         Perm(5, {{0, 4, 2, 3, 1}}),
         Perm(5, {{0, 4, 2}}),
         Perm(5, {{0, 4, 3, 1, 2}}),
         Perm(5, {{0, 4, 3, 2, 1}}),
         Perm(5, {{0, 4, 3}}),
         Perm(5, {{0, 4}, {1, 2}}),
         Perm(5, {{0, 4}, {1, 3}}),
         Perm(5, {{0, 4}, {2, 3}}),
         Perm(5, {{1, 2, 3}}),
         Perm(5, {{1, 2, 4}}),
         Perm(5, {{1, 2}, {3, 4}}),
         Perm(5, {{1, 3}, {2, 4}}),
         Perm(5, {{1, 3, 2}}),
         Perm(5, {{1, 3, 4}}),
         Perm(5, {{1, 4, 2}}),
         Perm(5, {{1, 4, 3}}),
         Perm(5, {{1, 4}, {2, 3}}),
         Perm(5, {{2, 3, 4}}),
         Perm(5, {{2, 4, 3}})})
    },
  };

  auto verified_group(verified_groups[group]);

  PermGroup pg(verified_group.degree, verified_group.generators);

  if (!verified_group.verified) {
    assert(perm_group_equal(verified_group.elements, pg));
    verified_group.verified = true;
  }

  return pg;
}
