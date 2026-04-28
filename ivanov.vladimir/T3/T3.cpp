#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <numeric>
#include <iomanip>
#include <limits>
#include <cmath>
#include <stdexcept>
#include <iterator>
#include <functional>

struct Point {
    int x;
    int y;
};

bool operator==(const Point& a, const Point& b) {
    return a.x == b.x && a.y == b.y;
}

struct Polygon {
    std::vector<Point> points;
    Polygon() = default;
    Polygon(const Polygon& other) = default;
    Polygon(Polygon&& other) noexcept = default;
    Polygon& operator=(const Polygon& other) = default;
    Polygon& operator=(Polygon&& other) noexcept = default;
    ~Polygon() = default;
};

bool operator==(const Polygon& a, const Polygon& b) {
    if (a.points.size() != b.points.size()) {
        return false;
    }
    return std::equal(a.points.begin(), a.points.end(),
                      b.points.begin());
}

std::istream& operator>>(std::istream& in, Point& pt) {
    std::istream::sentry sentry(in);
    if (!sentry) return in;
    char c1 = 0, c2 = 0, c3 = 0;
    int x = 0, y = 0;
    in >> c1 >> x >> c2 >> y >> c3;
    if (in && c1 == '(' && c2 == ';' && c3 == ')') {
        pt.x = x;
        pt.y = y;
    } else {
        in.setstate(std::ios::failbit);
    }
    return in;
}

std::istream& operator>>(std::istream& in, Polygon& p) {
    std::istream::sentry sentry(in);
    if (!sentry) return in;
    size_t count = 0;
    if (!(in >> count) || count < 3) {
        in.setstate(std::ios::failbit);
        return in;
    }
    std::vector<Point> pts;
    pts.reserve(count);
    auto it = std::istream_iterator<Point>(in);
    std::copy_n(it, count, std::back_inserter(pts));
    if (in && pts.size() == count) {
        p.points = std::move(pts);
    } else {
        in.setstate(std::ios::failbit);
    }
    return in;
}

struct AreaAcc {
    Point first;
    Point prev;
    double sum;
};

AreaAcc accumulateArea(AreaAcc acc, const Point& curr) {
    double term = static_cast<double>(acc.prev.x) * curr.y;
    term -= static_cast<double>(curr.x) * acc.prev.y;
    acc.sum += term;
    acc.prev = curr;
    return acc;
}

double getArea(const Polygon& p) {
    if (p.points.size() < 3) return 0.0;
    AreaAcc init{p.points.front(), p.points.front(), 0.0};
    auto start = std::next(p.points.begin());
    AreaAcc res = std::accumulate(start, p.points.end(),
                                  init, accumulateArea);
    res.sum += static_cast<double>(res.prev.x) * res.first.y;
    res.sum -= static_cast<double>(res.first.x) * res.prev.y;
    return std::abs(res.sum) / 2.0;
}

int ccw(Point a, Point b, Point c) {
    long long v = 1LL * (b.x - a.x) * (c.y - a.y);
    v -= 1LL * (b.y - a.y) * (c.x - a.x);
    if (v > 0) return 1;
    if (v < 0) return -1;
    return 0;
}

bool onSeg(Point p, Point a, Point b) {
    bool x = p.x >= std::min(a.x, b.x) && p.x <= std::max(a.x, b.x);
    bool y = p.y >= std::min(a.y, b.y) && p.y <= std::max(a.y, b.y);
    return x && y;
}

bool intersectL(Point a, Point b, Point c, Point d) {
    int o1 = ccw(a, b, c);
    int o2 = ccw(a, b, d);
    int o3 = ccw(c, d, a);
    int o4 = ccw(c, d, b);
    if (o1 != o2 && o3 != o4) return true;
    if (o1 == 0 && onSeg(c, a, b)) return true;
    if (o2 == 0 && onSeg(d, a, b)) return true;
    if (o3 == 0 && onSeg(a, c, d)) return true;
    if (o4 == 0 && onSeg(b, c, d)) return true;
    return false;
}

struct RayChecker {
    Point p;
    const Polygon* poly;
    bool operator()(size_t i) const {
        size_t j = (i == 0) ? poly->points.size() - 1 : i - 1;
        Point p1 = poly->points[i];
        Point p2 = poly->points[j];
        if ((p1.y > p.y) != (p2.y > p.y)) {
            double xI = static_cast<double>(p2.x - p1.x) * (p.y - p1.y);
            xI = xI / (p2.y - p1.y) + p1.x;
            return p.x < xI;
        }
        return false;
    }
};

bool isPointIn(const Polygon& poly, const Point& p) {
    std::vector<size_t> idx(poly.points.size());
    std::iota(idx.begin(), idx.end(), 0);
    RayChecker chk{p, &poly};
    return std::count_if(idx.begin(), idx.end(), chk) % 2 != 0;
}

struct EdgeChecker {
    Point p1, p2;
    const Polygon* b;
    bool operator()(size_t idxB) const {
        Point p3 = b->points[idxB];
        Point p4 = b->points[(idxB + 1) % b->points.size()];
        return intersectL(p1, p2, p3, p4);
    }
};

