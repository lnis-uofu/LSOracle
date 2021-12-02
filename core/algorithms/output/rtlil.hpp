/* LSOracle: A learning based Oracle for Logic Synthesis
 * Copyright 2021 Laboratory for Nano Integrated Systems (LNIS)
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

#include <string>
#include <unordered_map>
#include <variant>

#include <nlohmann/json.hpp>

namespace oracle
{
namespace rtlil
{
namespace json
{

enum port_direction {
    Input,
    Output,
    Inout
};


void from_json(nlohmann::json const& j, port_direction& p)
{
    std::string dir = j;
    if (dir == "input") {
        p = port_direction::Input;
    } else if (dir == "output") {
        p = port_direction::Output;
    } else if (dir == "inout") {
        p = port_direction::Inout;
    } else {
        assert(false && "Unknown port directionality");
    }
}


void to_json(nlohmann::json& j, port_direction const& p)
{
    if (p == port_direction::Input) {
        j = "input";
    } else if (p == port_direction::Output) {
        j = "output";
    } else { 
        j = "inout";
    }
}


enum const_bit {
    Zero,
    One,
    Undefined,
    Tristate,
};


void from_json(nlohmann::json const& j, const_bit& b)
{
    std::string bit = j;
    if (bit == "0") {
        b = const_bit::Zero;
    } else if (bit == "1") {
        b = const_bit::One;
    } else if (bit == "x") {
        b = const_bit::Undefined;
    } else if (bit == "z") {
        b = const_bit::Tristate;
    } else {
        assert(false && "Unknown bit value");
    }
}


void to_json(nlohmann::json& j, const_bit const& b)
{
    if (b == const_bit::Zero) {
        j = "0";
    } else if (b == const_bit::One) {
        j = "1";
    } else if (b == const_bit::Undefined) {
        j = "x";
    } else {
        j = "z";
    }
}


struct bit {
    std::variant<const_bit, int> net;
};


void from_json(nlohmann::json const& j, bit& b)
{
    std::string bit = j;
    // Is it a constant?
    if (bit == "0") {
        b.net = const_bit::Zero;
    } else if (bit == "1") {
        b.net = const_bit::One;
    } else if (bit == "x") {
        b.net = const_bit::Undefined;
    } else if (bit == "z") {
        b.net = const_bit::Tristate;
    } else {
        // Assume it's a net ID.
        b.net = std::stoi(bit);
    }
}


void to_json(nlohmann::json& j, bit const& b)
{
    if (std::holds_alternative<const_bit>(b.net)) {
        j = std::get<const_bit>(b.net);
    } else {
        j = std::get<int>(b.net);
    }
}


struct port {
    // Port direction.
    port_direction direction;
    // Net connected to each port bit, either a constant or a net ID.
    std::vector<bit> bits;
    // The lowest bit index in use.
    int offset;
    // True if the port is MSB-first.
    bool upto;
};


void from_json(nlohmann::json const& j, port& p)
{
    p.direction = j.at("direction").get<port_direction>();
    p.bits = j.at("bits").get<std::vector<bit>>();
    if (j.find("offset") != j.end()) {
        p.offset = j.at("offset").get<int>();
    } else {
        p.offset = 0;
    }
    if (j.find("upto") != j.end()) {
        p.upto = j.at("upto").get<bool>();
    } else {
        p.upto = false;
    }
}


void to_json(nlohmann::json& j, port const& p)
{
    j = nlohmann::json{{"direction", p.direction}, {"bits", p.bits}, {"offset", p.offset}, {"upto", p.upto}};
}


struct cell {
    // Cell name is automatically generated and possibly not of use to users.
    bool hide_name;
    // The name of the module this is an instance of.
    std::string type;
    // The parameters of this cell.
    std::unordered_map<std::string, std::string> parameters;
    // The attributes of this cell.
    std::unordered_map<std::string, std::string> attributes;
    // The directionality of the ports.
    std::unordered_map<std::string, port_direction> port_directions;
    // The net IDs of each bit of each port.
    std::unordered_map<std::string, std::vector<bit>> connections;
};


void from_json(nlohmann::json const& j, cell& c)
{
    c.hide_name = j.at("hide_name").get<bool>();
    c.type = j.at("type").get<std::string>();
    c.parameters = j.at("parameters").get<std::unordered_map<std::string, std::string>>();
    c.attributes = j.at("attributes").get<std::unordered_map<std::string, std::string>>();
    c.port_directions = j.at("port_directions").get<std::unordered_map<std::string, port_direction>>();
    c.connections = j.at("connections").get<std::unordered_map<std::string, std::vector<bit>>>();
}


void to_json(nlohmann::json& j, cell const& c)
{
    j = nlohmann::json{{"hide_name", c.hide_name}, {"type", c.type}, {"parameters", c.parameters}, {"attributes", c.attributes}, {"port_directions", c.port_directions}, {"connections", c.connections}};
}


struct netname {
    // Net name is automatically generated and possibly not of use to users.
    bool hide_name;
    // Net connected to each net bit, either a constant or a net ID.
    std::vector<bit> bits;
    // The lowest bit index in use.
    int offset;
    // True if the port is MSB-first.
    bool upto;
};


void from_json(nlohmann::json const& j, netname& n)
{
    n.hide_name = j.at("hide_name").get<bool>();
    n.bits = j.at("bits").get<std::vector<bit>>();
    if (j.find("offset") != j.end()) {
        n.offset = j.at("offset").get<int>();
    } else {
        n.offset = 0;
    }
    if (j.find("offset") != j.end()) {
        n.upto = j.at("upto").get<bool>();
    } else {
        n.upto = false;
    }
}


void to_json(nlohmann::json& j, netname const& n)
{
    j = nlohmann::json{{"hide_name", n.hide_name}, {"bits", n.bits}, {"offset", n.offset}, {"upto", n.upto}};
}


struct yosys_module {
    std::unordered_map<std::string, std::string> attributes;
    std::unordered_map<std::string, std::string> parameter_default_values;
    std::unordered_map<std::string, port> ports;
    std::unordered_map<std::string, cell> cells;
    std::unordered_map<std::string, netname> netnames;
    std::string name;
};


void from_json(nlohmann::json const& j, yosys_module& m)
{
    m.attributes = j.at("attributes").get<std::unordered_map<std::string, std::string>>();
    m.parameter_default_values = j.at("parameter_default_values").get<std::unordered_map<std::string, std::string>>();
    m.ports = j.at("ports").get<std::unordered_map<std::string, port>>();
    m.cells = j.at("cells").get<std::unordered_map<std::string, cell>>();
    m.netnames = j.at("netnames").get<std::unordered_map<std::string, netname>>();
    m.name = j.at("name").get<std::string>();
}


void to_json(nlohmann::json& j, yosys_module const& m)
{
    j = nlohmann::json{{"attributes", m.attributes}, {"parameter_default_values", m.parameter_default_values}, {"ports", m.ports}, {"cells", m.cells}, {"netnames", m.netnames}, {"name", m.name}};
}


struct netlist {
    std::string creator;
    std::vector<yosys_module> modules;
};


void from_json(nlohmann::json const& j, netlist& n)
{
    n.creator = j.at("creator").get<std::string>();
    n.modules = j.at("modules").get<std::vector<yosys_module>>();
}


void to_json(nlohmann::json& j, netlist const& n)
{
    j = nlohmann::json{{"creator", "LSOracle"}, {"modules", n.modules}};
}

}
}
}