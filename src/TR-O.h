#include "graphs.h"
#include "BFL.h"
#include "dagUtil.h"

#include <queue>

std::queue<Edge> sort_edge(graph& graph) {
    std::queue<Edge> queue;

    set_to_topological_order(graph); // add sorting into topological order

    for (auto node : graph.nodes_) {
        for (auto adjacent_node : node->outgoing_edges_) { // loop in ascending order
            queue.push({node, adjacent_node});
        }
    }

    return std::move(queue);
}

template <size_t hash_range>
bool is_redundant(labeled_graph<hash_range> labeled_graph, Edge edge) {
    auto [u, v] = edge;
    for (auto outgoing_from_u : u->outgoing_edges_) {
        if (outgoing_from_u->index_ < v->index_ && query_reachability(labeled_graph, *outgoing_from_u, *v)) { // add index check
            return true;
        }
    }
    return false;
}

// Algorithm 2 TR-O
template <size_t hash_range>
graph tr_o(graph& graph) {
    auto labeled_graph = build_labeled_graph<hash_range>(graph, [](const node* n) { return n->index_; }, hash_range*10);
    auto queue = sort_edge(labeled_graph.graph_);

    while(!queue.empty()) {
        auto edge = queue.front();
        queue.pop();

        if(is_redundant(labeled_graph, edge)) {
            labeled_graph.graph_.remove_edge(*std::get<0>(edge), *std::get<1>(edge));
        }
    }

    return std::move(labeled_graph.graph_);
}

void tr_o_dense(graph& graph) { tr_o<160>(graph); }
void tr_o_sparse(graph& graph) { tr_o<64>(graph); }