#include <algorithm>
#include <iterator>
#include <iostream>
#include <fstream>
#include <utility>
#include <vector>
#include <string>

extern "C" {
    #include <exactcolors/mwis_sewell/mwss.h>
    #include <exactcolors/mwis_sewell/mwss_ext.h>
}

using namespace std;

struct Graph {
    uint32_t n_nodes;
    uint32_t n_edges;
    uint32_t lb;
    vector<pair<uint32_t, uint32_t>> edges;
    vector<uint32_t> weights;
    
    Graph(string filename);
    vector<uint32_t> solve_mwss() const;
    void verify_stable_set(const vector<uint32_t>& s) const;
};

Graph::Graph(string filename) {
    ifstream fd(filename);
    
    // Read number of vertices and edges
    fd >> n_nodes >> n_edges >> lb;
    
    edges.reserve(n_edges);
    weights.reserve(n_nodes);
    
    // Read the list of edges
    uint32_t src, trg;
    for(auto i = 0u; i < n_edges; i++) {
        fd >> src >> trg;
        edges.emplace_back(src, trg);
    }
    assert(edges.size() == n_edges);
    
    // Read the list of integer weights
    copy_n(istream_iterator<uint32_t>(fd), n_nodes, back_inserter(weights));
    assert(weights.size() == n_nodes);
}

vector<uint32_t> Graph::solve_mwss() const {
    MWSSgraph m_graph;
    MWSSdata m_data;
    wstable_info m_info;
    wstable_parameters m_params;
    
    auto m_weight_lower_bound = lb;
    auto m_weight_goal = MWISNW_MAX;
    
    int m_graph_allocated = 0, m_initialised = 0, m_called = 0;
    
    vector<uint32_t> stable_set;
    
    // Reset (just to be safe)
    reset_pointers(&m_graph, &m_data, &m_info);
    
    // Use default parameters
    default_parameters(&m_params);
    
    // Allocate memory for the graph
    m_graph_allocated = allocate_graph(&m_graph, n_nodes);
    MWIScheck_rval(m_graph_allocated, "Cannot allocate m_graph");
    
    // Set the number of nodes
    m_graph.n_nodes = n_nodes;
    
    for(auto i = 1u; i <= n_nodes; i++) {
        // Set the weight
        m_graph.weight[i] = weights[i - 1];
        // Zero out the adjacency matrix
        for(auto j = 1u; j <= n_nodes; j++) { m_graph.adj[i][j] = 0; }
    }
    
    // Fill in the adjacency matrix
    for(const auto& e : edges) {
        m_graph.adj[e.first + 1][e.second + 1] = 1;
        m_graph.adj[e.second + 1][e.first + 1] = 1;
    }
    
    // build_graph fills in:
    //  * m_graph.n_edges
    //  * m_graph.edge_list
    //  * m_graph.adj_last
    //  * m_graph.node_list[i].adjacent
    //  * m_graph.node_list[i].name
    //  * m_graph.node_list[i].degree
    //  * m_graph.node_list[i].adjv
    //  * m_graph.node_list[i].adj2
    // (see wstable.c:1562)
    build_graph(&m_graph);
    
    // Checks consistency of the internal variables of m_graph
    assert(check_graph(&m_graph) == 1);

    // Get ready to solve
    m_initialised = initialize_max_wstable(&m_graph, &m_info);
    MWIScheck_rval(m_initialised, "Cannot initialise max wstable");
    
    // Solve the Maximum Weight Stable Set Problem
    m_called = call_max_wstable(&m_graph, &m_data, &m_params, &m_info, m_weight_goal, m_weight_lower_bound);
    
    // If something failed, clean up memory and return an empty set
    if(m_called != 0) { goto CLEANUP; }
    
    // Otherwise, "collect" the solution
    for(auto i = 1u; i <= n_nodes; i++) {
        if(m_data.best_sol[i] != NULL) {
            stable_set.push_back(m_data.best_sol[i]->name - 1);
        }
    }
    
    CLEANUP:
    free_max_wstable(&m_graph, &m_data, &m_info);
    return stable_set;
}

void Graph::verify_stable_set(const vector<uint32_t>& s) const {
    for(auto i : s) {
        for(auto j : s) {
            if(find(edges.begin(), edges.end(), make_pair(i, j)) != edges.end()) {
                cout << "Vertices " << i << " and " << j << " are in the stable set, but they are connected by an edge!" << endl;
            }
        }
    }
}

int main() {
    auto g = Graph{"../data/graph.txt"};
    auto s = g.solve_mwss();
    
    copy(s.begin(), s.end(), ostream_iterator<uint32_t>(cout, " "));
    cout << endl;
        
    g.verify_stable_set(s);
    
    return 0;
}