#include "BFL.h"

#include <stack>

void depth_first_search_visit(const node& start_node, LabelDiscovery& label_discover, LabelFinish& label_finish, std::vector<const node*>& post_order, long& current, long& order_index) {
    // label_discover[n.id_] = ++current;
    // for (auto const e : n.outgoing_edges_) {
    //     if (label_discover[e->id_] != 0) continue; // if e was visited
    //
    //     depth_first_search_visit(*e, label_discover, label_finish, post_order, current, order_index);
    // }
    // post_order[order_index++] = &n;
    // label_finish[n.id_] = ++current;
    std::stack<const node*> node_stack;
    std::stack<size_t> edge_index_stack; // to track edges for each node
    std::unordered_map<long, bool> finished; // track when all neighbors of a node are processed

    node_stack.push(&start_node);
    edge_index_stack.push(0); // start with first edge of the node

    while (!node_stack.empty()) {
        const node* current_node = node_stack.top();
        size_t& edge_index = edge_index_stack.top(); // get reference to current edge index
        long current_id = current_node->id_;

        if (label_discover[current_id] == 0) {
            // This node is being visited for the first time
            label_discover[current_id] = ++current;
        }

        // Process all outgoing edges
        if (edge_index < current_node->outgoing_edges_.size()) {
            const node* neighbor = current_node->outgoing_edges_[edge_index++]; // get current neighbor and increment edge index
            if (label_discover[neighbor->id_] == 0) {
                node_stack.push(neighbor);
                edge_index_stack.push(0); // push a new entry for the neighbor's edge traversal
            }
        } else {
            // All edges of current_node processed, mark it as finished
            if (!finished[current_id]) {
                post_order[order_index++] = current_node;
                label_finish[current_id] = ++current;
                finished[current_id] = true;
            }
            node_stack.pop();
            edge_index_stack.pop();
        }
    }
}

std::tuple<std::vector<const node*>, LabelDiscovery, LabelFinish> depth_first_search(const graph& g) {
    LabelDiscovery label_discovery(g.nodes_.size());
    LabelFinish label_finish(g.nodes_.size());
    std::vector<const node*> post_order(g.nodes_.size());
    long current = 0;
    long order_index = 0;

    for(const auto& n : g.nodes_) {
        if(n.incoming_edges_.empty()) {
            depth_first_search_visit(n, label_discovery, label_finish, post_order, current, order_index);
        }
    }

    if(order_index < g.nodes_.size() - 1) {
        throw std::invalid_argument( "the input graph is not a dag" );
    }

    return std::make_tuple(post_order, label_discovery, label_finish);
}

std::vector<const node*> merge_vertices(const std::vector<const node*>& post_order, const long d) {
    auto const num_of_intervals = std::min(d, static_cast<long>(post_order.size()));
    std::vector<const node*> g(post_order.size());

    // Calculate the width of each interval
    long interval_width = std::max(static_cast<long>(post_order.size()) / num_of_intervals, 1l);
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