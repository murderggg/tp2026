#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <functional>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <istream>
#include <numeric>
#include <sstream>
#include <iterator>
#include <string>
#include <vector>

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

std::istream& operator>>(std::istream& in, Point& dest) {
    std::istream::sentry sentry(in);
    if (!sentry) return in;
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
    std::istream::sentry sentry(in);
    if (!sentry) return in;

    size_t n;
    if (!(in >> n) || n < 3) {
        in.setstate(std::ios::failbit);
        return in;
    }

    Polygon tmp;
    tmp.points.reserve(n);
    for (size_t i = 0; i < n; ++i) {
        Point p;
        if (!(in >> p)) {
            in.setstate(std::ios::failbit);
            return in;
        }
        tmp.points.push_back(p);
    }

    dest = std::move(tmp);
    return in;
}

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

struct SumArea {
    double operator()(double acc, const Polygon& p) const {
        return acc + AreaCalculator()(p);
    }
};

struct SumAreaIf {
    std::function<bool(const Polygon&)> pred;
    SumAreaIf(std::function<bool(const Polygon&)> p) : pred(p) {}
    double operator()(double acc, const Polygon& p) const {
        return acc + (pred(p) ? AreaCalculator()(p) : 0.0);
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

struct MinX {
    int operator()(int acc, const Point& p) const {
        return std::min(acc, p.x);
    }
};

struct MinY {
    int operator()(int acc, const Point& p) const {
        return std::min(acc, p.y);
    }
};

struct NormalizePointX {
    int min_x;
    NormalizePointX(int mx) : min_x(mx) {}
    Point operator()(const Point& p) const {
        return { p.x - min_x, p.y };
    }
};

struct NormalizePointY {
    int min_y;
    NormalizePointY(int my) : min_y(my) {}
    Point operator()(const Point& p) const {
        return { p.x, p.y - min_y };
    }
};

struct NormalizePolygon {
    Polygon operator()(const Polygon& p) const {
        if (p.points.empty()) return Polygon{};
        int min_x = std::accumulate(p.points.begin(), p.points.end(), p.points[0].x, MinX());
        int min_y = std::accumulate(p.points.begin(), p.points.end(), p.points[0].y, MinY());
        Polygon result;
        result.points.resize(p.points.size());
        std::transform(p.points.begin(), p.points.end(), result.points.begin(), NormalizePointX(min_x));
        std::transform(result.points.begin(), result.points.end(), result.points.begin(), NormalizePointY(min_y));
        return result;
    }
};

struct IsSameWithShift {
    Polygon target_norm;
    size_t target_size;
    IsSameWithShift(const Polygon& t) : target_size(t.points.size()) {
        if (!t.points.empty()) {
            target_norm = NormalizePolygon()(t);
        }
    }
    bool operator()(const Polygon& other) const {
        if (target_size != other.points.size()) return false;
        if (target_size == 0) return true;
        Polygon other_norm = NormalizePolygon()(other);
        std::vector<Point> doubled = other_norm.points;
        doubled.insert(doubled.end(), other_norm.points.begin(), other_norm.points.end() - 1);
        return std::search(doubled.begin(), doubled.end(),
            target_norm.points.begin(), target_norm.points.end()) != doubled.end();
    }
};

struct RemoveConsecutiveTarget {
    Polygon target;
    mutable bool last_was_target_and_kept;
    RemoveConsecutiveTarget(const Polygon& t) : target(t), last_was_target_and_kept(false) {}
    bool operator()(const Polygon& p) const {
        if (p == target) {
            if (last_was_target_and_kept) return false;
            last_was_target_and_kept = true;
            return true;
        }
        last_was_target_and_kept = false;
        return true;
    }
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <filename>\n";
        return 1;
    }
    std::ifstream file(argv[1]);
    if (!file.is_open()) {
        std::cerr << "Error: cannot open file " << argv[1] << "\n";
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
                polygons.push_back(std::move(p));
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
                    std::cout << "<INVALID COMMAND>\n";
                }
                else {
                    double total = std::accumulate(polygons.begin(), polygons.end(), 0.0, SumArea());
                    std::cout << total / polygons.size() << "\n";
                }
            }
            else if (arg == "EVEN") {
                double sum = std::accumulate(polygons.begin(), polygons.end(), 0.0,
                    SumAreaIf(IsEvenVertexCount()));
                std::cout << sum << "\n";
            }
            else if (arg == "ODD") {
                auto odd_pred = std::bind(std::logical_not<bool>(),
                    std::bind(IsEvenVertexCount(), std::placeholders::_1));
                double sum = std::accumulate(polygons.begin(), polygons.end(), 0.0,
                    SumAreaIf(odd_pred));
                std::cout << sum << "\n";
            }
            else {
                std::istringstream num_stream(arg);
                size_t n;
                if (num_stream >> n && num_stream.eof() && n >= 3) {
                    double sum = std::accumulate(polygons.begin(), polygons.end(), 0.0,
                        SumAreaIf(VertexCountEquals(n)));
                    std::cout << sum << "\n";
                }
                else {
                    std::cout << "<INVALID COMMAND>\n";
                }
            }
        }
        else if (cmd == "MAX") {
            std::string arg;
            iss >> arg;
            if (polygons.empty()) {
                std::cout << "<INVALID COMMAND>\n";
            }
            else if (arg == "AREA") {
                auto it = std::max_element(polygons.begin(), polygons.end(), AreaLess());
                std::cout << AreaCalculator()(*it) << "\n";
            }
            else if (arg == "VERTEXES") {
                auto it = std::max_element(polygons.begin(), polygons.end(), VertexCountLess());
                std::cout << it->points.size() << "\n";
            }
            else {
                std::cout << "<INVALID COMMAND>\n";
            }
        }
        else if (cmd == "MIN") {
            std::string arg;
            iss >> arg;
            if (polygons.empty()) {
                std::cout << "<INVALID COMMAND>\n";
            }
            else if (arg == "AREA") {
                auto it = std::min_element(polygons.begin(), polygons.end(), AreaLess());
                std::cout << AreaCalculator()(*it) << "\n";
            }
            else if (arg == "VERTEXES") {
                auto it = std::min_element(polygons.begin(), polygons.end(), VertexCountLess());
                std::cout << it->points.size() << "\n";
            }
            else {
                std::cout << "<INVALID COMMAND>\n";
            }
        }
        else if (cmd == "COUNT") {
            std::string arg;
            iss >> arg;
            if (arg == "EVEN") {
                std::cout << std::count_if(polygons.begin(), polygons.end(), IsEvenVertexCount()) << "\n";
            }
            else if (arg == "ODD") {
                auto odd_pred = std::bind(std::logical_not<bool>(),
                    std::bind(IsEvenVertexCount(), std::placeholders::_1));
                std::cout << std::count_if(polygons.begin(), polygons.end(), odd_pred) << "\n";
            }
            else {
                std::istringstream num_stream(arg);
                size_t n;
                if (num_stream >> n && num_stream.eof() && n >= 3) {
                    std::cout << std::count_if(polygons.begin(), polygons.end(), VertexCountEquals(n)) << "\n";
                }
                else {
                    std::cout << "<INVALID COMMAND>\n";
                }
            }
        }
        else if (cmd == "SAME") {
            std::string rest;
            std::getline(iss, rest);
            std::istringstream rest_iss(rest);
            Polygon target;
            if (!(rest_iss >> target) || !(rest_iss >> std::ws) || !rest_iss.eof()) {
                std::cout << "<INVALID COMMAND>\n";
            }
            else {
                int count = std::count_if(polygons.begin(), polygons.end(), IsSameWithShift(target));
                std::cout << count << "\n";
            }
        }
        else if (cmd == "RMECHO") {
            std::string rest;
            std::getline(iss, rest);
            std::istringstream rest_iss(rest);
            Polygon target;
            if (!(rest_iss >> target) || !(rest_iss >> std::ws) || !rest_iss.eof()) {
                std::cout << "<INVALID COMMAND>\n";
            }
            else {
                std::vector<Polygon> new_polygons;
                RemoveConsecutiveTarget remover(target);
                std::copy_if(polygons.begin(), polygons.end(),
                    std::back_inserter(new_polygons), remover);
                size_t removed = polygons.size() - new_polygons.size();
                polygons.swap(new_polygons);
                std::cout << removed << "\n";
            }
        }
        else {
            std::cout << "<INVALID COMMAND>\n";
        }
    }
    return 0;
}
