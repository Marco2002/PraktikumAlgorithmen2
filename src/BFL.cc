#include "BFL.h"

void depth_first_search_visit(const node& n, LabelDiscovery& label_discover, LabelFinish& label_finish, std::vector<const node*>& post_order, long& current, long& order_index) {
    label_discover[n.id_] = ++current;
    for (auto const e : n.outgoing_edges_) {
        if (label_discover[e->id_] != 0) continue; // if e was visited

        depth_first_search_visit(*e, label_discover, label_finish, post_order, current, order_index);
    }
    post_order[order_index++] = &n;
    label_finish[n.id_] = ++current;

}

std::tuple<std::vector<const node*>, LabelDiscovery, LabelFinish> depth_first_search(const graph& g) {
    LabelDiscovery label_discovery(g.nodes_.size());
    LabelFinish label_finish(g.nodes_.size());
    std::vector<const node*> post_order(g.nodes_.size());
    long current = 0;
    long order_index = 0;

    for(auto n : g.nodes_) {
        if(n->incoming_edges_.empty()) {
            depth_first_search_visit(*n, label_discovery, label_finish, post_order, current, order_index);
        }
    }

    return std::make_tuple(post_order, label_discovery, label_finish);
}

std::vector<const node*> merge_vertices(const std::vector<const node*>& post_order, const long d) {
    auto const num_of_intervals = std::min(d, (long) post_order.size());
    std::vector<const node*> g(post_order.size());

    // Calculate the width of each interval
    long interval_width = std::max((long) post_order.size() / num_of_intervals, 1l);
    long lower_bounds[num_of_intervals+1];

    // Vector to store the intervals as pairs
    std::vector<std::pair<int, int>> intervals;

    for (int i = 0; i < num_of_intervals; ++i) {
        lower_bounds[i] = i * interval_width;
    }
    lower_bounds[num_of_intervals] = post_order.size(); // add one "extra" lower bound to make the loop simpler

    for(int i = 0; i < num_of_intervals; ++i) {
        for(int j = lower_bounds[i]; j < lower_bounds[i+1]; ++j) {
            g[post_order[j]->id_] = post_order[lower_bounds[i]];
        }
    }

    return g;
}