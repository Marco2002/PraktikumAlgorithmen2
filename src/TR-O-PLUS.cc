#include "TR-O-PLUS.h"

#include <unordered_set>

#include "dagUtil.h"
#include "MurmurHash3.h"

struct up_down_node {
    node* node_;
    bool is_up_;
    size_t degree;

    up_down_node(node* node, bool const is_up, size_t const degree) : node_(node), is_up_(is_up), degree(degree) {}
};

std::vector<Edge> sort_edge_tro_plus(graph& graph, std::vector<long> const& to) {
    set_edges_in_topological_order(graph, to); // add sorting into topological order

    std::vector<Edge> queue;
    queue.reserve(graph.number_of_edges_);

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

    std::unordered_set<Edge, EdgeHash> handled_edges(graph.number_of_edges_);

    for(auto const& up_down_node : up_and_down_nodes) {
        if(up_down_node.is_up_) {
            for (auto incoming_node : up_down_node.node_->incoming_edges_) { // loop in descending order through incoming_edges
                auto edge = std::make_tuple(incoming_node, up_down_node.node_);
                auto [it, inserted] = handled_edges.insert(edge);
                if (inserted) {
                    queue.emplace_back(std::move(edge));
                }
            }
        } else {
            for (auto outgoing_node : up_down_node.node_->outgoing_edges_) { // loop in ascending order through outgoing_edges
                auto edge = std::make_tuple(up_down_node.node_, outgoing_node);
                auto [it, inserted] = handled_edges.insert(edge);
                if (inserted) {
                    queue.emplace_back(std::move(edge));
                }
            }
        }
    }

    return std::move(queue);
}

template <size_t hash_range>
bool is_redundant_tro_plus(labeled_graph<hash_range> const& labeled_graph, Edge const& edge, std::vector<long> const& to) {
    auto const [u, v] = edge;
    auto const u_index = to[u->id_];
    auto const v_index = to[v->id_];
    if(u->outgoing_edges_.size() > v->incoming_edges_.size()) {
        for (auto const w : v->incoming_edges_) { // loop in descending order through incoming_edges
            if (to[w->id_] <= u_index) break; // add index check
            if (query_reachability(labeled_graph, *u, *w)) {
                return true;
            }
        }
    } else {
        for (auto const w : u->outgoing_edges_) { // loop in ascending order through outgoing_edges
            if (to[w->id_] >= v_index) break; // add index check
            if (query_reachability(labeled_graph, *w, *v)) {
                return true;
            }
        }
    }
    return false;
}

// Algorithm 3 TR-O-Plus
void tr_o_plus(graph& graph) {
    auto const hash_range = 1024;
    auto const [to, to_reverse] = get_topological_order(graph);
    auto const labeled_graph = build_labeled_graph<hash_range>(graph, [](node const* n) { return hash_in_range(n->id_, hash_range); }, hash_range*10);

    auto queue = sort_edge_tro_plus(graph, to);

    for(auto edge : queue) {
        if(is_redundant_tro_plus(labeled_graph, edge, to)) {
            graph.remove_edge(*std::get<0>(edge), *std::get<1>(edge));
        }
    }
}