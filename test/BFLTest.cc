#include "gtest/gtest.h"

#include "graphs.h"
#include "BFL.cc"
#include "dagGenerator.h"
#include "dagUtil.h"

#include <stack>
#include <unordered_set>
#include <random>

std::stringstream buffer;

/**
* @brief Generates the graph from the example in the paper "Reachability Querying: Can It Be Even Faster?"
*/
labeled_graph<3> generate_example_graph(graph& g) {
    for(int i = 0; i < 12; ++i) {
        g.nodes_.emplace_back(i);
    }

    g.add_edge(0, 1);
    g.add_edge(0, 2);
    g.add_edge(1, 3);
    g.add_edge(1, 4);
    g.add_edge(1, 5);
    g.add_edge(2, 3);
    g.add_edge(3, 6);
    g.add_edge(4, 6);
    g.add_edge(7, 2);
    g.add_edge(7, 8);
    g.add_edge(8, 9);
    g.add_edge(8, 10);
    g.add_edge(8, 11);
    g.add_edge(9, 5);
    g.add_edge(10, 11);

    auto h = [](const node* n) {
      switch(n->id_) {
        case 1:
        case 2:
            return 0;
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
            return 1;
        case 0:
        case 9:
        case 10:
        case 11:
            return 2;
        default:
            throw std::invalid_argument("node index out of bounds");
      }
     };

    return build_labeled_graph<3>(g, h, 12);
}

TEST(BFL, labelsAreCorrectlyBuilt) {
    graph g = graph();
    auto g_labeled = generate_example_graph(g);

    // check that discovery labels are correct
    ASSERT_EQ(g_labeled.label_discovery_[0], 1);
    ASSERT_EQ(g_labeled.label_discovery_[1], 2);
    ASSERT_EQ(g_labeled.label_discovery_[2], 12);
    ASSERT_EQ(g_labeled.label_discovery_[3], 3);
    ASSERT_EQ(g_labeled.label_discovery_[4], 7);
    ASSERT_EQ(g_labeled.label_discovery_[5], 9);
    ASSERT_EQ(g_labeled.label_discovery_[6], 4);
    ASSERT_EQ(g_labeled.label_discovery_[7], 15);
    ASSERT_EQ(g_labeled.label_discovery_[8], 16);
    ASSERT_EQ(g_labeled.label_discovery_[9], 17);
    ASSERT_EQ(g_labeled.label_discovery_[10], 19);
    ASSERT_EQ(g_labeled.label_discovery_[11], 20);
    // check that finish labels are correct
    ASSERT_EQ(g_labeled.label_finish_[0], 14);
    ASSERT_EQ(g_labeled.label_finish_[1], 11);
    ASSERT_EQ(g_labeled.label_finish_[2], 13);
    ASSERT_EQ(g_labeled.label_finish_[3], 6);
    ASSERT_EQ(g_labeled.label_finish_[4], 8);
    ASSERT_EQ(g_labeled.label_finish_[5], 10);
    ASSERT_EQ(g_labeled.label_finish_[6], 5);
    ASSERT_EQ(g_labeled.label_finish_[7], 24);
    ASSERT_EQ(g_labeled.label_finish_[8], 23);
    ASSERT_EQ(g_labeled.label_finish_[9], 18);
    ASSERT_EQ(g_labeled.label_finish_[10], 22);
    ASSERT_EQ(g_labeled.label_finish_[11], 21);
    // check that in labels are correct
    // the bitset represents the inclusion of the numbers {2, 1, 0} in the label
    // so bitset 100 means that the label only contains the number 2
    ASSERT_EQ(g_labeled.label_in_[0], std::bitset<3>("100"));
    ASSERT_EQ(g_labeled.label_in_[1], std::bitset<3>("101"));
    ASSERT_EQ(g_labeled.label_in_[2], std::bitset<3>("111"));
    ASSERT_EQ(g_labeled.label_in_[3], std::bitset<3>("111"));
    ASSERT_EQ(g_labeled.label_in_[4], std::bitset<3>("111"));
    ASSERT_EQ(g_labeled.label_in_[5], std::bitset<3>("111"));
    ASSERT_EQ(g_labeled.label_in_[6], std::bitset<3>("111"));
    ASSERT_EQ(g_labeled.label_in_[7], std::bitset<3>("010"));
    ASSERT_EQ(g_labeled.label_in_[8], std::bitset<3>("010"));
    ASSERT_EQ(g_labeled.label_in_[9], std::bitset<3>("110"));
    ASSERT_EQ(g_labeled.label_in_[10], std::bitset<3>("110"));
    ASSERT_EQ(g_labeled.label_in_[11], std::bitset<3>("110"));
    // check that out labels are correct
    ASSERT_EQ(g_labeled.label_out_[0], std::bitset<3>("111"));
    ASSERT_EQ(g_labeled.label_out_[1], std::bitset<3>("011"));
    ASSERT_EQ(g_labeled.label_out_[2], std::bitset<3>("011"));
    ASSERT_EQ(g_labeled.label_out_[3], std::bitset<3>("010"));
    ASSERT_EQ(g_labeled.label_out_[4], std::bitset<3>("010"));
    ASSERT_EQ(g_labeled.label_out_[5], std::bitset<3>("010"));
    ASSERT_EQ(g_labeled.label_out_[6], std::bitset<3>("010"));
    ASSERT_EQ(g_labeled.label_out_[7], std::bitset<3>("111"));
    ASSERT_EQ(g_labeled.label_out_[8], std::bitset<3>("110"));
    ASSERT_EQ(g_labeled.label_out_[9], std::bitset<3>("110"));
    ASSERT_EQ(g_labeled.label_out_[10], std::bitset<3>("100"));
    ASSERT_EQ(g_labeled.label_out_[11], std::bitset<3>("100"));
}

