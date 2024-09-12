#include "dagUtil.h"

#include <chrono>
#include <unordered_map>
#include <iostream>

/**
 * sorts the given dag in topological order and sets the id_ of each node to its position in the topological order
 * also sorts each nodes outgoing_edges in topological order and each nodes incoming_edges in reverse topological order
 * @param dag the directed acyclic graph
 */
NodeOrder get_topological_order(graph& dag) {
    // the algorithm used for creating a topological order of nodes is Kahn's Algorithm
    std::vector<long> topological_order(dag.nodes_.size());
    std::vector<long> topological_order_reverse(dag.nodes_.size());
    std::vector<node*> nodes_without_incoming_edge = {};
    std::unordered_map<node*, long> num_of_visited_edges_for_node = {}; // this map keeps track of the number of visited edges by Kahn's Algorithm for each node
    long long visited_edges_total = 0; // this variable keeps track of the total number of visited edges
    long current_index = 0;

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

        // set the index of the current node
        topological_order[n->id_] = current_index;
        topological_order_reverse[current_index++] = n->id_;

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
    }

    return std::make_tuple(topological_order, topological_order_reverse);
}

void set_edges_in_topological_order(graph& dag, const std::vector<long>& to) {

    // sort outgoing and incoming edges
    for(auto n : dag.nodes_) {
        std::sort(n->outgoing_edges_.begin(), n->outgoing_edges_.end(), [&to](const node* a, const node* b) { return to[a->id_] < to[b->id_]; });
        std::sort(n->incoming_edges_.begin(), n->incoming_edges_.end(), [&to](const node* a, const node* b) { return to[a->id_] > to[b->id_]; });
    }

}
