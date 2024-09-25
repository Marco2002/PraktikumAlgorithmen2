#include "dagUtil.h"

#include <chrono>
#include <iostream>
#include <stack>

/**
 * sorts the given dag in topological order and sets the id_ of each node to its position in the topological order
 * also sorts each nodes outgoing_edges in topological order and each nodes incoming_edges in reverse topological order
 * @param dag the directed acyclic graph
 */
NodeOrder get_topological_order(graph& dag) {
    // the algorithm used for creating a topological order of nodes is Kahn's Algorithm
    std::vector<long> topological_order(dag.nodes_.size());
    std::vector<long> topological_order_reverse(dag.nodes_.size());
    std::vector<const node*> nodes_without_incoming_edge = {};
    std::vector<long> num_of_visited_edges_for_node(dag.nodes_.size(), 0); // this map keeps track of the number of visited edges by Kahn's Algorithm for each node
    long long visited_edges_total = 0; // this variable keeps track of the total number of visited edges
    long current_index = 0;

    // Look for all nodes that have no incoming edges and store them in nodes_without_incoming_edge
    for(auto const& node : dag.nodes_) {
        if(node.incoming_edges_.empty()) {
            nodes_without_incoming_edge.push_back(&node);
        }
    }
    // Kahn's Algorithm
    while(!nodes_without_incoming_edge.empty()) {
        // get the last node n from the nodes without incoming edge
        const node* n = nodes_without_incoming_edge.back();
        nodes_without_incoming_edge.pop_back();

        // set the index of the current node
        topological_order[n->id_] = current_index;
        topological_order_reverse[current_index++] = n->id_;

        // loop through each edge e of each node m that has an incoming edge from n to m
        for(auto const m : n->outgoing_edges_) {

            // increment num_of_visited_edges_for_node for node m
            num_of_visited_edges_for_node[m->id_] = num_of_visited_edges_for_node[m->id_] + 1;
            visited_edges_total += 1;

            // check if node m has no more incoming edges and if so add it to the topological order
            if(num_of_visited_edges_for_node[m->id_] == m->incoming_edges_.size()) {
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
    for(auto& n : dag.nodes_) {
        std::sort(n.outgoing_edges_.begin(), n.outgoing_edges_.end(), [&to](const node* a, const node* b) { return to[a->id_] < to[b->id_]; });
        std::sort(n.incoming_edges_.begin(), n.incoming_edges_.end(), [&to](const node* a, const node* b) { return to[a->id_] > to[b->id_]; });
    }

}

std::unordered_set<const node*> find_all_reachable_nodes(const node& u, const bool include_root) {
    std::unordered_set<const node*> visited;
    std::stack<const node*> to_visit;

    to_visit.push(&u);

    while (!to_visit.empty()) {
        const node* current = to_visit.top();
        to_visit.pop();

        if (visited.contains(current)) continue;

        visited.insert(current);
        for (const node* neighbor : current->outgoing_edges_) {
            if (!visited.contains(neighbor)) {
                to_visit.push(neighbor);
            }
        }
    }

    if(!include_root) {
        visited.erase(&u);
    }

    return visited;
}

void build_tr_by_dfs(graph& g) {
    for(auto& n : g.nodes_) {
        std::unordered_set<const node*> reachable_nodes;
        std::vector<node*> nodes_to_remove;
        for(const auto m : n.outgoing_edges_) {
            auto reachable_from_m = find_all_reachable_nodes(*m, false);
            reachable_nodes.insert(reachable_from_m.begin(), reachable_from_m.end());
        }

        for(const auto m : n.outgoing_edges_) {
            if(reachable_nodes.contains(m)) {
                nodes_to_remove.push_back(m);
            }
        }

        for(const auto m : nodes_to_remove) {
            g.remove_edge(n, *m);
        }
    }
}