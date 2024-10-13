#include "graphs.h"
#include <unordered_set>

using namespace graphs;

using NodeOrder = std::tuple<std::vector<long>, std::vector<long>>;

NodeOrder get_topological_order(graph&);

void set_edges_in_topological_order(graph& dag, std::vector<long> const& to);

bool all_nodes_edges_are_in_topological_order(graph const& graph);

std::unordered_set<node const*> find_all_reachable_nodes(node const& u, bool include_root = true);

void build_tr_by_dfs(graph& g);

graph copy_graph(graph& g);

void shuffle_graph(graph& g, long seed);