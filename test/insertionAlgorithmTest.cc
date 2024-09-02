#include "gtest/gtest.h"

#include "insertionAlgorithm.h"
#include "dagGenerator.h"
#include "dagUtil.h"

TEST(insertionAlgorithm, ShiftTestHardcoded) {
    // hard coded test case based on graph from Figure 3
    // in "A Batch Algorithm for Maintaining a Topological Order" by David J Pearce
    int starting_index = 0;
    int number_of_nodes = 9;

    std::vector<node> nodes;
    nodes.resize(number_of_nodes);
    for(int i = 0; i < number_of_nodes; ++i) {
        nodes[i] = node(i);
    }
    std::vector<node*> inv_order;
    std::vector<bool> vacant(number_of_nodes, false);
    for(int i = 0; i < number_of_nodes; ++i) {
        inv_order.push_back(&nodes[i]);
    }

    // new edges in queue
    EdgeQueue queue;

    queue.emplace(&nodes[2], &nodes[8]);
    queue.emplace(&nodes[4], &nodes[8]);
    queue.emplace(&nodes[0], &nodes[6]);

    // vacant slots
    vacant[0] = true;
    vacant[2] = true;
    vacant[4] = true;

    shift(starting_index,queue, inv_order, vacant);
    // compare solution with result in paper
    ASSERT_EQ(inv_order[0], &nodes[1]);
    ASSERT_EQ(inv_order[1], &nodes[3]);
    ASSERT_EQ(inv_order[2], &nodes[5]);
    ASSERT_EQ(inv_order[3], &nodes[6]);
    ASSERT_EQ(inv_order[4], &nodes[0]);
    ASSERT_EQ(inv_order[5], &nodes[7]);
    ASSERT_EQ(inv_order[6], &nodes[8]);
    ASSERT_EQ(inv_order[7], &nodes[4]);
    ASSERT_EQ(inv_order[8], &nodes[2]);
}

TEST(insertionAlgorithm, DiscoverTestHardcoded) {
    // hard coded test case based on graph from Affected Region 3 in Figure 2
    // in "A Batch Algorithm for Maintaining a Topological Order" by David J Pearce
    int number_of_nodes = 9;

    std::vector<node> nodes;
    std::vector<Edge> edge_insertions;
    nodes.resize(number_of_nodes);
    for(int i = 0; i < number_of_nodes; ++i) {
        nodes[i] = node(i);
    }
    std::vector<node*> inv_order;
    std::vector<bool> vacant(number_of_nodes, false);
    for(int i = 0; i < number_of_nodes; ++i) {
        inv_order.push_back(&nodes[i]);
    }

    nodes[3].outgoing_edges_.push_back(&nodes[5]);
    nodes[5].incoming_edges_.push_back(&nodes[3]);
    nodes[6].outgoing_edges_.push_back(&nodes[8]);
    nodes[8].incoming_edges_.push_back(&nodes[6]);
    // invalidating edges
    edge_insertions.emplace_back(&nodes[8], &nodes[4]);
    nodes[8].outgoing_edges_.push_back(&nodes[4]);
    nodes[4].incoming_edges_.push_back(&nodes[8]);
    edge_insertions.emplace_back(&nodes[4], &nodes[2]);
    nodes[4].outgoing_edges_.push_back(&nodes[2]);
    nodes[2].incoming_edges_.push_back(&nodes[4]);
    edge_insertions.emplace_back(&nodes[6], &nodes[0]);
    nodes[6].outgoing_edges_.push_back(&nodes[0]);
    nodes[0].incoming_edges_.push_back(&nodes[6]);

    auto queue = discover(edge_insertions, inv_order, vacant);

    for(int i = 0; i<number_of_nodes; ++i) {
        ASSERT_TRUE((i == 0 || i == 2 || i == 4) ? vacant[i] : !vacant[i]); // assert only nodes 0, 2, 4 are vacant
    }

    // compare queue result to expected result from figure
    auto [n1, n2] = queue.top();
    ASSERT_EQ(n1, &nodes[0]);
    ASSERT_EQ(n2, &nodes[6]);
    queue.pop();
    auto [n3, n4] = queue.top();
    ASSERT_EQ(n3, &nodes[4]);
    ASSERT_EQ(n4, &nodes[8]);
    queue.pop();
    auto [n5, n6] = queue.top();
    ASSERT_EQ(n5, &nodes[2]);
    ASSERT_EQ(n6, &nodes[8]);
    queue.pop();
    ASSERT_TRUE(queue.empty());
}

