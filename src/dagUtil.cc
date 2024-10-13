#include "dagUtil.h"

#include <random>
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
    std::stack<node const*> nodes_without_incoming_edge = {};
    std::vector<long> num_of_visited_edges_for_node(dag.nodes_.size(), 0); // this map keeps track of the number of visited edges by Kahn's Algorithm for each node
    long long visited_edges_total = 0; // this variable keeps track of the total number of visited edges
    long current_index = 0;

    // Look for all nodes that have no incoming edges and store them in nodes_without_incoming_edge
    for(auto const& node : dag.nodes_) {
        if(node.incoming_edges_.empty()) {
            nodes_without_incoming_edge.push(&node);
        }
    }
    // Kahn's Algorithm
    while(!nodes_without_incoming_edge.empty()) {
        // get the last node n from the nodes without incoming edge
        auto const* n = nodes_without_incoming_edge.top();
        nodes_without_incoming_edge.pop();

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
                nodes_without_incoming_edge.push(m);
            }
        }
    }

    // check if input graph is a dag
    if(visited_edges_total != dag.number_of_edges_) {
        throw std::invalid_argument( "the input graph is not a dag" );
    }

    return std::make_tuple(topological_order, topological_order_reverse);
}

void set_edges_in_topological_order(graph& dag, std::vector<long> const& to) {

    // sort outgoing and incoming edges
    for(auto& n : dag.nodes_) {
        std::sort(n.outgoing_edges_.begin(), n.outgoing_edges_.end(), [&to](node const* a, node const* b) { return to[a->id_] < to[b->id_]; });
        std::sort(n.incoming_edges_.begin(), n.incoming_edges_.end(), [&to](node const* a, node const* b) { return to[a->id_] > to[b->id_]; });
    }

}

std::unordered_set<node const*> find_all_reachable_nodes(node const& u, bool const include_root) {
    std::unordered_set<node const*> visited;
    std::stack<node const*> to_visit;

    to_visit.push(&u);

    // find all reachable nodes via DFS
    while (!to_visit.empty()) {
        auto const* current = to_visit.top();
        to_visit.pop();

        if (visited.contains(current)) continue;

        visited.insert(current);
        for (auto const* neighbor : current->outgoing_edges_) {
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
        std::unordered_set<node const*> reachable_nodes;
        std::vector<node*> nodes_to_remove;
        for(auto const m : n.outgoing_edges_) {
            // find all reachable nodes from m (not including m)
            auto reachable_from_m = find_all_reachable_nodes(*m, false);
            reachable_nodes.insert(reachable_from_m.begin(), reachable_from_m.end());
        }

        for(auto const m : n.outgoing_edges_) {
            // check if m is reachable from another successor node of n and if marks it for removal
            if(reachable_nodes.contains(m)) {
                nodes_to_remove.push_back(m);
            }
        }

        for(auto const m : nodes_to_remove) {
            g.remove_edge(n, *m);
        }
    }
}

// creates a full copy of graph g
graph copy_graph(graph& g) {
    graph new_graph;
    new_graph.nodes_.resize(g.nodes_.size());
    // new_graph.number_of_edges_ = g.number_of_edges_;

    for (auto i = 0; i < g.nodes_.size(); ++i) {
        new_graph.nodes_[i].id_ = g.nodes_[i].id_;
    }

    for (auto i = 0; i < g.nodes_.size(); ++i) {
        for (auto const& m : g.nodes_[i].outgoing_edges_) {
            new_graph.add_edge(i, m->id_);
        }
    }

    return std::move(new_graph);
}

// shuffles the nodes of a graph and each nodes' incoming and outgoing edges, while maintaining the structure of the graph
// this is only used in some test cases to ensure that the nodes and edges aren't in any topological order
void shuffle_graph(graph& g, long const seed) {
    // Create a vector of pointers to nodes for shuffling
    std::vector<node *> node_ptrs(g.nodes_.size());
    for (size_t i = 0; i < g.nodes_.size(); ++i) {
        node_ptrs[i] = &g.nodes_[i];  // Store pointers to each node
    }

    // Shuffle the pointers using a random engine
    std::mt19937 eng(seed); // Seed the generator
    std::shuffle(node_ptrs.begin(), node_ptrs.end(), eng);

    // Create a temporary vector to hold the shuffled nodes
    std::vector<node> shuffled_nodes(g.nodes_.size());
    std::vector<long> new_indexes(g.nodes_.size());

    // Update shuffled_nodes with the shuffled pointers and set their ids
    for (size_t i = 0; i < node_ptrs.size(); ++i) {
        shuffled_nodes[i] = *node_ptrs[i]; // Copy the content of the node
        new_indexes[shuffled_nodes[i].id_] = i;
        shuffled_nodes[i].id_ = i;          // Set the new id to the index
    }

    // Update outgoing and incoming edges to point to the new nodes
    for (size_t i = 0; i < node_ptrs.size(); ++i) {
        auto& current_node = shuffled_nodes[i];
        // Update outgoing edges
        for (auto &outgoing: current_node.outgoing_edges_) {
            outgoing = &shuffled_nodes[new_indexes[outgoing->id_]];
        }
        // Update incoming edges
        for (auto &incoming: current_node.incoming_edges_) {
            incoming = &shuffled_nodes[new_indexes[incoming->id_]];

        }
        // shuffle outgoing edges order
        std::shuffle(current_node.outgoing_edges_.begin(), current_node.outgoing_edges_.end(), eng);
        // shuffle incoming edges order
        std::shuffle(current_node.incoming_edges_.begin(), current_node.incoming_edges_.end(), eng);
    }

    // Replace the original nodes_ with the shuffled nodes
    g.nodes_ = std::move(shuffled_nodes);
}