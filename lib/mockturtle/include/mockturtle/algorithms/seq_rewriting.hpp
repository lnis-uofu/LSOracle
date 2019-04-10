/*
 * This class implements the invariant sequential rewriting presented in Sequential Rewriting and Synthesis by R. Brayton and A. Mischenko.
 * Code based in cut rewriting from EPFL logic synthesis libraries.
 */

namespace mockturtle {

/*! \brief Parameters for cut_rewriting.
 *
 * The data structure `cut_rewriting_params` holds configurable parameters with
 * default arguments for `cut_rewriting`.
 */

struct cut_rewriting_params
{
  cut_rewriting_params()
  {
    cut_enumeration_ps.cut_size = 4;
    cut_enumeration_ps.cut_limit = 12;
    cut_enumeration_ps.minimize_truth_table = true;
  }

  /*! \brief Cut enumeration parameters. */
  cut_enumeration_params cut_enumeration_ps{};

  /*! \brief Allow zero-gain substitutions. */
  bool allow_zero_gain{false};

  /*! \brief Use don't cares for optimization. */
  bool use_dont_cares{false};

  /*! \brief Candidate selection strategy. */
  enum
  {
    minimize_weight,
    greedy
  } candidate_selection_strategy = minimize_weight;

  /*! \brief Show progress. */
  bool progress{false};

  /*! \brief Be verbose. */
  bool verbose{false};

  /*! \brief Be very verbose. */
  bool very_verbose{false};
};

/*! \brief Statistics for cut_rewriting.
 *
 * The data structure `cut_rewriting_stats` provides data collected by running
 * `cut_rewriting`.
 */
struct cut_rewriting_stats
{
  /*! \brief Total runtime. */
  stopwatch<>::duration time_total{0};

  /*! \brief Runtime for cut enumeration. */
  stopwatch<>::duration time_cuts{0};

  /*! \brief Accumulated runtime for rewriting. */
  stopwatch<>::duration time_rewriting{0};

  /*! \brief Runtime to find minimal independent set. */
  stopwatch<>::duration time_mis{0};

  void report() const
  {
    std::cout << fmt::format( "[i] total time     = {:>5.2f} secs\n", to_seconds( time_total ) );
    std::cout << fmt::format( "[i] cut enum. time = {:>5.2f} secs\n", to_seconds( time_cuts ) );
    std::cout << fmt::format( "[i] rewriting time = {:>5.2f} secs\n", to_seconds( time_rewriting ) );
    std::cout << fmt::format( "[i] ind. set time  = {:>5.2f} secs\n", to_seconds( time_mis ) );
  }
};



}