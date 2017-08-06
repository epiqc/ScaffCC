//===--------------------------- braidflash.cpp ---------------------------===//
// This file simulates flash (not hop-by-hop) braid space occupancies on 
// the surface code mesh.
//
//                              Ali JavadiAbhari
//                            Princeton University
//                                February 2016
//
//===----------------------------------------------------------------------===//

#define VERSION 1.0

// Usage:
// $ ./braidflash </path/to/benchmark_name/without/.lpfs/.cg/.freq/suffix>

#include <iostream>   //std::cout, std::cin
#include <fstream>    //std::ifstream
#include <utility>    //std::pair
#include <vector>     //std::vector
#include <list>       //std::list
#include <algorithm>  //std::erase, std::find, std::sort
#include <queue>      //std::queue
#include <stdlib.h>   //system
#include <cstdlib>
#include <cmath>      //sqrt
#include <time.h>     //clock
#include <sstream>    //std::stringstream
#include <map>
#include <unordered_map>
#include <stack>
#include <cstring>    //strcmp
#include <iterator>
#include <limits.h>
#include <unistd.h>
#include <memory>     //std::shared_ptr, std::make_unique
#include <limits>     //std::numeric_limits
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/reverse_graph.hpp>
#include <boost/graph/copy.hpp>
#include <boost/graph/graphviz.hpp>

//#define _DEBUG    // optional: debug flag
#define _PROGRESS   // optional: progress flag

using namespace std;

bool visualize_mesh;           // to print the network state at every cycle, or not
ofstream vis_file;

unsigned int attempt_th_yx;    // when should i switch DOR routing path? is evaluated first.
unsigned int attempt_th_drop;  // when should i drop the entire operation and reinject it? is evaluated second.

string tech;                // backend tech: sup, ion, qdot
int priority_policy;        // 0: no priorities. in program order.
                            // 1: criticality only.
                            // 2: braid length only. short2long.
                            // 3: braid length only. long2short.
                            // 4: close2open only.
                            // 5: crticiality + short2long + close2open
                            // 6: criticality + short2long (highest crit) + long2short (lower crit) + close2open

#define P_th 2              // surface code threshold = 10^-2
#define epsilon 0.5         // total desired logical error
double L_error_rate;        // desired logical error rate = 10^-(L_error_rate)
int P_error_rate;           // device error rate parameter = 10^-(P_error_rate)
int code_distance;          // coding distance of the surface code

// magic state distillation
#define short_A_error 0.005
unsigned int data_to_factory_ratio = 50;  // ratio of 1:M magic state factories to data qubits
unsigned int distillation_level = 0;
map< string, unsigned int > num_magic_factories;

// Physical operation latencies -- also determines surface code cycle length
std::unordered_map<std::string, int> op_delays_ion; 
std::unordered_map<std::string, int> op_delays_sup; 
std::unordered_map<std::string, int> op_delays_qdot; 
int surface_code_cycle_ion, surface_code_cycle_sup, surface_code_cycle_qdot;
int surface_code_cycle;


// Note: this is for logical qubit layouts.
// the number of router/nodes is one larger in both row and column
unsigned int num_rows;
unsigned int num_cols;

// global clock cycle
unsigned long long clk;
unsigned long long total_cycles;

// Mesh:
struct Node {
  unsigned int owner;
  Node() : owner(0) {}
};
struct Link {
  unsigned int owner;
  Link() : owner(0) {}
};
typedef boost::adjacency_list<boost::setS, boost::vecS, boost::undirectedS, Node, Link> mesh_t;
typedef mesh_t::vertex_descriptor node_descriptor;
typedef mesh_t::edge_descriptor link_descriptor;
map<unsigned int, node_descriptor> node_map;
mesh_t mesh; 

// Braid: generic type, specifies engaged nodes and links
struct Braid {
  vector<node_descriptor> nodes;
  vector<link_descriptor> links;
  Braid(vector<node_descriptor> nodes={}, vector<link_descriptor> links={}) : 
    nodes(nodes), links(links) {}
};
Braid operator+( Braid const& lhs, Braid const& rhs);

// Gate: operation and list of operands
struct Gate {
  unsigned int seq;
  string op_type;  
  vector<unsigned int> qid; 
  int criticality;
  Gate(unsigned int seq=0, string op_type="CNOT", vector<unsigned int> qid={0,0}, int criticality=-1) : 
    seq(seq), op_type(op_type), qid(qid), criticality(criticality) {}
};
map<string, unsigned int> gate_latencies; 

// dag: directed acyclic graph of gate dependencies
typedef boost::adjacency_list<boost::setS, boost::vecS, boost::bidirectionalS, Gate> dag_t;
typedef dag_t::vertex_descriptor gate_descriptor;
typedef dag_t::edge_descriptor dependency_descriptor;
map<unsigned int, gate_descriptor> gate_map;
dag_t dag;
int highest_criticality;

// Event: which braids should be opened/closed at which time.
enum event_type {cnot1, cnot2, cnot3, cnot4, cnot5, cnot6, cnot7, h1, h2, t1};
map<event_type, int> event_timers;
struct Event {
  Braid braid;            // which nodes/links does it contain  
  bool close_open;        // 0: close, 1: open
  gate_descriptor gate;   // gate to which this event belongs
  event_type type;        // type of event
  int timer;              // -1: invalid timer. 0: Event ready to go. >0: counting down.
  unsigned int attempts;  // number of attempts to complete event    
  
  Event(Braid braid, bool close_open, gate_descriptor gate, event_type type, int timer=-1, unsigned int attempts=0)
    : braid(braid), close_open(close_open), gate(gate), type(type), timer(timer), attempts(attempts){}

  bool operator< (const Event &other) const { // event priority based on its attributes
    bool res = false;
    bool co1 = close_open;
    bool co2 = other.close_open;
    int crit1 = dag[gate].criticality;
    int crit2 = dag[other.gate].criticality;
    int len1 = braid.links.size();
    int len2 = other.braid.links.size();
      // 0: no priorities. in program order.    
    switch (priority_policy) {
      // 1: criticality only.      
      case 1: res = (dag[gate].criticality > dag[other.gate].criticality); break;
      // 2: braid length only. short2long.              
      case 2: res = (braid.links.size() < other.braid.links.size()); break;
      // 3: braid length only. long2short.              
      case 3: res = (braid.links.size() > other.braid.links.size()); break;
      // 4: close2open only.              
      case 4: res = (close_open==false && other.close_open==true); break;
      // 5: crticiality + short2long + close2open              
      case 5:
              if (close_open==false && other.close_open==true)
                res = true;
              else if (close_open==true && other.close_open==false)
                res = false;                
              else {
                if (dag[gate].criticality > dag[other.gate].criticality)
                  res = true;
                else if (dag[gate].criticality == dag[other.gate].criticality)
                  if (braid.links.size() < other.braid.links.size())
                    res = true;
                  else
                    res = false;
                else
                  res = false;
              }
              break; 
      // 6: criticality + short2long (highest crit) + long2short (lower crit) + close2open              
      case 6:
              if (close_open==false && other.close_open==true)
                res = true;
              else if (close_open==true && other.close_open==false)
                res = false;                
              else {              
                if (dag[gate].criticality > dag[other.gate].criticality)
                  res = true;
                else if (dag[gate].criticality == dag[other.gate].criticality) {
                  if (dag[gate].criticality == highest_criticality) {
                    if (braid.links.size() < other.braid.links.size())
                      res = true;
                    else
                      res = false;
                  }
                  else {
                    if (braid.links.size() > other.braid.links.size())
                      res = true;
                    else
                      res = false;
                  }
                }                
                else
                  res = false;
              }
              break;
      // invalid prioritization policy              
      default:
             res = false; 
             break;
    }
    return res;    
  }
};
void print_event(Event &event) {
  cout << "Event: ";
  switch (event.type) {
    case cnot1: cout << "cnot1"; break;
    case cnot2: cout << "cnot2"; break;
    case cnot3: cout << "cnot3"; break;
    case cnot4: cout << "cnot4"; break;                
    case cnot5: cout << "cnot5"; break;                
    case cnot6: cout << "cnot6"; break;                
    case cnot7: cout << "cnot7"; break;                
    case h1: cout << "h1"; break;                                
    case h2: cout << "h2"; break;  
    case t1: cout << "tq"; break;             
  }
  cout << endl;
  cout << "\tGate: " << dag[event.gate].op_type;
  for (auto &i : dag[event.gate].qid)
    cout << "\t" << i;
  cout << "\t(Crit: " << dag[event.gate].criticality << ")";
  cout << endl;
  cout << "\tAttempts: " << event.attempts;
  cout << endl;
}

// data structures to keep track of pending/ready events/gates, qubit names, module freqs
map< string, vector<Gate> > all_gates;
map< string, vector<Gate> > all_gates_opt;
map< string, dag_t> all_dags;
map< string, dag_t> all_dags_opt;
map< string, unsigned int > all_q_counts;
vector<Gate> module_gates;
vector<gate_descriptor> ready_gates;
map< gate_descriptor, queue<Event> > event_queues;
vector<Event> ready_events;
map< string, unsigned long long > module_freqs;

// data structures for results
list<pair<gate_descriptor, event_type>> success_events;
list<pair<gate_descriptor, event_type>> total_conflict_events;
list<pair<gate_descriptor, event_type>> unique_conflict_events;
list<gate_descriptor> total_dropped_gates;
list<gate_descriptor> unique_dropped_gates;

// histograms
map< unsigned int, unsigned int > attempts_hist;
map< unsigned int, unsigned int > criticality_hist;
map< unsigned int, unsigned int > length_hist;
map< unsigned int, unsigned int > criticality_hist_opt;
map< unsigned int, unsigned int > length_hist_opt;
unsigned int max_crit = 0;
unsigned int max_len = 0;
unsigned int num_bins = 20;
unsigned int len_binwidth = 0;    
unsigned int crit_binwidth = 0;  

// mesh utility
double avg_module_mesh_utility = 0.0;
map< string, double> avg_mesh_utility;

