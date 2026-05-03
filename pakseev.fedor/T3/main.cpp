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

    bool operator==(const Point& other) const  {
        return x == other.x  && y == other.y;
    }

    bool operator!=(const Point& other) const {
        return !(*this == other);
    }

    bool operator<(const Point& other) const {
        if (x != other.x) {
            return x < other.x;
        }
        return y < other.y;
    }
};

struct Polygon {
    std::vector<Point> points;

    bool operator<(const Polygon& other) const {
        if (points.size() != other.points.size())
            return points.size() < other.points.size();
        return points < other.points;
    }

    bool operator==(const Polygon& other) const {
        if (points.size() != other.points.size()) {
            return false;
        }
        return std::equal(points.begin(), points.end(), other.points.begin());
    }
};

struct MaxSeqState {
    int currentSeq = 0;
    int maxSeq = 0;
};

double computeArea(const Polygon& p) {
    if (p.points.size() < 3) return 0.0;

    double area = 0.0;
    size_t n = p.points.size();

    for (int i = 0; i < n; ++i) {
        area += p.points[i].x * p.points[(i + 1) % n].y - p.points[(i + 1) % n].x * p.points[i].y;
    }

    return std::abs(area) / 2.0;
}


struct isVertexCountOdd {
    bool operator()(const Polygon& p) const {
        return p.points.size() % 2 != 0;
    }
};

struct isVertexCountEven {
    bool operator()(const Polygon& p) const {
        return p.points.size() % 2 == 0;
    }
};

struct meanCount {
    bool operator()(const Polygon& p) {
        return true;
    }
};

struct hasVertexCount {
    size_t n_;

    hasVertexCount(size_t size) : n_(size) {}

    bool operator()(const Polygon& p) {
        return n_ == p.points.size();
    }
};

bool isVertexCountEqual(const Polygon& p, size_t n) {
    return p.points.size() == n;
}
struct AreaSummator {
    std::function<bool(const Polygon&)> filter;

    AreaSummator(std::function<bool(const Polygon&)> f) : filter(f) {}

    double operator()(double acc, const Polygon& p) const {
        return filter(p) ? acc + computeArea(p) : acc;
    }
};

struct CompareByArea {
    bool operator()(const Polygon& a, const Polygon& b) const {
        return computeArea(a) < computeArea(b);
    }
};

struct CompareByVertexes {
    bool operator()(const Polygon& a, const Polygon& b) const {
        return a.points.size() < b.points.size();
    }
};


