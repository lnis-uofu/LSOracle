#include <catch.hpp>

#include <alice/alice.hpp>

using namespace alice;

TEST_CASE("Basic store operations", "[store]")
{
  store_container<std::string> s( "name" );

  CHECK_THROWS_WITH( s.current(), "[e] no current name available" );
  CHECK_THROWS( *s );
  CHECK_THROWS_WITH( s[0], "[e] index 0 is out of bounds" );
  CHECK( s.empty() );
  CHECK( s.size() == 0 );

  s.extend();

  CHECK_NOTHROW( s.current() );
  CHECK_NOTHROW( s[0] );
  CHECK( !s.empty() );
  CHECK( s.size() == 1 );

  s.current() = "Name";

  CHECK( s.current() == "Name" );

  s.clear();

  CHECK( s.empty() );
}

