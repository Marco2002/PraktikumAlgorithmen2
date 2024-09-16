#include "TR-B.h"

#include "BFL.h"

#include <queue>

std::queue<Edge> sort_edge(graph& graph) {
    std::queue<Edge> queue;

    for (auto node : graph.nodes_) {
        for (auto adjacent_node : node->outgoing_edges_) {
            queue.push({node, adjacent_node});
        }
    }

    return std::move(queue);
}

template <size_t hash_range>
bool is_redundant(labeled_graph<hash_range> labeled_graph, Edge edge) {
    auto [u, v] = edge;
    for (auto outgoing_from_u : u->outgoing_edges_) {
        if (outgoing_from_u != v && query_reachability(labeled_graph, *outgoing_from_u, *v)) {
            return true;
        }
    }
    return false;
}

// Algorithm 1 TR-B
template <size_t hash_range>
void tr_b(graph& graph) {
    auto labeled_graph = build_labeled_graph<hash_range>(graph, [](const node* n) { return n->id_ % hash_range; }, hash_range*10);
    auto queue = sort_edge(labeled_graph.graph_);

    while(!queue.empty()) {
        auto edge = queue.front();
        queue.pop();

        if(is_redundant(labeled_graph, edge)) {
            graph.remove_edge(*std::get<0>(edge), *std::get<1>(edge));
        }
    }
}

void tr_b_dense(graph& graph) { tr_b<160>(graph); }
void tr_b_sparse(graph& graph) { tr_b<64>(graph); }