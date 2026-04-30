#include "Commands.h"
#include <algorithm>
#include <functional>
#include <numeric>
#include <iomanip>
#include <cmath>

using namespace std::placeholders;

//----------------------------Area------------------------------------------

struct AreaState {
    double area;
    Point prev;
};

struct AccumulateArea {
    AreaState operator()(const AreaState& state, const Point& current) const {
        double cross = static_cast<double>(state.prev.x) * current.y -
                       static_cast<double>(state.prev.y) * current.x;
        return {state.area + cross, current};
    }
};

double getPolygonArea(const Polygon& pol) {
    if (pol.points.size() < 3) return 0.0;

    AreaState result = std::accumulate(
        pol.points.begin() + 1, pol.points.end(),
        AreaState{0.0, pol.points.front()},
        AccumulateArea()
    );

    double final_cross = static_cast<double>(result.prev.x) * pol.points.front().y -
                         static_cast<double>(result.prev.y) * pol.points.front().x;

    return std::abs(result.area + final_cross) / 2.0;
}

struct AreaIfEven {
    double operator()(double sum, const Polygon& p) const {
        return (p.points.size() % 2 == 0) ? sum + getPolygonArea(p) : sum;
    }
};

struct AreaIfOdd {
    double operator()(double sum, const Polygon& p) const {
        return (p.points.size() % 2 != 0) ? sum + getPolygonArea(p) : sum;
    }
};

struct AreaIfNum {
    size_t n;
    explicit AreaIfNum(size_t n) : n(n) {}
    double operator()(double sum, const Polygon& p) const {
        return (p.points.size() == n) ? sum + getPolygonArea(p) : sum;
    }
};

struct AreaAll {
    double operator()(double sum, const Polygon& p) const {
        return sum + getPolygonArea(p);
    }
};

void cmdArea(const std::vector<Polygon>& polygons, const std::string& arg) {
    if (arg == "EVEN") {
        std::cout << std::fixed << std::setprecision(1)
                  << std::accumulate(polygons.begin(), polygons.end(), 0.0, AreaIfEven()) << "\n";
    } else if (arg == "ODD") {
        std::cout << std::fixed << std::setprecision(1)
                  << std::accumulate(polygons.begin(), polygons.end(), 0.0, AreaIfOdd()) << "\n";
    } else if (arg == "MEAN") {
        if (polygons.empty()) throw std::invalid_argument("");
        double total = std::accumulate(polygons.begin(), polygons.end(), 0.0, AreaAll());
        std::cout << std::fixed << std::setprecision(1) << (total / polygons.size()) << "\n";
    } else {
        size_t n = std::stoull(arg);
        if (n < 3) throw std::invalid_argument("");
        std::cout << std::fixed << std::setprecision(1)
                  << std::accumulate(polygons.begin(), polygons.end(), 0.0, AreaIfNum(n)) << "\n";
    }
}

//---------------------------------Max/Min----------------------------------------

struct CompareArea {
    bool operator()(const Polygon& a, const Polygon& b) const {
        return getPolygonArea(a) < getPolygonArea(b);
    }
};

struct CompareVertices {
    bool operator()(const Polygon& a, const Polygon& b) const {
        return a.points.size() < b.points.size();
    }
};

void cmdMinMax(const std::vector<Polygon>& polygons, const std::string& type, const std::string& arg) {
    if (polygons.empty()) throw std::invalid_argument("");

    if (arg == "AREA") {
        auto it = (type == "MAX") ? std::max_element(polygons.begin(), polygons.end(), CompareArea())
                                  : std::min_element(polygons.begin(), polygons.end(), CompareArea());
        std::cout << std::fixed << std::setprecision(1) << getPolygonArea(*it) << "\n";
    } else if (arg == "VERTEXES") {
        auto it = (type == "MAX") ? std::max_element(polygons.begin(), polygons.end(), CompareVertices())
                                  : std::min_element(polygons.begin(), polygons.end(), CompareVertices());
        std::cout << it->points.size() << "\n";
    } else {
        throw std::invalid_argument("");
    }
}

//--------------------------------------Count-------------------------------------------

