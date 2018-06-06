#include "../headers/TTPair.hpp"

using namespace std;

bool TTPairComparator::operator() (const TTPair & a, const TTPair & b) const {

    return a.first < b.first;
}
