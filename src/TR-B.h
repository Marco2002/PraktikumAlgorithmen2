#include "graphs.h"

using namespace graphs;

// Algorithm 1 TR-B
template <size_t hash_range>
void tr_b(graph& graph);

void tr_b_dense(graph& graph);
void tr_b_sparse(graph& graph);