Polygon parsePolygon(const std::string& line) {
    std::istringstream iss(line);
    int n;
    if (!(iss >> n) || n < 3) {
        return {};
    }

    Polygon p;

    for (int i = 0; i < n; ++i) {
        char open, semicolon, close;
        int x, y;
        if (!(iss >> open >> x >> semicolon >> y >> close)) {
            return {};
        }
        if (open != '(' || semicolon != ';' || close != ')') {
            return {};
        }
        p.points.push_back({x, y});
    }

    std::string extra;
    if (iss >> extra) {
        return {};
    }

    return p;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "ERROR: No filename provided\n";
        return 1;
    }

    std::ifstream file(argv[1]);
    if (!file) {
        std::cerr << "ERROR: Cannot open file\n";
        return 1;
    }

    std::vector<Polygon> polygons;
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) {
            continue;
        }
        Polygon p = parsePolygon(line);
        if (!p.points.empty()) {
            polygons.push_back(p);
        }
    }

    std::cout << std::fixed << std::setprecision(1);

    std::string cmd;
    while (std::cin >> cmd) {
        try {
            if (cmd == "AREA") {
                std::string sub;
                std::cin >> sub;

                if (sub == "ODD") {
                    double res = std::accumulate(polygons.begin(), polygons.end(), 0.0,
                    AreaSummator(isVertexCountOdd()));
                    std::cout << res << std::endl;
                }

                else if (sub == "EVEN") {
                    double res = std::accumulate(polygons.begin(), polygons.end(), 0.0,
                    AreaSummator(isVertexCountEven()));
                    std::cout << res << std::endl;
                }

                else if (sub == "MEAN") {
                    if (polygons.empty()) {
                        std::cout << "<INVALID COMMAND>\n";
                        continue;
                    }
                    double res = std::accumulate(polygons.begin(), polygons.end(), 0.0,
                    AreaSummator(meanCount()));
                    std::cout << res / polygons.size() << std::endl;
                }

                else {
                    bool isNumber = !sub.empty() && std::all_of(sub.begin(), sub.end(), ::isdigit);
                    if (isNumber) {
                        size_t n = std::stoul(sub);
                        if (n > 2) {
                            double res = std::accumulate(polygons.begin(), polygons.end(), 0.0,
                            AreaSummator(hasVertexCount(n)));
                            std::cout << res << std::endl;
                        } else {
                            std::cout << "<INVALID COMMAND>\n";
                        }
                    } else {
                        std::cout << "<INVALID COMMAND>\n";
                    }
                }

            } else if (cmd == "MAX") {
                std::string sub;
                std::cin >> sub;

                if (polygons.empty()) {
                    std::cout << "<INVALID COMMAND>\n";
                    continue;
                }

                if (sub == "AREA") {
                    auto it = std::max_element(polygons.begin(), polygons.end(), CompareByArea());
                    std::cout << computeArea(*it) << std::endl;

                } else if (sub == "VERTEXES") {
                    auto it = std::max_element(polygons.begin(), polygons.end(), CompareByVertexes());
                    std::cout << it->points.size() << std::endl;

                } else {
                    std::cout << "<INVALID COMMAND>\n";
                }

            } else if (cmd == "MIN") {
                std::string sub;
                std::cin >> sub;

                if (polygons.empty()) {
                    std::cout << "<INVALID COMMAND>\n";
                    continue;
                }

                if (sub == "AREA") {
                    auto it = std::min_element(polygons.begin(), polygons.end(), CompareByArea());
                    std::cout << computeArea(*it) << std::endl;

                } else if (sub == "VERTEXES") {
                    auto it = std::min_element(polygons.begin(), polygons.end(), CompareByVertexes());
                    std::cout << it->points.size() << std::endl;
                } else {
                    std::cout << "<INVALID COMMAND>\n";
                }

            } else if (cmd == "COUNT") {
                std::string sub;
                std::cin >> sub;

                if (sub == "ODD") {
                    std::cout << std::count_if(polygons.begin(), polygons.end(), isVertexCountOdd()) << std::endl;

                } else if (sub == "EVEN") {
                    std::cout << std::count_if(polygons.begin(), polygons.end(), isVertexCountEven()) << std::endl;

                } else {
                    bool isNumber = !sub.empty() && std::all_of(sub.begin(), sub.end(), ::isdigit);
                    if (isNumber) {
                        size_t n = std::stoul(sub);
                        if (n > 2) {
                            std::cout << std::count_if(polygons.begin(), polygons.end(),
                            std::bind(isVertexCountEqual, _1, n)) << std::endl;
                        }
                        else {
                            std::cout << "<INVALID COMMAND>\n";
                        }
                    }
                    else {
                        std::cout << "<INVALID COMMAND>\n";
                    }
                }

            } else if (cmd == "ECHO") {
                std::string pLine;
                std::getline(std::cin >> std::ws, pLine);

                Polygon target = parsePolygon(pLine);
                if (target.points.empty()) {
                    std::cout << "<INVALID COMMAND>\n";
                    continue;
                }

                int targetCount = std::count_if(polygons.begin(), polygons.end(),
                std::bind(std::equal_to<Polygon>(), _1, target));

                std::vector<Polygon> newPolygons = std::accumulate(
                    polygons.begin(), polygons.end(),
                    std::vector<Polygon>(),
                    [&target](std::vector<Polygon> vec, const Polygon& p) {
                        vec.push_back(p);
                        if (p == target) {
                            vec.push_back(p);
                        }
                        return vec;
                    }
                );

                polygons = std::move(newPolygons);
                std::cout << targetCount << std::endl;


            } else if (cmd == "MAXSEQ") {
                std::string pLine;
                std::getline(std::cin >> std::ws, pLine);

                Polygon target = parsePolygon(pLine);
                if (target.points.empty()) {
                    std::cout << "<INVALID COMMAND>\n";
                    continue;
                }

                MaxSeqState res = std::accumulate(
                    polygons.begin(), polygons.end(),
                    MaxSeqState{},
                    [&target](MaxSeqState state, const Polygon& poly) {
                        if (poly == target) {
                            state.currentSeq++;
                            if (state.currentSeq > state.maxSeq) {
                                state.maxSeq = state.currentSeq;
                            }
                        } else {
                            state.currentSeq = 0;
                        }
                        return state;
                    }
                );

                std::cout << res.maxSeq << std::endl;

            } else {
                std::cout << "<INVALID COMMAND>\n";
            }

        } catch (std::exception& e) {
            std::cout << "<INVALID COMMAND>\n";
        }
    }

    return 0;
}