// find the diagonal node with respect to qubit_num
unsigned int find_diagonal(unsigned int qubit_num, unsigned int node) {
  unsigned int const top_left_node = qubit_num+(qubit_num/num_cols);
  unsigned int const top_right_node = qubit_num+(qubit_num/num_cols)+1;
  unsigned int const bottom_left_node = qubit_num+(qubit_num/num_cols)+num_cols+1;
  unsigned int const bottom_right_node = qubit_num+(qubit_num/num_cols)+num_cols+2;
  unsigned int result = 0;
  if      (node == top_left_node)     result = bottom_right_node;
  else if (node == top_right_node)    result = bottom_left_node;
  else if (node == bottom_left_node)  result = top_right_node;
  else if (node == bottom_right_node) result = top_left_node;                        
  return result;
}

// find the vertical node with respect to qubit_num
unsigned int find_vertical(unsigned int qubit_num, unsigned int node) {
  unsigned int const top_left_node = qubit_num+(qubit_num/num_cols);
  unsigned int const top_right_node = qubit_num+(qubit_num/num_cols)+1;
  unsigned int const bottom_left_node = qubit_num+(qubit_num/num_cols)+num_cols+1;
  unsigned int const bottom_right_node = qubit_num+(qubit_num/num_cols)+num_cols+2;
  unsigned int result = 0;
  if      (node == top_left_node)     result = bottom_left_node;
  else if (node == top_right_node)    result = bottom_right_node;
  else if (node == bottom_left_node)  result = top_left_node;
  else if (node == bottom_right_node) result = top_right_node;                        
  return result;
}
// find the horizontal node with respect to qubit_num
unsigned int find_horizontal(unsigned int qubit_num, unsigned int node) {
  unsigned int const top_left_node = qubit_num+(qubit_num/num_cols);
  unsigned int const top_right_node = qubit_num+(qubit_num/num_cols)+1;
  unsigned int const bottom_left_node = qubit_num+(qubit_num/num_cols)+num_cols+1;
  unsigned int const bottom_right_node = qubit_num+(qubit_num/num_cols)+num_cols+2;
  unsigned int result = 0;
  if      (node == top_left_node)     result = top_right_node;
  else if (node == top_right_node)    result = top_left_node;
  else if (node == bottom_left_node)  result = bottom_right_node;
  else if (node == bottom_right_node) result = bottom_left_node;                        
  return result;
}

// find which corner of qubit_num is closest router node to src_node
unsigned int find_nearest(unsigned int qubit_num, unsigned int src_node) {
  unsigned int top_left_node = qubit_num+(qubit_num/num_cols);
  unsigned int top_right_node = qubit_num+(qubit_num/num_cols)+1;
  unsigned int bottom_left_node = qubit_num+(qubit_num/num_cols)+num_cols+1;
  unsigned int bottom_right_node = qubit_num+(qubit_num/num_cols)+num_cols+2;
  unsigned int qubit_top_left_row = qubit_num / num_cols;
  unsigned int qubit_top_left_col = qubit_num % num_cols;
  unsigned int src_row = src_node / (num_cols+1);
  unsigned int src_col = src_node % (num_cols+1);
  unsigned int result = 0;
  if (src_row <= qubit_top_left_row && src_col <= qubit_top_left_col)
    result = top_left_node;
  else if (src_row <= qubit_top_left_row && src_col > qubit_top_left_col)
    result = top_right_node;
  else if (src_row > qubit_top_left_row && src_col <= qubit_top_left_col)
    result = bottom_left_node;
  else if (src_row > qubit_top_left_row && src_col > qubit_top_left_col)
    result = bottom_right_node;
  return result;
}

bool are_adjacent(unsigned int src_qubit, unsigned int dest_qubit) {
  bool result = false;
  unsigned int src_row = src_qubit / num_cols;
  unsigned int src_col = src_qubit % num_cols;  
  unsigned int dest_row = dest_qubit / num_cols;
  unsigned int dest_col = dest_qubit % num_cols;    
  if (src_row == dest_row && (max(src_col,dest_col) - min(src_col,dest_col) == 1) )
    result = true;
  else
    result = false;
  return result;
}

// merge the nodes and links of two braids
Braid braid_merge(Braid braid1, Braid braid2) {
  vector<node_descriptor> combined_nodes;
  combined_nodes.reserve( braid1.nodes.size() + braid2.nodes.size() );
  combined_nodes.insert( combined_nodes.end(), braid1.nodes.begin(), braid1.nodes.end() );
  combined_nodes.insert( combined_nodes.end(), braid2.nodes.begin(), braid2.nodes.end() );
  
  vector<link_descriptor> combined_links;
  combined_links.reserve( braid1.links.size() + braid2.links.size() );
  combined_links.insert( combined_links.end(), braid1.links.begin(), braid1.links.end() );
  combined_links.insert( combined_links.end(), braid2.links.begin(), braid2.links.end() );
  
  return Braid(combined_nodes, combined_links);
}

// make an 'L' around qubit_num, starting from src_node.
// 'short L' means do the short part first then long part
Braid braid_short_L (unsigned int qubit_num, unsigned int src_node) {
  Braid short_L_route;  // return this
  unsigned int top_left_node = qubit_num+(qubit_num/num_cols);
  unsigned int top_right_node = qubit_num+(qubit_num/num_cols)+1;
  unsigned int bottom_left_node = qubit_num+(qubit_num/num_cols)+num_cols+1;
  unsigned int bottom_right_node = qubit_num+(qubit_num/num_cols)+num_cols+2;
  assert(
      ( (src_node == top_left_node) ||
        (src_node == top_right_node)  ||  
        (src_node == bottom_left_node)  ||  
        (src_node == bottom_right_node) )     
      && "Error: starting position for L-shaped braid not a corner of qubit.");
  // find the 3 nodes of the 'L' and its 2 links  
  node_descriptor n1, n2, n3;  
  link_descriptor l1, l2;
  n1 = node_map[src_node];
  n2 = node_map[find_horizontal(qubit_num, src_node)];
  n3 = node_map[find_diagonal(qubit_num, src_node)];  
  l1 = edge(n1, n2, mesh).first;
  l2 = edge(n2, n3, mesh).first;
#ifdef _DEBUG  
  if (mesh[n2].owner || mesh[n3].owner || mesh[l1].owner || mesh[l2].owner)
    cerr << "CONFLICT: opening short L: from node " << src_node 
      << " around qubit " << qubit_num << "." << endl;
#endif
  short_L_route.nodes.push_back( n2 );
  short_L_route.nodes.push_back( n3 );
  short_L_route.links.push_back( l1 );    
  short_L_route.links.push_back( l2 );        
  return short_L_route;
}

// make an 'S' through qubit_num, starting from src_node
Braid braid_S(unsigned int qubit_num, unsigned int src_node) {
  Braid S_route;  // return this
  unsigned int top_left_node = qubit_num+(qubit_num/num_cols);
  unsigned int top_right_node = qubit_num+(qubit_num/num_cols)+1;
  unsigned int bottom_left_node = qubit_num+(qubit_num/num_cols)+num_cols+1;
  unsigned int bottom_right_node = qubit_num+(qubit_num/num_cols)+num_cols+2;
  assert(
      ( (src_node == top_left_node) ||
        (src_node == top_right_node)  ||  
        (src_node == bottom_left_node)  ||  
        (src_node == bottom_right_node) )     
      && "Error: starting position for S-shaped braid not a corner of qubit.");
  // find the 2 nodes of 'S' and its two vertical links
  node_descriptor n1, n2;
  link_descriptor l1, l2;
  n1 = node_map[src_node];
  n2 = node_map[find_diagonal(qubit_num, src_node)];
  l1 = edge(n1, find_vertical(qubit_num, src_node), mesh).first;
  l2 = edge(n2, find_horizontal(qubit_num, src_node), mesh).first;  
  
  // make diagonal node busy
#ifdef _DEBUG
  if (mesh[n2].owner || mesh[l1].owner || mesh[l2].owner)
    cerr << "CONFLICT: opening S: from node " << src_node 
    << " through qubit " << qubit_num << "." << endl;
#endif
  S_route.nodes.push_back( n2 );
  S_route.links.push_back( l1 );
  S_route.links.push_back( l2 );      
  return S_route;
}

// Dimension Ordered Routing from src_node to dest_node
Braid braid_dor (unsigned int src_node, unsigned int dest_node, bool YX) {
  Braid dor_route; // return this
  
  unsigned int src_row = src_node / (num_cols+1);
  unsigned int src_col = src_node % (num_cols+1);
  unsigned int dest_row = dest_node / (num_cols+1);
  unsigned int dest_col = dest_node % (num_cols+1);
  
  int row_dir = (src_row < dest_row) ? 1 : -1;
  int col_dir = (src_col < dest_col) ? 1 : -1;

  if (YX) { // do YX  
    while (src_col != dest_col) {
      src_col += col_dir; // move 1 col closer
      unsigned int src_node_next = src_row*(num_cols+1)+src_col; // update src_node
      dor_route.nodes.push_back( node_map[src_node_next] );
      auto e1 = edge(node_map[src_node], node_map[src_node_next], mesh);
      dor_route.links.push_back(e1.first);
      src_node = src_node_next;    
    }
    while (src_row != dest_row) {
      src_row += row_dir; // move 1 row closer
      unsigned int src_node_next = src_row*(num_cols+1)+src_col; // update src_node
      dor_route.nodes.push_back( node_map[src_node_next] ); 
      auto e1 = edge(node_map[src_node], node_map[src_node_next], mesh);
      dor_route.links.push_back(e1.first);
      src_node = src_node_next;
    }    
  }

  else {  // do XY
    while (src_row != dest_row) {
      src_row += row_dir; // move 1 row closer
      unsigned int src_node_next = src_row*(num_cols+1)+src_col; // update src_node
      dor_route.nodes.push_back( node_map[src_node_next] ); 
      auto e1 = edge(node_map[src_node], node_map[src_node_next], mesh);
      dor_route.links.push_back(e1.first);
      src_node = src_node_next;
    }
    while (src_col != dest_col) {
      src_col += col_dir; // move 1 col closer
      unsigned int src_node_next = src_row*(num_cols+1)+src_col; // update src_node
      dor_route.nodes.push_back( node_map[src_node_next] );
      auto e1 = edge(node_map[src_node], node_map[src_node_next], mesh);
      dor_route.links.push_back(e1.first);
      src_node = src_node_next;        
    }
  }
  
  return dor_route;
}

