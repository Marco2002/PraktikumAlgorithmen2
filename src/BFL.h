#pragma once
#include "graphs.h"

#include <bitset>
#include <functional>
#include <utility>
#include <iostream>
#include <unordered_set>
#include <queue>

using namespace graphs;

template <size_t hash_range>
using LabelIn = std::vector<std::bitset<hash_range>>;

template <size_t hash_range>
using LabelOut = std::vector<std::bitset<hash_range>>;

using LabelDiscovery = std::vector<long>;
using LabelFinish = std::vector<long>;

template <size_t hash_range>
struct labeled_graph {
    graph& graph_;
    LabelDiscovery label_discovery_;
    LabelFinish label_finish_;
    LabelIn<hash_range> label_in_;
    LabelOut<hash_range> label_out_;

    labeled_graph(graph& graph, LabelDiscovery label_discovery, LabelFinish label_finish, LabelIn<hash_range> label_in, LabelOut<hash_range> label_out)
        : graph_(graph), label_discovery_(std::move(label_discovery)), label_finish_(std::move(label_finish)), label_in_(label_in), label_out_(label_out) {}
};

std::tuple<std::vector<node const*>, LabelDiscovery, LabelFinish> depth_first_search(graph const& g);

std::vector<node const*> merge_vertices(std::vector<node const*> const& post_order, long d);

template <size_t hash_range>
void compute_label_out(graph const& graph, std::vector<node const*> const& g, std::function<long(node const*)> const& h, node const& n, std::vector<std::bitset<hash_range>>& label_out) {
    label_out[n.id_].set(h(g[n.id_]));
    for(auto const successor : n.outgoing_edges_) {
        if(label_out[successor->id_].none()) { // if successor has not been visited
            compute_label_out<hash_range>(graph, g, h, *successor, label_out);
        }
        label_out[n.id_] |= label_out[successor->id_]; // label_out[n] = label_out[n] union label_out[successor]
    }
}

template <size_t hash_range>
void compute_label_in(graph const& graph, std::vector<node const*> const& g, std::function<long(node const*)> const& h, node const& n, LabelIn<hash_range>& label_in) {
    label_in[n.id_].set(h(g[n.id_]));
    for(auto const predecessor : n.incoming_edges_) {
        if(label_in[predecessor->id_].none()) { // if successor has not been visited
            compute_label_in<hash_range>(graph, g, h, *predecessor, label_in);
        }
        label_in[n.id_] |= label_in[predecessor->id_]; // label_in[n] = label_in[n] union label_in[predecessor]
    }
}

// the hash should map to values in a range from 0...hash_range-1
template <size_t hash_range> // the range is the number of values that can be possible outputs of the hash function
labeled_graph<hash_range> build_labeled_graph(graph& graph, std::function<long(node const*)> const& h, long const d) {
    LabelIn<hash_range> label_in(graph.nodes_.size());
    LabelOut<hash_range> label_out(graph.nodes_.size());

    auto [post_order, label_discovery, label_finish] = depth_first_search(graph);

    auto g = merge_vertices(post_order, d);

    for(auto n : post_order) {
        if(label_out[n->id_].none()) {
            compute_label_out<hash_range>(graph, g, h, *n, label_out);
        }
        if(label_in[n->id_].none()) {
            compute_label_in<hash_range>(graph, g, h, *n, label_in);
        }
    }

    return labeled_graph<hash_range>(graph, label_discovery, label_finish, label_in, label_out);
}

template <size_t hash_range>
bool query_reachability(const labeled_graph<hash_range>& graph, const node& u, const node& v) {
    std::vector<bool> visited(graph.graph_.nodes_.size(), false);
    return query_reachability<hash_range>(graph, u, v, visited);
}

template <size_t hash_range>
bool query_reachability(labeled_graph<hash_range> const& graph, node const& u, node const& v, std::vector<bool>& visited) {
    // ReachabilityLogger::getInstance().increment_with_dfs();
    visited[u.id_] = true;

    if(graph.label_discovery_[u.id_] <= graph.label_discovery_[v.id_] && graph.label_finish_[v.id_] <= graph.label_finish_[u.id_]) {
        // std::cout << "reachability confirmed by label_discovery and label_finish" << std::endl;
        return true;
    }
    // if L_out(v) !subset_of L_out(u) or L_in(u) !subset_of L_in(v)
    if((graph.label_out_[v.id_] & graph.label_out_[u.id_]) != graph.label_out_[v.id_]
        || (graph.label_in_[u.id_] & graph.label_in_[v.id_]) != graph.label_in_[u.id_]) {
        // std::cout << "reachability denied by label_in and label_out" << std::endl;
        return false;
    }
    for (auto const w : u.outgoing_edges_) {
        if (visited[w->id_]) continue;

        if (query_reachability<hash_range>(graph, *w, v, visited)) {
            // std::cout << "reachability confirmed by a (possibly) early stopped DFS" << std::endl;
            return true;
        }
    }
    // std::cout << "reachability denied by a (possibly) early stopped DFS" << std::endl;
    return false;
}