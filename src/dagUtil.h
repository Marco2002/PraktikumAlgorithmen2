#include "graphs.h"
#include <unordered_set>

using namespace graphs;

using NodeOrder = std::tuple<std::vector<long>, std::vector<long>>;

NodeOrder get_topological_order(graph&);

void set_edges_in_topological_order(graph& dag, const std::vector<long>& to);

bool all_nodes_edges_are_in_topological_order(graph const& graph);

std::unordered_set<const node*> find_all_reachable_nodes(const node& u, bool include_root = true);