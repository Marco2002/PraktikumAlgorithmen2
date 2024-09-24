#include "TR-O-PLUS.h"

#include "dagUtil.h"

#include <queue>
#include <unordered_set>

struct up_down_node {
    node* node_;
    bool is_up_;
    size_t degree;

    up_down_node(node* node, const bool is_up, const size_t degree) : node_(node), is_up_(is_up), degree(degree) {}
};

std::queue<Edge> sort_edge_tro_plus(graph& graph, const std::vector<long>& to, const std::vector<long>& to_reverse) {
    std::queue<Edge> queue;

    set_edges_in_topological_order(graph, to);

    std::vector<up_down_node> up_and_down_nodes;
    up_and_down_nodes.reserve(graph.nodes_.size()*2);
    // divide nodes into UP-nodes and DOWN-nodes
    for (auto const node_id : to_reverse) {
        auto& node = graph.nodes_[node_id];
        up_and_down_nodes.emplace_back(&node, true, node.incoming_edges_.size());
        up_and_down_nodes.emplace_back(&node, false, node.outgoing_edges_.size());
    }
    // sort up and down nodes by their degree in ascending order
    std::sort(up_and_down_nodes.begin(), up_and_down_nodes.end(), [](const up_down_node& a, const up_down_node& b) {
        return a.degree < b.degree;
    });

    std::vector<std::unordered_set<long>> handled_edges_for_node(graph.nodes_.size());

    for(auto const& up_down_node : up_and_down_nodes) {
        if(up_down_node.is_up_) {
            for (auto incoming_node : up_down_node.node_->incoming_edges_) { // loop in descending order through incoming_edges
                if(!handled_edges_for_node[incoming_node->id_].contains(up_down_node.node_->id_)) {
                    queue.emplace(incoming_node, up_down_node.node_);
                    handled_edges_for_node[incoming_node->id_].insert(up_down_node.node_->id_);
                }
            }
        } else {
            for (auto outgoing_node : up_down_node.node_->outgoing_edges_) { // loop in descending order through incoming_edges
                if(!handled_edges_for_node[up_down_node.node_->id_].contains(outgoing_node->id_)) {
                    queue.emplace(up_down_node.node_, outgoing_node);
                    handled_edges_for_node[up_down_node.node_->id_].insert(outgoing_node->id_);
                }
            }
        }
    }

    return std::move(queue);
}

template <size_t hash_range>
bool is_redundant_tro_plus(const labeled_graph<hash_range>& labeled_graph, const Edge& edge, const std::vector<long>& to) {
    const auto [u, v] = edge;
    if(u->outgoing_edges_.size() > v->incoming_edges_.size()) {
        for (auto w : v->incoming_edges_) {
            if (to[w->id_] <= to[u->id_]) break; // add index check
            if (query_reachability(labeled_graph, *u, *w)) {
                return true;
            }
        }
    } else {
        for (auto w : u->outgoing_edges_) {
            if (to[w->id_] >= to[v->id_]) break; // add index check
            if (query_reachability(labeled_graph, *w, *v)) {
                return true;
            }
        }
    }
    return false;
}

// Algorithm 3 TR-O-Plus
template <size_t hash_range>
void tr_o_plus(graph& graph) {
    auto const [to, to_reverse] = get_topological_order(graph); // add sorting into topological order
    auto queue = sort_edge_tro_plus(graph, to, to_reverse);

    auto labeled_graph = build_labeled_graph<hash_range>(graph, [](const node* n) { return n->id_ % hash_range; }, hash_range*10);

    while(!queue.empty()) {
        auto edge = queue.front();
        queue.pop();

        if(is_redundant_tro_plus(labeled_graph, edge, to)) {
            graph.remove_edge(*std::get<0>(edge), *std::get<1>(edge));
        }
    }
}

void tr_o_plus_dense(graph& graph) { tr_o_plus<160>(graph); }
void tr_o_plus_sparse(graph& graph) { tr_o_plus<64>(graph); }