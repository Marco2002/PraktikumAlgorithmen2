#include "graphs.h"

#include <bitset>
#include <functional>
#include <utility>

using namespace graphs;

template <size_t hash_range>
using LabelIn = std::vector<std::bitset<hash_range>>;

template <size_t hash_range>
using LabelOut = std::vector<std::bitset<hash_range>>;

using LabelDiscovery = std::vector<long>;
using LabelFinish = std::vector<long>;

template <size_t hash_range>
struct labeled_graph {
    graph graph_;
    LabelDiscovery label_discovery_;
    LabelFinish label_finish_;
    LabelIn<hash_range> label_in_;
    LabelOut<hash_range> label_out_;

    labeled_graph(graph graph, LabelDiscovery label_discovery, LabelFinish label_finish, LabelIn<hash_range> label_in, LabelOut<hash_range> label_out)
        : graph_(std::move(graph)), label_discovery_(std::move(label_discovery)), label_finish_(std::move(label_finish)), label_in_(label_in), label_out_(label_out) {}
};


void depth_first_search_visit(const node& n, LabelDiscovery& label_discover, LabelFinish& label_finish, std::vector<const node*>& post_order, long& current) {
    label_discover[n.index_] = ++current;
    for (auto const e : n.outgoing_edges_) {
        if (label_discover[e->index_] != 0) continue; // if e was visited

        depth_first_search_visit(*e, label_discover, label_finish, post_order, current);
    }
    post_order.push_back(&n);
    label_finish[n.index_] = ++current;

}

std::tuple<std::vector<const node*>, LabelDiscovery, LabelFinish> depth_first_search(const graph& g) {
    LabelDiscovery label_discovery(g.nodes_.size());
    LabelFinish label_finish(g.nodes_.size());
    std::vector<const node*> post_order = {};
    long current = 0;

    for(auto n : g.nodes_) {
        if(n->incoming_edges_.empty()) {
            depth_first_search_visit(*n, label_discovery, label_finish, post_order, current);
        }
    }

    return std::make_tuple(post_order, label_discovery, label_finish);
}

std::vector<const node*> merge_vertices(const std::vector<const node*>& post_order, const long d) {
    std::vector<const node*> g = {};
    g.resize(post_order.size());

    // Calculate the width of each interval
    long interval_width = std::max((long) post_order.size() / d, 1l);
    long lower_bounds[d+1];

    // Vector to store the intervals as pairs
    std::vector<std::pair<int, int>> intervals;

    for (int i = 0; i < d; ++i) {
        lower_bounds[i] = i * interval_width;
    }
    lower_bounds[d] = post_order.size(); // add one "extra" lower bound to make the loop simpler

    for(int i = 0; i < d; ++i) {
        for(int j = lower_bounds[i]; j < lower_bounds[i+1]; ++j) {
            g[post_order[j]->index_] = post_order[lower_bounds[i]];
        }
    }

    return g;
}

template <size_t hash_range>
void compute_label_out(const graph& graph, const std::vector<const node*>& g, const std::function<int(const node*)>& h, const node& n, std::vector<std::bitset<hash_range>>& label_out) {
    label_out[n.index_].set(h(g[n.index_]));
    for(auto const successor : n.outgoing_edges_) {
        if(label_out[successor->index_].none()) { // if successor has not been visited
            compute_label_out<hash_range>(graph, g, h, *successor, label_out);
        }
        label_out[n.index_] |= label_out[successor->index_]; // label_out[n] = label_out[n] union label_out[successor]
    }
}

template <size_t hash_range>
void compute_label_in(const graph& graph, const std::vector<const node*>& g, const std::function<int(const node*)>& h, const node& n, LabelIn<hash_range>& label_in) {
    label_in[n.index_].set(h(g[n.index_]));
    for(auto const predecessor : n.incoming_edges_) {
        if(label_in[predecessor->index_].none()) { // if successor has not been visited
            compute_label_in<hash_range>(graph, g, h, *predecessor, label_in);
        }
        label_in[n.index_] |= label_in[predecessor->index_]; // label_in[n] = label_in[n] union label_in[predecessor]
    }
}

// the hash should map to values in a range from 0...hash_range-1
template <size_t hash_range> // the range is the number of values that can be possible outputs of the hash function
labeled_graph<hash_range> build_labeled_graph(graph& graph, const std::function<int(const node*)>& h, const long d) {
    LabelIn<hash_range> label_in(graph.nodes_.size());
    LabelOut<hash_range> label_out(graph.nodes_.size());

    auto [post_order, label_discovery, label_finish] = depth_first_search(graph);
    auto g = merge_vertices(post_order, d);

    for(auto n : post_order) {
        if(label_out[n->index_].none()) {
            compute_label_in<hash_range>(graph, g, h, *n, label_in);
            compute_label_out<hash_range>(graph, g, h, *n, label_out);
        }
    }

    return labeled_graph<hash_range>(graph, label_discovery, label_finish, label_in, label_out);
}

constexpr size_t hash_range_dense = 160;

inline labeled_graph<hash_range_dense> build_labeled_graph_dense(graph& graph) {
    return build_labeled_graph<160>(graph, [](const node* n) { return n->index_ % hash_range_dense; }, 10*hash_range_dense);
}

constexpr size_t hash_range_sparse = 64;

inline labeled_graph<hash_range_sparse> build_labeled_graph_sparse(graph& graph) {
    return build_labeled_graph<hash_range_sparse>(graph, [](const node* n) { return n->index_ % hash_range_dense; }, hash_range_sparse*hash_range_dense);
}

template <size_t hash_range>
bool query_reachability(const labeled_graph<hash_range>& graph, const node& u, const node& v) {
    std::vector<bool> visited(graph.graph_.nodes_.size());
    return query_reachability(graph, u, v, visited);
}

template <size_t hash_range>
bool query_reachability(const labeled_graph<hash_range>& graph, const node& u, const node& v, std::vector<bool>& visited) {
    visited[u.index_] = true;

    if(graph.label_discovery_[u.index_] <= graph.label_discovery_[v.index_] && graph.label_finish_[v.index_] <= graph.label_finish_[u.index_]) {
        std::cout << "reachability confirmed by label_discovery and label_finish" << std::endl;
        return true;
    }
    // if L_out(v) !subset_of L_out(u) or L_in(u) !subset_of L_in(v)
    if((graph.label_out_[v.index_] & graph.label_out_[u.index_]) != graph.label_out_[v.index_]
        || (graph.label_in_[u.index_] & graph.label_in_[v.index_]) != graph.label_in_[u.index_]) {
        std::cout << "reachability denied by label_in and label_out" << std::endl;
        return false;
    }
    for(auto const w : u.outgoing_edges_) {
        if(visited[w->index_]) continue;

        if(query_reachability<hash_range>(graph, *w, v, visited)) {
            std::cout << "reachability confirmed by a (possibly) early stopped DFS" << std::endl;
            return true;
        }
    }
    std::cout << "reachability denied by a (possibly) early stopped DFS" << std::endl;
    return false;
}