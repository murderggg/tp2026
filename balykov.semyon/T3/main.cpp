#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <functional>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <numeric>
#include <sstream>
#include <iterator>
#include <string>
#include <vector>
//Variant 3 LESSAREA  INTERSECTIONS

using namespace std::placeholders;
struct Point
{
    int x, y;
};
struct Polygon
{
    std::vector< Point > points;
};

bool operator==(const Point& a, const Point& b) {
    return (a.x == b.x) && (a.y == b.y);
}
bool operator==(const Polygon& a, const Polygon& b) {
    if (a.points.size() == b.points.size()) {
        return std::equal(a.points.begin(),
        a.points.end(),
        b.points.begin());
    }
    return false;
}
std::istream& operator>>(std::istream& in, Point& a) {
    std::istream::sentry sentry(in);
    if (!sentry) {
        return in;
    }
    char s;
    int x, y;
    in >> s;
    if (s != '(') {
        in.setstate(std::ios::failbit);
        return in;
    }
    in >> x >> s;
    if (s != ';') {
        in.setstate(std::ios::failbit);
        return in;
    }
    in >> y >> s;
    if (s != ')') {
        in.setstate(std::ios::failbit);
        return in;
    }
    a.x = x;
    a.y = y;
    return in;
}
std::istream& operator>>(std::istream& in, Polygon& a) {
    std::istream::sentry sentry(in);
    if (!sentry) {
        return in;
    }
    Polygon temp;
    std::size_t n;
    in >> n;
    if (!(in) || n < 3) {
        in.setstate(std::ios::failbit);
        return in;
    }
    temp.points.resize(n);
    std::copy_n(std::istream_iterator<Point>(in), n, temp.points.begin());
    if (!in) {
        in.setstate(std::ios::failbit);
        return in;
    }
    a = std::move(temp);
    return in;
}
struct AreaCalculator {
    const std::vector<Point>& pts;
    std::size_t size;
    mutable std::size_t idx;

    explicit AreaCalculator(const std::vector<Point>& pts)
        : pts(pts), size(pts.size()), idx(0) {}

    long long operator()(long long acc, const Point& curr) const{
        const Point& next = pts[(idx + 1) % size];
        long long term = static_cast<long long>(curr.x) * next.y
            - static_cast<long long>(next.x) * curr.y;
        ++idx;
        return acc + term;
    }
};

double getArea(const Polygon& poly) {
    if (poly.points.size() < 3) return 0.0;
    long long sum = std::accumulate(
        poly.points.begin(), poly.points.end(), 0LL,
        AreaCalculator(poly.points)
    );
    return std::abs(sum) / 2.0;
}
struct SumAreaIfEven {
    double operator()(double acc, const Polygon& a) const{
        return (a.points.size() % 2 == 0) ? acc + getArea(a) : acc;
    }
};
struct SumAreaIfOdd {
    double operator()(double acc, const Polygon& a) const{
        return (a.points.size() % 2 != 0) ? acc + getArea(a) : acc;
    }
};
struct SumArea {
    double operator()(double acc, const Polygon& a) const{
        return acc + getArea(a);
    }
};

struct SumAreaIfVertexCount {
    std::size_t n;
    explicit SumAreaIfVertexCount(std::size_t n_) : n(n_) {}
    double operator()(double acc, const Polygon& p) const {
        return (p.points.size() == n) ? acc + getArea(p) : acc;
    }
};
struct CompareByArea {
    bool operator()(const Polygon& a, const Polygon& b) const {
        return getArea(a) < getArea(b);
    }
};

struct CompareByVertexCount {
    bool operator()(const Polygon& a, const Polygon& b) const {
        return a.points.size() < b.points.size();
    }
};

struct IsEvenVertex {
    bool operator()(const Polygon& p) const {
        return p.points.size() % 2 == 0;
    }
};

struct HasVertexCount {
    std::size_t target;
    explicit HasVertexCount(std::size_t n) : target(n) {}
    bool operator()(const Polygon& p) const {
        return p.points.size() == target;
    }
};

