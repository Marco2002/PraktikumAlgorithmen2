#include "insertionAlgorithm.h"

#include <algorithm>
#include <iostream>

// insertion algorithm based on PK2
void insert_edges(graph& dag, std::vector<Edge> const& edges) {
    // setup
    std::vector<node*>* inv_order = &dag.nodes_;
    std::vector<bool> vacant(dag.nodes_.size(), false); // stores the vacant state of all nodes by node index_
    std::vector<Edge> invalidating_edges;

    // add edges to graph
    for(auto const [x, y] : edges) {
        x->outgoing_edges_.push_back(y);
        y->incoming_edges_.push_back(x);

        if(x->index_ >= y->index_) { // add invalidating edge
            invalidating_edges.emplace_back(x, y);
        }
    }
    dag.number_of_edges_ += edges.size();

    std::cout << "Inserting " << edges.size() << " edges\n";
    std::cout << "of which " << invalidating_edges.size() << " are invalidating\n";

    if(invalidating_edges.empty()) return;

    // calculate new topological order (PK2)

    // sort invalidating edges into descending order by index of tail
    std::sort (invalidating_edges.begin(), invalidating_edges.end(), [](auto n1, auto n2) -> bool {
        return std::get<0>(n1)->index_ > std::get<0>(n2)->index_;
    });
    long lower_bound = dag.nodes_.size();
    long start = 0;

    for(long i = 0; i < invalidating_edges.size(); ++i) {
        auto [x, y] = invalidating_edges[i];
        if(x->index_ < lower_bound && i > 0) {
            std::vector<Edge> edge_region = {invalidating_edges.begin() + start, invalidating_edges.begin() + i};
            auto queue = discover(edge_region, *inv_order, vacant);
            shift(lower_bound, queue, *inv_order, vacant);
            start = i; // set the start of new region
        }
        lower_bound = std::min(y->index_, lower_bound);
    }
    // Process final region
    std::vector<Edge> edge_region = {invalidating_edges.begin() + start, invalidating_edges.end()};
    auto queue = discover(edge_region, *inv_order, vacant);
    shift(lower_bound, queue, *inv_order, vacant);
}

void shift(long starting_index, EdgeQueue& queue, std::vector<node*>& inv_order, std::vector<bool>& vacant) {
    long num_of_removed_nodes = 0;
    long i = starting_index;
    while(!queue.empty()) {
        // move vacant slots up
        node* current_node = inv_order[i];
        if(vacant[current_node->index_]) {
            ++num_of_removed_nodes;
            vacant[current_node->index_] = false;
        } else {
            place_node(current_node, i - num_of_removed_nodes, inv_order);
        }

        // insert all nodes from queue, that can be safely added
        Edge head = queue.top();
        while(!queue.empty() && current_node == std::get<1>(head)) {
            --num_of_removed_nodes;
            place_node(std::get<0>(head), i - num_of_removed_nodes, inv_order);
            queue.pop();
            if(!queue.empty()) head = queue.top();
        }
        ++i;
    }
}

void place_node(node* n, long i, std::vector<node*>& inv_order) {
    n->index_ = i;
    inv_order[i] = n;
}

EdgeQueue discover(std::vector<Edge>& edge_insertions, std::vector<node*>& inv_order, std::vector<bool>& vacant) {
    EdgeQueue queue;

    // sort invalidating edges into descending order by index of tail
    std::sort (edge_insertions.begin(), edge_insertions.end(), [](auto n1, auto n2) -> bool {
        return std::get<0>(n1)->index_ > std::get<0>(n2)->index_;
    });
    for(auto const [x, y] : edge_insertions) {
        if(!vacant[y->index_]) {
            // if the slot at y->index_ is vacant, it means that a frontier pair that passes over this edge
            // has already been found for a head node with a higher index.
            depth_first_search(y, x->index_, queue, inv_order, vacant);
        }
    }
    return queue;
}

void depth_first_search(node* start, long upper_bound, EdgeQueue& queue, std::vector<node*>& inv_order, std::vector<bool>& vacant) {
    // depth first search with stack and while loop
    std::vector<bool> visited(inv_order.size(), false);
    std::stack<node*> stack;
    stack.push(start);
    vacant[start->index_] = true;
    visited[start->index_] = true;

    while(!stack.empty()) {
        node* v = stack.top();
        bool has_unvisited_neighbor = false;

        for (auto const dest : v->outgoing_edges_) {
            if (visited[dest->index_]) {
                throw std::runtime_error("cycle detected");
            }
            if (!vacant[dest->index_] && dest->index_ < upper_bound) {
                // new node found that needs to be rearranged (new frontier pair)
                stack.push(dest);
                vacant[dest->index_] = true;
                visited[dest->index_] = true;
                has_unvisited_neighbor = true;
                break; // Move to the next unvisited neighbor
            }
        }

        if (!has_unvisited_neighbor) {
            stack.pop();
            visited[v->index_] = false;
            queue.emplace(v, inv_order[upper_bound]);
        }
    }
}
