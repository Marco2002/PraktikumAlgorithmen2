#include "gtest/gtest.h"

#include "graphs.h"
#include "TR-B.h"
#include "TR-O.h"
#include "TR-O-PLUS.h"

#include "dagGenerator.h"
#include "dagUtil.h"

/**
* @brief Generates the graph from the example in the paper "One Edge at a Time: Novel Approach Towards Efficient Transitive Reduction Computation on DAGs"
*/
graph generate_example_graph_tr_test() {
    graph g = {};
    for(int i = 0; i < 15; ++i) {
        g.nodes_.emplace_back(i);
    }

    g.add_edge(0, 1);
    g.add_edge(0, 4);
    g.add_edge(0, 5);
    g.add_edge(0, 6);
    g.add_edge(0, 8);
    g.add_edge(1, 2);
    g.add_edge(1, 3);
    g.add_edge(1, 9);
    g.add_edge(2, 9);
    g.add_edge(2, 14);
    g.add_edge(3, 9);
    g.add_edge(3, 10);
    g.add_edge(4, 5);
    g.add_edge(4, 13);
    g.add_edge(5, 9);
    g.add_edge(6, 7);
    g.add_edge(7, 8);
    g.add_edge(7, 13);
    g.add_edge(8, 9);
    g.add_edge(8, 12);
    g.add_edge(9, 10);
    g.add_edge(9, 11);
    g.add_edge(9, 12);
    g.add_edge(9, 13);
    g.add_edge(10, 14);
    g.add_edge(11, 14);
    g.add_edge(12, 14);
    g.add_edge(13, 14);

    return g;
}

bool has_edge(graph& g, int from, int to) {
    return std::find(g.nodes_[from].outgoing_edges_.begin(), g.nodes_[from].outgoing_edges_.end(), &g.nodes_[to]) != g.nodes_[from].outgoing_edges_.end();
}

void graph_is_correct_transitive_reduction_on_example(graph& g) {
    ASSERT_TRUE(has_edge(g, 0, 1));
    ASSERT_TRUE(has_edge(g, 0, 4));
    ASSERT_FALSE(has_edge(g, 0, 5));
    ASSERT_TRUE(has_edge(g, 0, 6));
    ASSERT_FALSE(has_edge(g, 0, 8));
    ASSERT_TRUE(has_edge(g, 1, 2) && has_edge(g, 1, 3) && !has_edge(g, 1, 9));
    ASSERT_TRUE(has_edge(g, 2, 9) && !has_edge(g, 2, 14));
    ASSERT_TRUE(has_edge(g, 3, 9) && !has_edge(g, 3, 10));
    ASSERT_TRUE(has_edge(g, 4, 5) && !has_edge(g, 4, 13));
    ASSERT_TRUE(has_edge(g, 5, 9));
    ASSERT_TRUE(has_edge(g, 6, 7));
    ASSERT_TRUE(has_edge(g, 7, 8) && !has_edge(g, 7, 13));
    ASSERT_TRUE(has_edge(g, 8, 9) && !has_edge(g, 8, 12));
    ASSERT_TRUE(has_edge(g, 9, 10) && has_edge(g, 9, 11) && has_edge(g, 9, 12) && has_edge(g, 9, 13));
    ASSERT_TRUE(has_edge(g, 10, 14));
    ASSERT_TRUE(has_edge(g, 11, 14));
    ASSERT_TRUE(has_edge(g, 12, 14));
    ASSERT_TRUE(has_edge(g, 13, 14));
}

TEST(TRB, correctlyBuildTransitiveReductionOnExample) {
    auto g = generate_example_graph_tr_test();
    tr_b_sparse(g);

    for(const auto& n : g.nodes_) {
        std::cout << "Node " << n.id_ << " has ";
        for(const auto e : n.outgoing_edges_) {
            std::cout << e->id_ << ", ";
        }
        std::cout << "outgoing edges" << std::endl;
    }

    graph_is_correct_transitive_reduction_on_example(g);
}


TEST(TRO, correctlyBuildTransitiveReductionOnExample) {
    auto g = generate_example_graph_tr_test();
    tr_o_sparse(g);

    for(const auto& n : g.nodes_) {
        std::cout << "Node " << n.id_ << " has ";
        for(const auto e : n.outgoing_edges_) {
            std::cout << e->id_ << ", ";
        }
        std::cout << "outgoing edges" << std::endl;
    }

    graph_is_correct_transitive_reduction_on_example(g);
}

TEST(TRO_PLUS, correctlyBuildTransitiveReductionOnExample) {
    auto g = generate_example_graph_tr_test();
    tr_o_plus_sparse(g);

    for(const auto& n : g.nodes_) {
        std::cout << "Node " << n.id_ << " has ";
        for(const auto e : n.outgoing_edges_) {
            std::cout << e->id_ << ", ";
        }
        std::cout << "outgoing edges" << std::endl;
    }

    graph_is_correct_transitive_reduction_on_example(g);
}

TEST(TRB, correctlyBuildTransitiveReductionOnLargeGeneratedGraphs) {
    int number_of_nodes = 1000;
    int number_of_edges = 20000;

    set_seed(12092024);
    auto g = generate_graph(number_of_nodes, number_of_edges, true);

    set_seed(12092024);
    auto g2 = generate_graph(number_of_nodes, number_of_edges, true);
    ASSERT_EQ(g, g2);

    tr_b_dense(g);
    set_edges_in_topological_order(g, std::get<0>(get_topological_order(g)));
    build_tr_by_dfs(g2);
    set_edges_in_topological_order(g2, std::get<0>(get_topological_order(g2)));

    ASSERT_EQ(g, g2);
}

TEST(TRO, correctlyBuildTransitiveReductionOnLargeGeneratedGraphs) {
    int number_of_nodes = 1000;
    int number_of_edges = 20000;

    set_seed(12092024);
    auto g = generate_graph(number_of_nodes, number_of_edges, true);

    set_seed(12092024);
    auto g2 = generate_graph(number_of_nodes, number_of_edges, true);
    ASSERT_EQ(g, g2);

    tr_o_dense(g);
    build_tr_by_dfs(g2);
    set_edges_in_topological_order(g2, std::get<0>(get_topological_order(g2)));

    ASSERT_EQ(g, g2);
}

TEST(TRO_PLUS, correctlyBuildTransitiveReductionOnLargeGeneratedGraphs) {
    int number_of_nodes = 1000;
    int number_of_edges = 20000;

    set_seed(12092024);
    auto g = generate_graph(number_of_nodes, number_of_edges, true);

    set_seed(12092024);
    auto g2 = generate_graph(number_of_nodes, number_of_edges, true);
    ASSERT_EQ(g, g2);

    tr_o_plus_dense(g);
    build_tr_by_dfs(g2);
    set_edges_in_topological_order(g2, std::get<0>(get_topological_order(g2)));

    ASSERT_EQ(g, g2);
}
