/*
 *  oracle -- Yosys Open SYnthesis Suite plugin for LSOracle
 *
 *  Copyright (C) 2012  Clifford Wolf <clifford@clifford.at>
 *  Copyright (C) 2019 Laboratory for Nano Integrated Systems (LNIS)
 *
 *  Permission to use, copy, modify, and/or distribute this software for any
 *  purpose with or without fee is hereby granted, provided that the above
 *  copyright notice and this permission notice appear in all copies.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 *  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 *  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 *  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 *  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 *  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */

// [[CITE]] ABC
// Berkeley Logic Synthesis and Verification Group, ABC: A System for Sequential Synthesis and Verification
// http://www.eecs.berkeley.edu/~alanmi/abc/

// [[CITE]] Berkeley Logic Interchange Format (BLIF)
// University of California. Berkeley. July 28, 1992
// http://www.ece.cmu.edu/~ee760/760docs/blif.pdf

// [[CITE]] Kahn's Topological sorting algorithm
// Kahn, Arthur B. (1962), "Topological sorting of large networks", Communications of the ACM 5 (11): 558-562, doi:10.1145/368996.369025
// http://en.wikipedia.org/wiki/Topological_sorting

#define ABC_COMMAND_LIB "strash; ifraig; scorr; dc2; dretime; retime {D}; strash; &get -n; &dch -f; &nf {D}; &put"
#define ABC_COMMAND_CTR "strash; ifraig; scorr; dc2; dretime; retime {D}; strash; &get -n; &dch -f; &nf {D}; &put; buffer; upsize {D}; dnsize {D}; stime -p"
#define ABC_COMMAND_LUT "strash; ifraig; scorr; dc2; dretime; retime {D}; strash; dch -f; if; mfs2"
#define ABC_COMMAND_SOP "strash; ifraig; scorr; dc2; dretime; retime {D}; strash; dch -f; cover {I} {P}"
#define ABC_COMMAND_DFL "strash; ifraig; scorr; dc2; dretime; retime {D}; strash; &get -n; &dch -f; &nf {D}; &put"

#define ABC_FAST_COMMAND_LIB "strash; dretime; retime {D}; map {D}"
#define ABC_FAST_COMMAND_CTR "strash; dretime; retime {D}; map {D}; buffer; upsize {D}; dnsize {D}; stime -p"
#define ABC_FAST_COMMAND_LUT "strash; dretime; retime {D}; if"
#define ABC_FAST_COMMAND_SOP "strash; dretime; retime {D}; cover -I {I} -P {P}"
#define ABC_FAST_COMMAND_DFL "strash; dretime; retime {D}; map"

#define LSO_COMMAND_MIG "ps -m; migscript; ps -m; crit_path_stats; ntk_stats "
#define LSO_COMMAND_AIG "ps -a; aigscript; ps -a "
#define LSO_COMMAND_PART_EXCLU_AIG "ps -a; partitioning {P}; optimization -a; ps -a "
#define LSO_COMMAND_PART_EXCLU_MIG "ps -a; partitioning {P}; optimization -m; ps -m; crit_path_stats; ntk_stats "
#define LSO_COMMAND_PART_DEEP "ps -a; partitioning {P}; optimization -i {D}; ps -m; crit_path_stats; ntk_stats "
#define LSO_COMMAND_PART_HIGH_EFFORT "ps -a; oracle; ps -m; crit_path_stats; ntk_stats "
#define LSO_COMMAND_PART_DEEP_M "ps -a; partitioning {P}; optimization -i {D} -c; ps -m; crit_path_stats; ntk_stats "
#define LSO_COMMAND_PART_HIGH_EFFORT_M "ps -a; oracle -c; ps -m; crit_path_stats; ntk_stats "

#include "kernel/register.h"
#include "kernel/sigtools.h"
#include "kernel/celltypes.h"
#include "kernel/log.h"
#include "kernel/yosys.h"
#include "libs/sha1/sha1.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <cctype>
#include <cerrno>
#include <sstream>
#include <climits>

#ifndef _WIN32
#  include <unistd.h>
#  include <dirent.h>
#endif

#include "frontends/blif/blifparse.h"
//#include "frontends/verilog/verilog_frontend.h"

#ifdef YOSYS_LINK_ABC
extern "C" int Abc_RealMain(int argc, char *argv[]);
#endif

USING_YOSYS_NAMESPACE
PRIVATE_NAMESPACE_BEGIN

enum class gate_type_t {
	G_NONE,
	G_FF,
	G_BUF,
	G_NOT,
	G_AND,
	G_NAND,
	G_OR,
	G_NOR,
	G_XOR,
	G_XNOR,
	G_ANDNOT,
	G_ORNOT,
	G_MUX,
	G_AOI3,
	G_OAI3,
	G_AOI4,
	G_OAI4
};

#define G(_name) gate_type_t::G_ ## _name

struct gate_t
{
	int id;
	gate_type_t type;
	int in1, in2, in3, in4;
	bool is_port;
	RTLIL::SigBit bit;
	RTLIL::State init;
};

bool map_mux4;
bool map_mux8;
bool map_mux16;

bool markgroups;
int map_autoidx;
SigMap assign_map;
RTLIL::Module *module;
std::vector<gate_t> signal_list;
std::map<RTLIL::SigBit, int> signal_map;
std::map<RTLIL::SigBit, RTLIL::State> signal_init;
pool<std::string> enabled_gates;
bool recover_init;

bool clk_polarity, en_polarity;
RTLIL::SigSpec clk_sig, en_sig;
dict<int, std::string> pi_map, po_map;

int map_signal(RTLIL::SigBit bit, gate_type_t gate_type = G(NONE), int in1 = -1, int in2 = -1, int in3 = -1, int in4 = -1)
{
	assign_map.apply(bit);

	if (signal_map.count(bit) == 0) {
		gate_t gate;
		gate.id = signal_list.size();
		gate.type = G(NONE);
		gate.in1 = -1;
		gate.in2 = -1;
		gate.in3 = -1;
		gate.in4 = -1;
		gate.is_port = false;
		gate.bit = bit;
		if (signal_init.count(bit))
			gate.init = signal_init.at(bit);
		else
			gate.init = State::Sx;
		signal_list.push_back(gate);
		signal_map[bit] = gate.id;
	}

	gate_t &gate = signal_list[signal_map[bit]];

	if (gate_type != G(NONE))
		gate.type = gate_type;
	if (in1 >= 0)
		gate.in1 = in1;
	if (in2 >= 0)
		gate.in2 = in2;
	if (in3 >= 0)
		gate.in3 = in3;
	if (in4 >= 0)
		gate.in4 = in4;

	return gate.id;
}

void mark_port(RTLIL::SigSpec sig)
{
	for (auto &bit : assign_map(sig))
		if (bit.wire != NULL && signal_map.count(bit) > 0)
			signal_list[signal_map[bit]].is_port = true;
}

