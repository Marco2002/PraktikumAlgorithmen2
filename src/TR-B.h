#include "graphs.h"

using namespace graphs;

// Algorithm 1 TR-B
template <size_t hash_range>
graph tr_b(graph& graph);

graph tr_b_dense(graph& graph);
graph tr_b_sparse(graph& graph);