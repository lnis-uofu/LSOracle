
namespace oracle
{
template<typename network_base>
class partition_manager_junior
{
public:
    using network = typename mockturtle::names_view<network_base>;
    using partition_map = typename mockturtle::node_map<int, network>;
    using storage = typename network::storage;
    using node = typename network::node;
    using signal = typename network::signal;
    using window_view = typename mockturtle::window_view<network>;
    using fanout_view = typename mockturtle::fanout_view<network>;
    partition_manager_junior(network &ntk, partition_map &partitions, int part_num):
        ntk(ntk),
        partitions(partitions),
        partition_count(part_num),
        fanout(mockturtle::fanout_view(ntk)) {}

    network &full_network()
    {
        return ntk;
    }

    window_view partition(int id)
    {
        std::vector<node> inputs;
        std::vector<signal> outputs;
        std::vector<node> gates;
        ntk.foreach_node([&](node const &n) {
           if (partitions[n] != id) {
               return;
           } else if (ntk.is_ci(n)) {
               inputs.push_back(n);
           } else {
               gates.push_back(n);
               ntk.foreach_fanin(n, [&](signal const &f){
                   node fin = ntk.get_node(f);
                   if (partitions[fin] != id) {
                       inputs.push_back(fin);
                   }
               });
               fanout.foreach_fanout(n, [&](node const &s) {
                   if (partitions[s] != id) {
                       outputs.push_back(ntk.make_signal(n));
                   }
               });
           }
        });
        std::sort(inputs.begin(), inputs.end());
        auto iend = std::unique(inputs.begin(), inputs.end());
        inputs.resize(std::distance(inputs.begin(),iend));
        std::sort(outputs.begin(), outputs.end());
        auto oend = std::unique(outputs.begin(), outputs.end());
        outputs.resize(std::distance(outputs.begin(),oend));
        return mockturtle::window_view(ntk, inputs, outputs, gates);
    }

    template<class optimized_network>
    void integrate(int id, mockturtle::names_view<optimized_network> &opt)
    {
        integrate(partition(id), opt);
    }

    template<class optimized_network>
    void integrate(window_view &part, mockturtle::names_view<optimized_network> &opt)
    {
        std::cout << "Running integration" << std::endl;
        assert(opt.num_cis() == part.num_cis());
        assert(opt.num_cos() == part.num_cos());

        mockturtle::node_map<signal, mockturtle::names_view<optimized_network>> old_to_new(opt);

        // WARNING!!!! This works by assuming that PIs and POs in the
        // optimized network were created in the same order as in the partition.
        // This does not deal with other types of inputs/outputs, window_view treats all
        // inputs/outputs as generic PI/PO.
        part.foreach_ci([&](auto n, auto i) {
            auto o = opt.ci_at(i);
            old_to_new[o] = ntk.make_signal(n);
        });
        std::cout << "Setup PIs" << std::endl;
        mockturtle::topo_view opt_top{opt};

        opt_top.foreach_gate([&](auto node) {
            // Insert node into original network.
            std::vector<signal> children;
            opt.foreach_fanin(node, [&](auto child) {
                signal mapped = old_to_new[child];
                signal fanin = opt.is_complemented(child) ? ntk.create_not(mapped) : mapped;
                children.push_back(fanin);
            });

            old_to_new[node] = ntk.clone_node(opt, node, children);
            // Clone names if present.
            auto signal = opt.make_signal(node);
            if (opt.has_name(signal)) {
                ntk.set_name(old_to_new[node], opt.get_name(signal));
            }
        });
        std::cout << "Inserted new nodes" << std::endl;
        // Calculate substitutions from partition outputs.
        std::unordered_map<node, signal> substitutions;
        opt.foreach_co([&](auto opt_signal, auto index) {
            auto opt_node = opt.get_node(opt_signal);
            signal new_out = old_to_new[opt_node];
            if (opt.is_complemented(opt_signal)) {
                new_out = ntk.create_not(new_out);
            }
            signal part_signal = part.co_at(index);
            node orig_node = ntk.get_node(part_signal);
            if (!opt.is_constant(opt_node) && !opt.is_ci(opt_node)) {
                substitutions[orig_node] = new_out;
            }
        });
        std::cout << "Calculated substitutions" << std::endl;
        std::list<std::pair<node, signal>> substitution_list(substitutions.begin(),
                                                               substitutions.end());
        ntk.substitute_nodes(substitution_list);
        std::cout << "Substituted nodes." << std::endl;
    }

    int node_partition(const node &n)
    {
        return partitions[n];
    }

    int number_of_partitions()
    {
        return partition_count;
    }
private:
    network &ntk;
    fanout_view fanout;
    partition_map &partitions;
    int partition_count;
};
}