void extract_cell(RTLIL::Cell *cell, bool keepff)
{
	if (cell->type == "$_DFF_N_" || cell->type == "$_DFF_P_")
	{
		if (clk_polarity != (cell->type == "$_DFF_P_"))
			return;
		if (clk_sig != assign_map(cell->getPort("\\C")))
			return;
		if (GetSize(en_sig) != 0)
			return;
		goto matching_dff;
	}

	if (cell->type == "$_DFFE_NN_" || cell->type == "$_DFFE_NP_" || cell->type == "$_DFFE_PN_" || cell->type == "$_DFFE_PP_")
	{
		if (clk_polarity != (cell->type == "$_DFFE_PN_" || cell->type == "$_DFFE_PP_"))
			return;
		if (en_polarity != (cell->type == "$_DFFE_NP_" || cell->type == "$_DFFE_PP_"))
			return;
		if (clk_sig != assign_map(cell->getPort("\\C")))
			return;
		if (en_sig != assign_map(cell->getPort("\\E")))
			return;
		goto matching_dff;
	}

	if (0) {
	matching_dff:
		RTLIL::SigSpec sig_d = cell->getPort("\\D");
		RTLIL::SigSpec sig_q = cell->getPort("\\Q");

		if (keepff)
			for (auto &c : sig_q.chunks())
				if (c.wire != NULL)
					c.wire->attributes["\\keep"] = 1;

		assign_map.apply(sig_d);
		assign_map.apply(sig_q);

		map_signal(sig_q, G(FF), map_signal(sig_d));

		module->remove(cell);
		return;
	}

	if (cell->type.in("$_BUF_", "$_NOT_"))
	{
		RTLIL::SigSpec sig_a = cell->getPort("\\A");
		RTLIL::SigSpec sig_y = cell->getPort("\\Y");

		assign_map.apply(sig_a);
		assign_map.apply(sig_y);

		map_signal(sig_y, cell->type == "$_BUF_" ? G(BUF) : G(NOT), map_signal(sig_a));

		module->remove(cell);
		return;
	}

	if (cell->type.in("$_AND_", "$_NAND_", "$_OR_", "$_NOR_", "$_XOR_", "$_XNOR_", "$_ANDNOT_", "$_ORNOT_"))
	{
		RTLIL::SigSpec sig_a = cell->getPort("\\A");
		RTLIL::SigSpec sig_b = cell->getPort("\\B");
		RTLIL::SigSpec sig_y = cell->getPort("\\Y");

		assign_map.apply(sig_a);
		assign_map.apply(sig_b);
		assign_map.apply(sig_y);

		int mapped_a = map_signal(sig_a);
		int mapped_b = map_signal(sig_b);

		if (cell->type == "$_AND_")
			map_signal(sig_y, G(AND), mapped_a, mapped_b);
		else if (cell->type == "$_NAND_")
			map_signal(sig_y, G(NAND), mapped_a, mapped_b);
		else if (cell->type == "$_OR_")
			map_signal(sig_y, G(OR), mapped_a, mapped_b);
		else if (cell->type == "$_NOR_")
			map_signal(sig_y, G(NOR), mapped_a, mapped_b);
		else if (cell->type == "$_XOR_")
			map_signal(sig_y, G(XOR), mapped_a, mapped_b);
		else if (cell->type == "$_XNOR_")
			map_signal(sig_y, G(XNOR), mapped_a, mapped_b);
		else if (cell->type == "$_ANDNOT_")
			map_signal(sig_y, G(ANDNOT), mapped_a, mapped_b);
		else if (cell->type == "$_ORNOT_")
			map_signal(sig_y, G(ORNOT), mapped_a, mapped_b);
		else
			log_abort();

		module->remove(cell);
		return;
	}

	if (cell->type == "$_MUX_")
	{
		RTLIL::SigSpec sig_a = cell->getPort("\\A");
		RTLIL::SigSpec sig_b = cell->getPort("\\B");
		RTLIL::SigSpec sig_s = cell->getPort("\\S");
		RTLIL::SigSpec sig_y = cell->getPort("\\Y");

		assign_map.apply(sig_a);
		assign_map.apply(sig_b);
		assign_map.apply(sig_s);
		assign_map.apply(sig_y);

		int mapped_a = map_signal(sig_a);
		int mapped_b = map_signal(sig_b);
		int mapped_s = map_signal(sig_s);

		map_signal(sig_y, G(MUX), mapped_a, mapped_b, mapped_s);

		module->remove(cell);
		return;
	}

	if (cell->type.in("$_AOI3_", "$_OAI3_"))
	{
		RTLIL::SigSpec sig_a = cell->getPort("\\A");
		RTLIL::SigSpec sig_b = cell->getPort("\\B");
		RTLIL::SigSpec sig_c = cell->getPort("\\C");
		RTLIL::SigSpec sig_y = cell->getPort("\\Y");

		assign_map.apply(sig_a);
		assign_map.apply(sig_b);
		assign_map.apply(sig_c);
		assign_map.apply(sig_y);

		int mapped_a = map_signal(sig_a);
		int mapped_b = map_signal(sig_b);
		int mapped_c = map_signal(sig_c);

		map_signal(sig_y, cell->type == "$_AOI3_" ? G(AOI3) : G(OAI3), mapped_a, mapped_b, mapped_c);

		module->remove(cell);
		return;
	}

	if (cell->type.in("$_AOI4_", "$_OAI4_"))
	{
		RTLIL::SigSpec sig_a = cell->getPort("\\A");
		RTLIL::SigSpec sig_b = cell->getPort("\\B");
		RTLIL::SigSpec sig_c = cell->getPort("\\C");
		RTLIL::SigSpec sig_d = cell->getPort("\\D");
		RTLIL::SigSpec sig_y = cell->getPort("\\Y");

		assign_map.apply(sig_a);
		assign_map.apply(sig_b);
		assign_map.apply(sig_c);
		assign_map.apply(sig_d);
		assign_map.apply(sig_y);

		int mapped_a = map_signal(sig_a);
		int mapped_b = map_signal(sig_b);
		int mapped_c = map_signal(sig_c);
		int mapped_d = map_signal(sig_d);

		map_signal(sig_y, cell->type == "$_AOI4_" ? G(AOI4) : G(OAI4), mapped_a, mapped_b, mapped_c, mapped_d);

		module->remove(cell);
		return;
	}
}

std::string remap_name(RTLIL::IdString abc_name, RTLIL::Wire **orig_wire = nullptr)
{
	std::string abc_sname = abc_name.substr(1);
	bool isnew = false;
	if (abc_sname.substr(0, 4) == "new_")
	{
		abc_sname.erase(0, 4);
		isnew = true;
	}
	if (abc_sname.substr(0, 5) == "ys__n")
	{
		abc_sname.erase(0, 5);
		if (std::isdigit(abc_sname.at(0)))
		{
			int sid = std::stoi(abc_sname);
			size_t postfix_start = abc_sname.find_first_not_of("0123456789");
			std::string postfix = postfix_start != std::string::npos ? abc_sname.substr(postfix_start) : "";

			if (sid < GetSize(signal_list))
			{
				auto sig = signal_list.at(sid);
				if (sig.bit.wire != nullptr)
				{
					std::stringstream sstr;
					sstr << "$abc$" << map_autoidx << "$" << sig.bit.wire->name.substr(1);
					if (sig.bit.wire->width != 1)
						sstr << "[" << sig.bit.offset << "]";
					if (isnew)
						sstr << "_new";
					sstr << postfix;
					if (orig_wire != nullptr)
						*orig_wire = sig.bit.wire;
					return sstr.str();
				}
			}
		}
	}
	std::stringstream sstr;
	sstr << "$abc$" << map_autoidx << "$" << abc_name.substr(1);
	return sstr.str();
}

void dump_loop_graph(FILE *f, int &nr, std::map<int, std::set<int>> &edges, std::set<int> &workpool, std::vector<int> &in_counts)
{
	if (f == NULL)
		return;

	log("Dumping loop state graph to slide %d.\n", ++nr);

	fprintf(f, "digraph \"slide%d\" {\n", nr);
	fprintf(f, "  label=\"slide%d\";\n", nr);
	fprintf(f, "  rankdir=\"TD\";\n");

	std::set<int> nodes;
	for (auto &e : edges) {
		nodes.insert(e.first);
		for (auto n : e.second)
			nodes.insert(n);
	}

	for (auto n : nodes)
		fprintf(f, "  ys__n%d [label=\"%s\\nid=%d, count=%d\"%s];\n", n, log_signal(signal_list[n].bit),
				n, in_counts[n], workpool.count(n) ? ", shape=box" : "");

	for (auto &e : edges)
	for (auto n : e.second)
		fprintf(f, "  ys__n%d -> ys__n%d;\n", e.first, n);

	fprintf(f, "}\n");
}

