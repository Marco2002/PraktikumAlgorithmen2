#include "BFL.h"

void depth_first_search_visit(node& n, label<long>& discover_label, label<long>& finish_label, std::vector<node*>& post_order, long& current) {
    discover_label[&n] = ++current;
    for (auto const e : n.outgoing_edges_) {
        if (discover_label[e] == 0) { // if e was not visited
            depth_first_search_visit(*e, discover_label, finish_label, post_order, current);
        }
    }
    post_order.push_back(&n);
    finish_label[&n] = ++current;

}

void depth_first_search(graph& g) {
    // TODO check if discovery_label and finish_label are necessary
    label<long> discovery_label = {};
    label<long> finish_label = {};
    std::vector<node*> post_order = {};
    long current = 0;

    for(auto n : g.nodes_) {
        if(n->incoming_edges_.empty()) {
            depth_first_search_visit(*n, discovery_label, finish_label, post_order, current);
        }
    }
}

label<node*> merge_vertices(const std::vector<node*>& post_order, const long d) {
    label<node*> g = {};

    // Calculate the width of each interval
    long interval_width = post_order.size()-1 / d;
    long lower_bounds[d+1] = {};

    // Vector to store the intervals as pairs
    std::vector<std::pair<int, int>> intervals;

    for (int i = 0; i < d; ++i) {
        lower_bounds[i] = i * interval_width;
    }
    lower_bounds[d] = post_order.size(); // add one "extra" lower bound to make the loop simpler

    for(int i = 0; i < d; ++i) {
        for(int j = lower_bounds[i]; j < lower_bounds[i+1]; ++j) {
            g[post_order[j]] = post_order[lower_bounds[i]];
        }
    }

    return g;
}