struct LessArea {
    double target;
    explicit LessArea(double t) : target(t) {}
    bool operator()(const Polygon& p) const {
        return getArea(p) < target;
    }
};

static int orientation(const Point& a, const Point& b, const Point& c) {
    long long val = static_cast<long long>(b.y - a.y) * (c.x - b.x) -
        static_cast<long long>(b.x - a.x) * (c.y - b.y);
    if (val == 0) return 0;
    if (val > 0) {
        return 1;
    }
    else {
        return 2;
    }
}

static bool onSegment(const Point& a, const Point& b, const Point& c) {
    return c.x >= std::min(a.x, b.x) && c.x <= std::max(a.x, b.x) &&
        c.y >= std::min(a.y, b.y) && c.y <= std::max(a.y, b.y);
}

static bool segmentsIntersect(const Point& a, const Point& b,
    const Point& c, const Point& d) {
    int o1 = orientation(a, b, c);
    int o2 = orientation(a, b, d);
    int o3 = orientation(c, d, a);
    int o4 = orientation(c, d, b);

    if (o1 != o2 && o3 != o4) return true;
    if (o1 == 0 && onSegment(a, b, c)) return true;
    if (o2 == 0 && onSegment(a, b, d)) return true;
    if (o3 == 0 && onSegment(c, d, a)) return true;
    if (o4 == 0 && onSegment(c, d, b)) return true;
    return false;
}

struct AccumulateCrosses{
    const Polygon& poly;
    const Point& p;
    int n;
    AccumulateCrosses(const Polygon& pl, const Point& pt, int N)
        : poly(pl), p(pt), n(N){}

    int operator()(int count, int i) const{
        int j = (i + 1) % n;
        const Point& a = poly.points[i];
        const Point& b = poly.points[j];

        bool cond1 = (a.y > p.y) != (b.y > p.y);
        if (!cond1){
            return count;
        }
        double xIntersect = (b.x - a.x) * static_cast<double>(p.y - a.y) /
            (b.y - a.y) + a.x;
        if (static_cast<double>(p.x) < xIntersect) {
            return count + 1;
        }
        return count;
    }
};

struct PointInPolygon{
    const Polygon& poly;
    PointInPolygon(const Polygon& p) : poly(p) {}
    bool operator()(const Point& p) const{
        int n = poly.points.size();
        std::vector<int> index(n);
        std::iota(index.begin(), index.end(), 0);
        int count = std::accumulate(index.begin(), index.end(), 0,
            AccumulateCrosses(poly, p, n));
        return count % 2 == 1;
    }
};
struct OneSideVsOneSideWrapper{
    const Polygon& poly1;
    const Polygon& poly2;
    int i;
    int j;

    OneSideVsOneSideWrapper(const Polygon& p1, const Polygon& p2, int idx)
        : poly1(p1), poly2(p2), i(idx), j((idx + 1) % p1.points.size()){}

    bool operator()(int k) const{
        int l = (k + 1) % poly2.points.size();
        return segmentsIntersect(poly1.points[i], poly1.points[j],
            poly2.points[k], poly2.points[l]);
    }
};

struct OneSideVsAllSides{
    const Polygon& poly1;
    const Polygon& poly2;
    int i;

    OneSideVsAllSides(const Polygon& p1, const Polygon& p2, int idx)
        : poly1(p1), poly2(p2), i(idx) {}

    bool operator()() const{
        int n2 = poly2.points.size();
        std::vector<int> index2(n2);
        std::iota(index2.begin(), index2.end(), 0);

        std::vector<bool> intersections(n2);
        std::transform(index2.begin(), index2.end(),
            intersections.begin(),
            OneSideVsOneSideWrapper(poly1, poly2, i));

        auto it = std::find(intersections.begin(), intersections.end(), true);
        return it != intersections.end();
    }
};

