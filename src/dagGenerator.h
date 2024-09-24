#include "graphs.h"

using namespace graphs;

void set_seed(int seed);

graph generate_graph(long number_of_nodes, long long number_of_edges, bool should_be_dag, bool should_be_shuffled = false);

std::vector<ConstEdge> generate_extra_edges(graph const&, long long number_of_edges);