void assert_query(labeled_graph<3>& g, const int id_from, const int id_to, const bool expected_result, const std::string& expected_output) {
    std::streambuf* oldCout = std::cout.rdbuf(buffer.rdbuf());
    ASSERT_EQ(query_reachability(g, g.graph_.nodes_[id_from], g.graph_.nodes_[id_to]), expected_result);
    std::cout.rdbuf(oldCout);
    auto output = buffer.str();
    size_t last_newline_pos = output.rfind('\n', output.size() - 2);

    // If a newline is found, return the substring from the position after the newline to the end
    auto last_line = last_newline_pos != std::string::npos ? output.substr(last_newline_pos + 1) : output;

    EXPECT_EQ(last_line, expected_output);
}

TEST(BFL, reachabilityQueringIsCorrect) {
    // there are 4 cases of how a reachability query can be answered:
    // 1. reachability confirmed by label_discovery and label_finish
    // 2. reachability denied by label_in and label_out
    // 3. reachability confirmed by a (possibly) early stopped DFS
    // 4. reachability denied by a (possibly) early stopped DFS
    auto g = graph();
    auto g_labeled = generate_example_graph(g);

    // 3 manually selected tests for each case
    assert_query(g_labeled, 0, 6, true, "reachability confirmed by label_discovery and label_finish\n");
    assert_query(g_labeled, 8, 10, true, "reachability confirmed by label_discovery and label_finish\n");
    assert_query(g_labeled, 0, 4, true, "reachability confirmed by label_discovery and label_finish\n");

    assert_query(g_labeled, 9, 2, false, "reachability denied by label_in and label_out\n");
    assert_query(g_labeled, 2, 11, false, "reachability denied by label_in and label_out\n");
    assert_query(g_labeled, 3, 10, false, "reachability denied by label_in and label_out\n");

    assert_query(g_labeled, 7, 3, true, "reachability confirmed by a (possibly) early stopped DFS\n");
    assert_query(g_labeled, 8, 5, true, "reachability confirmed by a (possibly) early stopped DFS\n");
    assert_query(g_labeled, 2, 3, true, "reachability confirmed by a (possibly) early stopped DFS\n");

    assert_query(g_labeled, 0, 11, false, "reachability denied by a (possibly) early stopped DFS\n");
    assert_query(g_labeled, 8, 3, false, "reachability denied by a (possibly) early stopped DFS\n");
    assert_query(g_labeled, 9, 3, false, "reachability denied by a (possibly) early stopped DFS\n");
    //ASSERT_TRUE(query_reachability(g, *g.graph_.nodes_[0], *g.graph_.nodes_[5]));
}

bool query_reachability_DFS(const node& u, const node& v) {
    std::unordered_set<const node*> visited;
    std::stack<const node*> to_visit;

    to_visit.push(&u);

    while (!to_visit.empty()) {
        const node* current = to_visit.top();
        to_visit.pop();

        if (current == &v) return true;
        if (visited.find(current) != visited.end()) continue;

        visited.insert(current);
        for (const node* neighbor : current->outgoing_edges_) {
            if (visited.find(neighbor) == visited.end()) {
                to_visit.push(neighbor);
            }
        }
    }
    return false;
}

TEST(BFL, queringWorksOnLargeGeneratedGraphs) {
    constexpr int num_of_nodes = 5000;
    constexpr int num_of_edges = 20000;
    constexpr int num_of_queries = 1000;

    constexpr int hash_range = 160;
    constexpr int d = 1600;

    // seed graph generator randomly
    set_seed(std::chrono::system_clock::now().time_since_epoch().count());
    auto dag = generate_graph(num_of_nodes, num_of_edges, true);
    auto [to, to_reverse] = get_topological_order(dag);

    auto queries = generate_queries(dag, num_of_queries, to_reverse);

    auto h = [](const node* n) { return n->id_ % hash_range; };
    const auto labeled_graph = build_labeled_graph<hash_range>(dag, h, d);

    for(auto [from, to] : queries) {
        ASSERT_EQ(query_reachability(labeled_graph, *from, *to), query_reachability_DFS(*from, *to));
    }
}

TEST(BFL, queringIsCorrectOnLargeGeneratedGraphs) {
    constexpr int num_of_nodes = 5000;
    constexpr int num_of_edges = 20000;
    constexpr int num_of_test_nodes = 20;

    constexpr int hash_range = 160;
    constexpr int d = 1600;

    // seed graph generator randomly
    set_seed(9092024);
    auto dag = generate_graph(num_of_nodes, num_of_edges, true);
    auto h = [](const node* n) { return n->id_ % hash_range; };
    const auto labeled_graph = build_labeled_graph<hash_range>(dag, h, d);

    // Create a random number generator
    std::random_device rd; // Seed for the random number engine
    std::mt19937 gen(rd()); // Mersenne Twister engine
    gen.seed(9092024);

    // Define the distribution to be between 0 and x
    std::uniform_int_distribution<> random_node(0, num_of_nodes - 1);

    for(int i = 0; i < num_of_test_nodes; ++i) {
        auto const test_node = random_node(gen);

        auto const reachable_nodes = find_all_reachable_nodes(labeled_graph.graph_.nodes_[test_node]);

        for(int j = 0; j < num_of_nodes; ++j) {
            auto const& query_node = labeled_graph.graph_.nodes_[j];
            ASSERT_EQ(query_reachability(labeled_graph, labeled_graph.graph_.nodes_[test_node], query_node), reachable_nodes.contains(&query_node));
        }
    }

}