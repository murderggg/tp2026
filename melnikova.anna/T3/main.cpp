#include <iostream>
#include <string>
#include <functional>
#include <algorithm>
#include <fstream>
#include <numeric>
#include <iomanip>
#include <cmath>
#include <vector>
#include <utility>
#include <limits>
#include <iterator>

const int MAX_CRD = std::numeric_limits<int>::max();
//9. RMECHO INTERSECTIONS

using namespace std::placeholders;

struct Point
{
    int x, y;
};
struct Polygon
{
    std::vector< Point > points;
};

using Segment = std::pair<Point, Point>;

Segment makeSegment(Point a, Point b) {
    return std::make_pair(a, b);
}
std::istream& operator>>(std::istream& is, Point& p)
{
    std::istream::sentry sentry(is);
    if (!is) {
        return is;
    }
    char c1, c2, c3;
    is >> c1 >> p.x >> c2 >> p.y >> c3;
    if (is && c1 == '(' && c2 == ';' && c3 == ')') {
        return is;
    }
    is.setstate(std::ios_base::failbit);
    return is;
}

bool operator==(const Point& p1, const Point& p2) {
    return p1.x == p2.x && p1.y == p2.y;
}

bool operator==(const Polygon& p1, const Polygon& p2) {
    return p1.points == p2.points;
}

std::istream& operator>>(std::istream& is, Polygon& pg) {
    std::istream::sentry sentry(is);
    if (!is) {
        return is;
    }
    size_t count;
    if (!(is >> count) || count < 3) {
        is.setstate(std::ios_base::failbit);
        return is;
    }
    std::vector<Point> pts;
    std::copy_n(std::istream_iterator<Point>(is), count, std::back_inserter(pts));
    if (is) {
        pg.points = pts;
    }
    return is;
}

double calcAreaTerm(Point a, Point b) {
    return static_cast<double>(a.x * b.y - a.y * b.x);
}
double area(const Polygon& pg) {
    if (pg.points.size() < 3) return 0.0;
    std::vector<double> terms;
    std::transform(pg.points.begin(), pg.points.end() - 1, pg.points.begin() + 1, std::back_inserter(terms), calcAreaTerm);
    terms.push_back(calcAreaTerm(pg.points.back(), pg.points.front()));
    double sum = std::accumulate(terms.begin(), terms.end(), 0.0);
    return std::abs(sum) / 2.0;
}
struct AreaCalculator {
    double operator()(const Polygon& pg) const {
        return area(pg);
    }
};
struct isEven {
    bool operator()(const Polygon& pg) const {
        return pg.points.size() % 2 == 0;
    }
};

bool isEvenFunc(const Polygon& pg) { return pg.points.size() % 2 == 0; }
bool isOddFunc(const Polygon& pg) { return pg.points.size() % 2 != 0; }
bool hasNumVertices(const Polygon& pg, size_t n) { return pg.points.size() == n; }

double addAreaIfEven(double sum, const Polygon& p) { return isEvenFunc(p) ? sum + area(p) : sum; }
double addAreaIfOdd(double sum, const Polygon& p) { return isOddFunc(p) ? sum + area(p) : sum; }
double addAreaIfNum(double sum, const Polygon& p, size_t n) { return hasNumVertices(p, n) ? sum + area(p) : sum; }
double addArea(double sum, const Polygon& p) { return sum + area(p); }

bool compareArea(const Polygon& p1, const Polygon& p2) { return area(p1) < area(p2); }
bool compareVertexes(const Polygon& p1, const Polygon& p2) { return p1.points.size() < p2.points.size(); }
bool checkDuplicateTarget(const Polygon& p1, const Polygon& p2, const Polygon& target) { return p1 == target && p2 == target; }

std::vector<Segment> getSegments(const Polygon& pg) {
    std::vector<Segment> segments;
    if (pg.points.empty()) {
        return segments;
    }
    std::transform(pg.points.begin(), pg.points.end() - 1, pg.points.begin() + 1, std::back_inserter(segments), makeSegment);
    segments.push_back(makeSegment(pg.points.back(), pg.points.front()));
    return segments;
}

