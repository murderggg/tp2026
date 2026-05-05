#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <functional>
#include <cmath>
#include <limits>
#include <iterator>
#include <string>
#include <sstream>

using namespace std::placeholders;

struct Point {
    int x, y;
};

struct Polygon {
    std::vector<Point> points;
};

std::istream& operator>>(std::istream& is, Point& p) {
    std::istream::sentry sentry(is);
    if (!is) return is;
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
    if (p1.points.size() != p2.points.size()) return false;
    return std::equal(p1.points.begin(), p1.points.end(), p2.points.begin(), std::equal_to<Point>());
}

std::istream& operator>>(std::istream& is, Polygon& pg) {
    std::istream::sentry sentry(is);
    if (!is) return is;
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

double calcAreaTerm(const Point& a, const Point& b) {
    return static_cast<double>(a.x * b.y - a.y * b.x);
}

double area(const Polygon& pg) {
    if (pg.points.size() < 3) return 0.0;
    std::vector<double> terms;
    std::transform(pg.points.begin(), pg.points.end() - 1,
                   pg.points.begin() + 1,
                   std::back_inserter(terms), calcAreaTerm);
    terms.push_back(calcAreaTerm(pg.points.back(), pg.points.front()));
    double sum = std::accumulate(terms.begin(), terms.end(), 0.0);
    return std::abs(sum) / 2.0;
}

bool isEvenFunc(const Polygon& pg) { return pg.points.size() % 2 == 0; }
bool isOddFunc(const Polygon& pg)  { return pg.points.size() % 2 != 0; }
bool hasNumVertices(const Polygon& pg, size_t n) { return pg.points.size() == n; }

double addArea(double sum, const Polygon& p) {
    return sum + area(p);
}

double addAreaIfEven(double sum, const Polygon& p) {
    return isEvenFunc(p) ? sum + area(p) : sum;
}

double addAreaIfOdd(double sum, const Polygon& p) {
    return isOddFunc(p) ? sum + area(p) : sum;
}

double addAreaIfNum(double sum, const Polygon& p, size_t n) {
    return hasNumVertices(p, n) ? sum + area(p) : sum;
}

bool compareArea(const Polygon& p1, const Polygon& p2) {
    return area(p1) < area(p2);
}

bool compareVertexes(const Polygon& p1, const Polygon& p2) {
    return p1.points.size() < p2.points.size();
}

bool areaLessThanTarget(const Polygon& p, double targetArea) {
    return area(p) < targetArea;
}

bool isTargetPolygon(const Polygon& p, const Polygon& target) {
    return p == target;
}

std::vector<Polygon> readPolygons(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) return {};
    std::vector<Polygon> polys;
    while (file) {
        Polygon p;
        if (file >> p) {
            polys.push_back(p);
        } else if (!file.eof()) {
            file.clear();
            file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    }
    return polys;
}

bool parsePolygon(const std::string& line, Polygon& poly) {
    std::istringstream iss(line);
    size_t count;
    if (!(iss >> count) || count < 3) return false;
    std::vector<Point> pts;
    std::copy_n(std::istream_iterator<Point>(iss), count, std::back_inserter(pts));
    if (iss.fail()) return false;
    std::string leftover;
    if (iss >> leftover) return false;
    poly.points = pts;
    return true;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <filename>\n";
        return 1;
    }

    std::vector<Polygon> polys = readPolygons(argv[1]);

    std::cout << std::fixed << std::setprecision(1);

    std::string line;
    while (std::getline(std::cin, line)) {
        if (line.empty()) continue;

        std::istringstream iss(line);
        std::string cmd;
        iss >> cmd;

        if (cmd == "AREA") {
            std::string subcmd;
            if (!(iss >> subcmd)) {
                std::cout << "<INVALID COMMAND>\n";
                continue;
            }
            std::string leftover;
            if (iss >> leftover) {
                std::cout << "<INVALID COMMAND>\n";
                continue;
            }
            double res = 0.0;
            if (subcmd == "EVEN") {
                res = std::accumulate(polys.begin(), polys.end(), 0.0, addAreaIfEven);
            } else if (subcmd == "ODD") {
                res = std::accumulate(polys.begin(), polys.end(), 0.0, addAreaIfOdd);
            } else if (subcmd == "MEAN") {
                if (polys.empty()) { std::cout << "<INVALID COMMAND>\n"; continue; }
                res = std::accumulate(polys.begin(), polys.end(), 0.0, addArea) / polys.size();
            } else {
                try {
                    size_t n = std::stoull(subcmd);
                    if (n < 3) throw std::invalid_argument("n < 3");
                    res = std::accumulate(polys.begin(), polys.end(), 0.0,
                                          std::bind(addAreaIfNum, _1, _2, n));
                } catch (...) {
                    std::cout << "<INVALID COMMAND>\n";
                    continue;
                }
            }
            std::cout << res << "\n";
        }

        else if (cmd == "MIN") {
            std::string subcmd;
            if (!(iss >> subcmd) || polys.empty()) {
                std::cout << "<INVALID COMMAND>\n";
                continue;
            }
            std::string leftover;
            if (iss >> leftover) {
                std::cout << "<INVALID COMMAND>\n";
                continue;
            }
            if (subcmd == "AREA") {
                auto it = std::min_element(polys.begin(), polys.end(), compareArea);
                std::cout << area(*it) << "\n";
            } else if (subcmd == "VERTEXES") {
                auto it = std::min_element(polys.begin(), polys.end(), compareVertexes);
                std::cout << it->points.size() << "\n";
            } else {
                std::cout << "<INVALID COMMAND>\n";
            }
        }

        else if (cmd == "MAX") {
            std::string subcmd;
            if (!(iss >> subcmd) || polys.empty()) {
                std::cout << "<INVALID COMMAND>\n";
                continue;
            }
            std::string leftover;
            if (iss >> leftover) {
                std::cout << "<INVALID COMMAND>\n";
                continue;
            }
            if (subcmd == "AREA") {
                auto it = std::max_element(polys.begin(), polys.end(), compareArea);
                std::cout << area(*it) << "\n";
            } else if (subcmd == "VERTEXES") {
                auto it = std::max_element(polys.begin(), polys.end(), compareVertexes);
                std::cout << it->points.size() << "\n";
            } else {
                std::cout << "<INVALID COMMAND>\n";
            }
        }

        else if (cmd == "COUNT") {
            std::string subcmd;
            if (!(iss >> subcmd)) {
                std::cout << "<INVALID COMMAND>\n";
                continue;
            }
            std::string leftover;
            if (iss >> leftover) {
                std::cout << "<INVALID COMMAND>\n";
                continue;
            }
            if (subcmd == "EVEN") {
                std::cout << std::count_if(polys.begin(), polys.end(), isEvenFunc) << "\n";
            } else if (subcmd == "ODD") {
                std::cout << std::count_if(polys.begin(), polys.end(), isOddFunc) << "\n";
            } else {
                try {
                    size_t n = std::stoull(subcmd);
                    if (n < 3) throw std::invalid_argument("n < 3");
                    std::cout << std::count_if(polys.begin(), polys.end(),
                                               std::bind(hasNumVertices, _1, n)) << "\n";
                } catch (...) {
                    std::cout << "<INVALID COMMAND>\n";
                    continue;
                }
            }
        }

        else if (cmd == "LESSAREA") {
            std::string rest;
            std::getline(iss, rest);
            if (rest.empty()) {
                std::cout << "<INVALID COMMAND>\n";
                continue;
            }
            Polygon target;
            if (!parsePolygon(rest, target)) {
                std::cout << "<INVALID COMMAND>\n";
                continue;
            }
            double targetArea = area(target);
            int cnt = std::count_if(polys.begin(), polys.end(),
                std::bind(areaLessThanTarget, _1, targetArea));
            std::cout << cnt << "\n";
        }

        else if (cmd == "MAXSEQ") {
            std::string rest;
            std::getline(iss, rest);
            if (rest.empty()) {
                std::cout << "<INVALID COMMAND>\n";
                continue;
            }
            Polygon target;
            if (!parsePolygon(rest, target)) {
                std::cout << "<INVALID COMMAND>\n";
                continue;
            }
            size_t maxSeq = 0;
            auto it = polys.begin();
            while (it != polys.end()) {
                auto start = std::find_if(it, polys.end(),
                    std::bind(isTargetPolygon, _1, target));
                if (start == polys.end()) break;
                auto end = std::find_if_not(start, polys.end(),
                    std::bind(isTargetPolygon, _1, target));
                size_t len = std::distance(start, end);
                if (len > maxSeq) maxSeq = len;
                it = end;
            }
            std::cout << maxSeq << "\n";
        }

        else {
            std::cout << "<INVALID COMMAND>\n";
        }
    }

    return 0;
}
