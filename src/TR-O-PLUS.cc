#include "TR-O-PLUS.h"

#include <queue>
#include <unordered_set>

#include "dagUtil.h"
#include "MurmurHash3.h"

struct up_down_node {
    node* node_;
    bool is_up_;
    size_t degree;

    up_down_node(node* node, bool const is_up, size_t const degree) : node_(node), is_up_(is_up), degree(degree) {}
};

std::queue<Edge> sort_edge_tro_plus(graph& graph, std::vector<long> const& to, std::vector<long> const& to_reverse) {
    std::queue<Edge> queue;

    std::vector<up_down_node> up_and_down_nodes;
    up_and_down_nodes.reserve(graph.nodes_.size()*2);
    // divide nodes into UP-nodes and DOWN-nodes
    for (auto& node : graph.nodes_) {
        up_and_down_nodes.emplace_back(&node, true, node.incoming_edges_.size());
        up_and_down_nodes.emplace_back(&node, false, node.outgoing_edges_.size());
    }
    // sort up and down nodes by their degree in ascending order
    std::sort(up_and_down_nodes.begin(), up_and_down_nodes.end(), [](up_down_node const& a, up_down_node const& b) {
        return a.degree < b.degree;
    });

    std::unordered_set<Edge, EdgeHash> handled_edges(graph.nodes_.size());

    for(auto const& up_down_node : up_and_down_nodes) {
        if(up_down_node.is_up_) {
            for (auto incoming_node : up_down_node.node_->incoming_edges_) { // loop in descending order through incoming_edges
                auto edge = std::make_tuple(incoming_node, up_down_node.node_);
                if(!handled_edges.contains(edge)) {
                    queue.emplace(incoming_node, up_down_node.node_);
                    handled_edges.insert(edge);
                }
            }
        } else {
            for (auto outgoing_node : up_down_node.node_->outgoing_edges_) { // loop in ascending order through outgoing_edges
                auto edge = std::make_tuple(up_down_node.node_, outgoing_node);
                if(!handled_edges.contains(edge)) {
                    queue.emplace(up_down_node.node_, outgoing_node);
                    handled_edges.insert(edge);
                }
            }
        }
    }

    return std::move(queue);
}

template <size_t hash_range>
bool is_redundant_tro_plus(labeled_graph<hash_range> const& labeled_graph, Edge const& edge, std::vector<long> const& to) {
    auto const [u, v] = edge;
    if(u->outgoing_edges_.size() > v->incoming_edges_.size()) {
        for (auto it = v->incoming_edges_.begin(); it != v->incoming_edges_.end(); ++it) { // loop in descending order through incoming_edges
            if (to[(*it)->id_] <= to[u->id_]) break; // add index check
            if (query_reachability(labeled_graph, *u, *(*it))) {
                return true;
            }
        }
    } else {
        for (auto it = u->outgoing_edges_.begin(); it != u->outgoing_edges_.end(); ++it) { // loop in ascending order through outgoing_edges
            if (to[(*it)->id_] >= to[v->id_]) break; // add index check
            if (query_reachability(labeled_graph, *(*it), *v)) {
                return true;
            }
        }
    }
    return false;
}

// Algorithm 3 TR-O-Plus
template <size_t hash_range>
void tr_o_plus(graph& graph) {
    auto const [to, to_reverse] = get_topological_order(graph);
    set_edges_in_topological_order(graph, to); // add sorting into topological order

    auto const labeled_graph = build_labeled_graph<hash_range>(graph, [](node const* n) { return hash_in_range(n->id_, hash_range); }, hash_range*10);

    auto queue = sort_edge_tro_plus(graph, to, to_reverse);

    while(!queue.empty()) {
        auto const edge = queue.front();
        queue.pop();

        if(is_redundant_tro_plus(labeled_graph, edge, to)) {
            graph.remove_edge(*std::get<0>(edge), *std::get<1>(edge));
        }
    }
}

void tr_o_plus_dense(graph& graph) { tr_o_plus<512>(graph); }
void tr_o_plus_sparse(graph& graph) { tr_o_plus<64>(graph); }