void handle_loops()
{
	// http://en.wikipedia.org/wiki/Topological_sorting
	// (Kahn, Arthur B. (1962), "Topological sorting of large networks")

	std::map<int, std::set<int>> edges;
	std::vector<int> in_edges_count(signal_list.size());
	std::set<int> workpool;

	FILE *dot_f = NULL;
	int dot_nr = 0;

	// uncomment for troubleshooting the loop detection code
	// dot_f = fopen("test.dot", "w");

	for (auto &g : signal_list) {
		if (g.type == G(NONE) || g.type == G(FF)) {
			workpool.insert(g.id);
		} else {
			if (g.in1 >= 0) {
				edges[g.in1].insert(g.id);
				in_edges_count[g.id]++;
			}
			if (g.in2 >= 0 && g.in2 != g.in1) {
				edges[g.in2].insert(g.id);
				in_edges_count[g.id]++;
			}
			if (g.in3 >= 0 && g.in3 != g.in2 && g.in3 != g.in1) {
				edges[g.in3].insert(g.id);
				in_edges_count[g.id]++;
			}
			if (g.in4 >= 0 && g.in4 != g.in3 && g.in4 != g.in2 && g.in4 != g.in1) {
				edges[g.in4].insert(g.id);
				in_edges_count[g.id]++;
			}
		}
	}

	dump_loop_graph(dot_f, dot_nr, edges, workpool, in_edges_count);

	while (workpool.size() > 0)
	{
		int id = *workpool.begin();
		workpool.erase(id);

		// log("Removing non-loop node %d from graph: %s\n", id, log_signal(signal_list[id].bit));

		for (int id2 : edges[id]) {
			log_assert(in_edges_count[id2] > 0);
			if (--in_edges_count[id2] == 0)
				workpool.insert(id2);
		}
		edges.erase(id);

		dump_loop_graph(dot_f, dot_nr, edges, workpool, in_edges_count);

		while (workpool.size() == 0)
		{
			if (edges.size() == 0)
				break;

			int id1 = edges.begin()->first;

			for (auto &edge_it : edges) {
				int id2 = edge_it.first;
				RTLIL::Wire *w1 = signal_list[id1].bit.wire;
				RTLIL::Wire *w2 = signal_list[id2].bit.wire;
				if (w1 == NULL)
					id1 = id2;
				else if (w2 == NULL)
					continue;
				else if (w1->name[0] == '$' && w2->name[0] == '\\')
					id1 = id2;
				else if (w1->name[0] == '\\' && w2->name[0] == '$')
					continue;
				else if (edges[id1].size() < edges[id2].size())
					id1 = id2;
				else if (edges[id1].size() > edges[id2].size())
					continue;
				else if (w2->name.str() < w1->name.str())
					id1 = id2;
			}

			if (edges[id1].size() == 0) {
				edges.erase(id1);
				continue;
			}

			log_assert(signal_list[id1].bit.wire != NULL);

			std::stringstream sstr;
			sstr << "$abcloop$" << (autoidx++);
			RTLIL::Wire *wire = module->addWire(sstr.str());

			bool first_line = true;
			for (int id2 : edges[id1]) {
				if (first_line)
					log("Breaking loop using new signal %s: %s -> %s\n", log_signal(RTLIL::SigSpec(wire)),
							log_signal(signal_list[id1].bit), log_signal(signal_list[id2].bit));
				else
					log("                               %*s  %s -> %s\n", int(strlen(log_signal(RTLIL::SigSpec(wire)))), "",
							log_signal(signal_list[id1].bit), log_signal(signal_list[id2].bit));
				first_line = false;
			}

			int id3 = map_signal(RTLIL::SigSpec(wire));
			signal_list[id1].is_port = true;
			signal_list[id3].is_port = true;
			log_assert(id3 == int(in_edges_count.size()));
			in_edges_count.push_back(0);
			workpool.insert(id3);

			for (int id2 : edges[id1]) {
				if (signal_list[id2].in1 == id1)
					signal_list[id2].in1 = id3;
				if (signal_list[id2].in2 == id1)
					signal_list[id2].in2 = id3;
				if (signal_list[id2].in3 == id1)
					signal_list[id2].in3 = id3;
				if (signal_list[id2].in4 == id1)
					signal_list[id2].in4 = id3;
			}
			edges[id1].swap(edges[id3]);

			module->connect(RTLIL::SigSig(signal_list[id3].bit, signal_list[id1].bit));
			dump_loop_graph(dot_f, dot_nr, edges, workpool, in_edges_count);
		}
	}

	if (dot_f != NULL)
		fclose(dot_f);
}

std::string add_echos_to_abc_cmd(std::string str)
{
	std::string new_str, token;
	for (size_t i = 0; i < str.size(); i++) {
		token += str[i];
		if (str[i] == ';') {
			while (i+1 < str.size() && str[i+1] == ' ')
				i++;
			new_str += "echo + " + token + " " + token + " ";
			token.clear();
		}
	}

	if (!token.empty()) {
		if (!new_str.empty())
			new_str += "echo + " + token + "; ";
		new_str += token;
	}

	return new_str;
}

std::string fold_abc_cmd(std::string str)
{
	std::string token, new_str = "          ";
	int char_counter = 10;

	for (size_t i = 0; i <= str.size(); i++) {
		if (i < str.size())
			token += str[i];
		if (i == str.size() || str[i] == ';') {
			if (char_counter + token.size() > 75)
				new_str += "\n              ", char_counter = 14;
			new_str += token, char_counter += token.size();
			token.clear();
		}
	}

	return new_str;
}

std::string replace_tempdir(std::string text, std::string tempdir_name, bool show_tempdir)
{
	if (show_tempdir)
		return text;

	while (1) {
		size_t pos = text.find(tempdir_name);
		if (pos == std::string::npos)
			break;
		text = text.substr(0, pos) + "<abc-temp-dir>" + text.substr(pos + GetSize(tempdir_name));
	}

	std::string  selfdir_name = proc_self_dirname();
	if (selfdir_name != "/") {
		while (1) {
			size_t pos = text.find(selfdir_name);
			if (pos == std::string::npos)
				break;
			text = text.substr(0, pos) + "<yosys-exe-dir>/" + text.substr(pos + GetSize(selfdir_name));
		}
	}

	return text;
}

struct lso_output_filter
{
	bool got_cr;
	int escape_seq_state;
	std::string linebuf;
	std::string tempdir_name;
	bool show_tempdir;

	lso_output_filter(std::string tempdir_name, bool show_tempdir) : tempdir_name(tempdir_name), show_tempdir(show_tempdir)
	{
		got_cr = false;
		escape_seq_state = 0;
	}

	void next_char(char ch)
	{
		if (escape_seq_state == 0 && ch == '\033') {
			escape_seq_state = 1;
			return;
		}
		if (escape_seq_state == 1) {
			escape_seq_state = ch == '[' ? 2 : 0;
			return;
		}
		if (escape_seq_state == 2) {
			if ((ch < '0' || '9' < ch) && ch != ';')
				escape_seq_state = 0;
			return;
		}
		escape_seq_state = 0;
		if (ch == '\r') {
			got_cr = true;
			return;
		}
		if (ch == '\n') {
			log("LSOracle: %s\n", replace_tempdir(linebuf, tempdir_name, show_tempdir).c_str());
			got_cr = false, linebuf.clear();
			return;
		}
		if (got_cr)
			got_cr = false, linebuf.clear();
		linebuf += ch;
	}

	void next_line(const std::string &line)
	{
		int pi, po;
		if (sscanf(line.c_str(), "Start-point = pi%d.  End-point = po%d.", &pi, &po) == 2) {
			log("LSOracle: Start-point = pi%d (%s).  End-point = po%d (%s).\n",
					pi, pi_map.count(pi) ? pi_map.at(pi).c_str() : "???",
					po, po_map.count(po) ? po_map.at(po).c_str() : "???");
			return;
		}

		for (char ch : line)
			next_char(ch);
	}
};

struct abc_output_filter
{
	bool got_cr;
	int escape_seq_state;
	std::string linebuf;
	std::string tempdir_name;
	bool show_tempdir;

	abc_output_filter(std::string tempdir_name, bool show_tempdir) : tempdir_name(tempdir_name), show_tempdir(show_tempdir)
	{
		got_cr = false;
		escape_seq_state = 0;
	}

	void next_char(char ch)
	{
		if (escape_seq_state == 0 && ch == '\033') {
			escape_seq_state = 1;
			return;
		}
		if (escape_seq_state == 1) {
			escape_seq_state = ch == '[' ? 2 : 0;
			return;
		}
		if (escape_seq_state == 2) {
			if ((ch < '0' || '9' < ch) && ch != ';')
				escape_seq_state = 0;
			return;
		}
		escape_seq_state = 0;
		if (ch == '\r') {
			got_cr = true;
			return;
		}
		if (ch == '\n') {
			log("ABC: %s\n", replace_tempdir(linebuf, tempdir_name, show_tempdir).c_str());
			got_cr = false, linebuf.clear();
			return;
		}
		if (got_cr)
			got_cr = false, linebuf.clear();
		linebuf += ch;
	}

	void next_line(const std::string &line)
	{
		int pi, po;
		if (sscanf(line.c_str(), "Start-point = pi%d.  End-point = po%d.", &pi, &po) == 2) {
			log("ABC: Start-point = pi%d (%s).  End-point = po%d (%s).\n",
					pi, pi_map.count(pi) ? pi_map.at(pi).c_str() : "???",
					po, po_map.count(po) ? po_map.at(po).c_str() : "???");
			return;
		}

		for (char ch : line)
			next_char(ch);
	}
};

std::string prepend_script_file(std::string script_file, std::string input_file, std::string output_file)
{
	std::ifstream script;
	script.open(script_file);
	std::stringstream lso_script;
	lso_script << stringf("read %s; ", input_file.c_str());
	lso_script << script.rdbuf();
	lso_script << stringf("write_blif %s", output_file.c_str());
	return lso_script.str();
}

