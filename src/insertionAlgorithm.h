#include <stack>

#include "graphs.h"

using namespace graphs;

using EdgeQueue = std::stack<Edge>;

void insert_edges(graph&, std::vector<Edge> const&);

void shift(long starting_index, EdgeQueue&, std::vector<node*>& inv_order, std::vector<bool>& vacant);

void place_node(node* n, long i, std::vector<node*>& inv_order);

EdgeQueue discover(std::vector<Edge>& edge_insertions, std::vector<node*>& inv_order, std::vector<bool>& vacant);

void depth_first_search(node* v, long upper_bound, EdgeQueue& queue, std::vector<node*>& inv_order, std::vector<bool>& vacant);