/* LSOracle: A learning based Oracle for Logic Synthesis

 * MIT License
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
#pragma once

#include <set>
#include <regex>

namespace oracle {

const std::regex valid_id("^[a-zA-Z][_a-zA-Z0-9]*$");
const std::set<std::string> keywords = {
    // IEEE 1800-2017 Annex B
    "accept_on", "alias", "always", "always_comb", "always_ff", "always_latch",
    "and", "assert", "assign", "assume", "automatic", "before",
    "begin", "bind", "bins", "binsof", "bit", "break",
    "buf", "bufif0", "bufif1", "byte", "case", "casex",
    "casez", "cell", "chandle", "checker", "class", "clocking",
    "cmos", "config", "const", "constraint", "context", "continue",
    "cover", "covergroup", "coverpoint", "cross", "deassign", "default",
    "defparam", "design", "disable", "dist", "do", "edge",
    "else", "end", "endcase", "endchecker", "endclass", "endclocking",
    "endconfig", "endfunction", "endgenerate", "endgroup", "endinterface", "endmodule",
    "endpackage", "endprimitive", "endprogram", "endproperty", "endsequence", "endspecify",
    "endtable", "endtask", "enum", "event", "eventually", "expect",
    "export", "extends", "extern", "final", "first_match", "for",
    "force", "foreach", "forever", "fork", "forkjoin", "function",
    "generate", "genvar", "global", "highz0", "highz1", "if",
    "iff", "ifnone", "ignore_bins", "illegal_bins", "implements", "implies",
    "import", "incdir", "include", "initial", "inout", "input",
    "inside", "instance", "int", "integer", "interconnect", "interface",
    "intersect", "join", "join_any", "join_none", "large", "let",
    "liblist", "library", "local", "localparam", "logic", "longint",
    "macromodule", "matches", "medium", "modport", "module", "nand",
    "negedge", "nettype", "new", "nexttime", "nmos", "nor",
    "noshowcancelled", "not", "notif0", "notif1", "null", "or",
    "output", "package", "packed", "parameter", "pmos", "posedge",
    "primitive", "priority", "program", "property", "protected", "pull0",
    "pull1", "pulldown", "pullup", "pulsestyle_ondetect", "pulsestyle_onevent", "pure",
    "rand", "randc", "randcase", "randsequence", "rcmos", "real",
    "realtime", "ref", "reg", "reject_on", "release", "repeat",
    "restrict", "return", "rnmos", "rpmos", "rtran", "rtranif0",
    "rtranif1", "s_always", "s_eventually", "s_nexttime", "s_until", "s_until_with",
    "scalared", "sequence", "shortint", "shortreal", "showcancelled", "signed",
    "small", "soft", "solve", "specify", "specparam", "static",
    "string", "strong", "strong0", "strong1", "struct", "super",
    "supply0", "supply1", "sync_accept_on", "sync_reject_on", "table", "tagged",
    "task", "this", "throughout", "time", "timeprecision", "timeunit",
    "tran", "tranif0", "tranif1", "tri", "tri0", "tri1",
    "triand", "trior", "trireg", "type", "typedef", "union",
    "unique", "unique0", "unsigned", "until", "until_with", "untyped",
    "use", "uwire", "var", "vectored", "virtual", "void",
    "wait", "wait_order", "wand", "weak", "weak0", "weak1",
    "while", "wildcard", "wire", "with", "within", "wor",
    "xnor", "xor",
};

std::string escape_id(std::string id)
{
    if (id[0] == '\\') {
	id.erase(0, 1);
    }
    std::smatch match;
    if(!std::regex_search(id, match, valid_id)) {
	return "\\" + id + " ";;
    }
    if (keywords.find(id) != keywords.end()) {
	return "\\" + id + " ";
    }
    return id;
}
}