pair<unsigned int,unsigned int> cnot_ancillas(unsigned int src_qubit, unsigned int dest_qubit) {
  unsigned int anc1, anc2;    
  // four corners of the src and dest qubits
  unsigned int src_top_left = src_qubit+(src_qubit/num_cols);
  unsigned int src_top_right = src_qubit+(src_qubit/num_cols)+1;
  unsigned int src_bottom_left = src_qubit+(src_qubit/num_cols)+num_cols+1;
  unsigned int src_bottom_right = src_qubit+(src_qubit/num_cols)+num_cols+2;
  // qubit's (primal hole pair's) row and column
  unsigned int src_row = src_qubit / num_cols;
  unsigned int dest_row = dest_qubit / num_cols;
  if ( are_adjacent(src_qubit,dest_qubit) ) {
    // handle special case of lond-edge adjacent qubits...
    if (src_qubit < dest_qubit) {
      anc1 = src_top_left;
      anc2 = src_bottom_left;
    }
    else {
      anc1 = src_top_right;
      anc2 = src_bottom_right;
    }
  }  
  else {
    if (src_row < dest_row) {
      // top-left and top-right black holes
      anc1 = src_top_right;
      anc2 = src_top_left;
    }
    else {
      // bottom-left and bottom-right black holes
      anc1 = src_bottom_left;
      anc2 = src_bottom_right;
    }  
  }
  return make_pair(anc1,anc2);
}

pair<Braid,Braid> cnot_routes (unsigned int src_qubit, unsigned int dest_qubit, unsigned int anc1, bool YX=0) { 
  Braid cnot_route_1, cnot_route_2;
  cnot_route_1.nodes.clear(); cnot_route_1.links.clear();  
  cnot_route_2.nodes.clear(); cnot_route_2.links.clear();   

  if ( are_adjacent(src_qubit,dest_qubit) ) {
    unsigned int middle_top = find_horizontal(src_qubit, anc1);
    unsigned int middle_bottom = find_diagonal(src_qubit, anc1);
    unsigned int dest_bottom = find_horizontal(dest_qubit, middle_bottom);

    // cnot_route_1
    link_descriptor l1 = edge(node_map[anc1], node_map[find_vertical(src_qubit, anc1)], mesh).first;
    link_descriptor l2 = edge(node_map[middle_top], node_map[middle_bottom], mesh).first;
    link_descriptor l3 = edge(node_map[dest_bottom], node_map[find_vertical(dest_qubit, dest_bottom)], mesh).first;
    cnot_route_1.nodes.push_back(node_map[dest_bottom]);
    cnot_route_1.links.push_back(l1);
    cnot_route_1.links.push_back(l2);
    cnot_route_1.links.push_back(l3);    

    // cnot_route_2
    link_descriptor l4 = edge(node_map[dest_bottom], node_map[middle_bottom], mesh).first;
    cnot_route_2.nodes.push_back(node_map[middle_bottom]);
    cnot_route_2.nodes.push_back(node_map[find_vertical(src_qubit, anc1)]);
    cnot_route_2.links.push_back(l4);
    cnot_route_2.links.push_back(l2); 
    cnot_route_2.links.push_back(l1);     
  }
  else {
    unsigned int diag_anc1 = find_diagonal(src_qubit, anc1);
    unsigned int nearest_dest_node = find_nearest(dest_qubit, diag_anc1);

    // cnot_route_1
    // the 'S' braid which goes diagonally, from anc1
    Braid S_section_1 = braid_S(src_qubit, anc1);
    // dor to nearest node of dest
    Braid dor_section_1 = braid_dor (diag_anc1, nearest_dest_node, YX);
    // the final 'S' braid which goes through the destination
    Braid S_section_2 = braid_S(dest_qubit, nearest_dest_node);
    // merge the braid segments  
    cnot_route_1 = braid_merge(S_section_1, dor_section_1);
    cnot_route_1 = braid_merge(cnot_route_1, S_section_2);   

    // cnot_route_2
    // 'short L' from diagonal of nearest_dest_node
    unsigned int diag_nearest_dest_node = find_diagonal(dest_qubit, nearest_dest_node);
    Braid short_L_section_1 = braid_short_L(dest_qubit, diag_nearest_dest_node);
    // dor to node at long edge away from anc1
    unsigned int vertical_anc1 = find_vertical(src_qubit, anc1);
    Braid dor_section_2 = braid_dor(nearest_dest_node, vertical_anc1, YX);
    // 'S' braid through the source
    Braid S_section_3 = braid_S(src_qubit, vertical_anc1);
    // merge the braid segments
    cnot_route_2 = braid_merge(short_L_section_1, dor_section_2);
    cnot_route_2 = braid_merge(cnot_route_2, S_section_3);  
  }

  return make_pair(cnot_route_1, cnot_route_2);
}

void resolve_cnot (Event &event) {
  assert( (event.type == cnot3 || event.type == cnot5) && "invalid cnot resolve request\n.");
  unsigned int src_qubit = dag[event.gate].qid[0];
  unsigned int dest_qubit = dag[event.gate].qid[1];   
  // adjacent qubits FIXME: this can also become XY-->YX
  if ( are_adjacent(src_qubit,dest_qubit) )
    return;  
  // modify braid
  pair<unsigned int,unsigned int> anc1_anc2 = cnot_ancillas(src_qubit, dest_qubit);
  unsigned int anc1 = anc1_anc2.first;  
  if (event.type == cnot3) {
    Braid cnot_route_1 =  cnot_routes(src_qubit, dest_qubit, anc1, 1).first;  // calculate new route
    event.braid = cnot_route_1;                                               // update route for cnot3
    cnot_route_1.nodes.pop_back();                                            // exclude last node of cnot3 braid
    cnot_route_1.nodes.push_back(node_map[anc1]);                             // include anc1 node
    event_queues[event.gate].front().braid = cnot_route_1;                    // update route for cnot4
  }
  else if (event.type == cnot5) {
    Braid cnot_route_2 = cnot_routes(src_qubit, dest_qubit, anc1, 1).second;  // calculate new route
    cnot_route_2.nodes.pop_back();    
    event.braid = cnot_route_2;                                               // update route for cnot5
    node_descriptor n_last = event_queues[event.gate].front().braid.nodes.back(); // hold last node
    cnot_route_2.nodes.push_back(n_last);                                     // include last node of cnot3
    cnot_route_2.links.pop_back();                                            // exclude ancilla link
    event_queues[event.gate].front().braid = cnot_route_2;                    // update route cnot6  
  }
  return;
}

queue<Event> events_cnot(unsigned int src_qubit, unsigned int dest_qubit, gate_descriptor gate) {
  // return this
  queue<Event> cnot_events;
  // two routes are used in a cnot
  Braid cnot_anc_route;
  Braid cnot_route_1;
  Braid cnot_route_2; 
  unsigned int anc1, anc2;

  pair<unsigned int,unsigned int> anc1_anc2 = cnot_ancillas(src_qubit, dest_qubit);
  anc1 = anc1_anc2.first;
  anc2 = anc1_anc2.second;    
  // cnot_anc_route
  link_descriptor anc_link = edge(node_map[anc1], node_map[anc2], mesh).first;
  cnot_anc_route.nodes.push_back(node_map[anc2]);    
  cnot_anc_route.nodes.push_back(node_map[anc1]);
  cnot_anc_route.links.push_back(anc_link);  
  // cnot_route_1, cnot_route_2
  pair<Braid,Braid> cnot_route1_route2 = cnot_routes(src_qubit, dest_qubit, anc1);
  cnot_route_1 = cnot_route1_route2.first;
  cnot_route_2 = cnot_route1_route2.second;
  
  // queue event cnot1: opening ancilla nodes/link immediately
  cnot_events.push( Event(cnot_anc_route, 1, gate, cnot1, 1, 0) );

  // queue event cnot2: closing ancilla link after 1 cycle
  node_descriptor n_anc1 = cnot_anc_route.nodes.back();
  //cnot_anc_route.nodes.pop_back();                       //del
  //node_descriptor n_anc2 = cnot_anc_route.nodes.back();
  //cnot_anc_route.nodes.pop_back();  
  cnot_events.push( Event(cnot_anc_route, 0, gate, cnot2, -1, 0) );  

  // queue event cnot3: opening route_1 after 1 cycle
  cnot_route_1.nodes.push_back(n_anc1);                          //add
  cnot_events.push( Event(cnot_route_1, 1, gate, cnot3, -1, 0) );  
  
  // queue event cnot4: closing route_1 after 1 cycle
  cnot_route_1.nodes.pop_back();                           //add
  node_descriptor n_last = cnot_route_1.nodes.back();      //del
  //cnot_route_1.nodes.pop_back();                         //del
  cnot_route_1.nodes.push_back(n_anc1);                    //del
  cnot_events.push( Event(cnot_route_1, 0, gate, cnot4, -1, 0) );
  
  // queue event cnot5: opening route_2 after minimum d-1 cycles
  cnot_route_2.nodes.push_back(n_last);                    //add
  //cnot_route_2.nodes.pop_back();                         //del
  cnot_events.push( Event(cnot_route_2, 1, gate, cnot5, -1, 0) );    
  
  // queue event cnot6: closing route_2 after 1 cycle
  //cnot_route_2.nodes.push_back(n_last);                  //del
  link_descriptor l_anc = cnot_route_2.links.back();
  cnot_route_2.links.pop_back();
  cnot_events.push( Event(cnot_route_2, 0, gate, cnot6, -1, 0) ); 
  
  // queue event cnot7: closing ancillas after minimum d-1 cycles
  cnot_anc_route.links.pop_back();
  cnot_anc_route.links.push_back(l_anc);
  //cnot_anc_route.nodes.push_back(n_anc2);
  cnot_events.push( Event(cnot_anc_route, 0, gate, cnot7, -1, 0) );  
  // return events queue
  return cnot_events;
}

queue<Event> events_h(unsigned int src_qubit, gate_descriptor gate) {
  // return this
  queue<Event> h_events;
  // only the side (long) links are busy in an h
  Braid h_route;
  // four corners of the src and dest qubits
  unsigned int src_top_left = src_qubit+(src_qubit/num_cols);
  unsigned int src_top_right = src_qubit+(src_qubit/num_cols)+1;
  unsigned int src_bottom_left = src_qubit+(src_qubit/num_cols)+num_cols+1;
  unsigned int src_bottom_right = src_qubit+(src_qubit/num_cols)+num_cols+2;
  // right link
  link_descriptor left_link = edge(node_map[src_top_left], node_map[src_bottom_left], mesh).first;
  link_descriptor right_link = edge(node_map[src_top_right], node_map[src_bottom_right], mesh).first;
  // merge the braid segments
  //h_route.links.push_back(left_link); 
  //h_route.links.push_back(right_link);  
  // queue event: opening side links immediately
  h_events.push( Event(h_route, 1, gate, h1, 1, 0) );
  // queue event: closing it after the gate duration is over  
  h_events.push( Event(h_route, 0, gate, h2, -1, 0) );  
  // return events queue
  return h_events;  
}

