#include "gtest/gtest.h"

#include "dagGenerator.h"
#include "dagUtil.h"

TEST(dagGenerator, nodeGeneratedCorrectly) {
    long num_of_nodes = 1000000;
    long num_of_edges = 0;
    set_seed(30112023);
    graph const dag1 = generate_graph(num_of_nodes, num_of_edges, false);
    ASSERT_EQ(dag1.nodes_.size(), num_of_nodes);
    for(int i = 0; i < num_of_nodes; ++i) {
        ASSERT_EQ(i, dag1.nodes_[i].id_);
    }
}

TEST(dagGenerator, edgesGeneratedCorrectly) {
    long num_of_nodes = 1000000;
    long long num_of_edges = 3000000;
    set_seed(30112023);
    graph graph = generate_graph(num_of_nodes, num_of_edges, false);
    ASSERT_EQ(graph.nodes_.size(), num_of_nodes);
    long long counted_num_of_outgoing_edges = 0;
    long long counted_num_of_incoming_edges = 0;
    for(auto const& n: graph.nodes_) {
        counted_num_of_outgoing_edges += n.outgoing_edges_.size();
        counted_num_of_incoming_edges += n.outgoing_edges_.size();
    }
    ASSERT_EQ(counted_num_of_outgoing_edges, num_of_edges);
    ASSERT_EQ(counted_num_of_incoming_edges, num_of_edges);
    ASSERT_EQ(graph.number_of_edges_, num_of_edges);
}

TEST(dagGenerator, isDeterministicWithSeed) {
    int num_of_nodes = 1000;
    int num_of_edges = 2000;
    set_seed(30112023);
    graph const dag1 = generate_graph(num_of_nodes, num_of_edges, true);
    set_seed(30112023);
    graph const dag2 = generate_graph(num_of_nodes, num_of_edges, true);

    for(int i = 0; i < num_of_nodes; ++i) {
        ASSERT_EQ(dag1.nodes_[i].outgoing_edges_.size(), dag2.nodes_[i].outgoing_edges_.size());
        ASSERT_EQ(dag1.nodes_[i].incoming_edges_.size(), dag2.nodes_[i].incoming_edges_.size());

        for(int j = 0; j < dag1.nodes_[i].outgoing_edges_.size(); ++j) {
            ASSERT_EQ(dag1.nodes_[i].outgoing_edges_[j]->id_, dag2.nodes_[i].outgoing_edges_[j]->id_);
        }
        for(int j = 0; j < dag1.nodes_[i].incoming_edges_.size(); ++j) {
            ASSERT_EQ(dag1.nodes_[i].incoming_edges_[j]->id_, dag2.nodes_[i].incoming_edges_[j]->id_);
        }
    }
}

TEST(dagGenerator, generatesConsitentGraphs) {
    int number_of_nodes = 10000;
    int number_of_edges = 20000;
    set_seed(30112023);
    graph const dag = generate_graph(number_of_nodes, number_of_edges, true);
    graph const non_dag = generate_graph(number_of_nodes, number_of_edges, false);

    ASSERT_EQ(dag.nodes_.size(), number_of_nodes);
    ASSERT_EQ(dag.number_of_edges_, number_of_edges);

    ASSERT_EQ(non_dag.nodes_.size(), number_of_nodes);
    ASSERT_EQ(non_dag.number_of_edges_, number_of_edges);
    // assert that all the node's incoming and outgoing edges lead to nodes that are also part of the graph
    for(auto const& node : dag.nodes_) {
        for(auto const n: node.outgoing_edges_) {
            ASSERT_TRUE(std::find(dag.nodes_.begin(), dag.nodes_.end(), *n) != dag.nodes_.end());
        }
        for(auto const n: node.outgoing_edges_) {
            ASSERT_TRUE(std::find(dag.nodes_.begin(), dag.nodes_.end(), *n) != dag.nodes_.end());
        }
    }

    // repeat for the normal non_dag
    for(auto const& node : non_dag.nodes_) {
        for(auto const n: node.outgoing_edges_) {
            ASSERT_TRUE(std::find(non_dag.nodes_.begin(), non_dag.nodes_.end(), *n) != non_dag.nodes_.end());
        }
        for(auto const n : node.outgoing_edges_) {
            ASSERT_TRUE(std::find(non_dag.nodes_.begin(), non_dag.nodes_.end(), *n) != non_dag.nodes_.end());
        }
    }
}

TEST(dagGenerator, canGenerateDag) {
    int num_of_nodes = 1000;
    int num_of_edges = 2000;
    set_seed(21012024);
    graph dag = generate_graph(num_of_nodes, num_of_edges, true);
    EXPECT_NO_THROW(get_topological_order(dag));
}

TEST(dagGenerator, canGenerateNonDag) {
    int num_of_nodes = 1000;
    int num_of_edges = 2000;
    set_seed(21012024);
    graph non_dag = generate_graph(num_of_nodes, num_of_edges, false);
    EXPECT_THROW(get_topological_order(non_dag), std::invalid_argument);
}

TEST(dagGenerator, worksInEdgeCases) {
    int num_of_nodes = 2; // minimal input values
    int num_of_edges = 0;
    set_seed(21012024);
    graph g = generate_graph(num_of_nodes, num_of_edges, true);
    EXPECT_NO_THROW(get_topological_order(g));

    num_of_nodes = 100;
    num_of_edges = num_of_nodes * (num_of_nodes-1); // full non dag (density = 1)
    set_seed(21012024);
    g = generate_graph(num_of_nodes, num_of_edges, false);
    EXPECT_THROW(get_topological_order(g), std::invalid_argument);

    num_of_nodes = 100;
    num_of_edges = num_of_nodes * (num_of_nodes-1)/2; // full dag (density = 1)
    set_seed(21012024);
    g = generate_graph(num_of_nodes, num_of_edges, true);
    EXPECT_NO_THROW(get_topological_order(g));
}

TEST(dagGenerator, throwsInvalidArgumentWhenArgumentsInvalid) {
    int num_of_nodes = 1; // too few nodes
    int num_of_edges = 0;
    set_seed(21012024);
    EXPECT_THROW(generate_graph(num_of_nodes, num_of_edges, false), std::invalid_argument);

    num_of_nodes = 2;
    num_of_edges = -1; // invalid edge number
    EXPECT_THROW(generate_graph(num_of_nodes, num_of_edges, false), std::invalid_argument);

    num_of_nodes = 100;
    num_of_edges = num_of_nodes * (num_of_nodes-1)/2 +1; // too many edges for dag
    EXPECT_THROW(generate_graph(num_of_nodes, num_of_edges, true), std::invalid_argument);
    graph g = generate_graph(num_of_nodes, num_of_edges, false); // expect no throw for non dag

    num_of_nodes = 100;
    num_of_edges = num_of_nodes * (num_of_nodes-1) +1; // too many edges for non dag
    EXPECT_THROW(generate_graph(num_of_nodes, num_of_edges, false), std::invalid_argument);
}


