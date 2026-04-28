#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <functional>
#include <iomanip>
#include <string>
#include <cmath>
#include <cstdlib>

struct Point {
    int x, y;
    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }
};

struct Polygon {
    std::vector<Point> points;
    bool operator==(const Polygon& other) const {
        return points == other.points;
    }
};

struct AreaCalculator {
    struct GaussSum {
        const std::vector<Point>& p;
        mutable size_t i;
        GaussSum(const std::vector<Point>& points) : p(points), i(0) {}
        long long operator()(long long acc, const Point& curr) const {
            const Point& next = p[(i + 1) % p.size()];
            long long term = static_cast<long long>(curr.x) * next.y
                - static_cast<long long>(next.x) * curr.y;
            ++i;
            return acc + term;
        }
    };

    double operator()(const Polygon& p) const {
        if (p.points.empty()) return 0.0;
        long long sum = std::accumulate(p.points.begin(), p.points.end(), 0LL,
            GaussSum(p.points));
        return std::abs(sum) / 2.0;
    }
};

struct VertexCount {
    size_t operator()(const Polygon& p) const {
        return p.points.size();
    }
};

struct SumAreaIf {
    std::function<bool(const Polygon&)> pred;
    SumAreaIf(std::function<bool(const Polygon&)> p) : pred(p) {}
    double operator()(double acc, const Polygon& p) const {
        return acc + (pred(p) ? AreaCalculator()(p) : 0.0);
    }
};

struct SumArea {
    double operator()(double acc, const Polygon& p) const {
        return acc + AreaCalculator()(p);
    }
};

struct IsEvenVertexCount {
    bool operator()(const Polygon& p) const {
        return p.points.size() % 2 == 0;
    }
};

struct VertexCountEquals {
    size_t n;
    VertexCountEquals(size_t num) : n(num) {}
    bool operator()(const Polygon& p) const {
        return p.points.size() == n;
    }
};

struct AreaLess {
    bool operator()(const Polygon& a, const Polygon& b) const {
        return AreaCalculator()(a) < AreaCalculator()(b);
    }
};

struct VertexCountLess {
    bool operator()(const Polygon& a, const Polygon& b) const {
        return a.points.size() < b.points.size();
    }
};

struct NormalizePolygon {
    Polygon operator()(const Polygon& p) const {
        if (p.points.empty()) return Polygon{};
        int min_x = p.points[0].x;
        int min_y = p.points[0].y;
        for (const auto& pt : p.points) {
            if (pt.x < min_x) min_x = pt.x;
            if (pt.y < min_y) min_y = pt.y;
        }
        Polygon result;
        result.points.reserve(p.points.size());
        for (const auto& pt : p.points) {
            result.points.push_back({ pt.x - min_x, pt.y - min_y });
        }
        return result;
    }
};

struct IsSameWithShift {
    Polygon target;
    Polygon target_norm;

    IsSameWithShift(const Polygon& t) : target(t) {
        target_norm = NormalizePolygon()(target);
    }
    Polygon rotate(const Polygon& p, size_t shift) const {
        Polygon result;
        size_t n = p.points.size();
        result.points.reserve(n);
        for (size_t i = 0; i < n; ++i) {
            result.points.push_back(p.points[(i + shift) % n]);
        }
        return result;
    }

    bool operator()(const Polygon& other) const {
        if (target.points.size() != other.points.size()) return false;
        if (target.points.empty()) return true;

        Polygon other_norm = NormalizePolygon()(other);
        for (size_t shift = 0; shift < other_norm.points.size(); ++shift) {
            Polygon rotated = rotate(other_norm, shift);
            if (std::equal(target_norm.points.begin(), target_norm.points.end(),
                rotated.points.begin())) {
                return true;
            }
        }
        return false;
    }
};

struct RemoveConsecutiveTarget {
    Polygon target;
    mutable bool last_was_target_and_removed;

    RemoveConsecutiveTarget(const Polygon& t) : target(t), last_was_target_and_removed(false) {}

    bool operator()(const Polygon& p) const {
        if (p == target) {
            if (last_was_target_and_removed) {
                return false;
            }
            last_was_target_and_removed = true;
            return true;
        }
        last_was_target_and_removed = false;
        return true;
    }
};

bool parsePolygon(const std::string& str, Polygon& out) {
    std::istringstream iss(str);
    size_t n;
    if (!(iss >> n)) return false;
    if (n < 3) return false;

    std::vector<Point> pts;
    pts.reserve(n);
    for (size_t i = 0; i < n; ++i) {
        char c1, c2, c3;
        int x, y;
        if (!(iss >> c1 >> x >> c2 >> y >> c3)) return false;
        if (c1 != '(' || c2 != ';' || c3 != ')') return false;
        pts.push_back({ x, y });
    }
    std::string extra;
    if (iss >> extra) return false;
    out.points = pts;
    return true;
}

std::istream& operator>>(std::istream& in, Point& dest) {
    char c1, c2, c3;
    int x, y;
    if (in >> c1 >> x >> c2 >> y >> c3 && c1 == '(' && c2 == ';' && c3 == ')') {
        dest = { x, y };
    }
    else {
        in.setstate(std::ios::failbit);
    }
    return in;
}