queue<Event> events_t(unsigned int src_qubit, gate_descriptor gate) {
  // return this
  queue<Event> t_events;
  // only a local measurement along the Z axis for the T gate, no nodes and links become busy.
  t_events = queue<Event>();
  return t_events;
}

// close or open the given braid
bool do_event(Event event) {
#ifdef _DEBUG
  cout << "doing event " << (int)event.type+1 << " for gate " << dag[event.gate].seq << ":\t";
#endif
  // check conflict:
  // 1- opening something that's already open
  // 2- closing something that doesn't belong to you
  for (auto const &n : event.braid.nodes) {
    if (mesh[n].owner && 
        (event.close_open == 1 || mesh[n].owner != dag[event.gate].seq)) {
#ifdef _DEBUG
      cout << "CONFLICT." << endl;      
#endif
      return false;
    }
  }
  for (auto const &l : event.braid.links) {
    if (mesh[l].owner && 
        (event.close_open == 1 || mesh[l].owner != dag[event.gate].seq)) {
#ifdef _DEBUG
      cout << "CONFLICT." << endl;      
#endif
      return false;
    }
  }

  // do it if no conflict
  for (auto const &n : event.braid.nodes) {
    mesh[n].owner = (event.close_open)? dag[event.gate].seq : 0;
  }
  for (auto const &l : event.braid.links) {
    mesh[l].owner = (event.close_open)? dag[event.gate].seq : 0;
  }
#ifdef _DEBUG
  cout << "SUCCESS." << endl;
#endif
  return true;
}

// print a specific top-left portion of the mesh status
void print_2d_mesh(unsigned int max_rows, unsigned int max_cols) {
  vis_file << "CLOCK: " << clk << endl;    
  if (clk % 100 != 0) return;
  // in case requested printing size is larger that the mesh
  if (max_rows > num_rows+1)
    max_rows = num_rows+1;
  if (max_cols > num_cols+1)
    max_cols = num_cols+1;
  // print row by row
  for (unsigned int r=0; r<max_rows; r++) {
    for (unsigned int c=0; c<max_cols; c++) {
      unsigned int node_num = r*(num_cols+1)+c;
      vis_file << node_num << '(' << ( (mesh[node_map[node_num]].owner)?'*':' ' ) << ')' << "\t\t";
      // horizontal links
      if (c != max_cols-1) {
        auto e = edge(node_map[node_num], node_map[node_num+1], mesh);
        vis_file << "--(" << ( (mesh[e.first].owner)?'*':' ' ) << ")" << "\t\t\t";
      }
    }
    vis_file << "\n\n\n";
    // vertical links
    for (unsigned int c=0; c<max_cols; c++) {
      unsigned int node_num = r*(num_cols+1)+c;
      if (r != max_rows-1) {
        auto e = edge(node_map[node_num], node_map[node_num+num_cols+1], mesh);
        vis_file << "||(" << ( (mesh[e.first].owner)?'*':' ' ) << ")" << "\t\t\t";        
        if (c != max_cols-1) {
          vis_file << "Q" << node_num-r << "\t\t\t";
        }
      }
    }
    vis_file << "\n\n\n";
  }
  return;
}

// what percent of the mesh is busy
double get_mesh_util() {
  int max_rows = num_rows+1;
  int max_cols = num_cols+1;
  int busy_links=0; int busy_nodes=0; int node_count=0; int link_count=0;  
  for (unsigned int r = 0; r<max_rows; r++) {
    for (unsigned int c = 0; c<max_cols; c++) {
      unsigned int node_num = r*(num_cols+1)+c;
      node_count++;
      if (mesh[node_map[node_num]].owner)
        busy_nodes++;
      // horizontal links
      if (c != max_cols-1) {
        auto e = edge(node_map[node_num], node_map[node_num+1], mesh);
        link_count++;
        if (mesh[e.first].owner)
          busy_links++;
      }
    }
    // vertical links
    for (unsigned int c=0; c<max_cols; c++) {
      unsigned int node_num = r*(num_cols+1)+c;
      if (r != max_rows-1) {
        auto e = edge(node_map[node_num], node_map[node_num+num_cols+1], mesh);
        link_count++;
        if (mesh[e.first].owner)
          busy_links++;
      }      
    }
  }
 
  // return ((double)busy_nodes/(double)node_count + (double)busy_links/(double)link_count); 
  return (double)(busy_nodes/*+busy_links*/)/(double)(node_count/* + link_count*/);
}

void purge_gate_from_mesh (unsigned int gate_seq) {
  // purge nodes row by row
  for (unsigned int r=0; r<num_rows+1; r++) {
    for (unsigned int c=0; c<num_cols+1; c++) {
      unsigned int node_num = r*(num_cols+1)+c;
      if ( mesh[node_map[node_num]].owner == gate_seq ) {
#ifdef _DEBUG
        cout << "\t\tpurging node " << node_num << endl;
#endif        
        mesh[node_map[node_num]].owner = 0;
      }
      // horizontal links
      if (c != num_cols) {
        auto e = edge(node_map[node_num], node_map[node_num+1], mesh);
        if ( mesh[e.first].owner == gate_seq ) {
#ifdef _DEBUG          
          cout << "\t\tpurging link " << node_num << " == " << node_num+1 << endl;
#endif          
          mesh[e.first].owner = 0;
        }
      } 
    }
    // vertical links 
    for (unsigned int c=0; c<num_cols+1; c++) {
      unsigned int node_num = r*(num_cols+1)+c;
      if (r != num_rows) {
        auto e = edge(node_map[node_num], node_map[node_num+num_cols+1], mesh);
        if ( mesh[e.first].owner == gate_seq ) {
#ifdef _DEBUG          
          cout << "\t\tpurging link " << node_num << " == " << node_num+num_cols+1 << endl;
#endif
          mesh[e.first].owner = 0;
        }
      }
    }
  }    
}

unsigned int get_gate_latency (Gate g) {
  unsigned int result = 0;
  if ( g.op_type == "CNOT" ) {
    result += gate_latencies["CNOT"];
  }
  else if ( g.op_type == "H" ) {
    result += gate_latencies["H"];
  } 
  else if ( g.op_type == "T" ) {
    result += gate_latencies["T"];
  }     
  else if ( g.op_type == "Tdag" ) {
    result += gate_latencies["Tdag"];
  }       
  return result;
}

unsigned int manhattan_cost(unsigned int src_qubit, unsigned int dest_qubit) {
  // qubit's (primal hole pair's) row and column
  unsigned int src_row = src_qubit / num_cols;
  unsigned int src_col = src_qubit % num_cols;  
  unsigned int dest_row = dest_qubit / num_cols;
  unsigned int dest_col = dest_qubit % num_cols;   
  unsigned int row_dist = max(src_row,dest_row) - min(src_row,dest_row);
  unsigned int col_dist = max(src_col,dest_col) - min(src_col,dest_col);
  return (row_dist + col_dist); 
}

pair< pair<int,int>, pair<int,int> > compare_manhattan_costs () {
  pair< pair<int,int>, pair<int,int> > result;
  unsigned int mcost = 0;
  unsigned int mcost_opt = 0;
  unsigned int event_count = 0;
  unsigned int event_count_opt = 0;
  crit_binwidth = max_crit/num_bins;   
  len_binwidth = max_len/num_bins;
  if (crit_binwidth==0) crit_binwidth = 1;  
  if (len_binwidth==0) len_binwidth = 1;   
  cout << max_crit << " " << max_len << endl; 
  for (auto const &map_it : all_dags) {
    dag_t mdag = map_it.second;
    unsigned long long module_q_count = all_q_counts[map_it.first];
    num_rows = (unsigned int)ceil( sqrt( (double)module_q_count ) );
    num_cols = (num_rows*(num_rows-1) < module_q_count) ? num_rows : num_rows-1;
    for (auto g_it_range = vertices(mdag); g_it_range.first != g_it_range.second; ++g_it_range.first){
      gate_descriptor g = *(g_it_range.first);      
      if (mdag[g].op_type == "CNOT") {      
        unsigned int c = manhattan_cost(mdag[g].qid[0], mdag[g].qid[1]);
        mcost += c;
        event_count += 7;
        unsigned int crit_binidx = (unsigned int)(mdag[g].criticality/crit_binwidth);                        
        unsigned int len_binidx = (unsigned int)(c/len_binwidth);
        ++criticality_hist[crit_binidx];        
        ++length_hist[len_binidx];
      }
      else if (mdag[g].op_type == "H")
        event_count += 2;
      else
        event_count += 1;       
    }
  }
  for (auto const &map_it : all_dags_opt) {
    dag_t mdag = map_it.second;
    unsigned long long module_q_count = all_q_counts[map_it.first];
    num_rows = (unsigned int)ceil( sqrt( (double)module_q_count ) );
    num_cols = (num_rows*(num_rows-1) < module_q_count) ? num_rows : num_rows-1;
    for (auto g_it_range = vertices(mdag); g_it_range.first != g_it_range.second; ++g_it_range.first){
      gate_descriptor g = *(g_it_range.first);      
      if (mdag[g].op_type == "CNOT") {            
        unsigned int c = manhattan_cost(mdag[g].qid[0], mdag[g].qid[1]);
        mcost += c;
        event_count += 7;
        unsigned int crit_binidx = (unsigned int)(mdag[g].criticality/crit_binwidth);                        
        unsigned int len_binidx = (unsigned int)(c/len_binwidth);
        ++criticality_hist[crit_binidx];        
        ++length_hist[len_binidx];
      }
      else if (mdag[g].op_type == "H")
        event_count += 2;
      else
        event_count += 1;         
    }
  }

  result = make_pair(make_pair(mcost,mcost_opt), make_pair(event_count,event_count_opt));
  return result;
}

