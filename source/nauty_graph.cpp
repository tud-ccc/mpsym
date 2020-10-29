#include <cassert>
#include <map>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

extern "C" {
  #include "nauty.h"
  #include "nautinv.h"
}

#include "arch_graph_system.hpp"
#include "bsgs.hpp"
#include "dump.hpp"
#include "nauty_graph.hpp"
#include "perm_group.hpp"
#include "perm_set.hpp"

namespace
{

template<typename T>
T *_alloc(std::size_t sz)
{
  void *ret = ALLOCS(sz, sizeof(T));
  if (!ret)
    throw std::bad_alloc();

  return static_cast<T *>(ret);
}

mpsym::internal::PermSet _gens;
int _gen_degree;

void _save_gens(int, int *perm, int *, int, int, int)
{
  std::vector<unsigned> tmp(_gen_degree);
  for (int i = 0; i < _gen_degree; ++i)
    tmp[i] = perm[i] + 1;

  _gens.emplace(tmp);
}

} // anonymous namespace

namespace mpsym
{

namespace internal
{

NautyGraph::NautyGraph(int n,
                       int n_reduced,
                       bool directed,
                       bool effectively_directed)
: _directed(directed),
  _effectively_directed(effectively_directed),
  _n(n),
  _n_reduced(n_reduced),
  _m(SETWORDSNEEDED(n))
{
#ifndef NDEBUG
  nauty_check(WORDSIZE, _m, _n, NAUTYVERSIONID);
#endif

  _g = _alloc<graph>(_m * _n);
  _lab = _alloc<int>(_n);
  _ptn = _alloc<int>(_n);
  _orbits = _alloc<int>(_n);

  EMPTYGRAPH(_g, _m, _n);
}

NautyGraph::~NautyGraph()
{
  FREES(_g);
  FREES(_lab);
  FREES(_ptn);
  FREES(_orbits);

  naugraph_freedyn();
  nautil_freedyn();
  nauty_freedyn();
}

std::string NautyGraph::to_gap() const
{
  std::stringstream ss;

  ss << "ReduceGroup(GraphAutoms([";

  // edges
  for (auto const &edge : _edges) {
    int source = edge.first + 1;
    int target = edge.second + 1;

    if (source != target) {
      ss << "[" << source << "," << target << "],";

      if (!_directed)
         ss << "[" << target << "," << source << "],";
    }
  }

  ss << "],";

  // partition
  auto ptn_inc(_ptn_expl);
  for (auto &p : ptn_inc) {
    for (auto &v : p)
      ++v;
  }

  ss << DUMP(ptn_inc) << ",";

  // number of vertices
  ss << _n << "),";

  // number of vertices to reduce to
  ss << _n_reduced << ")";

  return ss.str();
}

void NautyGraph::add_edge(int from, int to)
{
  assert(from < _n);
  assert(to < _n);

  if (_directed)
    ADDONEARC(_g, from, to, _m);
  else
    ADDONEEDGE(_g, from, to, _m);

  _edges.emplace_back(from, to);
}

void NautyGraph::add_edges(std::map<int, std::vector<int>> const &adj)
{
  for (auto const &p : adj) {
    int from = p.first;
    for (int to : p.second)
      add_edge(from, to);
  }
}

void NautyGraph::set_partition(std::vector<std::vector<int>> const &ptn)
{
  _ptn_expl = ptn;

  int i = 0;
  for (auto const &p : _ptn_expl) {
    for (auto j = 0u; j < p.size(); ++j) {
      _lab[i] = p[j];
      _ptn[i] = (j == p.size() - 1u) ? 0 : 1;
      ++i;
    }
  }
}

PermSet NautyGraph::automorphism_generators() const
{
  static DEFAULTOPTIONS_DIGRAPH(nauty_options_directed);

  static DEFAULTOPTIONS_GRAPH(nauty_options_undirected);

  if (_edges.empty())
    return {};

  auto &nauty_options = _effectively_directed ? nauty_options_directed
                                              : nauty_options_undirected;

  nauty_options.defaultptn = _ptn_expl.empty() ? TRUE : FALSE;
  nauty_options.userautomproc = _save_gens;

  _gens.clear();
  _gen_degree = _n_reduced;

  statsblk stats;
  densenauty(_g, _lab, _ptn, _orbits, &nauty_options, &stats, _m, _n, nullptr);

  return _gens;
}

} // namespace internal

} // namespace mpsym
