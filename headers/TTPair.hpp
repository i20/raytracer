#ifndef _TTPAIR_HPP
#define _TTPAIR_HPP

#include <set>

#include "../headers/Triangle.hpp"

using namespace std;

typedef pair<float, const Triangle *> TTPair;

class TTPairComparator {

    public:
        bool operator() (const TTPair & a, const TTPair & b) const;
};

typedef multiset<TTPair, TTPairComparator> TTPairList;

#endif