long long pseudoscalar(Point a, Point b, Point c) {
    return (1LL * (b.x - a.x) * (c.y - a.y) - 1LL * (b.y - a.y) * (c.x - a.x));
}

bool pointOnSegment(Point a, Point b, Point p) {
    return (p.x >= std::min(a.x, b.x) && p.x <= std::max(a.x, b.x) && p.y >= std::min(a.y, b.y) && p.y <= std::max(a.y, b.y));
}

bool checkSegmentsIntersect(Point x1, Point y1, Point x2, Point y2) {
    long long d1 = pseudoscalar(x1, y1, x2);
    long long d2 = pseudoscalar(x1, y1, y2);
    long long d3 = pseudoscalar(x2, y2, x1);
    long long d4 = pseudoscalar(x2, y2, y1);
    if (((d1 > 0 && d2 < 0) || (d2 > 0 && d1 < 0)) && ((d3 > 0 && d4 < 0) || (d4 > 0 && d3 < 0))) {
        return true;
    }
    if (d1 == 0 && pointOnSegment(x1, y1, x2)) {
        return true;
    }
    if (d2 == 0 && pointOnSegment(x1, y1, y2)) {
        return true;
    }
    if (d3 == 0 && pointOnSegment(x2, y2, x1)) {
        return true;
    }
    if (d4 == 0 && pointOnSegment(x2, y2, y1)) {
        return true;
    }
    return false;
}

bool checkSegmentsIntersectForPair(const Segment& sg, Point ra, Point rb) {
    return checkSegmentsIntersect(sg.first, sg.second, ra, rb);
}
bool isPointInside(Point a, const Polygon& pg, int max_x) {
    auto segments = getSegments(pg);
    Point rayEnd = { max_x + 1, a.y + 1 };
    int intersections = std::count_if(segments.begin(), segments.end(), std::bind(checkSegmentsIntersectForPair, _1, a, rayEnd));
    return (intersections % 2 == 1);
}

bool segmentsIntersectWrapper(const Segment& s, const Polygon& pg) {
    auto segments = getSegments(pg);
    return std::any_of(segments.begin(), segments.end(), std::bind(checkSegmentsIntersectForPair, _1, s.first, s.second));
}

bool pointInsideWrapper(Point p, const Polygon& poly, int max_x) {
    return isPointInside(p, poly, max_x);
}

bool comparePointX(Point a, Point b) {
    return a.x < b.x;
}
int getPolygonMaxX(const Polygon& p) {
    if (p.points.empty()) {
        return 0;
    }
    return std::max_element(p.points.begin(), p.points.end(), comparePointX)->x;
}
bool comparePolygonMaxX(const Polygon& a, const Polygon& b) {
    return getPolygonMaxX(a) < getPolygonMaxX(b);
}
bool polygonsIntersect(const Polygon& p1, const Polygon& p2, int max_x) {
    auto segments = getSegments(p1);
    if (std::any_of(segments.begin(), segments.end(), std::bind(segmentsIntersectWrapper, _1, p2))) {
        return true;
    }
    if (std::any_of(p1.points.begin(), p1.points.end(), std::bind(pointInsideWrapper, _1, p2, max_x))) {
        return true;
    }
    if (std::any_of(p2.points.begin(), p2.points.end(), std::bind(pointInsideWrapper, _1, p1, max_x))) {
        return true;
    }
    return false;
}

