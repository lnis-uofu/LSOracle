#include <catch.hpp>

#include <iostream>

#include <nlohmann/json.hpp>

using namespace nlohmann;

TEST_CASE( "Merge different JSON objects", "[libs]" )
{
  json command1 = {{"command", "store"}, {"clear", true}};
  json command2 = {{"command", "print -c"}, {"runtime", 0.01}};
  json combined = {command1, command2};
  json combined2 = {};

  combined2.push_back( command1 );
  combined2.push_back( command2 );

  CHECK( command1.size() == 2 );
  CHECK( command2.size() == 2 );
  CHECK( combined.size() == 2 );
  CHECK( combined2.size() == 2 );

  CHECK( command1.is_object() );
  CHECK( command2.is_object() );
  CHECK( combined.is_array() );
  CHECK( combined2.is_array() );
}

TEST_CASE( "Make null an object", "[libs]" )
{
  json obj = nullptr;
  CHECK( obj.is_null() );

  obj["command"] = "ps";
  CHECK( obj.is_object() );
}