struct SideChecker{
    const Polygon& poly1;
    const Polygon& poly2;
    SideChecker(const Polygon& p1, const Polygon& p2) : poly1(p1), poly2(p2){}
    bool operator()(int i) const{
        return OneSideVsAllSides(poly1, poly2, i)();
    }
};

struct AnySideIntersects{
    const Polygon& poly1;
    const Polygon& poly2;
    AnySideIntersects(const Polygon& p1, const Polygon& p2) : poly1(p1), poly2(p2){}
    bool operator()() const{
        int n1 = poly1.points.size();
        std::vector<int> index1(n1);
        std::iota(index1.begin(), index1.end(), 0);
        std::vector<bool> hasIntersection(n1);
        std::transform(index1.begin(), index1.end(), hasIntersection.begin(),
            SideChecker(poly1, poly2));
        auto it = std::find(hasIntersection.begin(), hasIntersection.end(), true);
        return it != hasIntersection.end();
    }
};

struct PointToBool{
    const Polygon& poly;
    PointToBool(const Polygon& out) : poly(out){}
    bool operator()(const Point& p) const{
        PointInPolygon p1(poly);
        return p1(p);
    }
};

struct AnyPointInside{
    const Polygon& outer;
    const Polygon& inner;
    AnyPointInside(const Polygon& out, const Polygon& in) : outer(out), inner(in){}
    bool operator()() const{
        std::vector<bool> inside(inner.points.size());
        std::transform(inner.points.begin(), inner.points.end(), inside.begin(),
            PointToBool(outer));
        auto it = std::find(inside.begin(), inside.end(), true);
        return it != inside.end();
    }
};

struct PolygonsIntersect{
    bool operator()(const Polygon& poly1, const Polygon& poly2) const{
        if (AnySideIntersects(poly1, poly2)()){
            return true;
        }
        if (AnyPointInside(poly2, poly1)()){
            return true;
        }
        if (AnyPointInside(poly1, poly2)()){
            return true;
        }
        return false;
    }
};

