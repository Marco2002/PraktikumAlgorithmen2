#include "gtest/gtest.h"

#include <fstream>
#include <filesystem>

#include "TR-B.h"
#include "TR-O.h"
#include "TR-O-PLUS.h"
#include "dagUtil.h"
#include "dagGenerator.h"
#include "MurmurHash3.h"

std::chrono::microseconds evaluate(graph& graph, void (*algorithm)(graphs::graph&), std::string const& algorithm_name) {
    auto g = copy_graph(graph);
    auto const start = std::chrono::high_resolution_clock::now();
    algorithm(g);
    auto const stop = std::chrono::high_resolution_clock::now();
    auto const duration = duration_cast<std::chrono::microseconds>(stop - start);
    std::cout << "calculated transient reduction " << algorithm_name <<". TIME: " << duration.count() << "microseconds\n";
    return duration;
}

graph read_gra_file(std::string const& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) throw std::runtime_error("Unable to open the .gra file.");

    graph g;
    std::string line;

    // First line is the name (ignored)
    std::getline(file, line);

    // Second line is the number of nodes
    std::getline(file, line);
    long num_nodes = std::stol(line);

    // Initialize the graph nodes
    g.nodes_.reserve(num_nodes);
    for (long i = 0; i < num_nodes; ++i) {
        g.nodes_.emplace_back(i); // Create nodes with id
    }

    std::vector<std::unordered_set<long>> handled_edges_for_node(num_nodes);

    // Read each node's edges
    while (std::getline(file, line)) {
        if (line.empty()) continue;

        std::istringstream iss(line);
        long node_id;
        char colon;
        iss >> node_id >> colon;

        if (node_id >= num_nodes) throw std::runtime_error("Node ID exceeds number of nodes.");

        std::set<long> seen_neighbors; // To track added edges for this node

        long neighbor_id;
        while (iss >> neighbor_id) {
            // read next word
            if (neighbor_id == '#')  break;
            if (neighbor_id >= num_nodes) throw std::runtime_error("Neighbor ID exceeds number of nodes.");

            // Ignore self-loops
            if (node_id == neighbor_id) continue;

            // Check if the edge has already been added
            if (seen_neighbors.find(neighbor_id) != seen_neighbors.end()) continue;

            // Add edge from node_id to neighbor_id
            g.add_edge(node_id, neighbor_id);
            seen_neighbors.insert(neighbor_id); // Mark the edge as added
        }
    }
    return g;
}

graph read_txt_graph(std::string const& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) throw std::runtime_error("Unable to open the .txt file.");

    graph g;
    std::string line;
    long num_nodes = 0;
    std::map<long, long> node_id_map;
    long current_id = 0;

    // Skip the first two comment lines
    std::getline(file, line);
    std::getline(file, line);

    std::getline(file, line);
    if (line.empty() || line[0] != '#') throw std::runtime_error("Expected node and edge information on the third line.");

    std::istringstream iss(line);
    std::string temp;
    iss >> temp >> temp; // Skip "Nodes:"
    if (!(iss >> num_nodes)) throw std::runtime_error("Error parsing the number of nodes.");

    std::vector<long> outgoing_edge_count(num_nodes, 0);
    std::vector<long> incoming_edge_count(num_nodes, 0);

    // First pass: Count the number of outgoing and incoming edges for each node
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') {
            continue;
        }

        std::istringstream edge_stream(line);
        long from, to;
        if (!(edge_stream >> from >> to)) throw std::runtime_error("Error parsing edge line.");
        if(from == to) continue;

        if(!node_id_map.contains(from)) {
            node_id_map[from] = current_id++;
        }
        if(!node_id_map.contains(to)) {
            node_id_map[to] = current_id++;
        }

        // Increment the outgoing count for 'from' node and incoming count for 'to' node
        outgoing_edge_count[node_id_map[from]]++;
        incoming_edge_count[node_id_map[to]]++;
    }

    g.nodes_.reserve(num_nodes);
    for (long i = 0; i < num_nodes; ++i) {
        g.nodes_.emplace_back(i);
        g.nodes_[i].outgoing_edges_.reserve(outgoing_edge_count[i]);
        g.nodes_[i].incoming_edges_.reserve(incoming_edge_count[i]);
    }

    // Second pass: Actually add the edges now that the space is reserved
    file.clear();  // Clear EOF flag
    file.seekg(0); // Go back to the start of the file

    // Skip the first three lines again
    std::getline(file, line); // First comment line
    std::getline(file, line); // Second comment line
    std::getline(file, line); // Third line (node/edge count)

    // read the edges from the file
    while (std::getline(file, line)) {
        // Skip lines that start with '#' (comments)
        if (line.empty() || line[0] == '#') continue;

        std::istringstream edge_stream(line);
        long from, to;
        if (!(edge_stream >> from >> to)) throw std::runtime_error("Error parsing edge line.");
        if(from == to) continue;

        g.add_edge(node_id_map[from], node_id_map[to]);

    }

    return g;
}

