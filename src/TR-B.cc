#include "TR-B.h"

#include "BFL.h"

#include <queue>
#include <algorithm>

std::queue<Edge> sort_edge(graph& graph) {
    std::queue<Edge> queue;

    for (auto& node : graph.nodes_) {
        for (const auto adjacent_node : node.outgoing_edges_) {
            queue.emplace(&node, adjacent_node);
        }
    }

    return std::move(queue);
}

template <size_t hash_range>
bool is_redundant(const labeled_graph<hash_range>& labeled_graph, const Edge& edge) {
    const auto [u, v] = edge;
    // check weather any of the outgoing edges from u can reach v
    for(auto const& w : u->outgoing_edges_) {
        if(w == v) continue;
        if(query_reachability(labeled_graph, *w, *v)) {
            return true;
        }
    }
    return false;
}

// Algorithm 1 TR-B
template <size_t hash_range>
void tr_b(graph& graph) {
    auto labeled_graph = build_labeled_graph<hash_range>(graph, [](const node* n) { return std::hash<long>{}(n->id_) % hash_range; }, hash_range*10);
    auto queue = sort_edge(labeled_graph.graph_);

    while(!queue.empty()) {
        auto edge = queue.front();
        queue.pop();

        if(is_redundant(labeled_graph, edge)) {
            graph.remove_edge(*std::get<0>(edge), *std::get<1>(edge));
        }
    }
}

void tr_b_dense(graph& graph) { tr_b<512>(graph); }
void tr_b_sparse(graph& graph) { tr_b<64>(graph); }