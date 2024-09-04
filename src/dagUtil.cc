#include "dagUtil.h"

#include <algorithm>
#include <chrono>
#include <unordered_map>
#include <iostream>

/**
 * sorts the given dag in topological order and sets the index_ of each node to its position in the topological order
 * also sorts each nodes outgoing_edges in topological order and each nodes incoming_edges in reverse topological order
 * @param dag the directed acyclic graph
 */
void set_to_topological_order(graph& dag) {
    auto start = std::chrono::high_resolution_clock::now();

    // the algorithm used for creating a topological order of nodes is Kahn's Algorithm
    std::vector<node*> topological_order = {};
    std::vector<node*> nodes_without_incoming_edge = {};
    std::unordered_map<node*, long> num_of_visited_edges_for_node = {}; // this map keeps track of the number of visited edges by Kahn's Algorithm for each node
    long long visited_edges_total = 0; // this variable keeps track of the total number of visited edges

    // Look for all nodes that have no incoming edges and store them in nodes_without_incoming_edge
    for(auto const node : dag.nodes_) {
        if(node->incoming_edges_.empty()) {
            nodes_without_incoming_edge.push_back(node);
        }
    }
    // Kahn's Algorithm
    while(!nodes_without_incoming_edge.empty()) {
        // get the last node n from the nodes without incoming edge
        node* n = nodes_without_incoming_edge.back();
        nodes_without_incoming_edge.pop_back();
        // add node n to topological order
        n->index_ = topological_order.size();
        topological_order.push_back(n);

        // loop through each edge e of each node m that has an incoming edge from n to m
        for(auto const m : n->outgoing_edges_) {

            // increment num_of_visited_edges_for_node for node m
            num_of_visited_edges_for_node[m] = num_of_visited_edges_for_node[m] + 1;
            visited_edges_total += 1;

            // check if node m has no more incoming edges and if so add it to the topological order
            if(num_of_visited_edges_for_node[m] == m->incoming_edges_.size()) {
                nodes_without_incoming_edge.push_back(m);
            }
        }
    }

    // check if input graph is a dag
    if(visited_edges_total != dag.number_of_edges_) {
        throw std::invalid_argument( "the input graph is not a dag" );
    } else {
        dag.nodes_ = topological_order;
    }

    auto finish_topo_sort = std::chrono::high_resolution_clock::now();
    auto duration = duration_cast<std::chrono::microseconds>( finish_topo_sort - start);
    std::cout << "topological sort finished. duration: " << duration.count() << "microseconds\n";

    // sort outgoing and incoming edges
    for(auto n : dag.nodes_) {
        std::sort(n->outgoing_edges_.begin(), n->outgoing_edges_.end(), [](const node* a, const node* b) { return a->index_ < b->index_; });
        std::sort(n->incoming_edges_.begin(), n->incoming_edges_.end(), [](const node* a, const node* b) { return a->index_ > b->index_; });
    }

    auto finish_edges_sort = std::chrono::high_resolution_clock::now();
    duration = duration_cast<std::chrono::microseconds>( finish_edges_sort - finish_topo_sort);
    std::cout << "topological sorting of nodes edges finished. duration: " << duration.count() << "microseconds\n";
}

bool graph_is_in_topological_order(graph const& graph) {
    for(long i = 0; i < graph.nodes_.size(); ++i) {
        if(graph.nodes_[i]->index_ != i) return false;
        for(auto const n : graph.nodes_[i]->incoming_edges_) {
            if(n->index_ >= i) return false;
        }
        for(auto const n : graph.nodes_[i]->outgoing_edges_) {
            if(n->index_ <= i) return false;
        }
    }
    return true;
}

bool all_nodes_edges_are_in_topological_order(graph const& graph) {
    for(auto const n : graph.nodes_) {
        for(auto i = 0; i < static_cast<long>(n->outgoing_edges_.size()) - 1; ++i) {
            if(n->outgoing_edges_[i]->index_ >= n->outgoing_edges_[i+1]->index_) return false;
        }
        for(auto i = 0; i < static_cast<long>(n->incoming_edges_.size()) - 1; ++i) {
            if(n->incoming_edges_[i]->index_ <= n->incoming_edges_[i+1]->index_) return false;
        }
    }
    return true;
}