#include "gtest/gtest.h"

#include "dagUtil.h"
#include "dagGenerator.h"

TEST(dagUtil, topologicalOrderFinderThrowsNoErrorIfInputIsADag) {
    int num_of_nodes = 1000;
    int num_of_edges = 2000;
    set_seed(07122023);
    graph dag = generate_graph(num_of_nodes, num_of_edges, true);
    EXPECT_NO_THROW(get_topological_order(dag));
}

TEST(dagUtil, topologicalOrderFinderThrowsErrorIfInputIsNotADag) {
    int num_of_nodes = 1000;
    int num_of_edges = 2000;
    set_seed(07122023);
    graph non_dag = generate_graph(num_of_nodes, num_of_edges, false);
    EXPECT_THROW(get_topological_order(non_dag), std::invalid_argument);
}

bool graph_is_in_topological_order(graph const& graph, std::vector<long> const& to) {
    for(auto const& n : graph.nodes_) {
        for(auto const e : n.outgoing_edges_) {
            if(to[n.id_] >= to[e->id_]) return false;
        }
        for(auto const e : n.incoming_edges_) {
            if(to[n.id_] <= to[e->id_]) return false;
        }
    }
    return true;
}

TEST(dagUtil, findsValidTopologicalOrder) {
    int num_of_nodes = 1000;
    int num_of_edges = 3000;
    set_seed(21012024);
    graph dag = generate_graph(num_of_nodes, num_of_edges, true, true);

    auto fake_order = std::vector<long>(num_of_nodes);
    for(int i = 0; i<num_of_nodes; i++) {
        fake_order[i] = i;
    }
    ASSERT_FALSE(graph_is_in_topological_order(dag, fake_order));

    auto [to, to_reverse] = get_topological_order(dag);
    ASSERT_TRUE(graph_is_in_topological_order(dag, to));
}

bool all_nodes_edges_are_in_topological_order(graph const& graph, std::vector<long> const& to) {
    for(auto const& n : graph.nodes_) {
        for(auto i = 0; i < static_cast<long>(n.outgoing_edges_.size()) - 1; ++i) {
            if(to[n.outgoing_edges_[i]->id_] >= to[n.outgoing_edges_[i+1]->id_]) return false;
        }
        for(auto i = 0; i < static_cast<long>(n.incoming_edges_.size()) - 1; ++i) {
            if(to[n.incoming_edges_[i]->id_] <= to[n.incoming_edges_[i+1]->id_]) return false;
        }
    }
    return true;
}

TEST(dagUtl, sortsNodesOutgoingAndIncomingEdges) {
    int num_of_nodes = 100000;
    int num_of_edges = 300000;
    set_seed(4092024);
    graph dag = generate_graph(num_of_nodes, num_of_edges, true);
    auto [to, to_reverse] = get_topological_order(dag);
    ASSERT_FALSE(all_nodes_edges_are_in_topological_order(dag, to));
    set_edges_in_topological_order(dag, to);
    ASSERT_TRUE(all_nodes_edges_are_in_topological_order(dag, to));
}