// Parsing functions
// is there any of several words in a given string?
template<typename T, size_t N>
T * endof(T (&ra)[N]) {
    return ra + N;
}
string::size_type is_there(vector<string> needles, string haystack) {
  vector<string>::iterator needle;
  string::size_type pos;
  for(needle=needles.begin(); needle!=needles.end(); ++needle){
    pos = haystack.find(*needle);
    if(pos != string::npos){
      return pos;
    }
  }  
  return string::npos;
}
// tokenize string
vector<string> &split(const string &s, char delim, vector<string> &elems) {
    stringstream ss(s);
    string item;
    while (std::getline(ss, item, delim)) {
        if (!item.empty())       
          elems.push_back(item);
    }
    return elems;
}
void parse_LPFS (const string file_path) {
  ifstream LPFSfile (file_path);
  string line;
  string leaf_func = "";
  unsigned int seq = 1;      
  unsigned long long module_q_count = 0;    
  map<string, unsigned long long> q_name_to_num;            
  vector<Gate> module_gates; 
  const char* all_ops[] = {"PrepZ ", "X ", "Z ", "H ", "CNOT ", "T ", "Tdag ", "S ", "Sdag ", "MeasZ "};  
  vector<string> op_strings(all_ops, endof(all_ops));  
  if (LPFSfile.is_open()) {
    while ( getline (LPFSfile,line) ) {
      // FunctionHeaders
      if (line.find("Function") != string::npos) {
        // save result of previous iteration
        if (leaf_func != "") {
          all_gates[leaf_func] = module_gates;
          all_q_counts[leaf_func] = module_q_count;
        }
        // reset book keeping     
        vector<string> elems;          
        split(line, ' ', elems);        
        leaf_func = elems[1];
        seq = 1;
        module_q_count = 0;
        q_name_to_num.clear();        
        module_gates.clear();        
      }     
      // OPinsts
      else if (is_there(op_strings, line) != string::npos) {
        vector<string> elems;          
        split(line, ' ', elems);
        // FIXME: OLD FORMAT: 2,3,5,4
        // FIXME: NEW FORMAT: 1,2,4,3
        string op_type = elems[1];        
        vector<unsigned int> qid;        
        string qid1 = elems[2];     
        if (q_name_to_num.find(qid1) == q_name_to_num.end())
          q_name_to_num[qid1] = module_q_count++;    
        qid.push_back(q_name_to_num[qid1]);         
        if (elems.size() == 4) {
          string qid2 = elems[3];                    
          if (q_name_to_num.find(qid2) == q_name_to_num.end())
            q_name_to_num[qid2] = module_q_count++;          
          qid.push_back(q_name_to_num[qid2]);
        }
        if (/*op_type == "PrepZ" || op_type == "MeasZ" ||*/ op_type == "CNOT" || op_type == "H" /*|| op_type == "T" || op_type == "Tdag"*/) {
          Gate g = Gate(seq++, op_type, qid); 
          module_gates.push_back(g);        
        }
      }
    }
    // save result of last iteration
    if (leaf_func != "") {
      all_gates[leaf_func] = module_gates;
      all_q_counts[leaf_func] = module_q_count;
    }    
    LPFSfile.close();
  }
  else {
    cerr<<"Error: Unable to open file."<<endl;
    exit(1);
  }      
}

void parse_tr (const string file_path) {
  ifstream opt_tr_file (file_path);
  string line;
  string module_name = "";  
  vector<Gate> module_gates;   
  if (opt_tr_file.is_open()) {
    while ( getline (opt_tr_file,line) ) {
      if (line.find("module: ") != string::npos) {
        // save previous iteration
        if(module_name != "")
          all_gates_opt[module_name] = module_gates;
        // reset book keeping         
        module_gates.clear();           
        vector<string> elems;          
        split(line, ' ', elems);        
        module_name = elems[1];
      }
      else if (line.find("ID: ") != string::npos){
        vector<string> elems;          
        split(line, ' ', elems);
        unsigned int seq = (unsigned int)stol(elems[1]);
        string op_type = elems[3];
        vector<unsigned int> qid;
        qid.push_back( (unsigned int)stol(elems[5]) );
        if (elems.size() > 6)
          qid.push_back( (unsigned int)stol(elems[7]) );
        Gate g = Gate(seq, op_type, qid); 
        module_gates.push_back(g);
      }
    }
    // save last iteration
    if (module_name != "")
      all_gates_opt[module_name] = module_gates;
    opt_tr_file.close();    
  }
  else
    cerr << "Unable to open opt.tr file" << endl;
}
// parse profile of module frequencies
void parse_freq (const string file_path) {
  ifstream profile_freq_file (file_path);
  string line;
  string module_name = "";  
  unsigned long long freq = 0;
  if (profile_freq_file.is_open()) {
    while ( getline (profile_freq_file,line) ) {
      vector<string> elems; 
      elems.clear();     
      split(line, ' ', elems);              
      module_name = elems[0];
      freq = stoull(elems[9]);
      module_freqs[module_name] = freq;
    }
  }
  else
    cerr << "Unable to open .freq file" << endl;
}

// find total module critical path
unsigned long long get_critical_clk (dag_t dag_copy) {
  unsigned long long result = 0;
  vector<gate_descriptor> current_gates;        // currently evaluated gates
  vector<gate_descriptor> next_gates;           // next evaluated gates  
  map<gate_descriptor, unsigned long long> cps; // critical path of gates
  current_gates.clear();
  next_gates.clear();
  cps.clear();     
  for (auto g_it_range = vertices(dag_copy); g_it_range.first != g_it_range.second; ++g_it_range.first){
    gate_descriptor g = *(g_it_range.first);
    if (boost::in_degree(g, dag_copy)==0) { 
      current_gates.push_back(g);
      cps[g] = get_gate_latency(dag_copy[g]);
      if (cps[g] > result) {
        result = cps[g]; 
      }
    }
  }  
  while ( !(num_edges(dag_copy)==0) || !current_gates.empty() ) {
    for (auto &g : current_gates) {
      dag_t::adjacency_iterator neighborIt, neighborEnd;
      boost::tie(neighborIt, neighborEnd) = adjacent_vertices(g, dag_copy);
      for (; neighborIt != neighborEnd; ++neighborIt) {
        gate_descriptor g_out = *neighborIt;  
        if ( cps.find(g_out) == cps.end() ) {
          cps[g_out] = cps[g]+get_gate_latency(dag_copy[g_out]);
          if (cps[g_out] > result)
           result = cps[g_out];  
        }
        else {
          cps[g_out] = max(cps[g_out],cps[g]+get_gate_latency(dag_copy[g_out]));
          if (cps[g_out] > result)
            result = cps[g_out];  
        }   
        if(boost::in_degree(g_out, dag_copy) == 1) {          
          next_gates.push_back(g_out);
        }
      }
      boost::clear_out_edges(g, dag_copy);          
      assert(in_degree(g,dag_copy) == 0 && out_degree(g,dag_copy) == 0 && "removing gate prematurely from dag.");        
    }
    current_gates = next_gates;
    next_gates.clear();
  }
  return result;
}

// find criticality of each gate: exactly like the above function, but in reverse graph order
void assign_criticality () {
  dag_t r_dag;
  boost::copy_graph(boost::make_reverse_graph(dag), r_dag);  
  vector<gate_descriptor> current_gates;      // currently evaluated gates
  vector<gate_descriptor> next_gates;         // next evaluated gates  
  map<gate_descriptor, unsigned int> crits;   // critical path of gates
  current_gates.clear();
  next_gates.clear();
  crits.clear();     
  for (auto g_it_range = vertices(r_dag); g_it_range.first != g_it_range.second; ++g_it_range.first){
    gate_descriptor g = *(g_it_range.first);
    if (boost::in_degree(g, r_dag)==0) { 
      current_gates.push_back(g);
      crits[g] = get_gate_latency(r_dag[g]);
      dag[gate_map[r_dag[g].seq]].criticality = crits[g];      
    }
  }  
  while ( !(num_edges(r_dag)==0) || !current_gates.empty() ) {
  //  cout << "current_gates.size(): " << current_gates.size() << endl;    
//    cout << "num_edges(r_dag): " << num_edges(r_dag) << endl;
    for (auto &g : current_gates) {
      dag_t::adjacency_iterator neighborIt, neighborEnd;
      boost::tie(neighborIt, neighborEnd) = adjacent_vertices(g, r_dag);
      for (; neighborIt != neighborEnd; ++neighborIt) {
        gate_descriptor g_out = *neighborIt;  
        if ( crits.find(g_out) == crits.end() )
          crits[g_out] = crits[g]+get_gate_latency(r_dag[g_out]);
        else
          crits[g_out] = max(crits[g_out],crits[g]+get_gate_latency(r_dag[g_out]));
        dag[gate_map[r_dag[g_out].seq]].criticality = crits[g_out];                
        //cout << "assigned criticality: " << crits[g_out] << endl;
        if(boost::in_degree(g_out, r_dag) == 1)
          next_gates.push_back(g_out);     
      }
      boost::clear_out_edges(g, r_dag);          
      assert(in_degree(g,r_dag) == 0 && out_degree(g,r_dag) == 0 && "removing gate prematurely from dag.");        
    }
    current_gates = next_gates;
    next_gates.clear();
  }
}

void update_highest_criticality () {
  highest_criticality = 0;
  for (auto g_it_range = vertices(dag); g_it_range.first != g_it_range.second; ++g_it_range.first){
    gate_descriptor g = *(g_it_range.first);
    if (boost::in_degree(g, dag)!=0 || boost::out_degree(g,dag)!=0) { // don't look at completed gates.
      int crit = dag[g].criticality;
      if (crit > highest_criticality)
        highest_criticality = crit;
    }
  }    
}

void initialize_ready_list () {
  for (auto g_it_range = vertices(dag); g_it_range.first != g_it_range.second; ++g_it_range.first){
    gate_descriptor g = *(g_it_range.first);
    if (boost::in_degree(g, dag)==0) { 
      ready_gates.push_back(g);
    }
  }   
}