std::string generate_lso_script(std::string exe_file, std::string input_aig_file, std::string output_blif_file,
			std::string num_parts, bool partitioned, bool exclu_part, bool mig,
			bool deep, bool merge, bool test, bool aig, bool lut)
{

	std::string lso_script;

	std::string config_direc = exe_file;
	config_direc.erase(config_direc.begin() + config_direc.find("lsoracle"), config_direc.end());

	if(!partitioned)
		lso_script += mig ? stringf("read -m %s; ", input_aig_file.c_str()) : stringf("read %s; ", input_aig_file.c_str());
	else
		lso_script += stringf("read %s; ", input_aig_file.c_str());

	//Conversion from RTLIL to AIG readable by LSOracle
	if(!test){
		if(partitioned){
			if(exclu_part)
				lso_script += mig ? LSO_COMMAND_PART_EXCLU_MIG : LSO_COMMAND_PART_EXCLU_AIG;
			else{
				if(merge)
					lso_script += deep ? LSO_COMMAND_PART_DEEP_M : LSO_COMMAND_PART_HIGH_EFFORT_M;
				else
					lso_script += deep ? LSO_COMMAND_PART_DEEP : LSO_COMMAND_PART_HIGH_EFFORT;
			}
		}
		else{
			// lso_script += mig ? LSO_COMMAND_MIG : LSO_COMMAND_AIG;
			if(mig)
				lso_script += LSO_COMMAND_MIG;
			else if(aig)
				lso_script += LSO_COMMAND_AIG;
			else{
				if(merge)
					lso_script += LSO_COMMAND_PART_HIGH_EFFORT_M;
				else
					lso_script += LSO_COMMAND_PART_HIGH_EFFORT;
			}

		}

	}

	if(test){
		lso_script += !lut ? stringf("write_blif %s", output_blif_file.c_str()) : stringf("lut_map -o %s", output_blif_file.c_str());
	}
	else{
		if(!partitioned){
			if(aig)
				lso_script += !lut ? stringf("; write_blif %s", output_blif_file.c_str()) : stringf("; lut_map -o %s", output_blif_file.c_str());
			else
				lso_script += !lut ? stringf("; write_blif -m %s", output_blif_file.c_str()) : stringf("; lut_map -m -o %s", output_blif_file.c_str());
		}
		else{
			if(aig)
				lso_script += !lut ? stringf("; write_blif %s", output_blif_file.c_str()) : stringf("; lut_map -o %s", output_blif_file.c_str());
			else
				lso_script += !lut ? stringf("; write_blif -m %s", output_blif_file.c_str()) : stringf("; lut_map -m -o %s", output_blif_file.c_str());
		}
	}

	for (size_t pos = lso_script.find("{P}"); pos != std::string::npos; pos = lso_script.find("{P}", pos))
		lso_script = lso_script.substr(0, pos) + num_parts + " -c " + config_direc + "../../core/test.ini" + lso_script.substr(pos+3);

	for (size_t pos = lso_script.find("{C}"); pos != std::string::npos; pos = lso_script.find("{C}", pos))
		lso_script = lso_script.substr(0, pos) + config_direc + "../../core/test.ini" + lso_script.substr(pos+3);

	for (size_t pos = lso_script.find("{D}"); pos != std::string::npos; pos = lso_script.find("{D}", pos))
		lso_script = lso_script.substr(0, pos) + config_direc + "../../deep_learn_model.json" + lso_script.substr(pos+3);
	return lso_script;
}

std::string write_lso_script(std::string lso_script, std::string tempdir_name )
{
	log("LSOracle script: %s\n", lso_script.c_str());
	std::string filename = stringf("%s/lso.script", tempdir_name.c_str());
	FILE *f = fopen(filename.c_str(), "wt");
	fprintf(f, "%s\n", lso_script.c_str());
	fclose(f);
	return filename;
}

void lso_module(std::string exe_file, std::string tempdir_name, bool show_tempdir, std::string filename)
{
	std::string buffer = stringf("%s -f %s 2>&1", exe_file.c_str(), filename.c_str());
	log("Running LSOracle command: %s\n", replace_tempdir(buffer, tempdir_name, show_tempdir).c_str());
	lso_output_filter filt(tempdir_name, show_tempdir);
	int ret = run_command(buffer, std::bind(&lso_output_filter::next_line, filt, std::placeholders::_1));

	if (ret != 0)
		log_error("LSOracle: execution of command \"%s\" failed: return code %d.\n", buffer.c_str(), ret);
}

