#include <queue>
#include <stdexcept>
#include <type_traits>
#include <unordered_set>
#include <utility>
#include <vector>

#include <boost/graph/adjacency_list.hpp>

#include "arch_graph_system.h"
#include "dbg.h"
#include "perm.h"
#include "perm_set.h"
#include "task_allocation.h"
#include "task_orbits.h"
#include "timer.h"

namespace cgtl
{

TaskAllocation ArchGraphSystem::mapping(TaskAllocation const &allocation,
                                        unsigned offset,
                                        MappingOptions *options,
                                        TaskOrbits *orbits)
{
  options = get_options(options);

  DBG(DEBUG) << "Requested task mapping for: " << allocation;

  TaskAllocation representative =
    options->method == MappingMethod::ITERATE ?
      min_elem_iterate(allocation, offset, options, orbits) :
    options->method == MappingMethod::LOCAL_SEARCH ?
      min_elem_local_search(allocation, offset, options, orbits) :
    options->method == MappingMethod::ORBITS ?
      min_elem_orbits(allocation, offset, options, orbits) :
    throw std::logic_error("unreachable");

  if (orbits)
    orbits->insert(representative);

  return representative;
}

TaskAllocation ArchGraphSystem::min_elem_iterate(TaskAllocation const &tasks,
                                                 unsigned offset,
                                                 MappingOptions *options,
                                                 TaskOrbits *orbits)
{
  DBG(DEBUG) << "Performing mapping by iteration";

  TIMER_START("map bruteforce iterate");

  TaskAllocation representative(tasks);

  for (Perm const &element : automorphisms()) {
    if (tasks.less_than(representative, element, offset)) {
      representative = tasks.permuted(element, offset);

      if (is_representative(representative, options, orbits)) {
        TIMER_STOP("map bruteforce iterate");
        return representative;
      }
    }
  }

  TIMER_STOP("map bruteforce iterate");

  DBG(DEBUG) << "Found minimal orbit element: " << representative;

  return representative;
}

TaskAllocation ArchGraphSystem::min_elem_local_search(TaskAllocation const &tasks,
                                                      unsigned offset,
                                                      MappingOptions *options,
                                                      TaskOrbits *orbits)
{
  (void)options;
  (void)orbits;

  DBG(TRACE) << "Performing approximate mapping by local search";

  TIMER_START("map approx local search");

  TaskAllocation representative(tasks);

  bool stationary = false;
  while (!stationary) {
    stationary = true;

    for (Perm const &generator : automorphisms().generators()) {
      if (representative.less_than(representative, generator, offset)) {
        representative.permute(generator, offset);

        stationary = false;
      }
    }
  }

  TIMER_STOP("map approx local search");

  DBG(DEBUG) << "Found approximate minimal orbit element: " << representative;

  return representative;
}

TaskAllocation ArchGraphSystem::min_elem_orbits(TaskAllocation const &tasks,
                                                unsigned offset,
                                                MappingOptions *options,
                                                TaskOrbits *orbits)
{
  DBG(TRACE) << "Performing mapping by orbit construction";

  TIMER_START("map bruteforce orbits");

  TaskAllocation representative(tasks);

  std::unordered_set<TaskAllocation> processed;
  std::queue<TaskAllocation> unprocessed;

  unprocessed.push(tasks);

  while (!unprocessed.empty()) {
    TaskAllocation current(unprocessed.front());
    unprocessed.pop();

    processed.insert(current);

    if (current.less_than(representative))
      representative = current;

    for (Perm const &generator : automorphisms().generators()) {
      TaskAllocation next(current.permuted(generator, offset));

      if (is_representative(next, options, orbits)) {
        TIMER_STOP("map bruteforce orbits");
        return next;
      } else if (processed.find(next) == processed.end()) {
        unprocessed.push(next);
      }
    }
  }

  TIMER_STOP("map bruteforce orbits");

  return representative;;
}

ArchGraphSystem::MappingOptions ArchGraphSystem::_default_mapping_options;

} // namespace cgtl
