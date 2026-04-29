#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <numeric>
#include <functional>
#include <cmath>
#include <iomanip>
#include <cctype>


using namespace std::placeholders;


struct Point {
    int x, y;
} ;

bool operator==(const Point& a, const Point& b) {
    return ((a.x == b.x) && (a.y == b.y));
}


struct Polygon {
    std::vector<Point> points;
} ;

bool operator==(const Polygon& a, const Polygon& b) {
    if (a.points.size() != b.points.size()) {
        return false;
    }
    return std::equal(a.points.begin(), a.points.end(), b.points.begin());
}


double area(const Polygon& poly) {
    const auto& p = poly.points;
    int n = p.size();
    if (n < 3) return 0.0;

    long long sum = 0;
    for (int i = 0; i < n; ++i) {
        int j = (i + 1) % n;
        sum += (static_cast<long long>(p[i].x) * p[j].y) - (static_cast<long long>(p[j].x) * p[i].y);
    }
    return std::abs(sum) / 2.0;
}

struct AreaCalculator {
    double operator()(const Polygon& p) const {
        return area(p);
    }
} ;


int cross(const Point& a, const Point& b, const Point& c) {
    return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
}

bool onSegment(const Point& a, const Point& b, const Point& c) {
    return std::min(a.x, b.x) <= c.x && c.x <= std::max(a.x, b.x) &&
           std::min(a.y, b.y) <= c.y && c.y <= std::max(a.y, b.y);
}

bool segmentIntersect(const Point& a, const Point& b, const Point& c, const Point& d) {
    int o1 = cross(a, b, c);
    int o2 = cross(a, b, d);
    int o3 = cross(c, d, a);
    int o4 = cross(c, d, b);

    if (((o1 > 0 && o2 < 0) || (o1 < 0 && o2 > 0)) && ((o3 > 0 && o4 < 0) || (o3 < 0 && o4 > 0))) {
        return true;
    }

    if (o1 == 0 && onSegment(a, b, c)) {
        return true;
    }
    if (o2 == 0 && onSegment(a, b, d)) {
        return true;
    }
    if (o3 == 0 && onSegment(c, d, a)) {
        return true;
    }
    if (o4 == 0 && onSegment(c, d, b)) {
        return true;
    }

    return false;
}

bool polygonIntersect(const Polygon& a, const Polygon& b) {
    size_t na = a.points.size();
    size_t nb = b.points.size();

    for (size_t i = 0; i < na; ++i) {
        const Point& a1 = a.points[i];
        const Point& a2 = a.points[(i + 1) % na];

        for (size_t j = 0; j < nb; ++j) {
            const Point& b1 = b.points[j];
            const Point& b2 = b.points[(j + 1) % nb];

            if (segmentIntersect(a1, a2, b1, b2)) {
                return true;
            }
        }
    }
    return false;
}


struct IsVertexCountOdd {
    bool operator()(const Polygon& p) const {
        return p.points.size() % 2 != 0;
    }
} ;

struct IsVertexCountEven {
    bool operator()(const Polygon& p) const {
        return p.points.size() % 2 == 0;
    }
} ;

struct HasVertexCount {
    size_t num;
    HasVertexCount(size_t count) : num(count) {}

    bool operator()(const Polygon& p) const {
        return p.points.size() == num;
    }
};


bool isVertexCountEqual(const Polygon& p, size_t n) {
    return p.points.size() == n;
}


Polygon parsePolygon(const std::string& line) {
    std::istringstream iss(line);
    int n;
    if (!(iss >> n) || n < 3) {
        return {};
    }

    Polygon poly;

    for (int i = 0; i < n; ++i) {
        char open, semicolon, close;
        int x, y;
        if (!(iss >> open >> x >> semicolon >> y >> close)) {
            return {};
        }
        if (open != '(' || semicolon != ';' || close != ')') {
            return {};
        }
        poly.points.push_back({x, y});
    }

    std::string extra;
    if (iss >> extra) {
        return {};
    }

    return poly;
}


struct AreaSummator {
    std::function<bool(const Polygon&)> filter;

    AreaSummator(std::function<bool(const Polygon&)> f) : filter(f) {}

    double operator()(double acc, const Polygon& p) const {
        return filter(p) ? acc + area(p) : acc;
    }
} ;

struct CompareByArea {
    bool operator()(const Polygon& a, const Polygon& b) const {
        return area(a) < area(b);
    }
} ;

struct CompareByVertexes {
    bool operator()(const Polygon& a, const Polygon& b) const {
        return a.points.size() < b.points.size();
    }
} ;


struct BothEqualToTarget {
    Polygon target;

    BothEqualToTarget(const Polygon& t) : target(t) {}

    bool operator()(const Polygon& a, const Polygon& b) const {
        return (a == target) && (b == target);
    }
} ;


struct IntersectWithTarget {
    Polygon target;

    IntersectWithTarget(const Polygon& t) : target(t) {}

    bool operator()(const Polygon& p) const {
        if (p == target) {
            return false;
        }
        return polygonIntersect(p, target);
    }
} ;


