/* oracle: C++ logic network library
 * Copyright (C) 2018  EPFL
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

/*!
  \file oracle.hpp
  \brief Main header file for oracle

  \author Max Austin
*/

#pragma once

#include "partitioning/partition_manager.hpp"
#include "partitioning/partition_view.hpp"
#include "partitioning/hyperg.hpp"

#include "partitioning/cluster.hpp"
#include "partitioning/seed_partitioner.hpp"
#include "partitioning/fpga_seed_partitioner.hpp"
#include "partitioning/slack_view.hpp"

/*
#include "commands/testing/level_partition_manager.hpp"
#include "commands/testing/test_sort_fanout.hpp"
#include "commands/testing/test_cad_proj_part.hpp"
*/

#include "commands/depth_based.hpp"
#include "commands/mixed_2steps.hpp"
#include "commands/get_all_partitions.hpp"
#include "commands/partitioning.hpp"
#include "commands/generate_truth_tables.hpp"
#include "commands/print_karnaugh_map.hpp"
#include "commands/utility.hpp"
#include "commands/techmapping.hpp"
#include "commands/collapse_techmap.hpp"
// #include "commands/testing/test_aig_then_part.hpp"
#include "commands/testing/test_combine_part.hpp"
// #include "commands/testing/test_seed_partitioner.hpp"
// #include "commands/testing/test_fpga_seed.hpp"

#include "commands/testing/pattern_view.hpp"


