#include "BFL.h"

#include <functional>
#include <bitset>

void depth_first_search_visit(const node& n, std::vector<long>& discover_label, std::vector<long>& finish_label, std::vector<const node*>& post_order, long& current) {
    discover_label[n.index_] = ++current;
    for (auto const e : n.outgoing_edges_) {
        if (discover_label[e->index_] == 0) { // if e was not visited
            depth_first_search_visit(*e, discover_label, finish_label, post_order, current);
        }
    }
    post_order.push_back(&n);
    finish_label[n.index_] = ++current;

}

std::tuple<std::vector<const node*>, std::vector<long>, std::vector<long>> depth_first_search(const graph& g) {
    // TODO check if discovery_label and finish_label are necessary
    std::vector<long> discovery_label(g.nodes_.size());
    std::vector<long> finish_label(g.nodes_.size());
    std::vector<const node*> post_order = {};
    long current = 0;

    for(auto n : g.nodes_) {
        if(n->incoming_edges_.empty()) {
            depth_first_search_visit(*n, discovery_label, finish_label, post_order, current);
        }
    }

    return std::make_tuple(post_order, discovery_label, finish_label);
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
void compute_label_in(const graph& graph, std::vector<node*>& g, const std::function<int(node*)>& h, const node& n, std::vector<std::bitset<hash_range>>& label_in) {
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
void build_bfl_index(const graph& graph, const std::function<int(node*)>& h, const long d) {
    std::vector<std::bitset<hash_range>> label_out(graph.nodes_.size());
    std::vector<std::bitset<hash_range>> label_in(graph.nodes_.size());

    auto [post_order, discovery_label, finish_label] = depth_first_search(graph);
    auto g = merge_vertices(post_order, d);

    for(auto n : post_order) {
        if(label_out[n->index_].none()) {
            compute_label_out<hash_range>(graph, g, h, *n, label_out);
            compute_label_in<hash_range>(graph, g, h, *n, label_in);
        }
    }
}