TEST(insertionAlgorithm, InsertEdgeTestHardcoded) {
    // hard coded test case based on Figure 1
    // in "A Batch Algorithm for Maintaining a Topological Order" by David J Pearce
    int number_of_nodes = 7;
    std::vector<node*> nodes;
    nodes.resize(number_of_nodes);
    for(int i = 0; i < number_of_nodes; ++i) {
        nodes[i] = new node(i);
    }
    // replicate edges from example
    nodes[0]->outgoing_edges_.push_back(nodes[2]);
    nodes[2]->incoming_edges_.push_back(nodes[0]);
    nodes[2]->outgoing_edges_.push_back(nodes[4]);
    nodes[4]->incoming_edges_.push_back(nodes[2]);
    nodes[1]->outgoing_edges_.push_back(nodes[4]);
    nodes[4]->incoming_edges_.push_back(nodes[1]);
    nodes[5]->outgoing_edges_.push_back(nodes[6]);
    nodes[6]->incoming_edges_.push_back(nodes[5]);

    graph dag = {
        .nodes_ = nodes,
        .number_of_edges_ = 4,
    };

    // invalidating edge insertion
    std::vector<Edge> new_edges = {};
    new_edges.emplace_back(nodes[6], nodes[0]);

    insert_edges(dag, new_edges);
    ASSERT_EQ(dag.nodes_[0]->outgoing_edges_[0], dag.nodes_[6]);
    ASSERT_EQ(dag.nodes_[6]->incoming_edges_[1], dag.nodes_[0]);
    ASSERT_EQ(dag.nodes_[2]->outgoing_edges_[0], dag.nodes_[3]);
    ASSERT_EQ(dag.nodes_[3]->incoming_edges_[0], dag.nodes_[2]);
    ASSERT_EQ(dag.nodes_[3]->outgoing_edges_[0], dag.nodes_[4]);
    ASSERT_EQ(dag.nodes_[4]->incoming_edges_[0], dag.nodes_[3]);
    ASSERT_EQ(dag.nodes_[4]->outgoing_edges_[0], dag.nodes_[5]);
    ASSERT_EQ(dag.nodes_[5]->incoming_edges_[0], dag.nodes_[4]);
    ASSERT_EQ(dag.nodes_[5]->outgoing_edges_[0], dag.nodes_[6]);
    ASSERT_EQ(dag.nodes_[6]->incoming_edges_[0], dag.nodes_[5]);
    ASSERT_TRUE(graph_is_in_topological_order(dag));
}

TEST(insertionAlgorithm, FullAlgorihm) {
    int num_of_nodes = 10000;
    long long num_of_edges = 10000;
    int num_of_new_edges = 5000; // 517 will be invalidating
    set_seed(21012024);

    graph dag = generate_graph(num_of_nodes, num_of_edges, true);
    // generate new edges (all of them won't invalidate the current topological order)
    std::vector<Edge> new_edges = generate_extra_edges(dag, num_of_new_edges);

    // bring the dag into another topological order by shuffling and reordering
    shuffle_graph(dag);
    set_to_topological_order(dag);
    ASSERT_TRUE(graph_is_in_topological_order(dag));
    // now some of the edges will be invalidating the current topological order, but the graph will remain a dag

    // insert new edges
    insert_edges(dag, new_edges);
    ASSERT_TRUE(graph_is_in_topological_order(dag));
}
