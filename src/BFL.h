#include "graphs.h"

#include <unordered_map>

using namespace graphs;

// Define the template type alias
template<typename T>
using label = std::unordered_map<node*, T>;
