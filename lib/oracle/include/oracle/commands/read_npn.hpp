/* LSOracle: A learning based Oracle for Logic Synthesis
 * Copyright 2019 Laboratory for Nano Integrated Systems (LNIS)
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

/*
*   read_npn.hpp
*   Author: Walter Lau Neto
*   Reads a json data base containing info about NPN classes up to 4 inputs 
*   optimized and mapped with different DAGs. To be used as choices by a template based mapper. 
*/

#pragma once
#include <iostream>    
#include <fstream>     
#include <unordered_map>
#include <string>
#include <regex>
#include <nlohmann/json.hpp>
#include <kitty/operators.hpp>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <unordered_set>

namespace oracle
{

//struct to hold NPN templates info 
struct func_template
{
  //stores the hexadecimal function
  std::string func; 
  std::string complete_name{};

  //stores the DAG used for mapping
  std::string dag; 
  //bool to indicate if the function was optimized with resyn2 or not 
  bool opt = false; 

  float delay = 0;
  float area = 0;

//operator to use multi set as the storage container
  bool operator==( const func_template& obj ) const
  {
    if( func == obj.func )
      return true;
    else
      return false; 
  }
};

//hash function to be used by multi set - uses the function "hex" as key
struct hasher
{
  size_t
  operator()(const func_template & f) const
  {
    return std::hash<std::string>()(f.func);
  }
};

class store_db
{
public:
  store_db( )
  {
  }

  typedef std::unordered_multiset<func_template, hasher> multi_set; 
  typedef std::unordered_multiset<func_template, hasher>::iterator db_iterator; 

  void run( const std::string& file )
  {
    //library to read the mapped classes
    nlohmann::json json_library; 

    std::ifstream library( file ); 
    library >> json_library; 

    //for each function in the json data base 
    for(auto &func : json_library.items())
    {
      //gets the full name
      std::string complete_name = func.key(); 

      //creates a new struct to hold function's template info
      func_template function;

      std::vector<std::string> words;
      words = split( complete_name, '_' );

      //fills the struct with infos used by the mapper 
      function.complete_name = complete_name;
      function.func = words[0];
      function.dag = words[1];
      if(words[2]=="resyn")
        function.opt = true;
      
      auto tmp_area  = json_library[complete_name]["area"]; 
      auto tmp_delay = json_library[complete_name]["delay"]; 

      function.area  = convert( tmp_area ); 
      function.delay = convert( tmp_delay ); 

      //stores function's in the data base 
      npn_lut.insert(function);
    }
  }

  /* returns the best implementation for a given function
  *  if delay is true returns the best delay; otherwise the best area (extend later to trade off both)
  *  This function should be used to access data! 
  */
  
  func_template get_function ( func_template function, bool delay )
  {
    auto implementations = npn_lut.equal_range( function );
 
    func_template best_delay; 
    func_template best_area;

    bool first = true;

    while( implementations.first != implementations.second ){
      //if(low->first == function){
        //the first time the function is seem takes it as the best intermediate solution
        if(first){
          first = false;
          best_delay = *implementations.first; 
          best_area  = *implementations.first;
        }
        if(delay){
          if(best_delay.delay > implementations.first->delay)
            best_delay = *implementations.first;
        }
        else{
          if(best_area.area > implementations.first->area)
            best_area = *implementations.first;
        }
      //}
      implementations.first++; 
    }

    if(delay)
      return best_delay;
    
    return best_area;
  }

private: 
std::vector<std::string> split(const std::string& s, char delimiter)
{
   std::vector<std::string> tokens;
   std::string token;
   std::istringstream tokenStream(s);
   while (std::getline(tokenStream, token, delimiter))
   {
      tokens.push_back(token);
   }
   return tokens;
}

uint32_t convert (std::string str)
{
  uint32_t value = 0;
  str.erase(std::remove(str.begin(), str.end(), '"'), str.end());
  value = std::stof(str);

  return value; 
}

private:
  //npn classes look-up table
  multi_set npn_lut; 
};

store_db read_npn_db ( const std::string& file )
{
  /*
  include static asserts if any 
  */

  /* creates a new reader and populates the structure to hold the templates data */
  store_db reader; 
  reader.run( file );
  
  return reader; 
}
} /* namespace mockturtle */
