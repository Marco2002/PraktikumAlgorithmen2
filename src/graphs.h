#pragma once
#include <vector>

namespace graphs {

struct node {
	std::vector<node*> outgoing_edges_;
	std::vector<node*> incoming_edges_;
    long id_;

    node() : id_(0) {}
    node(const long index) : id_(index) {}
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
        add_edge(*nodes_[from], *nodes_[to]);
    }

    // TODO check if there is a more efficient way to remove an edge
    void remove_edge(node& from, node& to) {
        from.outgoing_edges_.erase(std::ranges::find(from.outgoing_edges_, &to));
        to.incoming_edges_.erase(std::ranges::find(to.incoming_edges_, &from));
        number_of_edges_ -= 1;
    }

    void remove_edge(const long from, const long to) {
        remove_edge(*nodes_[from], *nodes_[to]);
    }
};

using Edge = std::tuple<node*, node*>;
} // namespace dag - contains struct for nodes and edges