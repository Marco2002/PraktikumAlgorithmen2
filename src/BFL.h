#pragma once
#include "graphs.h"

#include <bitset>
#include <functional>
#include <utility>
#include <iostream>
#include <unordered_set>
#include <stack>

using namespace graphs;

template <size_t hash_range>
using BFLLabel = std::vector<std::tuple<long, long, std::bitset<hash_range>, std::bitset<hash_range>>>;

template <size_t hash_range>
struct labeled_graph {
    graph& graph_;
    BFLLabel<hash_range> bfl_label_;

    labeled_graph(graph& graph)
            : graph_(graph), bfl_label_(graph.nodes_.size()) {}

    labeled_graph(graph& graph, BFLLabel<hash_range> bfl_label)
        : graph_(graph), bfl_label_(bfl_label) {}
};

template <size_t hash_range>
void depth_first_search_visit(BFLLabel<hash_range>& label, const node& n, std::vector<const node*>& post_order, long& current, long& order_index) {
    std::get<0>(label[n.id_]) = ++current; // set label discovery
    for (auto const e : n.outgoing_edges_) {
        if (std::get<0>(label[e->id_])) continue; // if e was visited

        depth_first_search_visit(label, *e, post_order, current, order_index);
    }
    post_order[order_index++] = &n;
    std::get<1>(label[n.id_]) = ++current; // set label finish
}

template <size_t hash_range>
std::vector<const node*> populate_label_in_and_out(labeled_graph<hash_range>& g) {
    std::vector<const node*> post_order(g.graph_.nodes_.size());
    long current = 0;
    long order_index = 0;

    for(const auto& n : g.graph_.nodes_) {
        if(n.incoming_edges_.empty()) {
            depth_first_search_visit(g.bfl_label_, n, post_order, current, order_index);
        }
    }

    if(order_index < g.graph_.nodes_.size() - 1) {
        throw std::invalid_argument( "the input graph is not a dag" );
    }

    return std::move(post_order);
}

std::vector<const node*> merge_vertices(const std::vector<const node*>& post_order, const long d);

template <size_t hash_range>
void compute_label_out(const graph& graph, const std::vector<const node*>& g, const std::function<long(const node*)>& h, const node& n, BFLLabel<hash_range>& label) {
    std::bitset<hash_range> label_out;
    label_out.set(h(g[n.id_]));
    for(auto const successor : n.outgoing_edges_) {
        if(std::get<3>(label[successor->id_]).none()) { // if successor has not been visited
            compute_label_out<hash_range>(graph, g, h, *successor, label);
        }
        label_out |= std::get<3>(label[successor->id_]); // label_out[n] = label_out[n] union label_out[successor]
    }
    std::get<3>(label[n.id_]) = label_out;
}

template <size_t hash_range>
void compute_label_in(const graph& graph, const std::vector<const node*>& g, const std::function<long(const node*)>& h, const node& n, BFLLabel<hash_range>& label) {
    std::bitset<hash_range> label_in;
    label_in.set(h(g[n.id_]));
    for(auto const predecessor : n.incoming_edges_) {
        if(std::get<2>(label[predecessor->id_]).none()) { // if successor has not been visited
            compute_label_in<hash_range>(graph, g, h, *predecessor, label);
        }
        label_in |= std::get<2>(label[predecessor->id_]); // label_in[n] = label_in[n] union label_in[predecessor]
    }
    std::get<2>(label[n.id_]) = label_in;
}

// the hash should map to values in a range from 0...hash_range-1
template <size_t hash_range> // the range is the number of values that can be possible outputs of the hash function
labeled_graph<hash_range> build_labeled_graph(graph& graph, const std::function<long(const node*)>& h, const long d) {
    labeled_graph<hash_range> labeled_graph(graph);

    auto post_order = populate_label_in_and_out(labeled_graph);

    auto g = merge_vertices(post_order, d);

    for(auto n : post_order) {
        if(std::get<3>(labeled_graph.bfl_label_[n->id_]).none()) {
            compute_label_out<hash_range>(graph, g, h, *n, labeled_graph.bfl_label_);
        }
        if(std::get<2>(labeled_graph.bfl_label_[n->id_]).none()) {
            compute_label_in<hash_range>(graph, g, h, *n, labeled_graph.bfl_label_);
        }
    }

    return std::move(labeled_graph);
}

template <size_t hash_range>
bool query_reachability(const labeled_graph<hash_range>& graph, const node& u, const node& v) {
    std::unordered_set<long> visited;
    return query_reachability<hash_range>(graph, u, v, visited);
}

template <size_t hash_range>
bool query_reachability(const labeled_graph<hash_range>& graph, const node& u, const node& v, std::unordered_set<long>& visited) {
    visited.insert(u.id_);

    if(std::get<0>(graph.bfl_label_[u.id_]) <= std::get<0>(graph.bfl_label_[v.id_]) && std::get<1>(graph.bfl_label_[v.id_]) <= std::get<1>(graph.bfl_label_[u.id_])) {
        // std::cout << "reachability confirmed by label_discovery and label_finish" << std::endl;
        return true;
    }
    // if L_out(v) !subset_of L_out(u) or L_in(u) !subset_of L_in(v)
    if((std::get<3>(graph.bfl_label_[v.id_]) & std::get<3>(graph.bfl_label_[u.id_])) != std::get<3>(graph.bfl_label_[v.id_])
        || (std::get<2>(graph.bfl_label_[u.id_]) & std::get<2>(graph.bfl_label_[v.id_])) != std::get<2>(graph.bfl_label_[u.id_])) {
        // std::cout << "reachability denied by label_in and label_out" << std::endl;
        return false;
    }
    for (auto const w : u.outgoing_edges_) {
        if (visited.find(w->id_) != visited.end()) continue;

        if (query_reachability<hash_range>(graph, *w, v, visited)) {
            // std::cout << "reachability confirmed by a (possibly) early stopped DFS" << std::endl;
            return true;
        }
    }
    // std::cout << "reachability denied by a (possibly) early stopped DFS" << std::endl;
    return false;
}