#pragma once
#include <utility>
#include <vector>

namespace graphs {

struct node {
	std::vector<node*> outgoing_edges_;
	std::vector<node*> incoming_edges_;
    long id_;

    node() : id_(0) {}
    node(const long index) : id_(index) {}

    bool operator==(const node& other) const {
        return id_ == other.id_;
    }
};

struct graph {
    std::vector<node> nodes_;
    long long number_of_edges_;

    graph() : number_of_edges_(0) {}
    graph(std::vector<node> nodes, long long number_of_edges_) : nodes_(std::move(nodes)), number_of_edges_(number_of_edges_) {}

    void add_edge(node& from, node& to) {
        from.outgoing_edges_.push_back(&to);
        to.incoming_edges_.push_back(&from);
        number_of_edges_ += 1;
    }

    void add_edge(const long from, const long to) {
        add_edge(nodes_[from], nodes_[to]);
    }

    // TODO check if there is a more efficient way to remove an edge
    void remove_edge(node& from, node& to) {
        from.outgoing_edges_.erase(std::ranges::find(from.outgoing_edges_, &to));
        to.incoming_edges_.erase(std::ranges::find(to.incoming_edges_, &from));
        number_of_edges_ -= 1;
    }

    void remove_edge(const long from, const long to) {
        remove_edge(nodes_[from], nodes_[to]);
    }

    bool operator==(const graph& other) const {
        if (nodes_.size() != other.nodes_.size()) return false;
        if (number_of_edges_ != other.number_of_edges_) return false;

        for (long i = 0; i < nodes_.size(); i++) {
            if(nodes_[i].id_ != other.nodes_[i].id_) return false;
            if (nodes_[i].outgoing_edges_.size() != other.nodes_[i].outgoing_edges_.size()) return false;
            if (nodes_[i].incoming_edges_.size() != other.nodes_[i].incoming_edges_.size()) return false;

            for (long j = 0; j < nodes_[i].outgoing_edges_.size(); j++) {
                if (nodes_[i].outgoing_edges_[j]->id_ != other.nodes_[i].outgoing_edges_[j]->id_) return false;
            }

            for (long j = 0; j < nodes_[i].incoming_edges_.size(); j++) {
                if (nodes_[i].incoming_edges_[j]->id_ != other.nodes_[i].incoming_edges_[j]->id_) return false;
            }
        }

        return true;
    }
};

using Edge = std::tuple<node*, node*>;
using ConstEdge = std::tuple<const node*, const node*>;

struct EdgeHash {
    std::size_t operator()(const std::tuple<node*, node*>& edge) const {
        // Use std::hash for pointers and combine the results
        auto hash1 = std::hash<node*>{}(std::get<0>(edge));
        auto hash2 = std::hash<node*>{}(std::get<1>(edge));
        return hash1 ^ (hash2 << 1);  // Combine the two hashes
    }
};
} // namespace dag - contains struct for nodes and edges