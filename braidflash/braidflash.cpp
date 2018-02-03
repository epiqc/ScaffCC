//===-------------------------- braidflash.cpp ----------------------------===//
// This file simulates flash (not hop-by-hop) braid space occupancies 
// on the surface code mesh.
//
//                             Ali Javadi-Abhari
//                    Princeton University / IBM Research
//                                 2016-2018
//
//===----------------------------------------------------------------------===//

/*******************************************************************************
                                    Usage
********************************************************************************
$ braidflash [QASM_FILE] --config [CFG_FILE]

$ braidflash [QASM_FILE] [OPTIONS]
  --help      display this help and exit
  --version   output version information and exit
  --tech      tech [sup, ion, dot] (default: sup)
  --p         physical error rate (10^-p) [int] (default: 5)  
  --opt       optimize logical tiles layout? (default: none)
  --yx        stall threshold to switch DOR routing from xy to yx [int] 
              (default: 8)
  --drop      stall threshold to drop entire operation and reinject [int] 
              (default: 20)
  --pri       braid priority policy [0-6] (default: 0)
  --visualize show network state at each cycle (default: none)
              [Warning: only use on small circuits]
*******************************************************************************/

#define VERSION "2.0"

#include <iostream>   //std::cout, std::cin
#include <fstream>    //std::ifstream
#include <utility>    //std::pair
#include <vector>     //std::vector
#include <list>       //std::list
#include <algorithm>  //std::erase, std::find, std::sort
#include <numeric>    //std::accumulate
#include <queue>      //std::queue
#include <stdlib.h>   //system
#include <cstdlib>
#include <cmath>      //sqrt, pow 
#include <time.h>     //clock
#include <sstream>    //std::stringstream
#include <map>
#include <unordered_map>
#include <stack>
#include <cstring>    //strcmp
#include <iterator>
#include <limits.h>
#include <limits>     //std::numeric_limits
#include <unistd.h>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/reverse_graph.hpp>
#include <boost/graph/copy.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/program_options.hpp>
#include <boost/any.hpp>

using namespace std;


/*******************************************************************************
                                Optional Flags
*******************************************************************************/

//#define _DEBUG    // optional: debug flag
#define _PROGRESS   // optional: progress flag


/*******************************************************************************
                               Simulator Inputs
*******************************************************************************/
// input file
vector<string> input_files;

// physical device characteristics
struct tech_t {                // backend tech: sup, ion, dot
  public:
  string name;    
  tech_t(string const& val): name(val){}
  tech_t(): name(""){}
  private:
  friend ostream& operator<< (ostream &os, const tech_t& t) {
    return os << t.name;
  }
};
tech_t tech;
double P_error_rate;           // device error rate parameter
double short_Y_error;          // inject error for short Y states
double short_A_error;          // inject error for short A states

// surface code characteristics
double P_th;                   // surface code threshold = 10^-2
double acceptable_epsilon;     // total acceptable accumulated logical error

// magic state distillation
bool periphery;                       // factories on periphery or internal?
struct factory_design_t {             // design: bravyi-haah, reed-muller
  public:
  string name;    
  factory_design_t(string const& val): name(val){}
  factory_design_t(): name(""){}
  private:
  friend ostream& operator<< (ostream &os, const factory_design_t& fd) {
    return os << fd.name;
  }  
};
factory_design_t factory_design;             
//TODO: modular
unsigned rows_to_Y_factory_ratio; // # Y-factories/lattice side (left/right)
unsigned cols_to_A_factory_ratio; // # A-factories/lattice side (up/down)
unsigned num_Y_factories;         // count (X)
unsigned num_A_factories;         // count (X)
unsigned Y_factory_capacity;      // capacity (K)
unsigned A_factory_capacity;      // capacity (K)
bool replaceS;                    // replace S with 2 Ts to avoid Y factories?

// optimization policy
bool optimize_layout;          // optimize logical qubit tiles layout?
unsigned priority_policy;      // 0: no priorities. in program order.
                               // 1: criticality only.
                               // 2: braid length only. short2long.
                               // 3: braid length only. long2short.
                               // 4: close2open only.
                               // 5: crticiality + short2long + close2open
                               // 6: criticality + short2long (highest crit) 
                               //    + long2short (lower crit) + close2open

// deadlock resolution
unsigned attempt_th_yx;    // when to switch DOR route? evaluated first.
unsigned attempt_th_drop;  // when to drop & reinject entire operation? 
                               // evaluated second.

// outputs
bool visualize_mesh;           // print the network state at every cycle?

// inputs
string config_file;            // .cfg file to use

// tech option validator
void validate(boost::any& v, 
              const std::vector<std::string> values,
              tech_t*,
              int)
{
  namespace po = boost::program_options;

  // Make sure no previous assignment to 'v' was made.
  po::validators::check_first_occurrence(v);

  // Extract the first string from 'values'. If there is more than
  // one string, it's an error, and exception will be thrown.
  string const& s = po::validators::get_single_string(values);

  if (s == "sup" || s == "ion" || s == "dot") {
    v = boost::any(tech_t(s));
  } else {
    throw po::validation_error(po::validation_error::invalid_option_value);
  }
}
// factory_design option validator
void validate(boost::any& v, 
              const std::vector<std::string> values,
              factory_design_t*,
              int)
{
  namespace po = boost::program_options;

  // Make sure no previous assignment to 'v' was made.
  po::validators::check_first_occurrence(v);

  // Extract the first string from 'values'. If there is more than
  // one string, it's an error, and exception will be thrown.
  string const& s = po::validators::get_single_string(values);

  if (s == "bravyi-haah" || s == "reed-muller") {
    v = boost::any(factory_design_t(s));
  } else {
    throw po::validation_error(po::validation_error::invalid_option_value);
  }
}


/*******************************************************************************
                        Global Variables and Data Structures
*******************************************************************************/

ofstream vis_file;             // visualization output file

// Derived distillation parameters
double L_error_rate;           // desired logical error rate
unsigned code_distance;        // coding distance of the surface code
unsigned distillation_level_Y; // distillation (L)
unsigned distillation_level_A; // distillation (L)
unsigned single_Y_area;
unsigned single_Y_latency;
unsigned single_Y_ports;
unsigned single_A_area;
unsigned single_A_latency;
unsigned single_A_ports;
//map< string, unsigned > num_Y_factories_v;  // TODO: modular
//map< string, unsigned > num_A_factories_v;  // TODO: modular

// Physical operation latencies -- also determines surface code cycle length
std::unordered_map<std::string, int> op_delays_ion; 
std::unordered_map<std::string, int> op_delays_sup; 
std::unordered_map<std::string, int> op_delays_dot; 
unsigned surface_code_cycle_ion, surface_code_cycle_sup, surface_code_cycle_dot;
unsigned surface_code_cycle;

// Note: this is for logical qubit layouts.
// the number of router/nodes is one larger in both row and column
unsigned num_rows;
unsigned num_cols;
//hack
unsigned num_rows_Y_factory;
unsigned num_cols_Y_factory;
unsigned num_rows_A_factory;
unsigned num_cols_A_factory;

// global clock cycle
unsigned long long clk;
unsigned long long total_serial_cycles;
unsigned long long total_parallel_cycles;
unsigned long long total_critical_cycles;
unsigned long long gate_complete_count;

// Mesh:
struct Node {
  unsigned owner;
  Node() : owner(0) {}
};
struct Link {
  unsigned owner;
  Link() : owner(0) {}
};
typedef boost::adjacency_list<boost::setS, boost::vecS, boost::undirectedS, 
                              Node, Link> mesh_t;
typedef mesh_t::vertex_descriptor node_descriptor;
typedef mesh_t::edge_descriptor link_descriptor;
map<unsigned, node_descriptor> node_map;
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
  unsigned seq;
  string op_type;  
  vector<unsigned> qid; 
  int criticality;
  Gate(unsigned seq=0, string op_type="CNOT", 
       vector<unsigned> qid={0,0}, int criticality=-1) : 
    seq(seq), op_type(op_type), qid(qid), criticality(criticality) {}
};
map<string, unsigned> gate_latencies; 

// dag: directed acyclic graph of gate dependencies
typedef boost::adjacency_list<boost::setS, boost::vecS, boost::bidirectionalS, 
                              Gate> dag_t;
