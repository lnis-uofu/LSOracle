#include <catch.hpp>

#include <string>
#include <vector>

#include <alice/detail/utils.hpp>

using namespace alice::detail;

TEST_CASE( "split into commands", "[utils]" )
{
  CHECK( split_with_quotes<';'>( "read; write" ) == std::vector<std::string>{ "read", "write" } );
  CHECK( split_with_quotes<';'>( "read \"a; b\"; write" ) == std::vector<std::string>{ "read \"a; b\"", "write" } );
  CHECK( split_with_quotes<';'>( "read \"a; \\\"b; c\\\"\"; write" ) == std::vector<std::string>{ "read \"a; \\\"b; c\\\"\"", "write" } );

  CHECK( split_with_quotes<';'>( "  read   ;    write   " ) == std::vector<std::string>{ "read", "write" } );
}

TEST_CASE( "split command into parts", "[utils]" )
{
  CHECK( split_with_quotes<' '>( "read -n 10 --log \"a b\" filename" ) == std::vector<std::string>{ "read", "-n", "10", "--log", "\"a b\"", "filename" } );
}

