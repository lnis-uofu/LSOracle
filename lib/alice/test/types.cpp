#include <catch.hpp>

#include <string>
#include <tuple>
#include <vector>

#include <alice/alice.hpp>

template<typename... S>
struct cli_list
{
};

template<typename T, typename... S>
struct cli_list<T, alice::cli<S...>>
{
  using type = alice::cli<T, S...>;
};

template<>
struct cli_list<>
{
  using type = alice::cli<>;
};

template<typename T, typename Tuple>
struct tuple_append;

template<typename T, typename... S>
struct tuple_append<T, std::tuple<S...>>
{
  using type = std::tuple<S..., T>;
};

struct nil
{
};

template<typename T, typename U>
struct cons
{
};

template<typename List>
struct list_to_cli;

template<>
struct list_to_cli<nil>
{
  using type = alice::cli<>;
};

template<typename Head, typename Tail>
struct list_to_cli<cons<Head, Tail>>
{
  using type = typename cli_list<Head, typename list_to_cli<Tail>::type>::type;
};

template<typename T>
struct tuple_to_cli;

template<typename... S>
struct tuple_to_cli<std::tuple<S...>>
{
  using type = alice::cli<S...>;
};

template<typename T, int N>
struct list_maker_key : list_maker_key<T, N - 1>
{
};

template<typename T>
struct list_maker_key<T, 0>
{
};

#define _ALICE_START_LIST( name ) \
  struct name##_list_maker;       \
  std::tuple<> list_maker_helper_( list_maker_key<name##_list_maker, __COUNTER__> );

#define _ALICE_ADD_TO_LIST( name, added_type )                                                                       \
  tuple_append<added_type, decltype( list_maker_helper_( list_maker_key<name##_list_maker, __COUNTER__>{} ) )>::type \
      list_maker_helper_( list_maker_key<name##_list_maker, __COUNTER__> );

#define _ALICE_END_LIST( name ) \
  using name = decltype( list_maker_helper_( list_maker_key<name##_list_maker, __COUNTER__>{} ) );

TEST_CASE( "Cons list", "[meta]" )
{
  using list = cons<int, cons<char, cons<short, nil>>>;

  CHECK( std::is_same<list_to_cli<list>::type, alice::cli<int, char, short>>::value );
}

TEST_CASE( "Type concatenation", "[meta]" )
{
  CHECK( std::is_same<alice::cli<>, cli_list<>::type>::value );

  using t_a = cli_list<>::type;
  using t_b = cli_list<int, t_a>::type;
  using t_c = cli_list<char, t_b>::type;

  CHECK( std::is_same<alice::cli<char, int>, t_c>::value );
}

template<typename Tuple, std::size_t Index>
struct helper
{
  helper( const std::vector<std::string>& names, std::string& results )
  {
    helper<Tuple, Index - 1>( names, results );
    results += names[Index - 1];
  }
};

template<typename Tuple>
struct helper<Tuple, 0>
{
  helper( const std::vector<std::string>&, std::string& )
  {
  }
};

TEST_CASE( "Create list with macros", "[meta]" )
{
  _ALICE_START_LIST( types )
  _ALICE_ADD_TO_LIST( types, int )
  _ALICE_ADD_TO_LIST( types, char )
  _ALICE_END_LIST( types )

  CHECK( std::is_same<std::tuple<int, char>, types>::value );
  CHECK( std::is_same<int, std::tuple_element<0, types>::type>::value );

  CHECK( std::is_same<alice::cli<int, char>, tuple_to_cli<types>::type>::value );

  std::string result;
  helper<types, 2>( std::vector<std::string>{ "abc", "def" }, result );
  CHECK( result == "abcdef" );
}