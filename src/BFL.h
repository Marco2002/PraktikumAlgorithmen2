#include "graphs.h"

#include <bitset>

using namespace graphs;

template <size_t hash_range>
using LabelIn = std::vector<std::bitset<hash_range>>;

template <size_t hash_range>
using LabelOut = std::vector<std::bitset<hash_range>>;

using LabelDiscovery = std::vector<long>;
using LabelFinish = std::vector<long>;

template <size_t hash_range>
using BflLabels = std::tuple<LabelDiscovery, LabelFinish, LabelIn<hash_range>, LabelOut<hash_range>>;