struct IsEvenVertices {
    bool operator()(const Polygon& p) const { return p.points.size() % 2 == 0; }
};

struct IsOddVertices {
    bool operator()(const Polygon& p) const { return p.points.size() % 2 != 0; }
};

void cmdCount(const std::vector<Polygon>& polygons, const std::string& arg) {
    if (arg == "EVEN") {
        std::cout << std::count_if(polygons.begin(), polygons.end(), IsEvenVertices()) << "\n";
    } else if (arg == "ODD") {
        std::cout << std::count_if(polygons.begin(), polygons.end(), IsOddVertices()) << "\n";
    } else {
        try {
            size_t n = std::stoull(arg);
            if (n < 3) throw std::invalid_argument("");

            std::cout << std::count_if(polygons.begin(), polygons.end(),
                std::bind(std::equal_to<size_t>(),
                    std::bind(&std::vector<Point>::size,
                        std::bind(&Polygon::points, _1)), n)) << "\n";
        } catch (...) {
            throw std::invalid_argument("");
        }
    }
}

//--------------------------------PERMS----------------------------------------

struct ComparePoints {
    bool operator()(const Point& a, const Point& b) const {
        if (a.x == b.x) return a.y < b.y;
        return a.x < b.x;
    }
};

struct IsPermutation {
    Polygon target;

    explicit IsPermutation(Polygon t) : target(std::move(t)) {
        std::sort(target.points.begin(), target.points.end(), ComparePoints());
    }

    bool operator()(const Polygon& p) const {
        if (p.points.size() != target.points.size()) return false;
        Polygon copy = p;
        std::sort(copy.points.begin(), copy.points.end(), ComparePoints());
        return copy.points == target.points;
    }
};

void cmdPerms(const std::vector<Polygon>& polygons, std::istream& in) {
    Polygon target;
    if (!(in >> target)) throw std::invalid_argument("");

    in >> std::ws;
    if (!in.eof()) throw std::invalid_argument("");

    size_t result = std::count_if(polygons.begin(), polygons.end(), IsPermutation(std::move(target)));
    std::cout << result << "\n";
}

//-----------------------------------MAXSEQ-----------------------------------

struct MaxSeqFold {
    Polygon target;
    explicit MaxSeqFold(const Polygon& t) : target(std::move(t)) {}

    std::pair<int, int> operator()(const std::pair<int, int>& state, const Polygon& p) const {
        int current = (p == target) ? state.first + 1 : 0;
        return std::make_pair(current, std::max(state.second, current));
    }
};

void cmdMaxSeq(const std::vector<Polygon>& polygons, std::istream& in) {
    Polygon target;
    if (!(in >> target)) throw std::invalid_argument("");

    in >> std::ws;
    if (!in.eof()) {
        throw std::invalid_argument("");
    }

    std::pair<int, int> result = std::accumulate(polygons.begin(), polygons.end(),
                                 std::make_pair(0, 0), MaxSeqFold(std::move(target)));

    std::cout << result.second << "\n";
}

//-----------------------------------all commands---------------------------------------

void processCommands(const std::vector<Polygon>& polygons, std::istream& in, std::ostream& out) {
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        std::istringstream iss(line);
        std::string command;
        if (!(iss >> command)) continue;

        try {
            if (command == "AREA") {
                std::string arg;
                if (!(iss >> arg)) throw std::invalid_argument("");
                cmdArea(polygons, arg);
            } else if (command == "MAX" || command == "MIN") {
                std::string arg;
                if (!(iss >> arg)) throw std::invalid_argument("");
                cmdMinMax(polygons, command, arg);
            } else if (command == "COUNT") {
                std::string arg;
                if (!(iss >> arg)) throw std::invalid_argument("");
                cmdCount(polygons, arg);
            } else if (command == "PERMS") {
                cmdPerms(polygons, iss);
                continue;
            } else if (command == "MAXSEQ") {
                cmdMaxSeq(polygons, iss);
                continue;
            } else {
                throw std::invalid_argument("");
            }

            iss >> std::ws;
            if (!iss.eof()) {
                throw std::invalid_argument("");
            }
        } catch (...) {
            out << "<INVALID COMMAND>\n";
        }
    }
}
