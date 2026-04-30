#ifndef POLYGON_H
#define POLYGON_H

#include <vector>
#include <iostream>

struct Point {
    int x, y;
};

struct Polygon {
    std::vector<Point> points;
};

std::istream& operator>>(std::istream &in, Point &p);
std::istream& operator>>(std::istream &in, Polygon &pol);
bool operator==(const Point& lhs, const Point& rhs);
bool operator==(const Polygon& lhs, const Polygon& rhs);

#endif