int getGlobalMaxX(const std::vector<Polygon>& polys) {
    if (polys.empty()) return 0;
    return getPolygonMaxX(*std::max_element(polys.begin(), polys.end(), comparePolygonMaxX));
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <filename>\n";
        return 1;
    }
    std::ifstream file(argv[1]);
    if (!file) {
        std::cerr << "Error opening file\n";
        return 1;
    }
    std::vector<Polygon> polys;
    std::copy(std::istream_iterator<Polygon>(file), std::istream_iterator<Polygon>(), std::back_inserter(polys));
    int globalMaxX = getGlobalMaxX(polys);
    std::string cmd;
    while (std::cin >> cmd) {
        if (cmd == "AREA") {
            std::string subcmd;
            if (!(std::cin >> subcmd)) break;

            double res = 0.0;
            if (subcmd == "EVEN") res = std::accumulate(polys.begin(), polys.end(), 0.0, addAreaIfEven);
            else if (subcmd == "ODD") res = std::accumulate(polys.begin(), polys.end(), 0.0, addAreaIfOdd);
            else if (subcmd == "MEAN") {
                if (polys.empty()) { std::cout << "<INVALID COMMAND>\n"; continue; }
                res = std::accumulate(polys.begin(), polys.end(), 0.0, addArea) / polys.size();
            }
            else {
                try {
                    size_t n = std::stoull(subcmd);
                    res = std::accumulate(polys.begin(), polys.end(), 0.0, std::bind(addAreaIfNum, _1, _2, n));
                }
                catch (...) { std::cout << "<INVALID COMMAND>\n"; continue; }
            }
            std::cout << std::fixed << std::setprecision(1) << res << "\n";
        }
        else if (cmd == "MIN") {
            std::string subcmd;
            if (!(std::cin >> subcmd) || polys.empty()) { std::cout << "<INVALID COMMAND>\n"; continue; }
            if (subcmd == "AREA") {
                std::cout << std::fixed << std::setprecision(1) << area(*std::min_element(polys.begin(), polys.end(), compareArea)) << "\n";
            }
            else if (subcmd == "VERTEXES") {
                std::cout << std::min_element(polys.begin(), polys.end(), compareVertexes)->points.size() << "\n";
            }
            else { std::cout << "<INVALID COMMAND>\n"; }
        }
        else if (cmd == "MAX") {
            std::string subcmd;
            if (!(std::cin >> subcmd) || polys.empty()) { std::cout << "<INVALID COMMAND>\n"; continue; }
            if (subcmd == "AREA") {
                std::cout << std::fixed << std::setprecision(1) << area(*std::max_element(polys.begin(), polys.end(), compareArea)) << "\n";
            }
            else if (subcmd == "VERTEXES") {
                std::cout << std::max_element(polys.begin(), polys.end(), compareVertexes)->points.size() << "\n";
            }
            else { std::cout << "<INVALID COMMAND>\n"; }
        }
        else if (cmd == "COUNT") {
            std::string subcmd;
            if (!(std::cin >> subcmd)) break;

            if (subcmd == "EVEN") std::cout << std::count_if(polys.begin(), polys.end(), isEvenFunc) << "\n";
            else if (subcmd == "ODD") std::cout << std::count_if(polys.begin(), polys.end(), isOddFunc) << "\n";
            else {
                try {
                    size_t n = std::stoull(subcmd);
                    std::cout << std::count_if(polys.begin(), polys.end(), std::bind(hasNumVertices, _1, n)) << "\n";
                }
                catch (...) { std::cout << "<INVALID COMMAND>\n"; continue; }
            }
        }
        else if (cmd == "RMECHO") {
            Polygon target;
            if (!(std::cin >> target)) { std::cout << "<INVALID COMMAND>\n"; std::cin.clear(); continue; }

            auto it = std::unique(polys.begin(), polys.end(), std::bind(checkDuplicateTarget, _1, _2, target));
            int removed = std::distance(it, polys.end());
            polys.erase(it, polys.end());
            std::cout << removed << "\n";
        }
        else if (cmd == "INTERSECTIONS") {
            Polygon target;
            if (!(std::cin >> target)) { std::cout << "<INVALID COMMAND>\n"; std::cin.clear(); continue; }

            int currentMaxX = std::max(globalMaxX, getPolygonMaxX(target));
            std::cout << std::count_if(polys.begin(), polys.end(), std::bind(polygonsIntersect, _1, target, currentMaxX)) << "\n";
        }
        else {
            std::cout << "<INVALID COMMAND>\n";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    }
    return 0;
}
