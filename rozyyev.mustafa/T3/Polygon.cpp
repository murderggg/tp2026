#include "Polygon.h"
#include <algorithm>
#include <iterator>

std::istream& operator>>(std::istream &in, Point &p) {
    char c1, c2, c3;
    if (!(in >> c1 >> p.x >> c2 >> p.y >> c3) || c1 != '(' || c2 != ';' || c3 != ')') {
        in.setstate(std::ios_base::failbit);
    }
    return in;
}

std::istream& operator>>(std::istream &in, Polygon &pol) {
    size_t size{};
    if (!(in >> size) || size < 3) {
        in.setstate(std::ios_base::failbit);
        return in;
    }

    std::vector<Point> temp;
    temp.reserve(size);

    std::copy_n(std::istream_iterator<Point>(in), size, std::back_inserter(temp));

    if (temp.size() != size) {
        in.setstate(std::ios_base::failbit);
        return in;
    }

    pol.points = std::move(temp);
    return in;
}

bool operator==(const Point& lhs, const Point& rhs) {
    return lhs.x == rhs.x && lhs.y == rhs.y;
}

bool operator==(const Polygon& lhs, const Polygon& rhs) {
    return lhs.points == rhs.points;
}
