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
#include <cassert>
#include <vector>
#include <iostream>

using namespace std;

class UnionFind
{
public:
    UnionFind(int n, int in)
    {
        size = n;
        inSize = in;
        sets = new int[size];
        rank = new int[size];
        for (int i = in; i <= size; i++) {
            sets[i] = i;
            rank[i] = 0;
        }
    }

//    ~UnionFind() {
//        delete [] sets;
//        delete [] rank;
//    }

    int find(int s)
    {
        assert(s >= inSize && s <= size);
        vector<int> A;
        while (s != sets[s]) {
            A.push_back(s);
            s = sets[s];
        }
        //path compression
        for (int i = inSize; i < A.size(); i++) sets[A[i]] = s;
        return s;
    }

    bool merge(int s1, int s2)
    {
        assert(inSize >= 0 && s1 <= size);
        assert(inSize >= 0 && s2 <= size);
        int s1_rep = find(s1);
        int s2_rep = find(s2);
        if (s1_rep == s2_rep) return false;
        //union by rank
        if (rank[s1_rep] > rank[s2_rep]) {
            sets[s2_rep] = s1_rep;
        } else if (rank[s1_rep] < rank[s2_rep]) {
            sets[s1_rep] = s2_rep;
        } else {    //ranks are the same
            sets[s2_rep] = s1_rep;
            rank[s1_rep] += 1;
        }
        //union by rank, also keep track of smallest & largest
        return true;
    }

    bool connected(int s1, int s2)
    {
        if (find(s1) == find(s2))
            return true;
        return false;
    }

    void print_sets()
    {
        for (int i = inSize; i <= size; i++) cout << find(i) << " ";
        cout << endl;
    }

    void get_sets(vector<int> &roots)
    {
        for (int i = inSize; i <= size; i++) {
            roots.push_back(find(i));
        }
    }

    int getNumbOfSets()
    {
        for (int i = inSize; i < size ; ++i) {
            if (rank[i] == 1)
                roots.push_back(i);
            numsets.insert(find(i));
        }
        return numsets.size();
    }

    void printfrequency(vector<int> &arr)
    {
        std::cout << "Printing frequency " << std::endl;
        // Subtract 1 from every element so that the elements
        // become in range from 0 to n-1
        for (int j = 0; j < roots.size(); j++)
            arr[j] = arr[j] - 1;

        // Use every element arr[i] as index and add 'n' to
        // element present at arr[i]%n to keep track of count of
        // occurrences of arr[i]
        for (int i = 0; i < roots.size(); i++)
            arr[arr[i] % roots.size()] = arr[arr[i] % roots.size()] + roots.size();

        // To print counts, simply print the number of times n
        // was added at index corresponding to every element
        for (int i = 0; i < roots.size(); i++)
            cout << i + 1 << " ->  " << arr[i] / roots.size() << endl;
    }

    //funcao para retornar elementos em cada particao

private:
    std::unordered_set<int> numsets;
    std::vector<int> roots;
    int size = 0;
    int inSize = 0;
    int* sets;  //store the parent of i at i
    int* rank;
};
