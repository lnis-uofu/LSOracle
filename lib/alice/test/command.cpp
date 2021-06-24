#include <catch.hpp>

#include <sstream>
#include <string>

#include <alice/alice.hpp>

using namespace alice;

namespace alice
{

template<>
struct store_info<std::string>
{
  static constexpr const char* key = "string";
  static constexpr const char* option = "string";
  static constexpr const char* mnemonic = "s";
  static constexpr const char* name = "string";
  static constexpr const char* name_plural = "strings";
};

class test_command : public command
{
public:
  explicit test_command( const environment::ptr& env ) : command( env, "Simple test command" )
  {
  }

protected:
  void execute()
  {
    env->store<std::string>().extend() = "Hello world";
    env->out() << env->store<std::string>().current() << std::endl;
  }
};

struct io_file_tag_t;

template<>
bool can_write<std::string, io_file_tag_t>( command& cmd )
{
  cmd.add_option( "--value,-v", "give some value" );
  cmd.add_flag( "-p", "prepare something" );
  return true;
}

template<>
void write<std::string, io_file_tag_t>( const std::string& element, const std::string& filename, const command& cmd )
{
  if ( cmd.is_set( "-p" ) )
  {
    cmd.env->out() << "prepare" << std::endl;
  }
  cmd.env->out() << "write " << element << " to " << filename << std::endl;
  if ( cmd.is_set( "-v" ) )
  {
    cmd.env->out() << cmd.option_value( "-v" ) << std::endl;
  }
}
}

TEST_CASE( "Stores are added to the CLI", "[cli]" )
{
  alice::cli<std::string> cli( "test" );
  CHECK( cli.env->has_store<std::string>() );
  CHECK( cli.env->store<std::string>().empty() );
}

TEST_CASE( "Adding commands to CLI", "[cli]" )
{
  alice::cli<std::string> cli( "test" );

  std::stringstream sstr;
  cli.env->reroute( sstr, sstr );

  cli.insert_command( "test", std::make_shared<test_command>( cli.env ) );
  cli.insert_write_command<io_file_tag_t>( "write_file", "File" );

  char* args[] = {"", "-c", "store -s; test; store -s; write_file file; write_file -p -v abc file2"};
  cli.run( 3, args );

  CHECK( sstr.str() == "[w] no string in store\n"
                       "Hello world\n"
                       "[i] strings in store:\n"
                       "  *  0: \n"
                       "write Hello world to file\n"
                       "prepare\n"
                       "write Hello world to file2\n"
                       "abc\n" );
}
