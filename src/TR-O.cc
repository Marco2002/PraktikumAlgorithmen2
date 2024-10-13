#include "TR-O.h"

#include <queue>
#include <algorithm>
#include <ranges>

#include "BFL.h"
#include "dagUtil.h"
#include "MurmurHash3.h"

std::vector<Edge> sort_edge_tro(graph& graph, std::vector<long> const& to) {
    std::vector<Edge> queue;

    set_edges_in_topological_order(graph, to); // add sorting into topological order

    for (auto& node : graph.nodes_) {
        for (auto adjacent_node : node.outgoing_edges_) { // loop in ascending order
            queue.emplace_back(&node, adjacent_node);
        }
    }

    return std::move(queue);
}

template <size_t hash_range>
bool is_redundant_tro(labeled_graph<hash_range> const& labeled_graph, Edge const& edge, std::vector<long> const& to) {
    auto const [u, v] = edge;
    for (auto w : u->outgoing_edges_) {
        if (to[w->id_] >= to[v->id_]) break; // add index check
        if (query_reachability(labeled_graph, *w, *v)) {
            return true;
        }
    }
    return false;
}

// Algorithm 2 TR-O
template <size_t hash_range>
void tr_o(graph& graph) {
    auto const [to, to_revere] = get_topological_order(graph);
    auto const labeled_graph = build_labeled_graph<hash_range>(graph, [](node const* n) { return hash_in_range(n->id_, hash_range); }, hash_range*10);
    auto queue = sort_edge_tro(graph, to);

    for(auto edge : queue) {
        if(is_redundant_tro(labeled_graph, edge, to)) {
            graph.remove_edge(*std::get<0>(edge), *std::get<1>(edge));
        }
    }
}

void tr_o_dense(graph& graph) { tr_o<1024>(graph); }
void tr_o_sparse(graph& graph) { tr_o<64>(graph); }