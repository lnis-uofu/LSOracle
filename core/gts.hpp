
/********************************************
 *  Source file: gts.hpp                    *
 *  Description: Header file for gts class  *
 *  Author:      Xifan Tang                 *
 *  Affiliation: University of Utah, LNIS   *
 *  Date:        Feb. 19, 2019              *
 *  Revision:    Feb. 19, 2019              *
 ********************************************
 */

#define GTS_LOGIC_LEVEL_TAG "logic_level"
#define GTS_GATE_LEVEL_TAG "gate_level"
#define GTS_PNR_LEVEL_TAG "pnr_level"

#define GTS_FAIL 1

#define TRUE 1 
#define FALSE 0

namespace gts {
  class x_gts_opts {
  public:
    /* Configure internal variables */
    void add_benchmark_list(std::string input_list) {
      benchmark_list.assign(input_list);
    } 
    void add_output_label_file(std::string input_filename) {
      output_label_file.assign(input_filename);
    }
    void add_label_aig(bool input) {
      label_aig = input;
    }
    void add_label_mig(bool input) {
      label_mig = input;
    }
    void add_partition_size(int input_size) {
      partition_input_size = input_size;
    }
    void enable_logic_level_training_set(bool input) {
      gen_logic_level_training_set = input;
    }
    void enable_gate_level_training_set(bool input) {
      gen_gate_level_training_set = input;
    }
    void enable_pnr_level_training_set(bool input) {
      gen_pnr_level_training_set = input;
    }
    /* Read internal variables */
    std::string get_benchmark_list() {
      return benchmark_list;
    }
    std::string get_output_label_file() {
      return output_label_file;
    }
    bool get_label_aig() {
      return label_aig;
    } 
    bool get_label_mig() {
      return label_mig;
    } 
    int get_partition_input_size() {
      return partition_input_size;
    }
    bool get_flag_logic_level_training_set() {
      return gen_logic_level_training_set;
    }
    bool get_flag_gate_level_training_set() {
      return gen_gate_level_training_set;
    }
    bool get_flag_pnr_level_training_set() {
      return gen_pnr_level_training_set;
    }
  private:
    /* Variables to store Options */
    std::string benchmark_list{};
    std::string output_label_file{};
    bool label_aig{};
    bool label_mig{};
    int partition_input_size{};
    bool gen_logic_level_training_set{};
    bool gen_gate_level_training_set{};
    bool gen_pnr_level_training_set{};
  };
}

namespace alice {

  /* Implements the command gen_training_set */

  /*
  ALICE_ADD_STORE( std::string, "benchmark_list", "benchmark_list", "benchmark_list", "benchmark_lists")
  ALICE_ADD_STORE( std::string, "output_label_file", "output_label_file", "Output file name", "Output file name")
  ALICE_ADD_STORE( std::bool, "label_aig", "label", "Use AIG optimization", "Use AIG optimization")
  ALICE_ADD_STORE( std::bool, "label_mig", "label", "Use MIG optimization", "Use mIG optimization")
  ALICE_ADD_STORE( std::int, "partition_size", "partition_size", "partition_size", "partition_size")
  */

  /* Generate labels for  */
  class gts_output_labels_command : public alice::command {
  public:
    explicit gts_output_labels_command ( const environment::ptr& env)
            : command (env, "Output labels from training set into a file") {
      opts.add_option("--output_file,-o", output_label_file, "Filename to output")->required();
    }
    void echo_options() {
      std::cout << "Output label filename: " << output_label_file << "\n";
    }
  protected:
    void execute() {
    }
  private:
    std::string output_label_file{};
  };

  /* Generate scripts for P&R */
   

  /* Class for generate_training_set */
  class gts_command : public alice::command {
  public:
    explicit gts_command ( const environment::ptr& env)
            : command (env, "Generates a training set for the input benchmarks") {
      opts.add_option("--benchmark_list,benchmark_list", benchmark_list, "List of benchmarks for the training set")->required();
      opts.add_option("--output_file,-o", output_label_file, "Filename to output")->required();
      opts.add_flag("--label_aig,-a", "Use AIG optimization as a label");
      opts.add_flag("--label_mig,-m", "Use MIG optimization as a label");
      opts.add_option("--partition_size,partition_size", partition_size, "Specify the partition size when generating training set")->required();
      opts.add_option("--type,type", training_set_type, "Specify the type of training set generation: [logic_level|gate_level|pnr_level]")->required();
    } 
    /* Function to echo options */
    void echo_options() {
      std::cout << "Echo options for confirmation:\n";
      std::cout << "Benchmark list filename: " << benchmark_list << "\n";
      std::cout << "Output label filename: " << output_label_file << "\n";
      std::cout << "Input size of each training sample circuit: " << partition_size << "\n";
      std::cout << "Type of training set: " << training_set_type << "\n";
      std::cout << "Label AIG optimizer: " << is_set("label_aig") << "\n";
      std::cout << "Label MIG optimizer: " << is_set("label_mig") << "\n";
    }
  protected:
    void execute() {
      std::size_t found;

      echo_options();
      /* Organize options in a gts_opts*/
      gts::x_gts_opts gts_opts;
      gts_opts.add_benchmark_list(benchmark_list);
      gts_opts.add_output_label_file(output_label_file);
      gts_opts.add_partition_size(partition_size);
      gts_opts.add_label_aig(is_set("label_aig"));  
      gts_opts.add_label_mig(is_set("label_mig"));  
      /* Set flag for different types of training sets  */   
      /* Logic level */
      found = training_set_type.find(GTS_LOGIC_LEVEL_TAG);
      gts_opts.enable_logic_level_training_set(FALSE);
      if (found != std::string::npos) {
        gts_opts.enable_logic_level_training_set(TRUE);
      } 
      /* Gate level */
      found = training_set_type.find(GTS_GATE_LEVEL_TAG);
      gts_opts.enable_gate_level_training_set(FALSE);
      if (found != std::string::npos) {
        gts_opts.enable_gate_level_training_set(TRUE);
      } 
      /* P&R level */
      found = training_set_type.find(GTS_PNR_LEVEL_TAG);
      gts_opts.enable_pnr_level_training_set(FALSE);
      if (found != std::string::npos) {
        gts_opts.enable_pnr_level_training_set(TRUE);
      } 
      /* Start the top-level program */
      /* gts::run_gts(gts_opts); */
    }
  private:
    std::string benchmark_list{};
    std::string output_label_file{};
    std::string training_set_type{};
    int partition_size{};
    bool label_aig{};
    bool label_mig{};
    bool logic_level_training_set{};
    bool gate_level_training_set{};
    bool pnr_level_training_set{};
  };

  /* Add the command to ALICE interface*/
  ALICE_ADD_COMMAND(gts, "Training Set Generation"); 
}