std::istream& operator>>(std::istream& in, Polygon& dest) {
    size_t n;
    if (!(in >> n) || n < 3) {
        in.setstate(std::ios::failbit);
        return in;
    }
    Polygon tmp;
    tmp.points.resize(n);
    std::copy_n(std::istream_iterator<Point>(in), n, tmp.points.begin());
    if (in) dest = tmp;
    return in;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <filename>" << '\n';
        return 1;
    }

    std::ifstream file(argv[1]);
    if (!file.is_open()) {
        std::cerr << "Error: cannot open file " << argv[1] << '\n';
        return 1;
    }

    std::vector<Polygon> polygons;
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        std::istringstream iss(line);
        Polygon p;
        if (iss >> p) {
            iss >> std::ws;
            if (iss.eof()) {
                polygons.push_back(p);
            }
        }
    }
    file.close();

    std::cout << std::fixed << std::setprecision(1);

    std::string command;
    while (std::getline(std::cin, command)) {
        if (command.empty()) continue;
        std::istringstream iss(command);
        std::string cmd;
        iss >> cmd;

        if (cmd == "AREA") {
            std::string arg;
            iss >> arg;

            if (arg == "MEAN") {
                if (polygons.empty()) {
                    std::cout << "<INVALID COMMAND>" << '\n';
                }
                else {
                    double total = std::accumulate(polygons.begin(), polygons.end(), 0.0, SumArea());
                    std::cout << total / polygons.size() << '\n';
                }
            }
            else if (arg == "EVEN") {
                double sum = std::accumulate(polygons.begin(), polygons.end(), 0.0,
                    SumAreaIf(IsEvenVertexCount()));
                std::cout << sum << '\n';
            }
            else if (arg == "ODD") {
                auto odd_pred = std::bind(std::logical_not<bool>(),
                    std::bind(IsEvenVertexCount(), std::placeholders::_1));
                double sum = std::accumulate(polygons.begin(), polygons.end(), 0.0,
                    SumAreaIf(odd_pred));
                std::cout << sum << '\n';
            }
            else {
                std::istringstream num_stream(arg);
                size_t n;
                if (num_stream >> n && num_stream.eof()) {
                    double sum = std::accumulate(polygons.begin(), polygons.end(), 0.0,
                        SumAreaIf(VertexCountEquals(n)));
                    std::cout << sum << '\n';
                }
                else {
                    std::cout << "<INVALID COMMAND>" << '\n';
                }
            }
        }
        else if (cmd == "MAX") {
            std::string arg;
            iss >> arg;

            if (polygons.empty()) {
                std::cout << "<INVALID COMMAND>" << '\n';
            }
            else if (arg == "AREA") {
                auto it = std::max_element(polygons.begin(), polygons.end(), AreaLess());
                if (it != polygons.end()) {
                    std::cout << AreaCalculator()(*it) << '\n';
                }
                else {
                    std::cout << "<INVALID COMMAND>" << '\n';
                }
            }
            else if (arg == "VERTEXES") {
                auto it = std::max_element(polygons.begin(), polygons.end(), VertexCountLess());
                if (it != polygons.end()) {
                    std::cout << it->points.size() << '\n';
                }
                else {
                    std::cout << "<INVALID COMMAND>" << '\n';
                }
            }
            else {
                std::cout << "<INVALID COMMAND>" << '\n';
            }
        }
        else if (cmd == "MIN") {
            std::string arg;
            iss >> arg;

            if (polygons.empty()) {
                std::cout << "<INVALID COMMAND>" << '\n';
            }
            else if (arg == "AREA") {
                auto it = std::min_element(polygons.begin(), polygons.end(), AreaLess());
                if (it != polygons.end()) {
                    std::cout << AreaCalculator()(*it) << '\n';
                }
                else {
                    std::cout << "<INVALID COMMAND>" << '\n';
                }
            }
            else if (arg == "VERTEXES") {
                auto it = std::min_element(polygons.begin(), polygons.end(), VertexCountLess());
                if (it != polygons.end()) {
                    std::cout << it->points.size() << '\n';
                }
                else {
                    std::cout << "<INVALID COMMAND>" << '\n';
                }
            }
            else {
                std::cout << "<INVALID COMMAND>" << '\n';
            }
        }
        else if (cmd == "COUNT") {
            std::string arg;
            iss >> arg;

            if (arg == "EVEN") {
                std::cout << std::count_if(polygons.begin(), polygons.end(), IsEvenVertexCount()) << '\n';
            }
            else if (arg == "ODD") {
                auto odd_pred = std::bind(std::logical_not<bool>(),
                    std::bind(IsEvenVertexCount(), std::placeholders::_1));
                std::cout << std::count_if(polygons.begin(), polygons.end(), odd_pred) << '\n';
            }
            else {
                std::istringstream num_stream(arg);
                size_t n;
                if (num_stream >> n && num_stream.eof()) {
                    std::cout << std::count_if(polygons.begin(), polygons.end(), VertexCountEquals(n)) << '\n';
                }
                else {
                    std::cout << "<INVALID COMMAND>" << '\n';
                }
            }
        }
        else if (cmd == "SAME") {
            Polygon target;
            std::string rest;
            std::getline(iss, rest);
            std::istringstream rest_iss(rest);
            if (!(rest_iss >> target)) {
                std::cout << "<INVALID COMMAND>" << '\n';
            }
            else {
                int count = std::count_if(polygons.begin(), polygons.end(), IsSameWithShift(target));
                std::cout << count << '\n';
            }
        }
        else if (cmd == "RMECHO") {
            Polygon target;
            std::string rest;
            std::getline(iss, rest);
            std::istringstream rest_iss(rest);
            if (!(rest_iss >> target)) {
                std::cout << "<INVALID COMMAND>" << '\n';
            }
            else {
                std::vector<Polygon> new_polygons;
                RemoveConsecutiveTarget remover(target);
                std::copy_if(polygons.begin(), polygons.end(),
                    std::back_inserter(new_polygons),
                    remover);
                size_t removed = polygons.size() - new_polygons.size();
                polygons.swap(new_polygons);
                std::cout << removed << '\n';
            }
        }
        else {
            std::cout << "<INVALID COMMAND>" << '\n';
        }
    }

    return 0;
}
