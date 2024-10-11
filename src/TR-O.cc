#include "TR-O.h"

#include "BFL.h"
#include "dagUtil.h"

#include <queue>
#include <algorithm>
#include <ranges>

std::queue<Edge> sort_edge_tro(graph& graph, const std::vector<long>& to, const std::vector<long>& to_reverse) {
    std::queue<Edge> queue;

    set_edges_in_topological_order(graph, to);

    for (const auto node_id : to_reverse) {
        for (const auto adjacent_node : graph.nodes_[node_id].outgoing_edges_) { // loop in ascending order
            queue.emplace(&graph.nodes_[node_id], adjacent_node);
        }
    }

    return std::move(queue);
}

template <size_t hash_range>
bool is_redundant_tro(const labeled_graph<hash_range>& labeled_graph, const Edge& edge, const std::vector<long>& to) {
    const auto [u, v] = edge;
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
    auto labeled_graph = build_labeled_graph<hash_range>(graph, [](const node* n) { return n->id_ % hash_range; }, hash_range*10);
    auto const [to, to_revere] = get_topological_order(graph); // add sorting into topological order
    auto queue = sort_edge_tro(graph, to, to_revere);

    while(!queue.empty()) {
        auto edge = queue.front();
        queue.pop();

        if(is_redundant_tro(labeled_graph, edge, to)) {
            graph.remove_edge(*std::get<0>(edge), *std::get<1>(edge));
        }
    }
}

void tr_o_dense(graph& graph) { tr_o<160>(graph); }
void tr_o_sparse(graph& graph) { tr_o<64>(graph); }