int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Error: no filename provided\n";
        return 1;
    }

    std::ifstream file(argv[1]);
    if (!file) {
        std::cerr << "Error: cannot open file\n";
        return 1;
    }

    std::vector<Polygon> polygons;
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        Polygon p = parsePolygon(line);
        if (!p.points.empty()) {
            polygons.push_back(p);
        }
    }

    std::cout << std::fixed << std::setprecision(1);

    std::string cmd;
    while (std::cin >> cmd) {
        if (cmd == "AREA") {
            std::string sub;
            std::cin >> sub;

            if (sub == "ODD") {
                double res = std::accumulate(polygons.begin(), polygons.end(), 0.0,
                AreaSummator(IsVertexCountOdd()));
                std::cout << res << std::endl;
            }

            else if (sub == "EVEN") {
                double res = std::accumulate(polygons.begin(), polygons.end(), 0.0,
                AreaSummator(IsVertexCountEven()));
                std::cout << res << std::endl;
            }

            else if (sub == "MEAN") {
                if (polygons.empty()) {
                    std::cout << "<INVALID COMMAND>\n";
                    continue;
                }
                double res = std::accumulate(polygons.begin(), polygons.end(), 0.0,
                AreaSummator([](const Polygon&){
                    return true;
                }));
                std::cout << res / polygons.size() << std::endl;
            }

            else {
                bool isNumber = !sub.empty() && std::all_of(sub.begin(), sub.end(), ::isdigit);
                if (isNumber) {
                    size_t n = std::stoul(sub);
                    if (n > 2) {
                        double res = std::accumulate(polygons.begin(), polygons.end(), 0.0,
                        AreaSummator(HasVertexCount(n)));
                        std::cout << res << std::endl;
                    }
                    else {
                        std::cout << "<INVALID COMMAND>\n";
                    }
                }
                else {
                    std::cout << "<INVALID COMMAND>\n";
                }
            }
        }

        else if (cmd == "MAX") {
            std::string sub;
            std::cin >> sub;

            if (polygons.empty()) {
                std::cout << "<INVALID COMMAND>\n";
                continue;
            }

            if (sub == "AREA") {
                auto it = std::max_element(polygons.begin(), polygons.end(), CompareByArea());
                std::cout << area(*it) << std::endl;
            }

            else if (sub == "VERTEXES") {
                auto it = std::max_element(polygons.begin(), polygons.end(), CompareByVertexes());
                std::cout << it->points.size() << std::endl;
            }

            else {
                std::cout << "<INVALID COMMAND>\n";
            }
        }

        else if (cmd == "MIN") {
            std::string sub;
            std::cin >> sub;

            if (polygons.empty()) {
                std::cout << "<INVALID COMMAND>\n";
                continue;
            }

            if (sub == "AREA") {
                auto it = std::min_element(polygons.begin(), polygons.end(), CompareByArea());
                std::cout << area(*it) << std::endl;
            }

            else if (sub == "VERTEXES") {
                auto it = std::min_element(polygons.begin(), polygons.end(), CompareByVertexes());
                std::cout << it->points.size() << std::endl;
            }

            else {
                std::cout << "<INVALID COMMAND>\n";
            }
        }

        else if (cmd == "COUNT") {
            std::string sub;
            std::cin >> sub;

            if (sub == "ODD") {
                std::cout << std::count_if(polygons.begin(), polygons.end(), IsVertexCountOdd()) << std::endl;
            }

            else if (sub == "EVEN") {
                std::cout << std::count_if(polygons.begin(), polygons.end(), IsVertexCountEven()) << std::endl;
            }

            else {
                bool isNumber = !sub.empty() && std::all_of(sub.begin(), sub.end(), ::isdigit);
                if (isNumber) {
                    size_t n = std::stoul(sub);
                    if (n > 2) {
                        auto pred = std::bind(isVertexCountEqual, _1, n);
                        std::cout << std::count_if(polygons.begin(), polygons.end(), pred) << std::endl;
                    }
                    else {
                        std::cout << "<INVALID COMMAND>\n";
                    }
                }
                else {
                    std::cout << "<INVALID COMMAND>\n";
                }
            }
        }

        else if (cmd == "RMECHO") {
            std::string polyLine;
            std::getline(std::cin >> std::ws, polyLine);

            Polygon target = parsePolygon(polyLine);
            if (target.points.empty()) {
                std::cout << "<INVALID COMMAND>\n";
                continue;
            }

            size_t oldSize = polygons.size();

            auto last = std::unique(polygons.begin(), polygons.end(), BothEqualToTarget(target));
            polygons.erase(last, polygons.end());

            size_t removed = oldSize - polygons.size();
            std::cout << removed << std::endl;
        }

        else if (cmd == "INTERSECTIONS") {
            std::string polyLine;
            std::getline(std::cin >> std::ws, polyLine);

            Polygon target = parsePolygon(polyLine);
            if (target.points.empty()) {
                std::cout << "<INVALID COMMAND>\n";
                continue;
            }

            int count = std::count_if(polygons.begin(), polygons.end(), IntersectWithTarget(target));
            std::cout << count << std::endl;
        }

        else {
            std::cout << "<INVALID COMMAND>\n";
        }
    }
    return 0;
}