std::vector<Polygon> readPolygons(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open())
    {
        throw std::runtime_error("Cannot open file:  " + filename);
    }
    std::vector<Polygon> polygons;
    std::string line;
    while (std::getline(file, line))
    {
        if (line.empty()){
            continue;
        }
        std::istringstream stream(line);
        Polygon temp;
        if (stream >> temp){
            char s;
            if (!(stream >> s))
            {
                polygons.push_back(temp);
            }
        }
    }
    return polygons;
}
int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <filename>\n";
        return 1;
    }

    try {
        std::vector<Polygon> polygons = readPolygons(argv[1]);

        std::cout << std::fixed << std::setprecision(1);
        std::string line;

        while (std::getline(std::cin, line)) {
            if (line.empty()) continue;
            std::istringstream iss(line);
            std::string cmd;
            iss >> cmd;
            if (cmd == "AREA") {
                std::string param;
                iss >> param;

                char extra;
                if (iss >> extra) {
                    std::cout << "<INVALID COMMAND>\n";
                    continue;
                }

                if (param == "EVEN") {
                    double res = std::accumulate(polygons.begin(), polygons.end(),
                        0.0, SumAreaIfEven());
                    std::cout << res << "\n";
                }
                else if (param == "ODD") {
                    double res = std::accumulate(polygons.begin(), polygons.end(),
                        0.0, SumAreaIfOdd());
                    std::cout << res << "\n";
                }
                else if (param == "MEAN") {
                    if (polygons.empty()) {
                        std::cout << "<INVALID COMMAND>\n";
                        continue;
                    }
                    double total = std::accumulate(polygons.begin(), polygons.end(),
                        0.0, SumArea());
                    std::cout << total / polygons.size() << "\n";
                }
                else {
                    try {
                        std::size_t pos = 0;
                        std::size_t vcount = std::stoul(param, &pos);
                        if (pos != param.size() || vcount < 3) {
                            throw std::invalid_argument("");
                        }
                        double res = std::accumulate(polygons.begin(), polygons.end(),
                            0.0, SumAreaIfVertexCount(vcount));
                        std::cout << res << "\n";
                    }
                    catch (...) {
                        std::cout << "<INVALID COMMAND>\n";
                    }
                }
            }
            else if (cmd == "MAX") {
                std::string param;
                iss >> param;
                char extra;
                if (iss >> extra) {
                    std::cout << "<INVALID COMMAND>\n";
                    continue;
                }

                if (polygons.empty()) {
                    std::cout << "<INVALID COMMAND>\n";
                    continue;
                }

                if (param == "AREA") {
                    auto it = std::max_element(polygons.begin(), polygons.end(),
                        CompareByArea());
                    std::cout << getArea(*it) << "\n";
                }
                else if (param == "VERTEXES") {
                    auto it = std::max_element(polygons.begin(), polygons.end(),
                        CompareByVertexCount());
                    std::cout << it->points.size() << "\n";
                }
                else {
                    std::cout << "<INVALID COMMAND>\n";
                }
            }
            else if (cmd == "MIN") {
                std::string param;
                iss >> param;
                char extra;
                if (iss >> extra) {
                    std::cout << "<INVALID COMMAND>\n";
                    continue;
                }
                if (polygons.empty()) {
                    std::cout << "<INVALID COMMAND>\n";
                    continue;
                }

                if (param == "AREA") {
                    auto it = std::min_element(polygons.begin(), polygons.end(),
                        CompareByArea());
                    std::cout << getArea(*it) << "\n";
                }
                else if (param == "VERTEXES") {
                    auto it = std::min_element(polygons.begin(), polygons.end(),
                        CompareByVertexCount());
                    std::cout << it->points.size() << "\n";
                }
                else {
                    std::cout << "<INVALID COMMAND>\n";
                }
            }
            else if (cmd == "COUNT") {
                std::string param;
                iss >> param;

                char extra;
                if (iss >> extra) {
                    std::cout << "<INVALID COMMAND>\n";
                    continue;
                }

                if (param == "EVEN") {
                    auto cnt = std::count_if(polygons.begin(), polygons.end(),
                        IsEvenVertex());
                    std::cout << cnt << "\n";
                }
                else if (param == "ODD") {
                    auto cnt = std::count_if(polygons.begin(), polygons.end(),
                        std::bind(std::logical_not<bool>(),
                            std::bind(IsEvenVertex(), _1)));
                    std::cout << cnt << "\n";
                }
                else {
                    try {
                        std::size_t pos = 0;
                        std::size_t vcount = std::stoul(param, &pos);
                        if (pos != param.size() || vcount < 3) {
                            throw std::invalid_argument("");
                        }
                        auto cnt = std::count_if(polygons.begin(), polygons.end(),
                            HasVertexCount(vcount));
                        std::cout << cnt << "\n";
                    }
                    catch (...) {
                        std::cout << "<INVALID COMMAND>\n";
                    }
                }
            }
            else if (cmd == "LESSAREA") {
                Polygon target;
                if (!(iss >> target)) {
                    std::cout << "<INVALID COMMAND>\n";
                    continue;
                }
                char extra;
                if (iss >> extra) {
                    std::cout << "<INVALID COMMAND>\n";
                    continue;
                }

                double targetArea = getArea(target);
                auto cnt = std::count_if(polygons.begin(), polygons.end(),
                    LessArea(targetArea));
                std::cout << cnt << "\n";
            }
            else if (cmd == "INTERSECTIONS") {
                Polygon target;
                if (!(iss >> target)) {
                    std::cout << "<INVALID COMMAND>\n";
                    continue;
                }
                char extra;
                if (iss >> extra) {
                    std::cout << "<INVALID COMMAND>\n";
                    continue;
                }
                auto cnt = std::count_if(polygons.begin(), polygons.end(),
                    std::bind(PolygonsIntersect(), _1, target));
                std::cout << cnt << "\n";
            }
            else {

                std::cout << "<INVALID COMMAND>\n";
            }
        }
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << "\n";
        return 1;

    }
    return 0;
}
