#include "gtest/gtest.h"

#include "dagUtil.h"
#include "dagGenerator.h"


TEST(dagUtil, topologicalOrderFinderThrowsNoError) {
    int num_of_nodes = 1000;
    int num_of_edges = 2000;
    set_seed(07122023);
    graph dag = generate_graph(num_of_nodes, num_of_edges, true);
    EXPECT_NO_THROW(set_to_topological_order(dag));
    //std::vector<node*> topological_order = set_to_topological_order(&dag1);
}

TEST(dagUtil, topologicalOrderFinderThrowsErrorIfInputIsNotADag) {
    int num_of_nodes = 1000;
    int num_of_edges = 2000;
    set_seed(07122023);
    graph non_dag = generate_graph(num_of_nodes, num_of_edges, false);
    EXPECT_THROW(set_to_topological_order(non_dag), std::invalid_argument);
}

TEST(dagUtil, findsValidTopologicalOrder) {
    int num_of_nodes = 1000;
    int num_of_edges = 3000;
    set_seed(21012024);
    graph dag = generate_graph(num_of_nodes, num_of_edges, true);
    ASSERT_TRUE(graph_is_in_topological_order(dag));
    shuffle_graph(dag);
    ASSERT_FALSE(graph_is_in_topological_order(dag));
    set_to_topological_order(dag);
    ASSERT_TRUE(graph_is_in_topological_order(dag));
}