graph read_graph(std::string const& graph_name, std::string const& filetype) {
    if(filetype == "gra") {
        return read_gra_file("../../test/data/" + graph_name + ".gra");
    }
    if(filetype == "txt") {
        return read_txt_graph("../../test/data/" + graph_name + ".txt");
    }
    throw std::runtime_error("Unknown filetype.");
}

void execute_test_on_graph(std::string const& graph_name, graph& g, int number_of_times) {
    std::ofstream resultsFile("../../test/results/" + graph_name + ".txt");
    if (!resultsFile.is_open()) {
        std::cerr << "Error opening file!" << std::endl;
    }

    std::chrono::microseconds duration(0);
    for(int i = 0; i < number_of_times; ++i) {
        duration += evaluate(g, tr_b, "tr_b");
    }
    resultsFile << "TR-B: " << (duration.count() / number_of_times) << "\n";

    duration = std::chrono::microseconds(0);
    for(int i = 0; i < number_of_times; ++i) {
        duration += evaluate(g, tr_o, "tr_o");
    }
    resultsFile << "TR-O: " << (duration.count() / number_of_times) << "\n";

    duration = std::chrono::microseconds(0);
    for(int i = 0; i < number_of_times; ++i) {
        duration += evaluate(g, tr_o_plus, "tr_o_plus");
    }
    resultsFile << "TR-O+: " << (duration.count() / number_of_times) << "\n";

    duration = std::chrono::microseconds(0);
    for(int i = 0; i < number_of_times; ++i) {
        duration += evaluate(g, build_tr_by_dfs, "dfs");
    }
    resultsFile << "DFS: " << (duration.count() / number_of_times) << "\n";
}

void execute_test_on_dataset(std::string const& graph_name, std::string const& filetype, int number_of_times) {
    auto g = read_graph(graph_name, filetype);
    shuffle_graph(g, 12102024);

    execute_test_on_graph(graph_name, g, number_of_times);
}

TEST(evaluate, amaze) {
    execute_test_on_dataset("amaze", "gra", 10);
}

TEST(evaluate, go) {
    execute_test_on_dataset("go", "gra", 10);
}

TEST(evaluate, citPatents) {
    execute_test_on_dataset("cit-Patents", "txt", 1);
}

TEST(evaluate, citeseerx) {
    execute_test_on_dataset("citeseerx", "gra", 1);
}

TEST(TRO_PLUS, small_sparse_synthetic) {
    int number_of_nodes = 10000;
    int number_of_edges = 20000;

    set_seed(12092024);

    auto g = generate_graph(number_of_nodes, number_of_edges, true, true);
    execute_test_on_graph("small_sparse_synthetic", g, 10);
}

TEST(TRO_PLUS, small_dense_synthetic) {
    int number_of_nodes = 10000;
    int number_of_edges = 50000;

    set_seed(12092024);

    auto g = generate_graph(number_of_nodes, number_of_edges, true, true);
    execute_test_on_graph("small_dense_synthetic", g, 10);
}

TEST(TRO_PLUS, medium_sparse_synthetic) {
    int number_of_nodes = 100000;
    int number_of_edges = 200000;

    set_seed(12092024);

    auto g = generate_graph(number_of_nodes, number_of_edges, true, true);
    execute_test_on_graph("medium_sparse_synthetic", g, 10);
}

TEST(TRO_PLUS, medium_dense_synthetic) {
    int number_of_nodes = 100000;
    int number_of_edges = 500000;

    set_seed(12092024);

    auto g = generate_graph(number_of_nodes, number_of_edges, true, true);
    execute_test_on_graph("medium_dense_synthetic", g, 10);
}

TEST(TRO_PLUS, large_sparse_synthetic) {
    int number_of_nodes = 1000000;
    int number_of_edges = 2000000;

    set_seed(12092024);

    auto g = generate_graph(number_of_nodes, number_of_edges, true, true);
    execute_test_on_graph("large_sparse_synthetic", g, 1);
}

TEST(TRO_PLUS, large_dense_synthetic) {
    int number_of_nodes = 1000000;
    int number_of_edges = 5000000;

    set_seed(12092024);

    auto g = generate_graph(number_of_nodes, number_of_edges, true, true);
    execute_test_on_graph("large_dense_synthetic", g, 1);
}