void increment_clock() {
  if (visualize_mesh) {
    print_2d_mesh(num_rows+1, num_cols+1);
  }
  clk++;
  // record mesh utilization
  double mu = get_mesh_util();  
  avg_module_mesh_utility = ((double)(clk-1)*avg_module_mesh_utility + mu)/(double)clk;
  // decrement event timers for all head events
  // whose predecessor event has finished (timer => 0)
  vector<gate_descriptor> to_delete_key;
  for (auto i = event_queues.begin(); i!=event_queues.end(); ++i) { // is this too slow?
    Event &head_event = (*i).second.front();
    if (head_event.timer < 0)       // predecessor hasn't finished yet
      continue;
    if (head_event.timer != 0)
      head_event.timer--;
#ifdef _DEBUG
    cout << "gate " << dag[(*i).first].seq << ", head_event.timer: " << head_event.timer << endl;        
#endif
    if(head_event.timer == 0) {     // lapsed event
#ifdef _DEBUG
      cout << "\tevent lapsed: popping from queue." << endl;
#endif
      ready_events.push_back(head_event);
      (*i).second.pop();
      if ((*i).second.empty())
        to_delete_key.push_back((*i).first);
    }
  }
  for (auto &k : to_delete_key)
    event_queues.erase(k);
}

// ------------------------------- main -----------------------------------
int main (int argc, char *argv[]) {

  bool opt=false;
  P_error_rate = 5; 
  attempt_th_yx = 8;
  attempt_th_drop = 20;
  tech = "sup";  // default is the braiding approach
  visualize_mesh = false;
  
  for (int i = 0; i<argc; i++) {
    if (strcmp(argv[i],"--opt")==0)
      opt = true;
    if (strcmp(argv[i],"--p")==0) {
      if (argc > (i+1)) {
        P_error_rate = atoi(argv[i+1]);
      }
      else {
        cerr<<"Usage: braidflash <benchmark> --p <P_error_rate>";
        return 1;
      }
    }  
    if (strcmp(argv[i],"--yx")==0) {
      if (argc > (i+1)) {
        attempt_th_yx = atoi(argv[i+1]);
      }
      else {
        cerr<<"Usage: braidflash <benchmark> --yx <attempt_th_yx>";
        return 1;
      }
    }    
     if (strcmp(argv[i],"--drop")==0) {
      if (argc > (i+1)) {
        attempt_th_drop = atoi(argv[i+1]);
      }
      else {
        cerr<<"Usage: braidflash <benchmark> --drop <attempt_th_drop>";
        return 1;
      }
    }   
    if (strcmp(argv[i],"--tech")==0) {
      if (argc > (i+1)) {
        tech = argv[i+1];
      }
      else {
        cerr<<"Usage: braidflash <benchmark> --tech <technology>";
        return 1;
      }
    }   
    if (strcmp(argv[i],"--pri")==0) {
      if (argc > (i+1)) {
        priority_policy = atoi(argv[i+1]);
      }
      else {
        cerr<<"Usage: braidflash <benchmark> --pri <priority_policy>";
        return 1;
      }
    }     
    if (strcmp(argv[i],"--visualize")==0) {
      visualize_mesh = true;
      cerr << "Warning: only try to visualize very small circuits\n";
    }
    if (strcmp(argv[i],"--help")==0) {
      cerr<<
R"(Usage: braidflash [FILE] [OPTIONS]
Simulate braid space occupancies on the surface code mesh.
  --opt       optimize qubit layout (default: none)
  --p         physical error rate (10^-p) [int] (default: 5)
  --yx        stall threshold to switch DOR routing from xy to yx [int] (default: 8)
  --drop      stall threshold to drop entire operation and reinject [int] (default: 20)
  --tech      technology [sup, ion, qdot] (default: sup)
  --pri       braid priority policy [0-6] (default: 0)
  --visualize show network state at each cycle [Warning: only use on small circuits] (default: none)
  --help      display this help and exit
  --version   output version information and exit
)";
      return 0;
    }
    if (strcmp(argv[i],"--version")==0) {
      cerr<<"braidflash (ScaffCC Compiler Infrastructure) "<<VERSION<<endl;
      return 0;    
    }
  }  


  // technology parameters
  op_delays_ion["PrepZ"] = 1;
  op_delays_ion["X"] = 1;
  op_delays_ion["Z"] = 1;
  op_delays_ion["H"] = 1;
  op_delays_ion["CNOT"] = 40;
  op_delays_ion["T"] = 1;
  op_delays_ion["Tdag"] = 1;
  op_delays_ion["S"] = 1;
  op_delays_ion["Sdag"] = 1;
  op_delays_ion["MeasZ"] = 10;  
    
  op_delays_sup["PrepZ"] = 1;
  op_delays_sup["X"] = 1;
  op_delays_sup["Z"] = 1;
  op_delays_sup["H"] = 1;
  op_delays_sup["CNOT"] = 40;
  op_delays_sup["T"] = 1;
  op_delays_sup["Tdag"] = 1;
  op_delays_sup["S"] = 1;
  op_delays_sup["Sdag"] = 1;
  op_delays_sup["MeasZ"] = 140;

  op_delays_qdot["PrepZ"] = 1;
  op_delays_qdot["X"] = 1;
  op_delays_qdot["Z"] = 1;
  op_delays_qdot["H"] = 1;
  op_delays_qdot["CNOT"] = 600;
  op_delays_qdot["T"] = 1;
  op_delays_qdot["Tdag"] = 1;
  op_delays_qdot["S"] = 1;
  op_delays_qdot["Sdag"] = 1;
  op_delays_qdot["MeasZ"] = 20;
  
  // how long is each surface code cycle
  surface_code_cycle_ion = op_delays_ion.find("PrepZ")->second + 
                       2*op_delays_ion.find("H")->second + 
                       4*op_delays_ion.find("CNOT")->second + 
                       op_delays_ion.find("MeasZ")->second;
  surface_code_cycle_sup = op_delays_sup.find("PrepZ")->second + 
                       2*op_delays_sup.find("H")->second + 
                       4*op_delays_sup.find("CNOT")->second + 
                       op_delays_sup.find("MeasZ")->second;
  surface_code_cycle_qdot = op_delays_qdot.find("PrepZ")->second + 
                       2*op_delays_qdot.find("H")->second + 
                       4*op_delays_qdot.find("CNOT")->second + 
                       op_delays_qdot.find("MeasZ")->second;
  
  
  if (tech=="ion") surface_code_cycle = surface_code_cycle_ion;
  if (tech=="sup") surface_code_cycle = surface_code_cycle_sup;
  if (tech=="qdot") surface_code_cycle = surface_code_cycle_qdot;
  
  // read program gates
  // mark gate seq numbers from 1 upwards
  string benchmark_path(argv[1]);
  string benchmark_dir = benchmark_path.substr(0, benchmark_path.find_last_of('/'));  
  string benchmark_name = benchmark_path.substr(benchmark_path.find_last_of('/')+1, benchmark_path.length());  
  string LPFS_path = benchmark_path+".lpfs";
  string profile_freq_path = benchmark_path+".freq";
  parse_LPFS(LPFS_path);
  parse_freq(profile_freq_path);
 
  //calculate code distance
  if (P_error_rate < P_th) {
    cerr << "Physical error rate is higher than the code threshold. Terminating..\n";
    exit(1);
  }      
  unsigned long long total_logical_gates = 0;    // KQ parameter, needed for calculating L_error_rate  
  unsigned long long total_T_gates = 0;          // total number of logical T gates
  for (auto const &map_it : all_gates) {
    string module_name = map_it.first;     
    int module_size = map_it.second.size();
    int module_T_size = 0;
    for (auto const &i : map_it.second)
      if ( i.op_type == "T" || i.op_type == "Tdag")
        module_T_size++;
    unsigned long long module_freq = 1;
    if ( module_freqs.find(module_name) != module_freqs.end() )
      module_freq = module_freqs[module_name];
#ifdef _PROGRESS
      cerr<<"\nleaf: "<<module_name<<" - size: "<<module_size<<" - freq: "<<module_freq;
#endif      
    total_logical_gates += module_size * module_freq;   
    total_T_gates += module_T_size * module_freq;   
    num_magic_factories[module_name] = all_q_counts[module_name]/data_to_factory_ratio;
  }
  cerr << "\ntotal logical gates: " << total_logical_gates << endl;
  cerr << "total T gates: " << total_T_gates << endl;  
  L_error_rate = (double)epsilon/(double)total_logical_gates;   
  code_distance = 
          2*(int)( ceil(log( (100.0/3.0) * (double)L_error_rate ) / 
                       log( pow(10.0,(-1.0*(double)P_error_rate)) / pow(10.0,(-1.0*(double)P_th)) ) ) ) - 1;
  if ( L_error_rate > pow(10.0,(-1.0*(double)P_error_rate)) ) code_distance = 1; // very small circuit (large L_error_rate), means smallest possible mesh
#ifdef _PROGRESS  
  cerr << "Physical error rate (p): " << P_error_rate << endl;
  cerr << "Logical error rate (p_L): " << L_error_rate << endl;    
  cerr << "Code distance (d): " << code_distance << endl;
