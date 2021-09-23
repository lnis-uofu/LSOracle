#pragma once

namespace oracle
{

template<typename Ntk> struct xmg_node {
    using node = typename Ntk::node;
    using signal = typename Ntk::signal;
    node root;
    node first;
    node second;
    node third;
}
template <typename Ntk>
class fpxmg_mapping_view: public Ntk
{
    using storage = typename Ntk::storage;
    using node = typename Ntk::node;
    using signal = typename Ntk::signal;
    using xmg
public: fpxmg_mapping_view(Ntk const &ntk): Ntk(ntk)
    {
    }
private:

    void map()
    {



    }

}
}
