#ifndef GUARD_TASK_MAPPING_ORBIT_H
#define GUARD_TASK_MAPPING_ORBIT_H

#include <algorithm>
#include <cassert>
#include <functional>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "perm_set.hpp"
#include "task_mapping.hpp"
#include "util.hpp"

namespace mpsym
{

class TMO
{
  class IterationState
  {
  public:
    IterationState(TMO const *orbit)
    : _generators(&orbit->_generators),
      _unprocessed{orbit->_root}
    { current = _unprocessed.begin(); }

    std::unordered_set<TaskMapping>::iterator current;

    void advance();
    bool exhausted() const;

  private:
    internal::PermSet const *_generators;
    std::unordered_set<TaskMapping> _unprocessed, _processed;
  };

public:
  using value_type = TaskMapping;
  using const_reference = TaskMapping const &;

  class const_iterator : public util::Iterator<const_iterator, TaskMapping const>
  {
  public:
    const_iterator(std::shared_ptr<IterationState> state = nullptr)
    : _state(state)
    {}

    bool operator==(const_iterator const &rhs) const override
    { return end() && rhs.end(); }

  private:
    reference current() override
    { return *_state->current; }

    void next() override
    { _state->advance(); }

    bool end() const
    { return !_state || _state->exhausted(); }

    std::shared_ptr<IterationState> _state;
  };

  TMO(TaskMapping const &mapping, internal::PermSet const &generators)
  : _root(mapping),
    _generators(generators)
  {}

  const_iterator begin() const
  { return const_iterator(std::make_shared<IterationState>(this)); }

  const_iterator end() const
  { return const_iterator(); }

private:
  TaskMapping _root;
  internal::PermSet _generators;
};

class TMORs
{
  using orbit_reprs_map = std::unordered_map<TaskMapping, unsigned>;

public:
  class const_iterator
  : public util::Iterator<const_iterator, TaskMapping const>
  {
  public:
    const_iterator(orbit_reprs_map::const_iterator const &it)
    : _it(it)
    {}

    bool operator==(const_iterator const &rhs) const override
    { return _it == rhs._it; }

  private:
    reference current() override
    { return _it->first; }

    void next() override
    { ++_it; }

    orbit_reprs_map::const_iterator _it;
  };

  bool operator==(TMORs const &rhs) const
  { return orbit_repr_set() == rhs.orbit_repr_set(); }

  bool operator!=(TMORs const &rhs) const
  { return !(*this == rhs); }

  std::pair<bool, unsigned> insert(TaskMapping const &mapping);

  template<typename IT>
  void insert_all(IT first, IT last)
  {
    for (auto it = first; it != last; ++it)
      insert(*it);
  }

  bool is_repr(TaskMapping const &mapping) const
  {
    auto it(_orbit_reprs.find(mapping));

    return it != _orbit_reprs.end();
  }

  unsigned num_orbits() const
  { return static_cast<unsigned>(_orbit_reprs.size()); }

  const_iterator begin() const
  { return const_iterator(_orbit_reprs.begin()); }

  const_iterator end() const
  { return const_iterator(_orbit_reprs.end()); }

private:
  std::unordered_set<TaskMapping> orbit_repr_set() const
  {
    std::unordered_set<TaskMapping> ret;
    for (auto const &repr : _orbit_reprs)
      ret.insert(repr.first);

    return ret;
  }

  orbit_reprs_map _orbit_reprs;
};

} // namespace mpsym

#endif // GUARD_TASK_MAPPING_ORBIT_H