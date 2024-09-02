#pragma once
#include <vector>

namespace graphs {

struct node {
	std::vector<node*> outgoing_edges_;
	std::vector<node*> incoming_edges_;
    long index_; // current index_ in the vector of nodes of the graph

    node() : index_(0) {}
    node(const long index) : index_(index) {}
};

struct graph {
    std::vector<node*> nodes_;
    long long number_of_edges_;

    void add_edge(node& from, node& to) {
        from.outgoing_edges_.push_back(&to);
        to.incoming_edges_.push_back(&from);
        number_of_edges_ += 1;
    }

    void add_edge(const long from, const long to) {
        nodes_[from]->outgoing_edges_.push_back(nodes_[to]);
        nodes_[to]->incoming_edges_.push_back(nodes_[from]);
        number_of_edges_ += 1;
    }
};

using Edge = std::tuple<node*, node*>;
} // namespace dag - contains struct for nodes and edges