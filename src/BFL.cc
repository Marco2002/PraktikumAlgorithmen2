#include "BFL.h"

#include <functional>

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

    // Calculate the width of each interval
    long interval_width = post_order.size()-1 / d;
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
void compute_label_out(const graph& graph, std::vector<node*>& g, const std::function<int(node*)>& h, const node& n, std::vector<std::bitset<hash_range>>& label_out) {
    label_out[n.index_].set(h(g[n.index_]));
    for(auto const successor : n.outgoing_edges_) {
        if(label_out[successor->index_].none()) { // if successor has not been visited
            compute_label_out<hash_range>(graph, g, h, *successor, label_out);
        }
        label_out[n.index_] |= label_out[successor->index_]; // label_out[n] = label_out[n] union label_out[successor]
    }
}

template <size_t hash_range>
void compute_label_in(const graph& graph, std::vector<node*>& g, const std::function<int(node*)>& h, const node& n, LabelIn<hash_range>& label_in) {
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
BflLabels<hash_range> build_bfl_index(const graph& graph, const std::function<int(node*)>& h, const long d) {
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

    return std::make_tuple(label_discovery, label_finish, label_in, label_out);
}

template <size_t hash_range>
bool query_reachability(const graph& graph, BflLabels<hash_range> labels, const node& u, const node& v, std::vector<bool>& visited) {
    visited[u.index_] = true;

    auto const [label_discovery, label_finish, label_in, label_out] = labels;
    if(label_discovery[u.index_] <= label_discovery[v.index_] && label_finish[v.index_] <= label_finish[u.index_]) {
        return true;
    }
    if((label_out[v] & label_out[u] != label_out[v]) || (label_in[u] & label_in[v] != label_in[u])) { // if L_out(v) !subset_of L_out(u) or L_in(u) !subset_of L_in(v)
        return false;
    }
    for(auto const w : u.outgoing_edges_) {
        if(visited[w->index_]) continue;

        if(query_reachability<hash_range>(graph, labels, *w, v, visited)) {
            return true;
        }
    }
    return false;
}