struct PolyChecker {
    const Polygon* a;
    const Polygon* b;
    bool operator()(size_t idxA) const {
        Point p1 = a->points[idxA];
        Point p2 = a->points[(idxA + 1) % a->points.size()];
        std::vector<size_t> idxB(b->points.size());
        std::iota(idxB.begin(), idxB.end(), 0);
        return std::any_of(idxB.begin(), idxB.end(),
                           EdgeChecker{p1, p2, b});
    }
};

bool intersectPoly(const Polygon& a, const Polygon& b) {
    std::vector<size_t> idxA(a.points.size());
    std::iota(idxA.begin(), idxA.end(), 0);
    if (std::any_of(idxA.begin(), idxA.end(), PolyChecker{&a, &b})) {
        return true;
    }
    return isPointIn(b, a.points[0]) || isPointIn(a, b.points[0]);
}

bool isEven(const Polygon& p) { return p.points.size() % 2 == 0; }
bool isOdd(const Polygon& p) { return p.points.size() % 2 != 0; }
bool isSize(const Polygon& p, size_t n) { return p.points.size() == n; }

double addEven(double s, const Polygon& p) {
    return isEven(p) ? s + getArea(p) : s;
}
double addOdd(double s, const Polygon& p) {
    return isOdd(p) ? s + getArea(p) : s;
}
double addSize(double s, const Polygon& p, size_t n) {
    return isSize(p, n) ? s + getArea(p) : s;
}
double addArea(double s, const Polygon& p) { return s + getArea(p); }

bool cmpA(const Polygon& a, const Polygon& b) {
    return getArea(a) < getArea(b);
}
bool cmpV(const Polygon& a, const Polygon& b) {
    return a.points.size() < b.points.size();
}
bool isTgtPair(const Polygon& a, const Polygon& b, const Polygon& t) {
    return a == t && b == t;
}

size_t parseSize(const std::string& str) {
    size_t pos = 0;
    size_t val = std::stoull(str, &pos);
    if (pos != str.length()) throw std::invalid_argument("");
    return val;
}

int main(int argc, char* argv[]) {
    if (argc < 2) return 1;
    std::ifstream file(argv[1]);
    if (!file) return 1;
    std::vector<Polygon> polygons;
    auto it_poly = std::istream_iterator<Polygon>(file);
    std::copy(it_poly, std::istream_iterator<Polygon>(),
              std::back_inserter(polygons));
    std::cout << std::fixed << std::setprecision(1);
    std::string line;
    while (std::getline(std::cin, line)) {
        if (line.empty()) continue;
        std::istringstream iss(line);
        std::string cmd, arg;
        if (!(iss >> cmd)) continue;
        try {
            if (cmd == "AREA") {
                iss >> arg;
                if (arg == "EVEN") {
                    std::cout << std::accumulate(polygons.begin(),
                        polygons.end(), 0.0, addEven) << '\n';
                } else if (arg == "ODD") {
                    std::cout << std::accumulate(polygons.begin(),
                        polygons.end(), 0.0, addOdd) << '\n';
                } else if (arg == "MEAN") {
                    if (polygons.empty()) throw std::invalid_argument("");
                    double s = std::accumulate(polygons.begin(),
                        polygons.end(), 0.0, addArea);
                    std::cout << (s / polygons.size()) << '\n';
                } else {
                    size_t n = parseSize(arg);
                    using namespace std::placeholders;
                    auto f = std::bind(addSize, _1, _2, n);
                    std::cout << std::accumulate(polygons.begin(),
                        polygons.end(), 0.0, f) << '\n';
                }
            } else if (cmd == "MAX") {
                iss >> arg;
                if (polygons.empty()) throw std::invalid_argument("");
                if (arg == "AREA") {
                    auto it = std::max_element(polygons.begin(),
                        polygons.end(), cmpA);
                    std::cout << getArea(*it) << '\n';
                } else {
                    auto it = std::max_element(polygons.begin(),
                        polygons.end(), cmpV);
                    std::cout << it->points.size() << '\n';
                }
            } else if (cmd == "COUNT") {
                iss >> arg;
                if (arg == "EVEN") {
                    std::cout << std::count_if(polygons.begin(),
                        polygons.end(), isEven) << '\n';
                } else if (arg == "ODD") {
                    std::cout << std::count_if(polygons.begin(),
                        polygons.end(), isOdd) << '\n';
                } else {
                    size_t n = parseSize(arg);
                    using namespace std::placeholders;
                    auto f = std::bind(isSize, _1, n);
                    std::cout << std::count_if(polygons.begin(),
                        polygons.end(), f) << '\n';
                }
            } else if (cmd == "RMECHO") {
                Polygon t;
                if (!(iss >> t)) throw std::invalid_argument("");
                size_t old_s = polygons.size();
                using namespace std::placeholders;
                auto f = std::bind(isTgtPair, _1, _2, t);
                auto it = std::unique(polygons.begin(), polygons.end(), f);
                polygons.erase(it, polygons.end());
                std::cout << old_s - polygons.size() << '\n';
            } else if (cmd == "INTERSECTIONS") {
                Polygon t;
                if (!(iss >> t)) throw std::invalid_argument("");
                using namespace std::placeholders;
                auto f = std::bind(intersectPoly, _1, t);
                std::cout << std::count_if(polygons.begin(),
                    polygons.end(), f) << '\n';
            } else throw std::invalid_argument("");
        } catch (...) { std::cout << "<INVALID COMMAND>\n"; }
    }
    return 0;
}
