#include "graphs.h"
#include "BFL.h"
#include "dagUtil.h"

#include <queue>
#include <unordered_set>

struct up_down_node {
    node* node_;
    bool is_up_;
    size_t degree;

    up_down_node() {}
    up_down_node(node* node, bool is_up, size_t degree) : node_(node), is_up_(is_up), degree(degree) {}
};

std::queue<Edge> sort_edge(graph& graph) {
    std::queue<Edge> queue;

    set_to_topological_order(graph); // add sorting into topological order
    std::vector<up_down_node> up_and_down_nodes(graph.nodes_.size()*2);
    // divide nodes into UP-nodes and DOWN-nodes
    for (auto node : graph.nodes_) {
        up_and_down_nodes[2*node->index_] = up_down_node(node, true, node->incoming_edges_.size());
        up_and_down_nodes[2*node->index_ + 1] = up_down_node(node, false, node->outgoing_edges_.size());
    }
    // sort up and down nodes by their degree in ascending order
    std::sort(up_and_down_nodes.begin(), up_and_down_nodes.end(), [](const up_down_node& a, const up_down_node& b) {
        return a.degree < b.degree;
    });

    std::vector<std::unordered_set<long>> handled_edges_for_node(graph.nodes_.size());

    for(auto const& up_down_node : up_and_down_nodes) {
        if(up_down_node.is_up_) {
            for (auto incoming_node : up_down_node.node_->incoming_edges_) { // loop in descending order through incoming_edges
                if(handled_edges_for_node[incoming_node->index_].find(up_down_node.node_->index_) == handled_edges_for_node[incoming_node->index_].end()) {
                    queue.push({incoming_node, up_down_node.node_});
                }
            }
        } else {
            for (auto outgoing_node : up_down_node.node_->outgoing_edges_) { // loop in descending order through incoming_edges
                if(handled_edges_for_node[up_down_node.node_->index_].find(outgoing_node->index_) == handled_edges_for_node[up_down_node.node_->index_].end()) {
                    queue.push({up_down_node.node_, outgoing_node});
                }
            }
        }
    }

    return std::move(queue);
}

template <size_t hash_range>
bool is_redundant(labeled_graph<hash_range> labeled_graph, Edge edge) {
    auto [u, v] = edge;
    if(u->outgoing_edges_.size() > v->incoming_edges_.size()) {
        for (auto w : v->incoming_edges_) {
            if (w->index_ > u->index_ && query_reachability(labeled_graph, *u, *w)) { // add index check
                return true;
            }
        }
    } else {
        for (auto w : u->outgoing_edges_) {
            if (w->index_ < v->index_ && query_reachability(labeled_graph, *w, *v)) { // add index check
                return true;
            }
        }
    }
    return false;
}

// Algorithm 3 TR-O-Plus
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