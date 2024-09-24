#include <fstream>
#include <filesystem>

#include "gtest/gtest.h"

#include "TR-B.h"
#include "TR-O.h"
#include "TR-O-PLUS.h"
#include "dagUtil.h"
#include "dagGenerator.h"

std::chrono::microseconds evaluate(graph& graph, void (*algorithm)(graphs::graph&), const std::string& algorithm_name) {
    auto const start = std::chrono::high_resolution_clock::now();
    algorithm(graph);
    auto const stop = std::chrono::high_resolution_clock::now();
    auto const duration = duration_cast<std::chrono::microseconds>(stop - start);
    std::cout << "calculated transient reduction " << algorithm_name <<". TIME: " << duration.count() << "microseconds\n";
    return duration;
}

graph read_gra_file(const std::string& filename) {
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

    // Read each node's edges
    while (std::getline(file, line)) {
        if (line.empty()) continue;

        std::istringstream iss(line);
        long node_id;
        char colon;
        iss >> node_id >> colon;

        if (node_id >= num_nodes) throw std::runtime_error("Node ID exceeds number of nodes.");

        long neighbor_id;
        while (iss >> neighbor_id) {
            // read next word
            if (neighbor_id == '#')  break;
            if (neighbor_id >= num_nodes) throw std::runtime_error("Neighbor ID exceeds number of nodes.");

            // Add edge from node_id to neighbor_id
            g.add_edge(node_id, neighbor_id);
        }
    }
    return g;
}

graph read_txt_graph(const std::string& filename) {
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

graph read_graph(const std::string& graph_name, const std::string& filetype) {
    if(filetype == "gra") {
        return read_gra_file("../../test/data/" + graph_name + ".gra");
    }
    if(filetype == "txt") {
        return read_txt_graph("../../test/data/" + graph_name + ".txt");
    }
    throw std::runtime_error("Unknown filetype.");
}

void execute_test_on_graph(const std::string& graph_name, const std::string& filetype) {
    std::ofstream resultsFile("../../test/results/" + graph_name + ".txt");
    if (!resultsFile.is_open()) {
        std::cerr << "Error opening file!" << std::endl;
    }

    auto g = read_graph(graph_name, filetype);
    auto duration = evaluate(g, tr_b_sparse, "tr_b_sparse");
    resultsFile << "TR-B for sparse graphs: " << duration.count() << "\n";
    g = read_graph(graph_name, filetype);
    duration = evaluate(g, tr_o_sparse, "tr_o_sparse");
    resultsFile << "TR-O for sparse graphs: " << duration.count() << "\n";
    g = read_graph(graph_name, filetype);
    duration = evaluate(g, tr_o_plus_sparse, "tr_o_plus_sparse");
    resultsFile << "TR-O-PLUS for sparse graphs: " << duration.count() << "\n";

    g = read_graph(graph_name, filetype);
    duration = evaluate(g, tr_b_dense, "tr_b_dense");
    resultsFile << "TR-B for dense graphs: " << duration.count() << "\n";
    g = read_graph(graph_name, filetype);
    duration = evaluate(g, tr_o_dense, "tr_o_dense");
    resultsFile << "TR-O for dense graphs: " << duration.count() << "\n";
    g = read_graph(graph_name, filetype);
    duration = evaluate(g, tr_o_plus_dense, "tr_o_plus_dense");
    resultsFile << "TR-O-PLUS for dense  graphs: " << duration.count() << "\n";
}

TEST(evaluate, amaze) {
    execute_test_on_graph("amaze", "gra");
}

TEST(evaluate, go) {
    execute_test_on_graph("go", "gra");
}

// TEST(evaluate, citPatents2) {
//     execute_test_on_graph("cit-Patents2", "txt");
// }

TEST(TRO_PLUS, time_tests) {
    int number_of_nodes = 20000;
    int number_of_edges = 86000;

    set_seed(12092024);
    auto g = generate_graph(number_of_nodes, number_of_edges, true);


    tr_o_plus_dense(g);
}

// TODOs to improve performance
// Store node objects directly in graph to avoid pointer indirection.
// Replace edge pointers in node with node indices
// Consider optimizing remove_edge for faster deletion.
// Use std::array for labels if sizes are fixed
// make add_edge, remove_edge and operator=== inline