typedef dag_t::vertex_descriptor gate_descriptor;
typedef dag_t::edge_descriptor dependency_descriptor;
map<unsigned, gate_descriptor> gate_map;
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
  int timer;              // -1: invalid timer. 0: ready. >0: counting down.
  unsigned attempts;      // number of attempts to complete event
  unsigned policy;        // determines comparison of Events
  
  Event(Braid braid, bool close_open, gate_descriptor gate, 
        event_type type, int timer=-1, unsigned attempts=0, unsigned policy=6): 
    braid(braid), close_open(close_open), gate(gate), 
    type(type), timer(timer), attempts(attempts){}

  bool operator< (const Event &other) const { // determining event priority
    bool res = false;
      // 0: no priorities. in program order.    
    switch (policy) {
      // 1: criticality only.      
      case 1: res = (dag[gate].criticality > dag[other.gate].criticality); break;
      // 2: braid length only. short2long.              
      case 2: res = (braid.links.size() < other.braid.links.size()); break;
      // 3: braid length only. long2short.              
      case 3: res = (braid.links.size() > other.braid.links.size()); break;
      // 4: close2open only.              
      case 4: res = (close_open==false && other.close_open==true); break;
      // 5: close2open + crticiality + short2long              
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
      // 6: close2open + criticality + 
      //    short2long (highest crit) + long2short (lower crit)              
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
    case t1: cout << "t1"; break;             
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

// data structures to keep track of events, gates, qubit names, module freqs
map< string, vector<Gate> > all_gates;
map< string, vector<Gate> > all_gates_opt;
map< string, dag_t> all_dags;
map< string, dag_t> all_dags_opt;
map< string, unsigned > all_q_counts;
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
map< unsigned, unsigned > attempts_hist;
map< unsigned, unsigned > criticality_hist;
map< unsigned, unsigned > length_hist;
map< unsigned, unsigned > criticality_hist_opt;
map< unsigned, unsigned > length_hist_opt;
unsigned max_crit = 0;
unsigned max_len = 0;
unsigned num_bins = 20;
unsigned len_binwidth = 0;    
unsigned crit_binwidth = 0;  

// mesh utility
double avg_module_mesh_utility = 0.0;
map< string, double> avg_mesh_utility;


/*******************************************************************************
                                Parsing Functions
*******************************************************************************/

void argparse (int argc, char *argv[]) {

  // parse program options
  namespace po = boost::program_options;

  // hidden options; command line & config file
  po::options_description hidden("Hidden options");
  hidden.add_options()
    ("input_files", po::value< vector<string> >(&input_files), "input file(s)")
    ;     
  // command line only options
  po::options_description generic("");
  generic.add_options()
    ("version,v", "version info")
    ("help,h", "print help message and exit")
    ("config,c", po::value<string>(&config_file)->default_value("config.cfg"),
     "configuration file to use.")    
    ;  
  // command line & config file options
  po::options_description config("Configuration options");
  config.add_options()
    ("tech", po::value<tech_t>(&tech)->value_name("technology")->default_value(tech_t("sup")), 
     "tech [sup, ion, dot]")
    ("p", po::value<double>(&P_error_rate)->value_name("P_error_rate")->default_value(0.00001, "0.00001"), 
     "physical error rate")
    ("injectY", po::value<double>(&short_Y_error)->value_name("short_Y_error")->default_value(0.005, "0.005"), 
     "Y-state injection error rate")
    ("injectA", po::value<double>(&short_A_error)->value_name("short_A_error")->default_value(0.005, "0.005"), 
     "A-state injection error rate")
    ("pth", po::value<double>(&P_th)->value_name("P_th")->default_value(0.01, "0.01"), 
     "surface code threshold error")
    ("eps", po::value<double>(&acceptable_epsilon)->value_name("acceptable_epsilon")->default_value(0.5), 
     "acceptable total logical err")
    ("periphery", po::bool_switch(&periphery)->default_value(false), 
     "factories on periphery or internal?")
    ("factory", po::value<factory_design_t>(&factory_design)->value_name("factory_design")->default_value(factory_design_t("bravyi-haah")), 
     "factory [bravyi-haah, reed-muller]")
    ("xY", po::value<unsigned>(&num_Y_factories)->value_name("num_Y_factories")->default_value(1), 
     "number of Y factories")
    ("xA", po::value<unsigned>(&num_A_factories)->value_name("num_A_factories")->default_value(1), 
     "number of A factories")    
    ("kY", po::value<unsigned>(&Y_factory_capacity)->value_name("Y_factory_capacity")->default_value(1), 
     "total output capacity of Y factories")
    ("kA", po::value<unsigned>(&A_factory_capacity)->value_name("A_factory_capacity")->default_value(1), 
     "total output capacity of A factories")    
    ("replaceS", po::bool_switch(&replaceS)->default_value(false), 
     "replace S gates with 2 Ts?")    
    ("opt", po::bool_switch(&optimize_layout)->default_value(true), 
     "optimize logical tiles layout?")
    ("pri", po::value<unsigned>(&priority_policy)->value_name("priority_policy")->default_value(6), 
     "braid priority policy to use")
    ("yx", po::value<unsigned>(&attempt_th_yx)->value_name("attempt_th_yx")->default_value(8), 
     "threshold to switch route (xy -> yx)")
    ("drop", po::value<unsigned>(&attempt_th_drop)->value_name("attempt_th_drop")->default_value(20), 
     "threshold to drop and reinject")
    ("visualize", po::bool_switch(&visualize_mesh)->default_value(false), 
     "show network state at each cycle \n[Warning: only use on small circuits]")
    ;

  po::options_description cmdline_options;
  cmdline_options.add(generic).add(config).add(hidden);

  po::options_description config_file_options;
  config_file_options.add(config).add(hidden);

  po::options_description visible_options(
      "Usage: braidflash [QASM_FILE(S)] [OPTIONS]\n"
      "Simulate braid space occupancies on the surface code mesh.");
  visible_options.add(generic).add(config);

  po::positional_options_description pos_op;
  pos_op.add("input_files", -1);

  po::variables_map vm;
  store(po::command_line_parser(argc, argv).
      options(cmdline_options).positional(pos_op).run(), vm);
  notify(vm);

  ifstream ifs(config_file.c_str());
  if (!ifs) {
    cout << "cannot open config file: " << config_file << "\n";
    exit(1);
  }
  else
  {
    store(parse_config_file(ifs, config_file_options), vm);
    notify(vm);
  }
            
  if (vm.count("help")) {
    cout << visible_options << "\n";
    exit(0);
  }

  if (vm.count("version")) {
    cout << "Braidflash (ScaffCC Compiler Infrastructure): VERSION " 
      << VERSION << ".\n";
    exit(0);
  }  

  assert(input_files.size() > 0 && "Error: no input file specified.\n");
  cout<<"\n-----------------------------------------------";
  cout<<"\n---         Simulator Characteristics       ---";
  cout<<"\n-----------------------------------------------"<<endl;  
  for (auto &i : input_files)
    cout << "Input file(s): " << i << endl;  
  cerr << "Technology: " << tech << endl;
  cout << "Physical error: " << P_error_rate << endl;
  cout << "Y-state injection error: " << short_Y_error << endl;
  cout << "A-state injection error: " << short_A_error << endl;
  cout << "Threshold error: " << P_th << endl;
  cout << "Acceptable epsilon: " << acceptable_epsilon << endl;
  cout << "Periphery?: " << periphery << endl;
  cout << "Factory design: " << factory_design << endl;
  cout << "Num Y factories (X_Y): " << num_Y_factories << endl;
  cout << "Num A factories (X_A): " << num_A_factories << endl;
  cout << "total Y factory capacity (K_Y): " << Y_factory_capacity << endl;
  cout << "total A factory capacity (K_A): " << A_factory_capacity << endl;
  cout << "replace S?: " << replaceS << endl;  
  cout << "Optimize layout?: " << optimize_layout << endl;
  cout << "Priority policy: " << priority_policy << endl;
  cout << "threshold for xy->yx rerouting: " << attempt_th_yx << endl;
  cout << "threshold for drop and reinject: " << attempt_th_drop << endl;
  cout << "visualize mesh?: " << visualize_mesh << endl;
}

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
  unsigned seq = 1;      
  unsigned long long module_q_count = 0;    
  map<string, unsigned long long> q_name_to_num;            
  vector<Gate> module_gates; 
  const char* all_ops[] = {
    "PrepZ ", "X ", "Z ", "H ", "CNOT ", "T ", "Tdag ", "S ", "Sdag ", "MeasZ "};  
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
        string op_type = elems[1];        
        vector<unsigned> qid;        
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
        // assume X and Z gates are done in software
        if (op_type == "CNOT" || op_type == "H" 
            || op_type == "T" || op_type == "Tdag"
            || op_type == "S" || op_type == "Sdag") { 
          // for simplicity (not having 2 factory types)
          // replace S gates with two T gates
          if (replaceS) {
            if (op_type == "S") {
              Gate tg1 = Gate(seq++, "T", qid);
              Gate tg2 = Gate(seq++, "T", qid);
              module_gates.push_back(tg1);
              module_gates.push_back(tg2);
              continue;
            }
            if (op_type == "Sdag") {
              Gate tg1 = Gate(seq++, "Tdag", qid);
              Gate tg2 = Gate(seq++, "Tdag", qid);
              module_gates.push_back(tg1);
              module_gates.push_back(tg2);
              continue;
            }
          }
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
        unsigned seq = (unsigned)stol(elems[1]);
        string op_type = elems[3];
        vector<unsigned> qid;
        qid.push_back( (unsigned)stol(elems[5]) );
        if (elems.size() > 6)
          qid.push_back( (unsigned)stol(elems[7]) );
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


/*******************************************************************************
                                Helper Functions
*******************************************************************************/

// get surface code cycle latency (normalized to single-qubit gate latency) 
// based on technology parameters
unsigned set_surface_code_cycle (tech_t tech) {
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

  op_delays_dot["PrepZ"] = 1;
  op_delays_dot["X"] = 1;
  op_delays_dot["Z"] = 1;
  op_delays_dot["H"] = 1;
  op_delays_dot["CNOT"] = 600;
  op_delays_dot["T"] = 1;
  op_delays_dot["Tdag"] = 1;
  op_delays_dot["S"] = 1;
  op_delays_dot["Sdag"] = 1;
  op_delays_dot["MeasZ"] = 20;
  
  surface_code_cycle_ion = op_delays_ion.find("PrepZ")->second + 
                       2*op_delays_ion.find("H")->second + 
                       4*op_delays_ion.find("CNOT")->second + 
                       op_delays_ion.find("MeasZ")->second;
  surface_code_cycle_sup = op_delays_sup.find("PrepZ")->second + 
                       2*op_delays_sup.find("H")->second + 
                       4*op_delays_sup.find("CNOT")->second + 
                       op_delays_sup.find("MeasZ")->second;
  surface_code_cycle_dot = op_delays_dot.find("PrepZ")->second + 
                       2*op_delays_dot.find("H")->second + 
                       4*op_delays_dot.find("CNOT")->second + 
                       op_delays_dot.find("MeasZ")->second;

  if (tech.name=="ion") return surface_code_cycle_ion;
  else if (tech.name=="sup") return surface_code_cycle_sup;
  else if (tech.name=="dot") return surface_code_cycle_dot;
  else {cerr << "Error: Unknown tech.\n"; exit(1);}
}

// find the diagonal node with respect to qubit_num
unsigned find_diagonal(unsigned qubit_num, unsigned node) {
  unsigned const top_left_node = qubit_num+(qubit_num/num_cols);
  unsigned const top_right_node = qubit_num+(qubit_num/num_cols)+1;
  unsigned const bottom_left_node = qubit_num+(qubit_num/num_cols)+num_cols+1;
  unsigned const bottom_right_node = qubit_num+(qubit_num/num_cols)+num_cols+2;
  unsigned result = 0;
  if      (node == top_left_node)     result = bottom_right_node;
  else if (node == top_right_node)    result = bottom_left_node;
  else if (node == bottom_left_node)  result = top_right_node;
  else if (node == bottom_right_node) result = top_left_node;                        
  return result;
}

// find the vertical node with respect to qubit_num
unsigned find_vertical(unsigned qubit_num, unsigned node) {
  unsigned const top_left_node = qubit_num+(qubit_num/num_cols);
  unsigned const top_right_node = qubit_num+(qubit_num/num_cols)+1;
  unsigned const bottom_left_node = qubit_num+(qubit_num/num_cols)+num_cols+1;
  unsigned const bottom_right_node = qubit_num+(qubit_num/num_cols)+num_cols+2;
  unsigned result = 0;
  if      (node == top_left_node)     result = bottom_left_node;
  else if (node == top_right_node)    result = bottom_right_node;
  else if (node == bottom_left_node)  result = top_left_node;
  else if (node == bottom_right_node) result = top_right_node;                        
  return result;
}
// find the horizontal node with respect to qubit_num
unsigned find_horizontal(unsigned qubit_num, unsigned node) {
  unsigned const top_left_node = qubit_num+(qubit_num/num_cols);
  unsigned const top_right_node = qubit_num+(qubit_num/num_cols)+1;
  unsigned const bottom_left_node = qubit_num+(qubit_num/num_cols)+num_cols+1;
  unsigned const bottom_right_node = qubit_num+(qubit_num/num_cols)+num_cols+2;
  unsigned result = 0;
  if      (node == top_left_node)     result = top_right_node;
  else if (node == top_right_node)    result = top_left_node;
  else if (node == bottom_left_node)  result = bottom_right_node;
  else if (node == bottom_right_node) result = bottom_left_node;                        
  return result;
}

// find which corner of qubit_num is closest router node to src_node
unsigned find_nearest(unsigned qubit_num, unsigned src_node) {
  unsigned top_left_node = qubit_num+(qubit_num/num_cols);
  unsigned top_right_node = qubit_num+(qubit_num/num_cols)+1;
  unsigned bottom_left_node = qubit_num+(qubit_num/num_cols)+num_cols+1;
  unsigned bottom_right_node = qubit_num+(qubit_num/num_cols)+num_cols+2;
  unsigned qubit_top_left_row = qubit_num / num_cols;
  unsigned qubit_top_left_col = qubit_num % num_cols;
  unsigned src_row = src_node / (num_cols+1);
  unsigned src_col = src_node % (num_cols+1);
  unsigned result = 0;
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

bool are_adjacent(unsigned src_qubit, unsigned dest_qubit) {
  bool result = false;
  unsigned src_row = src_qubit / num_cols;
  unsigned src_col = src_qubit % num_cols;  
  unsigned dest_row = dest_qubit / num_cols;
  unsigned dest_col = dest_qubit % num_cols;    
  if (src_row == dest_row && (max(src_col,dest_col)-min(src_col,dest_col) == 1))
    result = true;
  else
    result = false;
  return result;
}

// merge the nodes and links of two braids
Braid braid_merge(Braid braid1, Braid braid2) {
  vector<node_descriptor> combined_nodes;
  combined_nodes.reserve(braid1.nodes.size() + braid2.nodes.size());
  combined_nodes.insert(combined_nodes.end(), braid1.nodes.begin(), braid1.nodes.end());
  combined_nodes.insert(combined_nodes.end(), braid2.nodes.begin(), braid2.nodes.end());
  
  vector<link_descriptor> combined_links;
  combined_links.reserve(braid1.links.size() + braid2.links.size());
  combined_links.insert(combined_links.end(), braid1.links.begin(), braid1.links.end());
  combined_links.insert(combined_links.end(), braid2.links.begin(), braid2.links.end());
  
  return Braid(combined_nodes, combined_links);
}

// make an 'L' around qubit_num, starting from src_node.
// 'short L' means do the short part first then long part
Braid braid_short_L (unsigned qubit_num, unsigned src_node) {
  Braid short_L_route;  // return this
  unsigned top_left_node = qubit_num+(qubit_num/num_cols);
  unsigned top_right_node = qubit_num+(qubit_num/num_cols)+1;
  unsigned bottom_left_node = qubit_num+(qubit_num/num_cols)+num_cols+1;
  unsigned bottom_right_node = qubit_num+(qubit_num/num_cols)+num_cols+2;
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
Braid braid_S(unsigned qubit_num, unsigned src_node) {
  Braid S_route;  // return this
  unsigned top_left_node = qubit_num+(qubit_num/num_cols);
  unsigned top_right_node = qubit_num+(qubit_num/num_cols)+1;
  unsigned bottom_left_node = qubit_num+(qubit_num/num_cols)+num_cols+1;
  unsigned bottom_right_node = qubit_num+(qubit_num/num_cols)+num_cols+2;
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
Braid braid_dor (unsigned src_node, unsigned dest_node, bool YX) {
  Braid dor_route; // return this
  
  unsigned src_row = src_node / (num_cols+1);
  unsigned src_col = src_node % (num_cols+1);
  unsigned dest_row = dest_node / (num_cols+1);
  unsigned dest_col = dest_node % (num_cols+1);
  
  int row_dir = (src_row < dest_row) ? 1 : -1;
  int col_dir = (src_col < dest_col) ? 1 : -1;

  if (YX) { // do YX  
    while (src_col != dest_col) {
      src_col += col_dir; // move 1 col closer
      unsigned src_node_next = src_row*(num_cols+1)+src_col; // update src_node
      dor_route.nodes.push_back( node_map[src_node_next] );
      auto e1 = edge(node_map[src_node], node_map[src_node_next], mesh);
      dor_route.links.push_back(e1.first);
      src_node = src_node_next;    
    }
    while (src_row != dest_row) {
      src_row += row_dir; // move 1 row closer
      unsigned src_node_next = src_row*(num_cols+1)+src_col; // update src_node
      dor_route.nodes.push_back( node_map[src_node_next] ); 
      auto e1 = edge(node_map[src_node], node_map[src_node_next], mesh);
      dor_route.links.push_back(e1.first);
      src_node = src_node_next;
    }    
  }

  else {  // do XY
    while (src_row != dest_row) {
      src_row += row_dir; // move 1 row closer
      unsigned src_node_next = src_row*(num_cols+1)+src_col; // update src_node
      dor_route.nodes.push_back( node_map[src_node_next] ); 
      auto e1 = edge(node_map[src_node], node_map[src_node_next], mesh);
      dor_route.links.push_back(e1.first);
      src_node = src_node_next;
    }
    while (src_col != dest_col) {
      src_col += col_dir; // move 1 col closer
      unsigned src_node_next = src_row*(num_cols+1)+src_col; // update src_node
      dor_route.nodes.push_back( node_map[src_node_next] );
      auto e1 = edge(node_map[src_node], node_map[src_node_next], mesh);
      dor_route.links.push_back(e1.first);
      src_node = src_node_next;        
    }
  }
  
  return dor_route;
}

pair<unsigned,unsigned> cnot_ancillas(unsigned src_qubit, unsigned dest_qubit) {
  unsigned anc1, anc2;    
  // four corners of the src and dest qubits
  unsigned src_top_left = src_qubit+(src_qubit/num_cols);
  unsigned src_top_right = src_qubit+(src_qubit/num_cols)+1;
  unsigned src_bottom_left = src_qubit+(src_qubit/num_cols)+num_cols+1;
  unsigned src_bottom_right = src_qubit+(src_qubit/num_cols)+num_cols+2;
  // qubit's (primal hole pair's) row and column
  unsigned src_row = src_qubit / num_cols;
  unsigned dest_row = dest_qubit / num_cols;
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

pair<Braid,Braid> cnot_routes (unsigned src_qubit, unsigned dest_qubit, unsigned anc1, bool YX=0) { 
  Braid cnot_route_1, cnot_route_2;
  cnot_route_1.nodes.clear(); cnot_route_1.links.clear();  
  cnot_route_2.nodes.clear(); cnot_route_2.links.clear();   

  if ( are_adjacent(src_qubit,dest_qubit) ) {
    unsigned middle_top = find_horizontal(src_qubit, anc1);
    unsigned middle_bottom = find_diagonal(src_qubit, anc1);
    unsigned dest_bottom = find_horizontal(dest_qubit, middle_bottom);

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
    unsigned diag_anc1 = find_diagonal(src_qubit, anc1);
    unsigned nearest_dest_node = find_nearest(dest_qubit, diag_anc1);

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
    unsigned diag_nearest_dest_node = find_diagonal(dest_qubit, nearest_dest_node);
    Braid short_L_section_1 = braid_short_L(dest_qubit, diag_nearest_dest_node);
    // dor to node at long edge away from anc1
    unsigned vertical_anc1 = find_vertical(src_qubit, anc1);
    Braid dor_section_2 = braid_dor(nearest_dest_node, vertical_anc1, YX);
    // 'S' braid through the source
    Braid S_section_3 = braid_S(src_qubit, vertical_anc1);
    // merge the braid segments
    cnot_route_2 = braid_merge(short_L_section_1, dor_section_2);
    cnot_route_2 = braid_merge(cnot_route_2, S_section_3);  
  }

  return make_pair(cnot_route_1, cnot_route_2);
}

queue<Event> events_cnot(unsigned src_qubit, unsigned dest_qubit, gate_descriptor gate) {
  // return this
  queue<Event> cnot_events;
  // two routes are used in a cnot
  Braid cnot_anc_route;
  Braid cnot_route_1;
  Braid cnot_route_2; 
  unsigned anc1, anc2;

  pair<unsigned,unsigned> anc1_anc2 = cnot_ancillas(src_qubit, dest_qubit);
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
  cnot_events.push( Event(cnot_anc_route, 1, gate, cnot1, 1, 0, priority_policy) );

  // queue event cnot2: closing ancilla link after 1 cycle
  node_descriptor n_anc1 = cnot_anc_route.nodes.back();
  //cnot_anc_route.nodes.pop_back();                       //del
  //node_descriptor n_anc2 = cnot_anc_route.nodes.back();
  //cnot_anc_route.nodes.pop_back();  
  cnot_events.push( Event(cnot_anc_route, 0, gate, cnot2, -1, 0, priority_policy) );  

  // queue event cnot3: opening route_1 after 1 cycle
  cnot_route_1.nodes.push_back(n_anc1);                    //add
  cnot_events.push( Event(cnot_route_1, 1, gate, cnot3, -1, 0, priority_policy) );  
  
  // queue event cnot4: closing route_1 after 1 cycle
  cnot_route_1.nodes.pop_back();                           //add
  node_descriptor n_last = cnot_route_1.nodes.back();      //del
  //cnot_route_1.nodes.pop_back();                         //del
  cnot_route_1.nodes.push_back(n_anc1);                    //del
  cnot_events.push( Event(cnot_route_1, 0, gate, cnot4, -1, 0, priority_policy) );
  
  // queue event cnot5: opening route_2 after minimum d-1 cycles
  cnot_route_2.nodes.push_back(n_last);                    //add
  //cnot_route_2.nodes.pop_back();                         //del
  cnot_events.push( Event(cnot_route_2, 1, gate, cnot5, -1, 0, priority_policy) );    
  
  // queue event cnot6: closing route_2 after 1 cycle
  //cnot_route_2.nodes.push_back(n_last);                  //del
  link_descriptor l_anc = cnot_route_2.links.back();
  cnot_route_2.links.pop_back();
  cnot_events.push( Event(cnot_route_2, 0, gate, cnot6, -1, 0, priority_policy) ); 
  
  // queue event cnot7: closing ancillas after minimum d-1 cycles
  cnot_anc_route.links.pop_back();
  cnot_anc_route.links.push_back(l_anc);
  //cnot_anc_route.nodes.push_back(n_anc2);
  cnot_events.push( Event(cnot_anc_route, 0, gate, cnot7, -1, 0, priority_policy) );  
  // return events queue
  return cnot_events;
}

queue<Event> events_h(unsigned src_qubit, gate_descriptor gate) {
  // return this
  queue<Event> h_events;
  // only the side (long) links are busy in an h
  Braid h_route;
  /* temporarily disable right/left link occupation of H gate to increase parallelism  
  // four corners of the src and dest qubits  
  unsigned src_top_left = src_qubit+(src_qubit/num_cols);
  unsigned src_top_right = src_qubit+(src_qubit/num_cols)+1;
  unsigned src_bottom_left = src_qubit+(src_qubit/num_cols)+num_cols+1;
  unsigned src_bottom_right = src_qubit+(src_qubit/num_cols)+num_cols+2;
  // right link
  link_descriptor left_link = edge(node_map[src_top_left], node_map[src_bottom_left], mesh).first;
  link_descriptor right_link = edge(node_map[src_top_right], node_map[src_bottom_right], mesh).first;
  // merge the braid segments
  h_route.links.push_back(left_link);
  h_route.links.push_back(right_link);
  */
  // queue event: opening side links immediately
  h_events.push( Event(h_route, 1, gate, h1, 1, 0) );
  // queue event: closing it after the gate duration is over  
  h_events.push( Event(h_route, 0, gate, h2, -1, 0) );  
  // return events queue
  return h_events;  
}

queue<Event> events_t(unsigned src_qubit, gate_descriptor gate) {
  // return this
  queue<Event> t_events;
  // only a local measurement along the Z axis for the T gate, no nodes and links become busy.
  t_events = queue<Event>();
  return t_events;
}

// print a specific top-left portion of the mesh status
void print_2d_mesh(unsigned max_rows, unsigned max_cols) {
  vis_file << "CLOCK: " << clk << endl;    
  //if (clk % 100 != 0) return; // print more intermittently
  // in case requested printing size is larger that the mesh
  if (max_rows > num_rows+1)
    max_rows = num_rows+1;
  if (max_cols > num_cols+1)
    max_cols = num_cols+1;
  // print row by row
  for (unsigned r=0; r<max_rows; r++) {
    for (unsigned c=0; c<max_cols; c++) {
      unsigned node_num = r*(num_cols+1)+c;
      vis_file << node_num << '(' << ( (mesh[node_map[node_num]].owner)?'*':' ' ) << ')' << "\t\t";
      // horizontal links
      if (c != max_cols-1) {
        auto e = edge(node_map[node_num], node_map[node_num+1], mesh);
        vis_file << "--(" << ( (mesh[e.first].owner)?'*':' ' ) << ")" << "\t\t\t";
      }
    }
    vis_file << "\n\n\n";
    // vertical links
    for (unsigned c=0; c<max_cols; c++) {
      unsigned node_num = r*(num_cols+1)+c;
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
  for (unsigned r = 0; r<max_rows; r++) {
    for (unsigned c = 0; c<max_cols; c++) {
      unsigned node_num = r*(num_cols+1)+c;
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
    for (unsigned c=0; c<max_cols; c++) {
      unsigned node_num = r*(num_cols+1)+c;
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

unsigned get_gate_latency (Gate g) {
  unsigned result = 0;
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

unsigned manhattan_cost(unsigned src_qubit, unsigned dest_qubit) {
  // qubit's (primal hole pair's) row and column
  unsigned src_row = src_qubit / num_cols;
  unsigned src_col = src_qubit % num_cols;  
  unsigned dest_row = dest_qubit / num_cols;
  unsigned dest_col = dest_qubit % num_cols;   
  unsigned row_dist = max(src_row,dest_row) - min(src_row,dest_row);
  unsigned col_dist = max(src_col,dest_col) - min(src_col,dest_col);
  return (row_dist + col_dist); 
}

pair< pair<int,int>, pair<int,int> > compare_manhattan_costs () {
  pair< pair<int,int>, pair<int,int> > result;
  unsigned mcost = 0;
  unsigned mcost_opt = 0;
  unsigned event_count = 0;
  unsigned event_count_opt = 0;
  crit_binwidth = max_crit/num_bins;   
  len_binwidth = max_len/num_bins;
  if (crit_binwidth==0) crit_binwidth = 1;  
  if (len_binwidth==0) len_binwidth = 1;   
  for (auto const &map_it : all_dags) {
    dag_t mdag = map_it.second;
    unsigned long long module_q_count = all_q_counts[map_it.first];
    num_rows = (unsigned)ceil( sqrt( (double)module_q_count ) );
    num_cols = (num_rows*(num_rows-1) < module_q_count) ? num_rows : num_rows-1;
    for (auto g_it_range = vertices(mdag); g_it_range.first != g_it_range.second; ++g_it_range.first){
      gate_descriptor g = *(g_it_range.first);      
      if (mdag[g].op_type == "CNOT") {      
        unsigned c = manhattan_cost(mdag[g].qid[0], mdag[g].qid[1]);
        mcost += c;
        event_count += 7;
        unsigned crit_binidx = (unsigned)(mdag[g].criticality/crit_binwidth);                        
        unsigned len_binidx = (unsigned)(c/len_binwidth);
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
    num_rows = (unsigned)ceil( sqrt( (double)module_q_count ) );
    num_cols = (num_rows*(num_rows-1) < module_q_count) ? num_rows : num_rows-1;
    for (auto g_it_range = vertices(mdag); g_it_range.first != g_it_range.second; ++g_it_range.first){
      gate_descriptor g = *(g_it_range.first);      
      if (mdag[g].op_type == "CNOT") {            
        unsigned c = manhattan_cost(mdag[g].qid[0], mdag[g].qid[1]);
        mcost += c;
        event_count += 7;
        unsigned crit_binidx = (unsigned)(mdag[g].criticality/crit_binwidth);                        
        unsigned len_binidx = (unsigned)(c/len_binwidth);
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

unsigned find_closest_magic (unsigned data_qid, vector<unsigned> magic_qids) {
  unsigned mcost = numeric_limits<unsigned>::max(); 
  unsigned d = numeric_limits<unsigned>::max();
  unsigned result = 0;
  for (auto &m : magic_qids) {
    d = manhattan_cost(data_qid, m);
    if (d < mcost) { 
      mcost = d;
      result = m;
    }
  }
  return result;
}


/*******************************************************************************
                            Surface Code Calculations
*******************************************************************************/

// code distance to keep accumulated logical errors below acceptable_epsilon
// [arxiv.org/pdf/1208.0928, eq. 11]
unsigned set_code_distance () {
  unsigned distance = 2*(int)ceil(log(100.0*L_error_rate/3.0) / log(P_error_rate/P_th)) - 1;
  if ( L_error_rate > P_error_rate ) distance = 1; // very small circuit (large L_error_rate)
                                                        // means smallest possible mesh
  if (distance < 1) {
    cerr << "Error: code distance too small for surface code operation. Try changing physical or logical error rates.\n";
    exit(1);
  }
  return code_distance;
}

// set distillation levels to bring last level error below L_error_rate
// [arxiv.org/pdf/1208.0928, Section XVI-B & Appendix M]
unsigned set_distillation_level (factory_design_t &factory_design, const string &magic) {
  unsigned distillation_level = 0;
  if (factory_design.name == "reed-muller") {
    double base, base_error, distillation_error;
    // P_0 = short_Y_error, P_1 = 7*(short_Y_error)^3, ..., P_n = 7*(P_(n-1))^3
    if (magic == "Y") {
      base = 7.0;
      base_error = short_Y_error;     
    }
    // P_0 = short_A_error, P_1 = 35*(short_A_error)^3, ..., P_n = 35*(P_(n-1))^3
    else if (magic == "A") {
      base = 35.0;
      base_error = short_A_error;    
    }
    else {
      cerr << "Error: Unknown magic state.\n";
      exit(1);
    }
    distillation_error = base_error;
    while (distillation_error > L_error_rate) {
      distillation_level++;
      double base_pow = 0.0;
      for (int j = 0; j < distillation_level; j++)
        base_pow += pow(3.0, (double)(j));
      distillation_error = pow(base, base_pow) * pow(base_error,pow(3.0,(double)distillation_level));
    }
  }
  return distillation_level;
}


// logical tile footprint of factory at distillation level l of total L levels
double get_footprint_at_level (factory_design_t &factory_design, const string &magic, 
                      unsigned &l, unsigned &L) {
  double footprint = 0.0;
  if (factory_design.name == "reed-muller") {
    if (magic == "Y")
      for (int i = 0; i <= L-l+1; i++)
        footprint += pow(7.0, i);
    else if (magic == "A")
      for (int i = 0; i <= L-l+1; i++)
        footprint += pow(15.0, i);
  }
  return footprint;
}

// max allowed error at level l so that level L meets L_error_rate
double get_max_error_at_level (factory_design_t &factory_design, const string &magic, 
                      unsigned &l, unsigned &L) {
  double max_error = 0.0;
  // errors get reduced as: P_Y[l+1] = 7*P_Y[l]^3 or P_A[l+1] = 35*P_A[l]^3
  // error at level L must have been reduced to L_error_rate
  if (factory_design.name == "reed-muller") {
    if (magic == "Y") {
      double pow7 = 0.0;
      for (int i = 0; i < L-l; i++)
        pow7 += pow(3.0,i);
      max_error = pow( L_error_rate/pow(7,pow7), 1/pow(3, (L-l)) );
    }
    else if (magic == "A") {
      double pow35 = 0.0;
      for (int i = 0; i < L-l; i++)
        pow35 += pow(3.0,i);
      max_error = pow( L_error_rate/pow(35,pow35), 1/pow(3, (L-l)) );
    }
  }
  return max_error;
}

// latency of distillation at level l of total L levels
// obtained by evaluating circuit depth (each CNOT = 2d cycles)
/*unsigned get_latency_at_level (factory_design_t &factory_design, string &magic, 
                        unsigned &l, unsigned &L) {
  unsigned latency = 0;
  if (factory_design.name == "reed-muller") {
    if (magic == "Y")
      latency = 8 * code_distance_at_level (factory_design, magic, l, L);
    else if (magic == "A")
      latency = 10 * code_distance_at_level (factory_design, magic, l, L);
  }
  return latency;
}*/

// set code distance at each distillation level such that
// last level error stays below L_error_rate, even with faulty circuits
// find overall footprint and latency -- assume footprint reuse, additive latency
pair<double, double> get_footprint_latency (factory_design_t &factory_design, const string &magic) {
  double single_area = 0.0;
  double single_latency = 0.0;
  unsigned distillation_level = 0;
  if (factory_design.name == "reed-muller") {
    if (magic == "Y") {
      distillation_level = distillation_level_Y;
    }
    else if (magic == "A") {
      distillation_level = distillation_level_A; 
    }
    else {
      cerr << "Error: Unknown magic state.\n";
      exit(1);
    }    
    vector<unsigned> code_distance_at_level (distillation_level, 0);
    vector<unsigned> footprint_at_level (distillation_level, 0);
    vector<unsigned> latency_at_level (distillation_level, 0);    
    vector<double> distillation_error_at_level (distillation_level, 0);
    for (unsigned l = 1; l <= distillation_level; l++) {
      unsigned logical_footprint = get_footprint_at_level(factory_design, magic, l, distillation_level);
      double max_error = get_max_error_at_level(factory_design, magic, l, distillation_level);
      // find smallest distance that satisfies:
      // logical_footprint * 2 * 3 * 1.25 * d * 0.03 * (p/pth)^(d+1)/2 < max_error
      // this is a transcendental inequality (d.a^d < b) -- solve iteratively
      double a = sqrt(P_error_rate/P_th);      
      double b = max_error / (0.225 * logical_footprint * (sqrt(P_error_rate/P_th)));
      unsigned high_limit = 50;
      unsigned low_limit = 0;
      unsigned d = (high_limit + low_limit) / 2;
      while (high_limit - low_limit > 1 && d > low_limit && d < high_limit) {
        if (d * pow(a,d) < b)
          high_limit = d;
        else
          low_limit = d;
        d = (high_limit + low_limit) / 2;
      }
      code_distance_at_level[l-1] = d;
      footprint_at_level[l-1] = logical_footprint * 2.5 * 1.25 * pow(2*d, 2) ;      
      latency_at_level[l-1] = 10 * d;
      distillation_error_at_level[l-1] = max_error;
    }
#ifdef _DEBUG
    cout << "magic state: " << magic << endl;
    for (int i = 0; i < code_distance_at_level.size(); i++)
      cout << "\tcode_distance[l=" << i+1 << "]: " << code_distance_at_level[i] << endl; 
    for (int i = 0; i < latency_at_level.size(); i++)
      cout << "\tlatency[l=" << i+1 << "]: " << latency_at_level[i] << endl; 
    for (int i = 0; i < footprint_at_level.size(); i++)
      cout << "\tfootprint[l=" << i+1 << "]: " << footprint_at_level[i] << endl; 
    for (int i = 0; i < distillation_error_at_level.size(); i++)
      cout << "\tdistillation_error[l=" << i+1 << "]: " << distillation_error_at_level[i] << endl;     
#endif
    if (distillation_level > 0) {
      single_area = *max_element(footprint_at_level.begin(), footprint_at_level.end());
      single_latency = accumulate(latency_at_level.begin(), latency_at_level.end(), 0);
    }
    else {
      single_area = 2.5 * 1.25 * pow(2*code_distance, 2);
      single_latency = 1;
    }
  }
  // [arxiv.org/pdf/1209.2426]
  else if (factory_design.name == "bravyi-haah") {
  }
  else {
    cerr<<"Error: Unknown distillation protocol."<<endl;
    exit(1);
  }
  
  return make_pair(single_area, single_latency);
}


/*******************************************************************************
                                Simulator Functions
*******************************************************************************/

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

void resolve_cnot (Event &event) {
  assert( (event.type == cnot3 || event.type == cnot5) && "invalid cnot resolve request\n.");
  unsigned src_qubit = dag[event.gate].qid[0];
  unsigned dest_qubit = dag[event.gate].qid[1];   
  // adjacent qubits:if expand("%") == ""|browse confirm w|else|confirm w|endif

  if ( are_adjacent(src_qubit,dest_qubit) )
    return;  
  // modify braid
  pair<unsigned,unsigned> anc1_anc2 = cnot_ancillas(src_qubit, dest_qubit);
  unsigned anc1 = anc1_anc2.first;  
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

void purge_gate_from_mesh (unsigned gate_seq) {
  // purge nodes row by row
  for (unsigned r=0; r<num_rows+1; r++) {
    for (unsigned c=0; c<num_cols+1; c++) {
      unsigned node_num = r*(num_cols+1)+c;
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
    for (unsigned c=0; c<num_cols+1; c++) {
      unsigned node_num = r*(num_cols+1)+c;
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
  map<gate_descriptor, unsigned> crits;       // critical path of gates
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


/*******************************************************************************
                                    Main
*******************************************************************************/

int main (int argc, char *argv[]) {

  // read simulator characteristics from input
  argparse (argc, argv);

  // how long is each surface code cycle
  surface_code_cycle = set_surface_code_cycle (tech);
  
  // read program gates
  // mark gate seq numbers from 1 upwards
  string benchmark_path(input_files[0]); //FIXME: loop on multiple input files
  string benchmark_dir = benchmark_path.substr(0, benchmark_path.find_last_of('/'));  
  string benchmark_name = benchmark_path.substr(benchmark_path.find_last_of('/')+1, benchmark_path.length());  
  string LPFS_path = benchmark_path+".lpfs";
  string profile_freq_path = benchmark_path+".freq";
  parse_LPFS(LPFS_path);
  parse_freq(profile_freq_path);
 
  //calculate code distance (based on Fowler et al. Equation 11)
  if (P_error_rate > P_th) {
    cerr << "Physical error rate is higher than the code threshold. Terminating..\n";
    exit(1);
  }      
  unsigned long long total_logical_gates = 0;    // KQ parameter, needed for calculating L_error_rate  
  unsigned long long total_S_gates = 0;          // total number of logical S gates  
  unsigned long long total_T_gates = 0;          // total number of logical T gates
 
#ifdef _PROGRESS
  cout<<"\n-----------------------------------------------";
  cout<<"\n---          Program Characteristics        ---";
  cout<<"\n-----------------------------------------------";  
#endif  
  for (auto const &map_it : all_gates) {
    string module_name = map_it.first;     
    int module_size = map_it.second.size();
    int module_S_size = 0;    
    int module_T_size = 0;
    for (auto const &i : map_it.second) {
      if ( i.op_type == "S" || i.op_type == "Sdag")
        module_S_size++;      
      if ( i.op_type == "T" || i.op_type == "Tdag")
        module_T_size++;
    }
    unsigned long long module_freq = 1;
    if ( module_freqs.find(module_name) != module_freqs.end() )
      module_freq = module_freqs[module_name];
#ifdef _PROGRESS
    cout<<"\nleaf: "<<module_name<<" - size: "<<module_size<<" - freq: "<<module_freq;
#endif      
    total_logical_gates += module_size * module_freq;   
    total_S_gates += module_S_size * module_freq;      
    total_T_gates += module_T_size * module_freq;   

    // TODO: modular
    /*
    // Find factory count for each module execution
    if (periphery) {
      unsigned module_num_rows = (unsigned)ceil( sqrt( (double)all_q_counts[module_name] ) );
      unsigned module_num_cols = (module_num_rows*(module_num_rows-1) < all_q_counts[module_name]) 
        ? module_num_rows : module_num_rows-1;
      num_Y_factories_v[module_name] = 2 * (module_num_rows) / rows_to_Y_factory_ratio;
      num_A_factories_v[module_name] = 2 * (module_num_cols+2) / cols_to_A_factory_ratio;    
    }
    else {
      num_Y_factories_v[module_name] = num_Y_factories;
      num_A_factories_v[module_name] = num_A_factories;
    }*/
  }
  cerr << "\ntotal logical gates: " << total_logical_gates << endl;
  cerr << "total logical S gates: " << total_S_gates << endl;    
  cerr << "total logical T gates: " << total_T_gates << endl;  
  L_error_rate = (double)acceptable_epsilon/(double)total_logical_gates;  

  // calculate parameters for surface code architecture 
  // (code distance, distillation levels, factory areas, factory latencies)
  code_distance = set_code_distance();
  distillation_level_Y = set_distillation_level(factory_design, "Y");
  distillation_level_A = set_distillation_level(factory_design, "A"); 
  single_Y_area = get_footprint_latency(factory_design, "Y").first;
  single_Y_latency = get_footprint_latency(factory_design, "Y").second;
  single_A_area = get_footprint_latency(factory_design, "A").first;
  single_A_latency = get_footprint_latency(factory_design, "A").second;    
  
  // the code distance inside factories is irrelavent to the larger mesh
  // calculate actual factory footprint in terms of logical tiles
  single_Y_area /= (2.5 * 1.5 * pow(2*code_distance, 2));  
  single_A_area /= (2.5 * 1.5 * pow(2*code_distance, 2));

  // number of magic states produced by each factory after a full distillation
  single_Y_ports = (unsigned)ceil((double)Y_factory_capacity / num_Y_factories);
  single_A_ports = (unsigned)ceil((double)A_factory_capacity / num_A_factories);
  
#ifdef _PROGRESS
  cout<<"\n-----------------------------------------------";
  cout<<"\n---    Derived Surface Code Architecture    ---";
  cout<<"\n-----------------------------------------------"<<endl;  
  cout << "Physical error rate (p): " << P_error_rate << endl;
  cout << "Logical error rate (p_L): " << L_error_rate << endl;
  cout << "Code distance (d): " << code_distance << endl;
  cout << "Y-state distillation level: " << distillation_level_Y << endl;  
  cout << "A-state distillation level: " << distillation_level_A << endl;
  cout << "Single Y-factory area: " << single_Y_area << endl;
  cout << "Single Y-factory latency: " << single_Y_latency << endl;
  cout << "Single Y-factory ports: " << single_Y_ports << endl;  
  cout << "Total Y-factories: " << num_Y_factories << endl;
  cout << "Single A-factory latency: " << single_A_latency << endl;
  cout << "Single A-factory area: " << single_A_area << endl;
  cout << "Single A-factory ports: " << single_A_ports << endl;    
  cout << "Total A-factories: " << num_A_factories << endl;    
#endif
  
  // optimize qubit placements
  // write all_gates to trace (.tr) file, then apply partition-based layout optimization    
  if (optimize_layout) {
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
              cerr << "Invalid gate." << endl;
          }
        }
      }
      tr_file.close();  
      string exe_path(argv[0]);
      string exe_dir = exe_path.substr(0, exe_path.find_last_of('/'));  
      string metis_command = "python "+exe_dir+"/arrange.py "+tr_path+
                           " "+to_string(P_error_rate)+" "+to_string(attempt_th_yx)+" "+to_string(attempt_th_drop);
      int status = system(metis_command.c_str());
      if (status==-1) {
        cerr << "Error: METIS partitioning failed.\n";
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
  string br_file_path;  
  ofstream br_file;    
  br_file_path = output_dir+benchmark_name
                    +".yratio."+(to_string(rows_to_Y_factory_ratio))
                    +".aratio."+(to_string(cols_to_A_factory_ratio))        
                    +".p."+(to_string(P_error_rate))
                    +".yx."+to_string(attempt_th_yx)
                    +".drop."+to_string(attempt_th_drop)                             
                    +".pri."+to_string(priority_policy)
                    +"."+tech.name
                    +(optimize_layout ? ".opt.br" : ".br");
  br_file.open(br_file_path);  

  // visualization file: print network states
  string vis_file_path; 
  vis_file_path = output_dir+benchmark_name
                    +".yratio."+(to_string(rows_to_Y_factory_ratio))
                    +".aratio."+(to_string(cols_to_A_factory_ratio))        
                    +".p."+(to_string(P_error_rate))
                    +".yx."+to_string(attempt_th_yx)
                    +".drop."+to_string(attempt_th_drop)    
                    +".pri."+to_string(priority_policy)                    
                    +"."+tech.name
                    +(optimize_layout ? ".opt.vis" : ".vis");
  if (visualize_mesh) {
    vis_file.open(vis_file_path);
  }

  // braidflash for each module of the benchmark
#ifdef _PROGRESS
  cout<<"\n-----------------------------------------------";
  cout<<"\n---          Braidflash Simulation          ---";
  cout<<"\n-----------------------------------------------";  
#endif  
  all_gates = (optimize_layout) ? all_gates_opt : all_gates;
  total_serial_cycles = 0;
  total_parallel_cycles = 0;
  total_critical_cycles = 0;
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
    gate_complete_count = 0;

    // retrieve parsed gates and q_count for this module
    string module_name = map_it.first;
    vector<Gate> module_gates = map_it.second;

    unsigned long long module_q_count = all_q_counts[module_name];
    num_rows = (unsigned)ceil( sqrt( (double)module_q_count ) );
    num_cols = (num_rows*(num_rows-1) < module_q_count) ? num_rows : num_rows-1;
#ifdef _PROGRESS
    cout << "\nModule: " << module_name << endl;    
    cout << "Size: " << num_rows << " X " << num_cols << endl;
#endif
    if (num_rows == 1 && num_cols == 1) continue;

    // augment mesh for factories
    vector<unsigned> Y_state_ids, A_state_ids;
    vector<unsigned> factory_block;
    set<unsigned> factory_block_set; 
    if (periphery) {
      // 2 rows for A states and 2 columns for Y states
      num_rows = num_rows + 2;
      num_cols = num_cols + 2;
      for (int i = 1; i < num_rows-1; i+=rows_to_Y_factory_ratio) {
        Y_state_ids.push_back( i * num_cols );
        Y_state_ids.push_back( (i+1) * num_cols - 1);
      }
      for (int j = 0; j < num_cols; j+=cols_to_A_factory_ratio) {
        A_state_ids.push_back( j );
        A_state_ids.push_back( (num_rows-1) * num_cols + j);
      }    
    }
    else {
      // interleaved factory blocks
      //num_rows = ...;
      //num_cols = ...;
      //Y_state_ids = ...;
      //A-state-ids = ...;
      //hack
      module_q_count += single_Y_area * num_Y_factories;
      module_q_count += single_A_area * num_A_factories;      
      num_rows = (unsigned)ceil( sqrt( (double)module_q_count ) );
      num_cols = (num_rows*(num_rows-1) < module_q_count) ? num_rows : num_rows-1;
      num_rows_Y_factory = (unsigned)ceil( sqrt( (double)single_Y_area ) );
      num_cols_Y_factory = (num_rows_Y_factory*(num_rows_Y_factory-1) < single_Y_area) ? num_rows_Y_factory : num_rows_Y_factory-1;      
      num_rows_A_factory = (unsigned)ceil( sqrt( (double)single_A_area ) );
      num_cols_A_factory = (num_rows_A_factory*(num_rows_A_factory-1) < single_A_area) ? num_rows_A_factory : num_rows_A_factory-1;
      int factory_start_i = (num_rows - num_rows_Y_factory)/2;
      int factory_start_j = (num_cols - num_cols_Y_factory)/2;
      for (int i = factory_start_i; i < factory_start_i+num_rows_Y_factory; i++) {
        for (int j = factory_start_j; j < factory_start_j+num_cols_Y_factory; j++) {
          factory_block.push_back( i * num_cols + j );
        }
      }            
      factory_start_i = (num_rows - num_rows_A_factory)/2;
      factory_start_j = (num_cols - num_cols_A_factory)/2;
      for (int i = factory_start_i; i < factory_start_i+num_rows_A_factory; i++) {
        for (int j = factory_start_j; j < factory_start_j+num_cols_A_factory; j++) {
          factory_block.push_back( i * num_cols + j );
        }
      }
      int ports = single_Y_ports;
      int stride = 0;
      while (ports!=0) {
        ports--;
        Y_state_ids.push_back( factory_start_i*num_cols + factory_start_j+stride ); //top
        if (ports==0) break;
        ports--;
        Y_state_ids.push_back( (factory_start_i+stride)*num_cols + factory_start_j+num_cols_Y_factory-1 ); // right
        if (ports==0) break;
        ports--;
        Y_state_ids.push_back( (factory_start_i+num_rows_Y_factory-1)*num_cols + factory_start_j+num_cols_Y_factory-1-stride ); // bottom
        if (ports==0) break;
        ports--;
        Y_state_ids.push_back( (factory_start_i+num_rows_Y_factory-1-stride)*num_cols + factory_start_j ); // left
        if (ports==0) break;
        stride++;        
      }      
      ports = single_A_ports;
      stride = 0;
      while (ports!=0) {
        ports--;
        A_state_ids.push_back( factory_start_i*num_cols + factory_start_j+stride ); //top
        if (ports==0) break;
        ports--;
        A_state_ids.push_back( (factory_start_i+stride)*num_cols + factory_start_j+num_cols_A_factory-1 ); // right
        if (ports==0) break;
        ports--;
        A_state_ids.push_back( (factory_start_i+num_rows_A_factory-1)*num_cols + factory_start_j+num_cols_A_factory-1-stride ); // bottom
        if (ports==0) break;
        ports--;
        A_state_ids.push_back( (factory_start_i+num_rows_A_factory-1-stride)*num_cols + factory_start_j ); // left
        if (ports==0) break;
        stride++;        
      }
    }
#ifdef _PROGRESS
    cout << "Size (after factories): " << num_rows << " X " << num_cols << endl;
    cout << "Y Factory area: " << num_rows_Y_factory << " X " << num_cols_Y_factory << endl;    
    cout << "A Factory area: " << num_rows_A_factory << " X " << num_cols_A_factory << endl;
#endif       

    cerr << "Factory block:" << endl;
    for (auto &fb : factory_block) {
      cerr << fb << " ";
      factory_block_set.insert(fb);
    }
    cerr << endl;

    cerr << "A-state IDs:" << endl;
    for (auto &as : A_state_ids) {
      cerr << as << " ";
    }
    cerr << endl;


    // update gate list to be simulated:
    // 1. update data indices in light of added rows/columns
    // 2. replace S and T gates by appropriate CNOTs (Fowler et al. Figure 29 & 30)
#ifdef _PROGRESS
    cout << "Updating gate list for S/T magic state interactions..." << endl;
#endif    
    unsigned seq = module_gates.size();
    vector<Gate> to_push_gates;
    for (auto g_it = module_gates.begin(); g_it != module_gates.end(); ++g_it) {
      for (auto &arg : g_it->qid) {
        if (periphery)
          arg = arg + num_cols + 2 * (int)( (arg) / (num_cols - 2) ) + 1;
        else { 
          //hack:
          // get the argth data qubit on this new mesh
          unsigned new_arg = 0;
          unsigned data_counter = 0;
          while (data_counter != arg) {
            if (factory_block_set.find(new_arg) != factory_block_set.end()) //is it a dissallowed factory?
              new_arg++;
            else {
              data_counter++;
              new_arg++;
            }
          }
          arg = new_arg;
        }
      }   
      if (g_it->op_type == "S" || g_it->op_type == "Sdag") {
        unsigned closest_Y = find_closest_magic(g_it->qid[0], Y_state_ids);
        Gate cx1 = Gate(++seq, "CNOT", (const vector<unsigned>){g_it->qid[0], closest_Y});
        Gate h1 = Gate(++seq, "H", (const vector<unsigned>){closest_Y});
        Gate cx2 = Gate(++seq, "CNOT", (const vector<unsigned>){g_it->qid[0], closest_Y});
        Gate h2 = Gate(++seq, "H", (const vector<unsigned>){closest_Y});
        to_push_gates.push_back(cx1);
        to_push_gates.push_back(h1);
        to_push_gates.push_back(cx2);
        to_push_gates.push_back(h2);
      }
      if (g_it->op_type == "T" || g_it->op_type == "Tdag") {
        unsigned closest_A = find_closest_magic(g_it->qid[0], A_state_ids);
        Gate cx1 = Gate(++seq, "CNOT", (const vector<unsigned>){closest_A, g_it->qid[0]});
        Gate cx2 = Gate(++seq, "CNOT", (const vector<unsigned>){g_it->qid[0], closest_A});
        Gate cx3 = Gate(++seq, "CNOT", (const vector<unsigned>){closest_A, g_it->qid[0]});
        Gate cx4 = Gate(++seq, "CNOT", (const vector<unsigned>){g_it->qid[0], closest_A});
        to_push_gates.push_back(cx1);
        to_push_gates.push_back(cx2);
        to_push_gates.push_back(cx3);
        to_push_gates.push_back(cx4);        
      }
    }
    for (auto t : to_push_gates)
      module_gates.push_back(t);
    module_gates.erase( remove_if(module_gates.begin(), module_gates.end(), 
          [](const Gate &g) {
          return (g.op_type=="S" || g.op_type=="Sdag" || g.op_type=="T" || g.op_type=="Tdag");
          }), module_gates.end() );

    // build mesh
    // add all nodes   
#ifdef _PROGRESS
    cout << "Building mesh..." << endl;
#endif 
    for (unsigned i=0; i < (num_rows+1) * (num_cols+1); i++) {
      node_descriptor n = boost::add_vertex(mesh);
      mesh[n].owner = 0;
      auto t = node_map.emplace(i, n);
      if (t.second == false)
        cerr << "Error: reinserting a node in the mesh." << endl;
    }
    // add all links
    for (unsigned i=0; i < (num_rows+1) * (num_cols+1); i++) {
      unsigned node_row = i / (num_cols+1);
      unsigned node_col = i % (num_cols+1);
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
    cout << "Building DAG..." << endl;
    cout << "module_gates.size() = " << module_gates.size() << endl;
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
      if (count % 10000 == 0) cout << count << endl;
#endif       
      vector<unsigned> qid;
      vector<unsigned> qid_next;    
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
    if (!optimize_layout)             // store all dags in table for future use
      all_dags[module_name] = dag;
    else
      all_dags_opt[module_name] = dag;

    update_highest_criticality();

    // find serial completion time
#ifdef _PROGRESS
    cout << "Calculating SerialCLOCK..." << endl;
#endif     
    unsigned long long serial_clk = 0;
    for (vector<Gate>::const_iterator I = module_gates.begin(); I != module_gates.end(); ++I) {    
      serial_clk += get_gate_latency(*I);     
    }     
    cerr << serial_clk << endl;

    // find critical path
#ifdef _PROGRESS
    cout << "Calculating CriticalCLOCK..." << endl;
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
    cout << "Calculating ParallelCLOCK..." << endl;
    cout << "surface cycle: " << surface_code_cycle << endl; //hack
    
    unsigned long long prev_remaining_edges = 0;
#endif       
    while ( !event_queues.empty() || !ready_events.empty() ||
            !(num_edges(dag)==0) || !ready_gates.empty() ) {

#ifdef _PROGRESS
      if (clk % 10000 == 0) {
        cout << "ParallelCLOCK = " << clk << " ..." << endl;        
        cout << num_edges(dag) << " edges remaining..." << endl;
        if (prev_remaining_edges == num_edges(dag) && num_edges(dag)!=0) {
          cout << "STUCK -- Terminating..." << endl;
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

#ifdef _PROGRESS
            gate_complete_count++;
            if (gate_complete_count % 1000 == 0) 
              cout << gate_complete_count << " gates completed." << endl;
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
    total_serial_cycles += serial_clk * module_freq;    
    total_parallel_cycles += clk * module_freq;    
    total_critical_cycles += critical_clk * module_freq;

    //hack
    total_serial_cycles *= single_A_latency;
    total_critical_cycles *= single_A_latency;
    total_parallel_cycles *= single_A_latency;

    avg_mesh_utility[module_name] = avg_module_mesh_utility;
    cerr << "avg_module_mesh_utility: " << avg_module_mesh_utility << endl;

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

  // Results 'ManhattanCost'    
  pair< pair<int,int>, pair<int,int> > mcost_ecount;    
  mcost_ecount = compare_manhattan_costs();    
  br_file << "mcost: " << mcost_ecount.first.first << endl;
  br_file << "mcost_opt: " << (optimize_layout ? to_string(mcost_ecount.first.second) : "N/A") << endl;
  br_file << "event_count: " << mcost_ecount.second.first << endl;
  br_file << "event_count_opt: " << (optimize_layout ? to_string(mcost_ecount.second.second) : "N/A") << endl;
 
  // Results 'BraidHistograms'
  br_file << "braid_length_histogram:" << endl;
  for (int i=0; i<num_bins; i++) {
    br_file << "[" << i*len_binwidth << "," << (i+1)*len_binwidth << ") " << length_hist[i] << endl;
  }
  br_file << "braid_criticality_histogram:" << endl;
  for (int i=0; i<num_bins; i++) {
    br_file << "[" << i*crit_binwidth << "," << (i+1)*crit_binwidth << ") " << criticality_hist[i] << endl;
  }    

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
  // TODO: modular
  //int max_Y_factories = 0;
  //for (auto &i : num_Y_factories_v)
  //  if (i.second > max_Y_factories) max_Y_factories = i.second;  
  //int max_A_factories = 0;  
  //for (auto &i : num_A_factories_v)
  //  if (i.second > max_A_factories) max_A_factories = i.second;
  int hole_side = 2*ceil(code_distance/4.0) + 1;
  int width_channel = hole_side;
  int hole_to_channel = 2*ceil(code_distance/2.0);
  int length_tile = 2*hole_side + width_channel + 4*hole_to_channel - 6;
  int width_tile = hole_side + 2*hole_to_channel - 2;
  int area_tile_plus = (width_tile + width_channel) * (length_tile + width_channel);
  int num_physical_qubits = 
    (max_q_count)*area_tile_plus 
    + single_Y_area * num_Y_factories    
    + single_A_area * num_A_factories; 
  br_file << "code_distance(d): " << code_distance << endl;
  br_file << "num_logical_data: " << max_q_count << endl;
  br_file << "num_physical_qubits: " << num_physical_qubits << endl;

  // KQ: total number of logical gates
  // k: total number of physical timesteps
  // q: total number of physical qubits
  string kq_file_path; 
  ofstream kq_file;      
  kq_file_path = output_dir+benchmark_name
                    +".yratio."+(to_string(rows_to_Y_factory_ratio))
                    +".aratio."+(to_string(cols_to_A_factory_ratio))    
                    +".p."+(to_string(P_error_rate))
                    +".yx."+to_string(attempt_th_yx)
                    +".drop."+to_string(attempt_th_drop)    
                    +".pri."+to_string(priority_policy)                    
                    +"."+tech.name
                    +(optimize_layout ? ".opt.kq" : ".kq");
  kq_file.open(kq_file_path);
  kq_file << "error rate: " << P_error_rate << endl;
  kq_file << "Y-row ratio: " << rows_to_Y_factory_ratio << endl;
  kq_file << "A-col ratio: " << cols_to_A_factory_ratio << endl;    
  kq_file << "code distance: " << code_distance << endl;
  kq_file << "Y distillation: " << distillation_level_Y << endl;
  kq_file << "A distillation: " << distillation_level_A << endl;
  kq_file << "serial cycles: " << total_serial_cycles << endl;  
  kq_file << "critical cycles: " << total_critical_cycles << endl;  
  kq_file << "parallel cycles: " << total_parallel_cycles << endl;  
  kq_file << "max qubits: " << num_physical_qubits << endl;
  kq_file << "logical KQ: " << total_logical_gates << endl;
  kq_file << "physical kq: " << total_parallel_cycles * num_physical_qubits << endl;  
  kq_file << "surface cycle(ns): " << surface_code_cycle << endl;
  kq_file.close();  

  cerr << "kq report written to:\n" << " \t" << kq_file_path << endl; 
  
  br_file << "\t****** FINISHED SIMULATION *******" << endl;
  br_file.close();

  cerr << "braid report written to:\n" << " \t" << br_file_path << endl;     

  if (visualize_mesh)  {
    vis_file.close();        
    cerr << "network visualizations written to:\n" << " \t" << vis_file_path << endl;     
  }

  return 0;
}