void abc_module(RTLIL::Design *design, RTLIL::Module *current_module, std::string script_file, std::string abcexe_file,std::string lsoexe_file,
		std::string liberty_file, std::string constr_file, bool cleanup, vector<int> lut_costs, bool dff_mode, std::string clk_str,
		bool keepff, std::string delay_target, std::string sop_inputs, std::string sop_products, std::string lutin_shared, bool fast_mode,
		const std::vector<RTLIL::Cell*> &cells, bool show_tempdir, bool sop_mode, bool abc_dress, std::string num_parts, bool partitioned,
		bool exclu_part, bool mig, bool deep, bool merge, bool test, bool aig, bool lut)
{
	module = current_module;
	map_autoidx = autoidx++;

	signal_map.clear();
	signal_list.clear();
	pi_map.clear();
	po_map.clear();
	recover_init = false;

	if (clk_str != "$")
	{
		clk_polarity = true;
		clk_sig = RTLIL::SigSpec();

		en_polarity = true;
		en_sig = RTLIL::SigSpec();
	}

	if (!clk_str.empty() && clk_str != "$")
	{
		if (clk_str.find(',') != std::string::npos) {
			int pos = clk_str.find(',');
			std::string en_str = clk_str.substr(pos+1);
			clk_str = clk_str.substr(0, pos);
			if (en_str[0] == '!') {
				en_polarity = false;
				en_str = en_str.substr(1);
			}
			if (module->wires_.count(RTLIL::escape_id(en_str)) != 0)
				en_sig = assign_map(RTLIL::SigSpec(module->wires_.at(RTLIL::escape_id(en_str)), 0));
		}
		if (clk_str[0] == '!') {
			clk_polarity = false;
			clk_str = clk_str.substr(1);
		}
		if (module->wires_.count(RTLIL::escape_id(clk_str)) != 0)
			clk_sig = assign_map(RTLIL::SigSpec(module->wires_.at(RTLIL::escape_id(clk_str)), 0));
	}

	if (dff_mode && clk_sig.empty())
		log_cmd_error("Clock domain %s not found.\n", clk_str.c_str());

	std::string tempdir_name = "/tmp/yosys-abc-XXXXXX";
	if (!cleanup)
		tempdir_name[0] = tempdir_name[4] = '_';
	tempdir_name = make_temp_dir(tempdir_name);

	std::string blif_input_file = tempdir_name + "/input.blif";
	std::string aiger_temp_file = tempdir_name + "/abc.aig";
	std::string tmp_script_name = tempdir_name + "/abc.script";
	std::string blif_output_file = tempdir_name + "/output.blif";

	log_header(design, "Extracting gate netlist of module `%s' to `%s/input.blif'..\n",
			module->name.c_str(), replace_tempdir(tempdir_name, tempdir_name, show_tempdir).c_str());

	std::string abc_script = stringf("read_blif %s; strash; write %s", blif_input_file.c_str(), aiger_temp_file.c_str());

	abc_script = add_echos_to_abc_cmd(abc_script);

	for (size_t i = 0; i+1 < abc_script.size(); i++)
		if (abc_script[i] == ';' && abc_script[i+1] == ' ')
			abc_script[i+1] = '\n';

	FILE *f = fopen(tmp_script_name.c_str(), "wt");
	fprintf(f, "%s\n", abc_script.c_str());
	fclose(f);

	if (dff_mode || !clk_str.empty())
	{
		if (clk_sig.size() == 0)
			log("No%s clock domain found. Not extracting any FF cells.\n", clk_str.empty() ? "" : " matching");
		else {
			log("Found%s %s clock domain: %s", clk_str.empty() ? "" : " matching", clk_polarity ? "posedge" : "negedge", log_signal(clk_sig));
			if (en_sig.size() != 0)
				log(", enabled by %s%s", en_polarity ? "" : "!", log_signal(en_sig));
			log("\n");
		}
	}

	for (auto c : cells)
		extract_cell(c, keepff);

	for (auto &wire_it : module->wires_) {
		if (wire_it.second->port_id > 0 || wire_it.second->get_bool_attribute("\\keep"))
			mark_port(RTLIL::SigSpec(wire_it.second));
	}

	for (auto &cell_it : module->cells_)
	for (auto &port_it : cell_it.second->connections())
		mark_port(port_it.second);

	if (clk_sig.size() != 0)
		mark_port(clk_sig);

	if (en_sig.size() != 0)
		mark_port(en_sig);

	handle_loops();

	f = fopen(blif_input_file.c_str(), "wt");
	if (f == NULL)
		log_error("Opening %s for writing failed: %s\n", blif_input_file.c_str(), strerror(errno));

	fprintf(f, ".model netlist\n");

	int count_input = 0;
	fprintf(f, ".inputs");
	for (auto &si : signal_list) {
		if (!si.is_port || si.type != G(NONE))
			continue;
		fprintf(f, " ys__n%d", si.id);
		pi_map[count_input++] = log_signal(si.bit);
	}
	if (count_input == 0)
		fprintf(f, " dummy_input\n");
	fprintf(f, "\n");

	int count_output = 0;
	fprintf(f, ".outputs");
	for (auto &si : signal_list) {
		if (!si.is_port || si.type == G(NONE))
			continue;
		fprintf(f, " ys__n%d", si.id);
		po_map[count_output++] = log_signal(si.bit);
	}
	fprintf(f, "\n");

	for (auto &si : signal_list)
		fprintf(f, "# ys__n%-5d %s\n", si.id, log_signal(si.bit));

	for (auto &si : signal_list) {
		if (si.bit.wire == NULL) {
			fprintf(f, ".names ys__n%d\n", si.id);
			if (si.bit == RTLIL::State::S1)
				fprintf(f, "1\n");
		}
	}

	int count_gates = 0;
	for (auto &si : signal_list) {
		if (si.type == G(BUF)) {
			fprintf(f, ".names ys__n%d ys__n%d\n", si.in1, si.id);
			fprintf(f, "1 1\n");
		} else if (si.type == G(NOT)) {
			fprintf(f, ".names ys__n%d ys__n%d\n", si.in1, si.id);
			fprintf(f, "0 1\n");
		} else if (si.type == G(AND)) {
			fprintf(f, ".names ys__n%d ys__n%d ys__n%d\n", si.in1, si.in2, si.id);
			fprintf(f, "11 1\n");
		} else if (si.type == G(NAND)) {
			fprintf(f, ".names ys__n%d ys__n%d ys__n%d\n", si.in1, si.in2, si.id);
			fprintf(f, "0- 1\n");
			fprintf(f, "-0 1\n");
		} else if (si.type == G(OR)) {
			fprintf(f, ".names ys__n%d ys__n%d ys__n%d\n", si.in1, si.in2, si.id);
			fprintf(f, "-1 1\n");
			fprintf(f, "1- 1\n");
		} else if (si.type == G(NOR)) {
			fprintf(f, ".names ys__n%d ys__n%d ys__n%d\n", si.in1, si.in2, si.id);
			fprintf(f, "00 1\n");
		} else if (si.type == G(XOR)) {
			fprintf(f, ".names ys__n%d ys__n%d ys__n%d\n", si.in1, si.in2, si.id);
			fprintf(f, "01 1\n");
			fprintf(f, "10 1\n");
		} else if (si.type == G(XNOR)) {
			fprintf(f, ".names ys__n%d ys__n%d ys__n%d\n", si.in1, si.in2, si.id);
			fprintf(f, "00 1\n");
			fprintf(f, "11 1\n");
		} else if (si.type == G(ANDNOT)) {
			fprintf(f, ".names ys__n%d ys__n%d ys__n%d\n", si.in1, si.in2, si.id);
			fprintf(f, "10 1\n");
		} else if (si.type == G(ORNOT)) {
			fprintf(f, ".names ys__n%d ys__n%d ys__n%d\n", si.in1, si.in2, si.id);
			fprintf(f, "1- 1\n");
			fprintf(f, "-0 1\n");
		} else if (si.type == G(MUX)) {
			fprintf(f, ".names ys__n%d ys__n%d ys__n%d ys__n%d\n", si.in1, si.in2, si.in3, si.id);
			fprintf(f, "1-0 1\n");
			fprintf(f, "-11 1\n");
		} else if (si.type == G(AOI3)) {
			fprintf(f, ".names ys__n%d ys__n%d ys__n%d ys__n%d\n", si.in1, si.in2, si.in3, si.id);
			fprintf(f, "-00 1\n");
			fprintf(f, "0-0 1\n");
		} else if (si.type == G(OAI3)) {
			fprintf(f, ".names ys__n%d ys__n%d ys__n%d ys__n%d\n", si.in1, si.in2, si.in3, si.id);
			fprintf(f, "00- 1\n");
			fprintf(f, "--0 1\n");
		} else if (si.type == G(AOI4)) {
			fprintf(f, ".names ys__n%d ys__n%d ys__n%d ys__n%d ys__n%d\n", si.in1, si.in2, si.in3, si.in4, si.id);
			fprintf(f, "-0-0 1\n");
			fprintf(f, "-00- 1\n");
			fprintf(f, "0--0 1\n");
			fprintf(f, "0-0- 1\n");
		} else if (si.type == G(OAI4)) {
			fprintf(f, ".names ys__n%d ys__n%d ys__n%d ys__n%d ys__n%d\n", si.in1, si.in2, si.in3, si.in4, si.id);
			fprintf(f, "00-- 1\n");
			fprintf(f, "--00 1\n");
		} else if (si.type == G(FF)) {
			if (si.init == State::S0 || si.init == State::S1) {
				fprintf(f, ".latch ys__n%d ys__n%d %d\n", si.in1, si.id, si.init == State::S1 ? 1 : 0);
				recover_init = true;
			} else
				fprintf(f, ".latch ys__n%d ys__n%d 2\n", si.in1, si.id);
		} else if (si.type != G(NONE))
			log_abort();
		if (si.type != G(NONE))
			count_gates++;
	}

	fprintf(f, ".end\n");
	fclose(f);

	log("Extracted %d gates and %d wires to a netlist network with %d inputs and %d outputs.\n",
			count_gates, GetSize(signal_list), count_input, count_output);
	log_push();
	if (count_output > 0)
	{
		log_header(design, "Executing ABC.\n");

		std::string abc_command = stringf("%s -s -f %s 2>&1", abcexe_file.c_str(), tmp_script_name.c_str());
		log("Running ABC command: %s\n", replace_tempdir(abc_command, tempdir_name, show_tempdir).c_str());

#ifndef YOSYS_LINK_ABC
		abc_output_filter filt(tempdir_name, show_tempdir);
		int ret = run_command(abc_command, std::bind(&abc_output_filter::next_line, filt, std::placeholders::_1));
#else
		// These needs to be mutable, supposedly due to getopt
		char *abc_argv[5];

		abc_argv[0] = strdup(abcexe_file.c_str());
		abc_argv[1] = strdup("-s");
		abc_argv[2] = strdup("-f");
		abc_argv[3] = strdup(tmp_script_name.c_str());
		abc_argv[4] = 0;
		int ret = Abc_RealMain(4, abc_argv);
		free(abc_argv[0]);
		free(abc_argv[1]);
		free(abc_argv[2]);
		free(abc_argv[3]);
#endif
		if (ret != 0)
			log_error("ABC: execution of command \"%s\" failed: return code %d.\n", abc_command.c_str(), ret);

		std::ifstream ifs;
		ifs.open(aiger_temp_file);
		if (ifs.fail())
			log_error("Can't open ABC output file `%s'.\n", aiger_temp_file.c_str());

		std::string lso_script;
		if (script_file == "") {
		// TODO pass temp filenames
			lso_script = generate_lso_script(lsoexe_file, aiger_temp_file, blif_output_file, num_parts,
							partitioned, exclu_part, mig, deep, merge, test, aig, lut);

		} else {
			lso_script = prepend_script_file(script_file, aiger_temp_file, blif_output_file);
		}
		std::string script = write_lso_script(lso_script, tempdir_name);

		lso_module(lsoexe_file, tempdir_name, show_tempdir, script);

		printf("%s\n", blif_output_file.c_str());
		std::ifstream ifs_lso;
		ifs_lso.open(blif_output_file);
		if (ifs_lso.fail())
			log_error("Can't open LSOracle output file `%s'.\n", blif_output_file.c_str());

		printf("Finished LSO\n");

		std::string cec_script = stringf("cec %s %s", aiger_temp_file.c_str(), blif_output_file.c_str());
		cec_script = add_echos_to_abc_cmd(cec_script);

		for (size_t i = 0; i+1 < cec_script.size(); i++)
			if (cec_script[i] == ';' && cec_script[i+1] == ' ')
				cec_script[i+1] = '\n';

		FILE *cec = fopen(stringf("%s/cec.script", tempdir_name.c_str()).c_str(), "wt");
		fprintf(cec, "%s\n", cec_script.c_str());
		fclose(cec);

		std::string cec_buffer = stringf("%s -s -f %s/cec.script 2>&1", abcexe_file.c_str(), tempdir_name.c_str());

		printf("Verifying LSOracle result is equivalent to original file\n");

		#ifndef YOSYS_LINK_ABC
				ret = run_command(cec_buffer, std::bind(&abc_output_filter::next_line, filt, std::placeholders::_1));
		#else
				// These needs to be mutable, supposedly due to getopt
				char *abc_argv[5];
				string tmp_script_name = stringf("%s/cec.script", tempdir_name.c_str());
				abc_argv[0] = strdup(abcexe_file.c_str());
				abc_argv[1] = strdup("-s");
				abc_argv[2] = strdup("-f");
				abc_argv[3] = strdup(tmp_script_name.c_str());
				abc_argv[4] = 0;
				ret = Abc_RealMain(4, abc_argv);
				free(abc_argv[0]);
				free(abc_argv[1]);
				free(abc_argv[2]);
				free(abc_argv[3]);
		#endif
				if (ret != 0)
					log_error("ABC: execution of command \"%s\" failed: return code %d.\n", cec_buffer.c_str(), ret);

		printf("Verification complete\n");

		bool builtin_lib = liberty_file.empty();
		RTLIL::Design *mapped_design = new RTLIL::Design;
		parse_blif(mapped_design, ifs_lso, builtin_lib ? "\\DFF" : "\\_dff_", false, sop_mode);

		ifs_lso.close();

		log_header(design, "Re-integrating LSOracle results.\n");
		RTLIL::Module *mapped_mod = mapped_design->modules_["\\top"];
		if (mapped_mod == NULL)
			log_error("LSOracle output file does not contain a module `top'.\n");
		for (auto &it : mapped_mod->wires_) {
			RTLIL::Wire *w = it.second;
			RTLIL::Wire *orig_wire = nullptr;
			RTLIL::Wire *wire = module->addWire(remap_name(w->name, &orig_wire));
			if (orig_wire != nullptr && orig_wire->attributes.count("\\src"))
				wire->attributes["\\src"] = orig_wire->attributes["\\src"];
			if (markgroups) wire->attributes["\\abcgroup"] = map_autoidx;
			design->select(module, wire);
		}

		std::map<std::string, int> cell_stats;
		for (auto c : mapped_mod->cells())
		{
			if (builtin_lib)
			{
				cell_stats[RTLIL::unescape_id(c->type)]++;
				if (c->type == "\\ZERO" || c->type == "\\ONE") {
					RTLIL::SigSig conn;
					conn.first = RTLIL::SigSpec(module->wires_[remap_name(c->getPort("\\Y").as_wire()->name)]);
					conn.second = RTLIL::SigSpec(c->type == "\\ZERO" ? 0 : 1, 1);
					module->connect(conn);
					continue;
				}
				if (c->type == "\\BUF") {
					RTLIL::SigSig conn;
					conn.first = RTLIL::SigSpec(module->wires_[remap_name(c->getPort("\\Y").as_wire()->name)]);
					conn.second = RTLIL::SigSpec(module->wires_[remap_name(c->getPort("\\A").as_wire()->name)]);
					module->connect(conn);
					continue;
				}
				if (c->type == "\\NOT") {
					RTLIL::Cell *cell = module->addCell(remap_name(c->name), "$_NOT_");
					if (markgroups) cell->attributes["\\abcgroup"] = map_autoidx;
					cell->setPort("\\A", RTLIL::SigSpec(module->wires_[remap_name(c->getPort("\\A").as_wire()->name)]));
					cell->setPort("\\Y", RTLIL::SigSpec(module->wires_[remap_name(c->getPort("\\Y").as_wire()->name)]));
					design->select(module, cell);
					continue;
				}
				if (c->type == "\\AND" || c->type == "\\OR" || c->type == "\\XOR" || c->type == "\\NAND" || c->type == "\\NOR" ||
						c->type == "\\XNOR" || c->type == "\\ANDNOT" || c->type == "\\ORNOT") {
					RTLIL::Cell *cell = module->addCell(remap_name(c->name), "$_" + c->type.substr(1) + "_");
					if (markgroups) cell->attributes["\\abcgroup"] = map_autoidx;
					cell->setPort("\\A", RTLIL::SigSpec(module->wires_[remap_name(c->getPort("\\A").as_wire()->name)]));
					cell->setPort("\\B", RTLIL::SigSpec(module->wires_[remap_name(c->getPort("\\B").as_wire()->name)]));
					cell->setPort("\\Y", RTLIL::SigSpec(module->wires_[remap_name(c->getPort("\\Y").as_wire()->name)]));
					design->select(module, cell);
					continue;
				}
				if (c->type == "\\MUX") {
					RTLIL::Cell *cell = module->addCell(remap_name(c->name), "$_MUX_");
					if (markgroups) cell->attributes["\\abcgroup"] = map_autoidx;
					cell->setPort("\\A", RTLIL::SigSpec(module->wires_[remap_name(c->getPort("\\A").as_wire()->name)]));
					cell->setPort("\\B", RTLIL::SigSpec(module->wires_[remap_name(c->getPort("\\B").as_wire()->name)]));
					cell->setPort("\\S", RTLIL::SigSpec(module->wires_[remap_name(c->getPort("\\S").as_wire()->name)]));
					cell->setPort("\\Y", RTLIL::SigSpec(module->wires_[remap_name(c->getPort("\\Y").as_wire()->name)]));
					design->select(module, cell);
					continue;
				}
				if (c->type == "\\MUX4") {
					RTLIL::Cell *cell = module->addCell(remap_name(c->name), "$_MUX4_");
					if (markgroups) cell->attributes["\\abcgroup"] = map_autoidx;
					cell->setPort("\\A", RTLIL::SigSpec(module->wires_[remap_name(c->getPort("\\A").as_wire()->name)]));
					cell->setPort("\\B", RTLIL::SigSpec(module->wires_[remap_name(c->getPort("\\B").as_wire()->name)]));
					cell->setPort("\\C", RTLIL::SigSpec(module->wires_[remap_name(c->getPort("\\C").as_wire()->name)]));
					cell->setPort("\\D", RTLIL::SigSpec(module->wires_[remap_name(c->getPort("\\D").as_wire()->name)]));
					cell->setPort("\\S", RTLIL::SigSpec(module->wires_[remap_name(c->getPort("\\S").as_wire()->name)]));
					cell->setPort("\\T", RTLIL::SigSpec(module->wires_[remap_name(c->getPort("\\T").as_wire()->name)]));
					cell->setPort("\\Y", RTLIL::SigSpec(module->wires_[remap_name(c->getPort("\\Y").as_wire()->name)]));
					design->select(module, cell);
					continue;
				}
				if (c->type == "\\MUX8") {
					RTLIL::Cell *cell = module->addCell(remap_name(c->name), "$_MUX8_");
					if (markgroups) cell->attributes["\\abcgroup"] = map_autoidx;
					cell->setPort("\\A", RTLIL::SigSpec(module->wires_[remap_name(c->getPort("\\A").as_wire()->name)]));
					cell->setPort("\\B", RTLIL::SigSpec(module->wires_[remap_name(c->getPort("\\B").as_wire()->name)]));
					cell->setPort("\\C", RTLIL::SigSpec(module->wires_[remap_name(c->getPort("\\C").as_wire()->name)]));
					cell->setPort("\\D", RTLIL::SigSpec(module->wires_[remap_name(c->getPort("\\D").as_wire()->name)]));
					cell->setPort("\\E", RTLIL::SigSpec(module->wires_[remap_name(c->getPort("\\E").as_wire()->name)]));
					cell->setPort("\\F", RTLIL::SigSpec(module->wires_[remap_name(c->getPort("\\F").as_wire()->name)]));
					cell->setPort("\\G", RTLIL::SigSpec(module->wires_[remap_name(c->getPort("\\G").as_wire()->name)]));
					cell->setPort("\\H", RTLIL::SigSpec(module->wires_[remap_name(c->getPort("\\H").as_wire()->name)]));
					cell->setPort("\\S", RTLIL::SigSpec(module->wires_[remap_name(c->getPort("\\S").as_wire()->name)]));
					cell->setPort("\\T", RTLIL::SigSpec(module->wires_[remap_name(c->getPort("\\T").as_wire()->name)]));
					cell->setPort("\\U", RTLIL::SigSpec(module->wires_[remap_name(c->getPort("\\U").as_wire()->name)]));
					cell->setPort("\\Y", RTLIL::SigSpec(module->wires_[remap_name(c->getPort("\\Y").as_wire()->name)]));
					design->select(module, cell);
					continue;
				}
				if (c->type == "\\MUX16") {
					RTLIL::Cell *cell = module->addCell(remap_name(c->name), "$_MUX16_");
					if (markgroups) cell->attributes["\\abcgroup"] = map_autoidx;
					cell->setPort("\\A", RTLIL::SigSpec(module->wires_[remap_name(c->getPort("\\A").as_wire()->name)]));
					cell->setPort("\\B", RTLIL::SigSpec(module->wires_[remap_name(c->getPort("\\B").as_wire()->name)]));
					cell->setPort("\\C", RTLIL::SigSpec(module->wires_[remap_name(c->getPort("\\C").as_wire()->name)]));
					cell->setPort("\\D", RTLIL::SigSpec(module->wires_[remap_name(c->getPort("\\D").as_wire()->name)]));
					cell->setPort("\\E", RTLIL::SigSpec(module->wires_[remap_name(c->getPort("\\E").as_wire()->name)]));
					cell->setPort("\\F", RTLIL::SigSpec(module->wires_[remap_name(c->getPort("\\F").as_wire()->name)]));
					cell->setPort("\\G", RTLIL::SigSpec(module->wires_[remap_name(c->getPort("\\G").as_wire()->name)]));
					cell->setPort("\\H", RTLIL::SigSpec(module->wires_[remap_name(c->getPort("\\H").as_wire()->name)]));
					cell->setPort("\\I", RTLIL::SigSpec(module->wires_[remap_name(c->getPort("\\I").as_wire()->name)]));
					cell->setPort("\\J", RTLIL::SigSpec(module->wires_[remap_name(c->getPort("\\J").as_wire()->name)]));
					cell->setPort("\\K", RTLIL::SigSpec(module->wires_[remap_name(c->getPort("\\K").as_wire()->name)]));
					cell->setPort("\\L", RTLIL::SigSpec(module->wires_[remap_name(c->getPort("\\L").as_wire()->name)]));
					cell->setPort("\\M", RTLIL::SigSpec(module->wires_[remap_name(c->getPort("\\M").as_wire()->name)]));
					cell->setPort("\\N", RTLIL::SigSpec(module->wires_[remap_name(c->getPort("\\N").as_wire()->name)]));
					cell->setPort("\\O", RTLIL::SigSpec(module->wires_[remap_name(c->getPort("\\O").as_wire()->name)]));
					cell->setPort("\\P", RTLIL::SigSpec(module->wires_[remap_name(c->getPort("\\P").as_wire()->name)]));
					cell->setPort("\\S", RTLIL::SigSpec(module->wires_[remap_name(c->getPort("\\S").as_wire()->name)]));
					cell->setPort("\\T", RTLIL::SigSpec(module->wires_[remap_name(c->getPort("\\T").as_wire()->name)]));
					cell->setPort("\\U", RTLIL::SigSpec(module->wires_[remap_name(c->getPort("\\U").as_wire()->name)]));
					cell->setPort("\\V", RTLIL::SigSpec(module->wires_[remap_name(c->getPort("\\V").as_wire()->name)]));
					cell->setPort("\\Y", RTLIL::SigSpec(module->wires_[remap_name(c->getPort("\\Y").as_wire()->name)]));
					design->select(module, cell);
					continue;
				}
				if (c->type == "\\AOI3" || c->type == "\\OAI3") {
					RTLIL::Cell *cell = module->addCell(remap_name(c->name), "$_" + c->type.substr(1) + "_");
					if (markgroups) cell->attributes["\\abcgroup"] = map_autoidx;
					cell->setPort("\\A", RTLIL::SigSpec(module->wires_[remap_name(c->getPort("\\A").as_wire()->name)]));
					cell->setPort("\\B", RTLIL::SigSpec(module->wires_[remap_name(c->getPort("\\B").as_wire()->name)]));
					cell->setPort("\\C", RTLIL::SigSpec(module->wires_[remap_name(c->getPort("\\C").as_wire()->name)]));
					cell->setPort("\\Y", RTLIL::SigSpec(module->wires_[remap_name(c->getPort("\\Y").as_wire()->name)]));
					design->select(module, cell);
					continue;
				}
				if (c->type == "\\AOI4" || c->type == "\\OAI4") {
					RTLIL::Cell *cell = module->addCell(remap_name(c->name), "$_" + c->type.substr(1) + "_");
					if (markgroups) cell->attributes["\\abcgroup"] = map_autoidx;
					cell->setPort("\\A", RTLIL::SigSpec(module->wires_[remap_name(c->getPort("\\A").as_wire()->name)]));
					cell->setPort("\\B", RTLIL::SigSpec(module->wires_[remap_name(c->getPort("\\B").as_wire()->name)]));
					cell->setPort("\\C", RTLIL::SigSpec(module->wires_[remap_name(c->getPort("\\C").as_wire()->name)]));
					cell->setPort("\\D", RTLIL::SigSpec(module->wires_[remap_name(c->getPort("\\D").as_wire()->name)]));
					cell->setPort("\\Y", RTLIL::SigSpec(module->wires_[remap_name(c->getPort("\\Y").as_wire()->name)]));
					design->select(module, cell);
					continue;
				}
				if (c->type == "\\DFF") {
					log_assert(clk_sig.size() == 1);
					RTLIL::Cell *cell;
					if (en_sig.size() == 0) {
						cell = module->addCell(remap_name(c->name), clk_polarity ? "$_DFF_P_" : "$_DFF_N_");
					} else {
						log_assert(en_sig.size() == 1);
						cell = module->addCell(remap_name(c->name), stringf("$_DFFE_%c%c_", clk_polarity ? 'P' : 'N', en_polarity ? 'P' : 'N'));
						cell->setPort("\\E", en_sig);
					}
					if (markgroups) cell->attributes["\\abcgroup"] = map_autoidx;
					cell->setPort("\\D", RTLIL::SigSpec(module->wires_[remap_name(c->getPort("\\D").as_wire()->name)]));
					cell->setPort("\\Q", RTLIL::SigSpec(module->wires_[remap_name(c->getPort("\\Q").as_wire()->name)]));
					cell->setPort("\\C", clk_sig);
					design->select(module, cell);
					continue;
				}
			}
			else
				cell_stats[RTLIL::unescape_id(c->type)]++;

			if (c->type == "\\_const0_" || c->type == "\\_const1_") {
				RTLIL::SigSig conn;
				conn.first = RTLIL::SigSpec(module->wires_[remap_name(c->connections().begin()->second.as_wire()->name)]);
				conn.second = RTLIL::SigSpec(c->type == "\\_const0_" ? 0 : 1, 1);
				module->connect(conn);
				continue;
			}

			if (c->type == "\\_dff_") {
				log_assert(clk_sig.size() == 1);
				RTLIL::Cell *cell;
				if (en_sig.size() == 0) {
					cell = module->addCell(remap_name(c->name), clk_polarity ? "$_DFF_P_" : "$_DFF_N_");
				} else {
					log_assert(en_sig.size() == 1);
					cell = module->addCell(remap_name(c->name), stringf("$_DFFE_%c%c_", clk_polarity ? 'P' : 'N', en_polarity ? 'P' : 'N'));
					cell->setPort("\\E", en_sig);
				}
				if (markgroups) cell->attributes["\\abcgroup"] = map_autoidx;
				cell->setPort("\\D", RTLIL::SigSpec(module->wires_[remap_name(c->getPort("\\D").as_wire()->name)]));
				cell->setPort("\\Q", RTLIL::SigSpec(module->wires_[remap_name(c->getPort("\\Q").as_wire()->name)]));
				cell->setPort("\\C", clk_sig);
				design->select(module, cell);
				continue;
			}

			if (c->type == "$lut" && GetSize(c->getPort("\\A")) == 1 && c->getParam("\\LUT").as_int() == 2) {
				SigSpec my_a = module->wires_[remap_name(c->getPort("\\A").as_wire()->name)];
				SigSpec my_y = module->wires_[remap_name(c->getPort("\\Y").as_wire()->name)];
				module->connect(my_y, my_a);
				continue;
			}

			RTLIL::Cell *cell = module->addCell(remap_name(c->name), c->type);
			if (markgroups) cell->attributes["\\abcgroup"] = map_autoidx;
			cell->parameters = c->parameters;
			for (auto &conn : c->connections()) {
				RTLIL::SigSpec newsig;
				for (auto &c : conn.second.chunks()) {
					if (c.width == 0)
						continue;
					log_assert(c.width == 1);
					newsig.append(module->wires_[remap_name(c.wire->name)]);
				}
				cell->setPort(conn.first, newsig);
			}
			design->select(module, cell);
		}

		for (auto conn : mapped_mod->connections()) {
			if (!conn.first.is_fully_const())
				conn.first = RTLIL::SigSpec(module->wires_[remap_name(conn.first.as_wire()->name)]);
			if (!conn.second.is_fully_const())
				conn.second = RTLIL::SigSpec(module->wires_[remap_name(conn.second.as_wire()->name)]);
			module->connect(conn);
		}

		if (recover_init)
			for (auto wire : mapped_mod->wires()) {
				if (wire->attributes.count("\\init")) {
					Wire *w = module->wires_[remap_name(wire->name)];
					log_assert(w->attributes.count("\\init") == 0);
					w->attributes["\\init"] = wire->attributes.at("\\init");
				}
			}

		for (auto &it : cell_stats)
			log("LSOracle RESULTS:   %15s cells: %8d\n", it.first.c_str(), it.second);
		int in_wires = 0, out_wires = 0;
		for (auto &si : signal_list)
			if (si.is_port) {
				char buffer[100];
				snprintf(buffer, 100, "\\ys__n%d", si.id);
				RTLIL::SigSig conn;
				if (si.type != G(NONE)) {
					conn.first = si.bit;
					conn.second = RTLIL::SigSpec(module->wires_[remap_name(buffer)]);
					out_wires++;
				} else {
					conn.first = RTLIL::SigSpec(module->wires_[remap_name(buffer)]);
					conn.second = si.bit;
					in_wires++;
				}
				module->connect(conn);
			}
		log("LSOracle RESULTS:        internal signals: %8d\n", int(signal_list.size()) - in_wires - out_wires);
		log("LSOracle RESULTS:           input signals: %8d\n", in_wires);
		log("LSOracle RESULTS:          output signals: %8d\n", out_wires);

		delete mapped_design;
	}
	else
	{
		log("Don't call LSOracle as there is nothing to map.\n");
	}

	if (cleanup)
	{
		log("Removing temp directory.\n");
		remove_directory(tempdir_name);
	}
	log_pop();

}

