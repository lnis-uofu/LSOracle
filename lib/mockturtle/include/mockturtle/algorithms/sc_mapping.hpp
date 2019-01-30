/*!
  \file sc_mapping.hpp
  \brief Standard Cell mapping

  \author Walter Lau Neto
*/

#pragma once

#include <cstdint>

#include <fmt/format.h>

#include "../views/topo_view.hpp"
#include "cut_enumeration.hpp"
#include "cut_enumeration/mf_cut.hpp"

namespace mockturtle {
    struct sc_mapping_params
    {
        sc_mapping_params()
        {
            cut_enumeration_ps.cut_size = 6;
            cut_enumeration_ps.cut_limit = 8;
        }

        /*! \brief Parameters for cut enumeration
         *
         * The default cut size is 6, the default cut limit is 8.
         */
        cut_enumeration_params cut_enumeration_ps{};

        /*! \brief Number of rounds for area flow optimization.
         *
         * The first round is used for delay optimization.
         */
        uint32_t rounds{2u};

        /*! \brief Number of rounds for exact area optimization. */
        uint32_t rounds_ela{1u};
    };

    namespace detail {

    }

}
