#include "graphs.h"
#include "BFL.h"

// Algorithm 3 TR-O-Plus
template <size_t hash_range>
void tr_o_plus(graph& graph);

void tr_o_plus_dense(graph& graph);
void tr_o_plus_sparse(graph& graph);