struct ORACLEPass : public Pass {
	ORACLEPass() : Pass("lsoracle", "use LSOracle for mixed synthesis with AIG and MIG optimization") { }
	void help() override
	{
		//   |---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|
		log("\n");
		log("    lsoracle [options] [selection]\n");
		log("\n");
		log("This pass uses the LSOracle tool to perform an MIG optimiztation flow\n");
		log("equivalent to ABC's resyn2 script.\n");
		log("\n");
		log("    -abc_exe <command>\n");
#ifdef ABCEXTERNAL
		log("        use the specified command instead of \"" ABCEXTERNAL "\" to execute ABC.\n");
#else
		log("        use the specified command to execute ABC. If not specified, \"yosys-abc\" from the current PATH will be used\n");
#endif
		log("    -lso_exe <command>\n");
		log("        specify where the LSOracle executable is. If not specified, \"lsoracle\" from the current PATH will be used.\n");
		log("\n");
		log("    -partition <number of partitions>\n");
		log("        use k-way hypergraph partitioning to partition circuit for optimization of partitions independently.\n");
		log("\n");
		log("    -exclu_part \n");
		log("        exclusively use either AIG or MIG optimization on all partitions (AIG is default).\n");
		log("\n");
		log("    -deep \n");
		log("        use Deep Neural Network for classification to determine what is best fit for each partition.\n");
		log("\n");
		log("    -mig \n");
		log("        use MIG optimization with no partitioning.\n");
		log("\n");
		log("    -aig \n");
		log("        use AIG optimization with no partitioning.\n");
		log("\n");
		log("    -lut \n");
		log("        use LUT mapping with LSOracle.\n");
		log("\n");
		log("    -merge \n");
		log("        merge partitions classified for the same optimization technique before optimization\n");
		log("\n");
		log("    -test \n");
		log("        test bringing output back into Yosys.\n");
		log("\n");
		log("    -script <file>\n");
		log("        use the specified LSOracle script file instead of the default script.\n");
		log("        if no -script parameter is given, the following scripts are used:\n");
		log("\n");

	}
	void execute(std::vector<std::string> args, RTLIL::Design *design) override
	{
		log_header(design, "Executing LSOracle pass (MIG optimization using LSOracle).\n");
		log_push();

		assign_map.clear();
		signal_list.clear();
		signal_map.clear();
		signal_init.clear();
		pi_map.clear();
		po_map.clear();

#ifdef ABCEXTERNAL
		std::string abcexe_file = ABCEXTERNAL;
#else
		std::string abcexe_file = "yosys-abc";
#endif
		std::string script_file, liberty_file, constr_file, clk_str;
		std::string delay_target, sop_inputs, sop_products, lutin_shared = "-S 1";
		bool fast_mode = false, dff_mode = false, keepff = false, cleanup = true;
		bool show_tempdir = false, sop_mode = false;
		bool abc_dress = false;
		vector<int> lut_costs;
		markgroups = false;

		map_mux4 = false;
		map_mux8 = false;
		map_mux16 = false;
		enabled_gates.clear();

		std::string num_parts;
		bool partitioned = false, exclu_part = false, mig = false, aig = false, lut = false, deep = false, merge = false, test = false;

#ifdef _WIN32
#ifndef ABCEXTERNAL
		if (!check_file_exists(abcexe_file + ".exe") && check_file_exists(proc_self_dirname() + "..\\yosys-abc.exe"))
			abcexe_file = proc_self_dirname() + "..\\yosys-abc";
#endif
#endif

		std::string lsoexe_file = "lsoracle";
		size_t argidx;
		char pwd [PATH_MAX];
		if (!getcwd(pwd, sizeof(pwd))) {
			log_cmd_error("getcwd failed: %s\n", strerror(errno));
			log_abort();
		}
		std::cout << "checking arguments\n";
		if(args.size() > 2){
			for (argidx = 1; argidx < args.size(); argidx++) {

				std::string arg = args[argidx];
				if (arg == "-lso_exe" && argidx+1 < args.size()) {
					lsoexe_file = args[++argidx];
					continue;
				}
				if (arg == "-abc_exe" && argidx+1 < args.size()) {
					abcexe_file = args[++argidx];
					continue;
				}
				if (arg == "-script" && argidx+1 < args.size()) {
					script_file = args[++argidx];
					rewrite_filename(script_file);
					if (!script_file.empty() && !is_absolute_path(script_file) && script_file[0] != '+')
						script_file = std::string(pwd) + "/" + script_file;
					continue;
				}
				if (arg == "-partition" && argidx+1 < args.size()) {
					num_parts = args[++argidx];
					partitioned = true;
					continue;
				}
				if (arg == "-exclu_part") {
					if(!partitioned){
						log("The circuit must be partitioned for this flag\n");
						break;
					}
					else
						exclu_part = true;
					continue;
				}
				if (arg == "-deep") {
					if(!partitioned){
						log("The circuit must be partitioned for this flag\n");
						break;
					}
					else{
						deep = true;
						continue;
					}
				}
				if (arg == "-merge") {
					merge = true;
					continue;
				}
				if (arg == "-mig") {
					mig = true;
					continue;
				}
				if (arg == "-aig") {
					aig = true;
					continue;
				}
				if (arg == "-lut") {
					lut = true;
					continue;
				}
				if (arg == "-test") {
					test = true;
					continue;
				}
			}
			extra_args(args, argidx, design);

			for (auto mod : design->selected_modules())
			{
				if (mod->processes.size() > 0) {
					log("Skipping module %s as it contains processes.\n", log_id(mod));
					continue;
				}

				assign_map.set(mod);
				signal_init.clear();

				for (Wire *wire : mod->wires())
					if (wire->attributes.count("\\init")) {
						SigSpec initsig = assign_map(wire);
						Const initval = wire->attributes.at("\\init");
						for (int i = 0; i < GetSize(initsig) && i < GetSize(initval); i++)
							switch (initval[i]) {
								case State::S0:
									signal_init[initsig[i]] = State::S0;
									break;
								case State::S1:
									signal_init[initsig[i]] = State::S1;
									break;
								default:
									break;
							}
					}

				std::cout << "lsoexe_file = " << lsoexe_file << "\n";
				if (!dff_mode || !clk_str.empty()) {
					abc_module(design, mod, script_file, abcexe_file, lsoexe_file, liberty_file, constr_file, cleanup, lut_costs, dff_mode, clk_str, keepff,
							delay_target, sop_inputs, sop_products, lutin_shared, fast_mode, mod->selected_cells(), show_tempdir, sop_mode, abc_dress,
							num_parts, partitioned, exclu_part, mig, deep, merge, test, aig, lut);
					continue;
				}
			}

		}
		else{
			log("Invalid number of arguments\n");
		}

                log_pop();
	}
} MIGPass;

PRIVATE_NAMESPACE_END
