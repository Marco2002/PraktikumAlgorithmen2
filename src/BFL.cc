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

void merge_vertecies(graph& g) {
     
}