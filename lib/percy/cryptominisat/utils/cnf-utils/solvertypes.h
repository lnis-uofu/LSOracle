/******************************************
Copyright (c) 2016, Mate Soos

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
***********************************************/

#ifndef SOLVERTYPES_H
#define SOLVERTYPES_H

#include "constants.h"

#include <sstream>
#include <algorithm>
#include <limits>
#include <vector>
#include <iostream>
#include <iomanip>
#include <string>
#include <limits>
#include "assert.h"
#include "solvertypesmini.h"

namespace CMSat {

using std::vector;
using std::cout;
using std::endl;
using std::string;

class BinaryClause {
    public:
        BinaryClause(const Lit _lit1, const Lit _lit2, const bool _red) :
            lit1(_lit1)
            , lit2(_lit2)
            , red(_red)
        {
            if (lit1 > lit2) std::swap(lit1, lit2);
        }

        bool operator<(const BinaryClause& other) const
        {
            if (lit1 < other.lit1) return true;
            if (lit1 > other.lit1) return false;

            if (lit2 < other.lit2) return true;
            if (lit2 > other.lit2) return false;
            return (red && !other.red);
        }

        bool operator==(const BinaryClause& other) const
        {
            return (lit1 == other.lit1
                    && lit2 == other.lit2
                    && red == other.red);
        }

        const Lit getLit1() const
        {
            return lit1;
        }

        const Lit getLit2() const
        {
            return lit2;
        }

        bool isRed() const
        {
            return red;
        }

    private:
        Lit lit1;
        Lit lit2;
        bool red;
};


inline std::ostream& operator<<(std::ostream& os, const BinaryClause val)
{
    os << val.getLit1() << " , " << val.getLit2()
    << " red: " << std::boolalpha << val.isRed() << std::noboolalpha;
    return os;
}

inline double ratio_for_stat(double a, double b)
{
    if (b == 0)
        return 0;
    return a/b;
}

inline double stats_line_percent(double num, double total)
{
    if (total == 0) {
        return 0;
    } else {
        return num/total*100.0;
    }
}

inline void print_value_kilo_mega(const uint64_t value)
{
    if (value > 20*1000ULL*1000ULL) {
        cout << " " << std::setw(4) << value/(1000ULL*1000ULL) << "M";
    } else if (value > 20ULL*1000ULL) {
        cout << " " << std::setw(4) << value/1000 << "K";
    } else {
        cout << " " << std::setw(5) << value;
    }
}

template<class T, class T2> void print_stats_line(
    string left
    , T value
    , T2 value2
    , string extra
) {
    cout
    << std::fixed << std::left << std::setw(27) << left
    << ": " << std::setw(11) << std::setprecision(2) << value
    << " (" << std::left << std::setw(9) << std::setprecision(2) << value2
    << " " << extra << ")"
    << std::right
    << endl;
}

inline void print_stats_line(
    string left
    , uint64_t value
    , uint64_t value2
    , uint64_t value3
) {
    cout
    << std::fixed << std::left << std::setw(27) << left
    << ": " << std::setw(11) << std::setprecision(2) << value
    << "/" << value2
    << "/" << value3
    << std::right
    << endl;
}

template<class T, class T2> void print_stats_line(
    string left
    , T value
    , string extra1
    , T2 value2
    , string extra2
) {
    cout
    << std::fixed << std::left << std::setw(27) << left
    << ": " << std::setw(11) << std::setprecision(2) << value
    << " " << extra1
    << " (" << std::left << std::setw(9) << std::setprecision(2) << value2
    << " " << extra2 << ")"
    << std::right
    << endl;
}

template<class T> void print_stats_line(
    string left
    , T value
    , string extra = ""
) {
    cout
    << std::fixed << std::left << std::setw(27) << left
    << ": " << std::setw(11) << std::setprecision(2)
    << value
    << " " << extra
    << std::right
    << endl;
}

struct AssignStats
{
    AssignStats() :
        sumAssignPos(0)
        , sumAssignNeg(0)
        , sumFlippedPolar(0)
        , sumFlippedPolarByDecider(0)
    {}

    uint64_t sumAssignPos;
    uint64_t sumAssignNeg;
    uint64_t sumFlippedPolar;
    uint64_t sumFlippedPolarByDecider;

};

inline void orderLits(
    Lit& lit1
    , Lit& lit2
    , Lit& lit3
 ) {
    if (lit1 > lit3)
        std::swap(lit1, lit3);

    if (lit1 > lit2)
        std::swap(lit1, lit2);

    if (lit2 > lit3)
        std::swap(lit2, lit3);

    //They are now ordered
    assert(lit1 < lit2);
    assert(lit2 < lit3);
}

inline vector<Lit> sortLits(const vector<Lit>& lits)
{
    vector<Lit> tmp(lits);

    std::sort(tmp.begin(), tmp.end());
    return tmp;
}

inline vector<Lit> vars_to_lits(const vector<Var>& vars)
{
    vector<Lit> ret;
    for(Var var: vars) {
        ret.push_back(Lit(var, false));
    }
    return ret;
}

} //end namespace

#endif //SOLVERTYPES_H
