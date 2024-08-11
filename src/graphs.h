#pragma once
#include <vector>

namespace graphs {
    struct node {
		std::vector<node*> outgoing_edges_;
		std::vector<node*> incoming_edges_;
        long index_; // current index_ in the vector of nodes of the graph
	};

    struct graph {
        std::vector<node*> nodes_;
        long long number_of_edges_;
    };

    using Edge = std::tuple<node*, node*>;
} // namespace dag - contains struct for nodes and edges