#endif
  if (code_distance < 1) {
    cerr << "code distance too small for surface code operation. Try changing physical or logical error rates.\n";
    exit(1);
  }

  // calculate magic distillation level and number of factories
  // P_0 = short_A_error, P_1 = 35*(short_A_error)^3, ..., P_n = 35*(P_(n-1))^3
  double distillation_error = short_A_error;
  while (distillation_error > epsilon/total_T_gates) {
    distillation_level++;
    double pow35 = 0.0;
    for (int j = 0; j < distillation_level; j++)
      pow35 += pow(3.0, (double)(j));
    distillation_error = pow(35.0, pow35) * pow(short_A_error,pow(3.0,(double)distillation_level));
  }

  //cerr << "distillation level: " << distillation_level << endl;

  // optimize qubit placements
  // write all_gates to trace (.tr) file    
  if (opt) {
    string tr_path = benchmark_path+".tr"; 
    string opt_tr_path = benchmark_path+".opt.tr";         
    ifstream f(opt_tr_path.c_str());
    if (!f.good()) {
      ofstream tr_file;
      tr_file.open(tr_path);  
      for (auto const &map_it : all_gates) {
        string module_name = map_it.first;     
        vector<Gate> module_gates = map_it.second;
        if (!module_gates.empty()) {
          tr_file << "module: " << module_name << endl;
          tr_file << "num_nodes: " << all_q_counts[module_name] << endl;
          for (auto &i : module_gates) {
            if (i.qid.size() == 1)  
              tr_file << "ID: " << i.seq << " TYPE: " << i.op_type << " SRC: " << i.qid[0] << endl;
            else if (i.qid.size() == 2)
              tr_file << "ID: " << i.seq << " TYPE: " << i.op_type << " SRC: " << i.qid[0] << " DST: " << i.qid[1] << endl;
            else
              cerr << "Invalide gate." << endl;
          }
        }
      }
      tr_file.close();  
      // use metis to rearrange qubits for more optimal interaction distances, if not already done.
      string exe_path(argv[0]);
      string exe_dir = exe_path.substr(0, exe_path.find_last_of('/'));  
      string metis_command = "python "+exe_dir+"/arrange.py "+tr_path+
                           " "+to_string(P_error_rate)+" "+to_string(attempt_th_yx)+" "+to_string(attempt_th_drop);
      int status = system(metis_command.c_str());
      if (status==-1) {
        cerr << "METIS partitioning failed.\n";
        return 1;
      }
    }
    // read opimized trace (.opt.tr) file into all_gates_opt          
    parse_tr(opt_tr_path);
  }
  
  // build event_timers lookup table  
  event_timers[cnot1] = 1;
  event_timers[cnot2] = 1;
  event_timers[cnot3] = 1;
  event_timers[cnot4] = 1;
  event_timers[cnot5] = code_distance-1;  
  event_timers[cnot6] = 1;
  event_timers[cnot7] = code_distance-1;
  event_timers[h1] = 1;
  event_timers[h2] = 8+code_distance;
  event_timers[t1] = 1;

  // build gate_latencies lookup table
  gate_latencies["CNOT"] = 0;
  gate_latencies["CNOT"] += event_timers[cnot1];
  gate_latencies["CNOT"] += event_timers[cnot2];
  gate_latencies["CNOT"] += event_timers[cnot3];
  gate_latencies["CNOT"] += event_timers[cnot4];
  gate_latencies["CNOT"] += event_timers[cnot5];
  gate_latencies["CNOT"] += event_timers[cnot6];
  gate_latencies["CNOT"] += event_timers[cnot7];
  gate_latencies["H"] = 0;  
  gate_latencies["H"] += event_timers[h1];
  gate_latencies["H"] += event_timers[h2];  
  gate_latencies["T"] = 0;    
  gate_latencies["T"] = event_timers[t1];  

  // braid file: all information to later collect results from
  string output_dir = benchmark_dir+"/braid_simulation/";
  string mkdir_command = "mkdir -p "+output_dir;
  int status = system(mkdir_command.c_str());
  string br_file_path;  
  ofstream br_file;      
  br_file_path = output_dir+benchmark_name
                    +".p."+(to_string(P_error_rate))
                    +".yx."+to_string(attempt_th_yx)
                    +".drop."+to_string(attempt_th_drop)                             
                    +".pri."+to_string(priority_policy)
                    +"."+tech                    
                    +(opt ? ".opt.br" : ".br");
  br_file.open(br_file_path);  

  // visualization file: print network states
  string vis_file_path; 
  vis_file_path = output_dir+benchmark_name
                    +".p."+(to_string(P_error_rate))
                    +".yx."+to_string(attempt_th_yx)
                    +".drop."+to_string(attempt_th_drop)    
                    +".pri."+to_string(priority_policy)                    
                    +"."+tech
                    +(opt ? ".opt.vis" : ".vis");
  vis_file.open(vis_file_path);

  // braidflash for each module of the benchmark
  all_gates = (opt) ? all_gates_opt : all_gates;
  total_cycles = 0;
  for (auto const &map_it : all_gates) {
    // reset clock, mesh and dag
    clk = 0;
    node_map.clear();
    gate_map.clear();
    mesh.clear();
    dag.clear();
    success_events.clear();
    total_conflict_events.clear();
    unique_conflict_events.clear();
    total_dropped_gates.clear();
    unique_dropped_gates.clear();
    attempts_hist.clear();
    avg_module_mesh_utility = 0.0;

    // retrieve parsed gates and q_count for this module
    string module_name = map_it.first;
    vector<Gate> module_gates = map_it.second;
    unsigned long long module_q_count = all_q_counts[module_name];
    num_rows = (unsigned int)ceil( sqrt( (double)module_q_count ) );
    num_cols = (num_rows*(num_rows-1) < module_q_count) ? num_rows : num_rows-1;
#ifdef _DEBUG    
    cout << "\nModule: " << module_name << endl;    
    cout << "Size: " << num_rows << "X" << num_cols << endl;
#endif
    //if (num_rows == 1 && num_cols == 1) continue;

#ifdef _PROGRESS
    cerr << "\nModule: " << module_name << endl;    
    cerr << "Size: " << num_rows << "X" << num_cols << endl;
#endif   
    
    // build mesh
    // add all nodes   
#ifdef _PROGRESS
    cerr << "Building mesh..." << endl;
#endif 
    for (unsigned int i=0; i < (num_rows+1) * (num_cols+1); i++) {
      node_descriptor n = boost::add_vertex(mesh);
      mesh[n].owner = 0;
      auto t = node_map.emplace(i, n);
      if (t.second == false)
        cerr << "Error: reinserting a node in the mesh." << endl;
    }
    // add all links
    for (unsigned int i=0; i < (num_rows+1) * (num_cols+1); i++) {
      unsigned int node_row = i / (num_cols+1);
      unsigned int node_col = i % (num_cols+1);
      link_descriptor l; bool b;
      if (node_row != 0) {  // north
        boost::tie(l,b) = boost::add_edge(node_map[i], node_map[i-num_cols-1], mesh);  
        mesh[l].owner = 0;
      }
      if (node_row != num_rows) { // south
        boost::tie(l,b) = boost::add_edge(node_map[i], node_map[i+num_cols+1], mesh);  
        mesh[l].owner = 0;      
      }
      if (node_col != num_cols) { // east
        boost::tie(l,b) = boost::add_edge(node_map[i], node_map[i+1], mesh);  
        mesh[l].owner = 0; 
      }
      if (node_col != 0) {  // west
        boost::tie(l,b) = boost::add_edge(node_map[i], node_map[i-1], mesh);    
        mesh[l].owner = 0;      
      }
    } 

    // build dag
    // add all gates 
#ifdef _PROGRESS
    cerr << "Building DAG..." << endl;
    cerr << "module_gates.size() = " << module_gates.size() << endl;
    int count = 0;
#endif     
    for (vector<Gate>::const_iterator I = module_gates.begin(); I != module_gates.end(); ++I) {    
      gate_descriptor g = boost::add_vertex(dag);
      dag[g].seq = (*I).seq;
      dag[g].op_type = (*I).op_type;
      dag[g].qid = (*I).qid;
      auto t = gate_map.emplace(dag[g].seq, g);
      if (t.second == false)
        cerr << "Error: reinserting a gate in the dag." << endl;
    }
    // add all gate dependencies
    for (vector<Gate>::const_iterator I = module_gates.begin(); I != module_gates.end(); ++I) {  
#ifdef _PROGRESS
      count++;
      if (count % 10000 == 0) cerr << count << endl;
#endif       
      vector<unsigned int> qid;
      vector<unsigned int> qid_next;    
      // for each argument of this Instruction
      qid = (*I).qid;    
      for (int i=0; i<qid.size(); i++) {    
        // iterate through later instructions until 
        // it either finds an inst with the same argument, or runs out of insts      
        auto I_next = std::next(I);
        bool found_next = 0;      
        while ( I_next != module_gates.end() ) {
          qid_next = (*I_next).qid;
          // for each argument of the later instruction
          for (int j=0; j<qid_next.size(); j++) {
            if (qid_next[j] == qid[i]) {
              boost::add_edge(gate_map[(*I).seq], gate_map[(*I_next).seq], dag);
              found_next = 1;   // set this flag. while loop will end.              
            }
            if(found_next)
              break;            
          }       
          if(found_next)
            break;
          std::advance(I_next,1);
        }          
      }
    }
    assign_criticality(); // assign criticality to dag nodes    
    if (!opt)             // store all dags in table for future use
      all_dags[module_name] = dag;
    else
      all_dags_opt[module_name] = dag;

    update_highest_criticality();

    // find serial completion time
#ifdef _PROGRESS
    cerr << "Calculating SerialCLOCK..." << endl;
#endif     
    unsigned long long serial_clk = 0;
    for (vector<Gate>::const_iterator I = module_gates.begin(); I != module_gates.end(); ++I) {    
      serial_clk += get_gate_latency(*I);     
    }     
    cerr << serial_clk << endl;

    // find critical path
#ifdef _PROGRESS
    cerr << "Calculating CriticalCLOCK..." << endl;
#endif         
    unsigned long long critical_clk = 0;
    critical_clk = get_critical_clk(dag); 
    cerr << critical_clk << endl;

    // update max_crit and max_len
    for (auto g_it_range = vertices(dag); g_it_range.first != g_it_range.second; ++g_it_range.first){
      gate_descriptor g = *(g_it_range.first);
      max_crit = max((int)max_crit, dag[g].criticality);
    }    
    max_len = max(max_len, num_rows+num_cols);

    initialize_ready_list();
   
    Braid braid;    
 
    // find parallel completion time
#ifdef _PROGRESS
    cerr << "Calculating ParallelCLOCK..." << endl;
    unsigned long long prev_remaining_edges = 0;
#endif       
    while ( !event_queues.empty() || !ready_events.empty() ||
            !(num_edges(dag)==0) || !ready_gates.empty() ) {

#ifdef _PROGRESS
      if (clk % 10000 == 0) {
        cerr << "ParallelCLOCK = " << clk << " ..." << endl;        
        cerr << num_edges(dag) << " edges remaining..." << endl;
        if (prev_remaining_edges == num_edges(dag) && num_edges(dag)!=0) {
          cerr << "STUCK -- Terminating..." << endl;
          return 1;
        }
        else
          prev_remaining_edges = num_edges(dag);
      }
#endif         
      // queue events of any ready gate
      auto it_g = ready_gates.begin();
      while (it_g != ready_gates.end()) {
#ifdef _DEBUG
        cout << "In ready_gate: " << dag[*it_g].seq << "\t" << dag[*it_g].op_type << "\t";
        for (auto const &arg : dag[*it_g].qid)
          cout << arg << "\t";
        cout << endl;
#endif        
        if (dag[*it_g].op_type == "CNOT") {
          queue<Event> cnot_events = events_cnot(dag[*it_g].qid[0], dag[*it_g].qid[1], *it_g);
          event_queues[*it_g] = cnot_events;
        }
        if (dag[*it_g].op_type == "H") {
          queue<Event> h_events = events_h(dag[*it_g].qid[0], *it_g);
          event_queues[*it_g] = h_events;
        }   
        if (dag[*it_g].op_type == "T") {
          queue<Event> t_events = events_t(dag[*it_g].qid[0], *it_g);
          event_queues[*it_g] = t_events;
        }            
        it_g = ready_gates.erase(it_g);
      }

      // decrements timer on all events
      // when timer=0, moves from event_queues to ready_events
      increment_clock();
      
      // do any lapsed event 
      bool YX_flag = false;
      bool drop_flag = false;
      if (priority_policy != 0)
        sort(ready_events.begin(), ready_events.end()); // sort by events' priorities
      auto it_e = ready_events.begin();
      while (it_e != ready_events.end()) {
        bool success = do_event(*it_e);
        if (success) {       
          if ( attempts_hist.find((*it_e).attempts) != attempts_hist.end() )
            attempts_hist[(*it_e).attempts]++;
          else
            attempts_hist[(*it_e).attempts] = 1;
          success_events.push_back( make_pair((*it_e).gate,(*it_e).type) );
          // remove it_e from ready_events
          gate_descriptor g = (*it_e).gate;        
          it_e = ready_events.erase(it_e);
          // was last event in its queue: remove node and edge to children        
          if ( event_queues[g].empty() ) {   // slow? 
#ifdef _DEBUG
            cout << "\tgate " << dag[g].seq << " completed." << endl;
#endif
            event_queues.erase(g);
            dag_t::adjacency_iterator neighborIt, neighborEnd;
            boost::tie(neighborIt, neighborEnd) = adjacent_vertices(g, dag);
            for (; neighborIt != neighborEnd; ++neighborIt) {
              gate_descriptor g_out = *neighborIt;
              if(boost::in_degree(g_out, dag) == 1) {
#ifdef _DEBUG                
                cout << "\t\tNext ready_gate: " << dag[g_out].seq << "\t" << dag[g_out].op_type << "\t";
                for (auto const &arg : dag[g_out].qid)
                  cout << arg << "\t";
                cout << endl;              
#endif                
                ready_gates.push_back(g_out);
              }
            }                   
            boost::clear_out_edges(g, dag);          
            assert(in_degree(g,dag) == 0 && out_degree(g,dag) == 0 && "removing gate prematurely from dag.");
            update_highest_criticality();
#ifdef _DEBUG
            cout << "\t\thighest_criticality: " << highest_criticality << endl;            
#endif            
          }
          else {
            // wasn't last event in its queue: set the timer for the next one off the queue
#ifdef _DEBUG
            cout << "\tsetting timer of next event in queue." << endl;
#endif
            event_type t = event_queues[g].front().type;      
            event_queues[g].front().timer = event_timers[t];
          }
        }
        else {
          (*it_e).attempts++;
          if ( (*it_e).attempts > attempt_th_yx && !YX_flag) {
            // deadlock: change route by substituting YX DOR for XY DOR
            // for maximum one event per clock cycle
#ifdef _DEBUG
            print_event(*it_e);
#endif
            if ( (*it_e).type == cnot3 || (*it_e).type == cnot5 ) {
#ifdef _DEBUG              
              cout << "\tYX DOR for above event..." << endl;
#endif
              resolve_cnot(*it_e);
              YX_flag = true;
            }
#ifdef _DEBUG            
            else              
              cout << "\twaiting for above event to resolve itself..." << endl;
#endif
          }
          if ( (*it_e).attempts > attempt_th_drop && !drop_flag ) {
            // deadlock: drop and reinject the entire gate
            // for maximum one event per clock cycle            
            gate_descriptor g = (*it_e).gate;                                    
#ifdef _DEBUG
            cout << "\tdropping gate..." << dag[g].seq << endl;
#endif
            gate_descriptor dropped_gate = (*it_e).gate;
            total_dropped_gates.push_back( dropped_gate );
            if ( find(unique_dropped_gates.begin(), unique_dropped_gates.end(), dropped_gate) == unique_dropped_gates.end() )
              unique_dropped_gates.push_back( dropped_gate );
            purge_gate_from_mesh( dag[g].seq );
            ready_gates.push_back(g);
            event_queues.erase(g);
            it_e = ready_events.erase(it_e);
            drop_flag = true;
            continue;
          }
          pair<gate_descriptor, event_type> conflict_event = make_pair( (*it_e).gate,(*it_e).type );         
          total_conflict_events.push_back( conflict_event );
          if ( find(unique_conflict_events.begin(), unique_conflict_events.end(), conflict_event) == unique_conflict_events.end() )
            unique_conflict_events.push_back( conflict_event );          
          ++it_e;
        }
      }
    }
 
    // print results
    unsigned long long module_freq = 1;
    if ( module_freqs.find(module_name) != module_freqs.end() ) {
      module_freq = module_freqs[module_name];
      cerr << "module_freq: " << module_freq << endl;
    }
    total_cycles += clk*module_freq;
    cerr << "avg_module_mesh_utility: " << avg_module_mesh_utility << endl;

    avg_mesh_utility[module_name] = avg_module_mesh_utility;

    // Results 'BraidFlash'
    br_file << "SerialCLOCK: " << serial_clk * module_freq << endl;    
    br_file << "ParallelCLOCK: " << clk * module_freq << endl;  
    br_file << "CriticalCLOCK: " << critical_clk * module_freq << endl;    
    br_file << "total_success: " << success_events.size() * module_freq << endl;
    br_file << "total_conflict: " << total_conflict_events.size() * module_freq << endl;    
    br_file << "unique_conflict: " << unique_conflict_events.size() * module_freq << endl;
    // Results 'DroppedGates'
    br_file << "total_dropped_gates: " << total_dropped_gates.size() * module_freq << endl;
    br_file << "unique_dropped_gates: " << unique_dropped_gates.size() * module_freq << endl; 
    // Results 'ConflictedAttempts'
    for (auto &i : attempts_hist)
      br_file << "attempt\t" << i.first << "\t" << i.second * module_freq << endl;
    // Results 'MeshUtil'
    br_file << "avg_module_mesh_utility: " << avg_module_mesh_utility << endl;
    br_file << endl;
  }

  //if (opt) {
    // Results 'ManhattanCost'    
    pair< pair<int,int>, pair<int,int> > mcost_ecount;    
    mcost_ecount = compare_manhattan_costs();    
    br_file << "mcost: " << mcost_ecount.first.first << endl;
    br_file << "mcost_opt: " << mcost_ecount.first.second << endl;
    br_file << "event_count: " << mcost_ecount.second.first << endl;
    br_file << "event_count_opt: " << mcost_ecount.second.second << endl; 
    // Results 'BraidHistograms'
    cerr << "braid_length_histogram:" << endl;
    for (int i=0; i<num_bins; i++) {
      cerr << "[" << i*len_binwidth << "," << (i+1)*len_binwidth << ") " << length_hist[i] << endl;
    }
    cerr << "braid_criticality_histogram:" << endl;
    for (int i=0; i<num_bins; i++) {
      cerr << "[" << i*crit_binwidth << "," << (i+1)*crit_binwidth << ") " << criticality_hist[i] << endl;
    }    
  //}

  // Results 'MeshUtil'
  double avg_total_mesh_utility = 0.0;
  int sum_freqs = 0;
  for (auto &i : avg_mesh_utility) {
    avg_total_mesh_utility += module_freqs[i.first]*i.second;
    sum_freqs += module_freqs[i.first];
  } 
  br_file << "avg_total_mesh_utility: " << avg_total_mesh_utility / (double)sum_freqs << endl;
  
  // Results 'Area'
  int max_q_count = 0;
  for (auto &i : all_q_counts)
    if (i.second > max_q_count) max_q_count = i.second;
  int hole_side = 2*ceil(code_distance/4.0) + 1;
  int width_channel = hole_side;
  int hole_to_channel = 2*ceil(code_distance/2.0);
  int length_tile = 2*hole_side + width_channel + 4*hole_to_channel - 6;
  int width_tile = hole_side + 2*hole_to_channel - 2;
  int area_tile_plus = (width_tile + width_channel) * (length_tile + width_channel);
  int num_physical_qubits = 
    max_q_count*area_tile_plus 
    + sqrt(max_q_count)*(width_channel*(width_channel+length_tile))
    + sqrt(max_q_count)*(width_channel*(width_channel+width_tile)) 
    + width_channel*width_channel;
  br_file << "code_distance(d): " << code_distance << endl;
  br_file << "num_logical_qubits: " << max_q_count << endl;
  br_file << "num_physical_qubits: " << num_physical_qubits << endl;

  // KQ: total number of logical gates
  // k: total number of physical timesteps
  // q: total number of physical qubits
  string kq_file_path; 
  ofstream kq_file;      
  kq_file_path = output_dir+benchmark_name
                    +".p."+(to_string(P_error_rate))
                    +".yx."+to_string(attempt_th_yx)
                    +".drop."+to_string(attempt_th_drop)    
                    +".pri."+to_string(priority_policy)                    
                    +"."+tech
                    +(opt ? ".opt.kq" : ".kq");
  kq_file.open(kq_file_path);
  kq_file << "error rate: " << "10^-" << P_error_rate << endl;
  kq_file << "code distance: " << code_distance << endl;
  kq_file << "total cycles: " << surface_code_cycle * total_cycles << endl;
  kq_file << "max qubits: " << num_physical_qubits << endl;
  kq_file << "logical KQ: " << total_logical_gates << endl;
  kq_file << "physical kq: " << (surface_code_cycle*total_cycles) * num_physical_qubits << endl;  
  kq_file.close();  

  cerr << "kq report written to:\n" << " \t" << kq_file_path << endl; 
  
  br_file << "\t****** FINISHED SIMULATION *******" << endl;
  br_file.close();

  cerr << "braid report written to:\n" << " \t" << br_file_path << endl;     

  vis_file.close();    
  if (visualize_mesh)  {
    cerr << "network visualizations written to:\n" << " \t" << vis_file_path << endl;     
  }

  return 0;
}
