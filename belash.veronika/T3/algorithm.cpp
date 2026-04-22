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
    bool operator<(const Point& other) const {
        return x < other.x || (x == other.x && y < other.y);
    }
};

struct Polygon {
    std::vector<Point> points;
    bool operator==(const Polygon& other) const {
        return points == other.points;
    }
};

struct AreaCalculator {
    double operator()(const Polygon& p) const {
        double area = 0.0;
        int n = p.points.size();
        for (int i = 0; i < n; ++i) {
            const Point& a = p.points[i];
            const Point& b = p.points[(i + 1) % n];
            area += static_cast<double>(a.x) * b.y - static_cast<double>(b.x) * a.y;
        }
        return std::abs(area) / 2.0;
    }
};

struct VertexCount {
    int operator()(const Polygon& p) const {
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

struct IsCompatibleWith {
    Polygon target;
    std::vector<Point> target_norm;
    IsCompatibleWith(const Polygon& t) : target(t) {
        target_norm = normalize(target);
    }
    std::vector<Point> normalize(const Polygon& p) const {
        if (p.points.empty()) return {};
        int min_x = p.points[0].x, min_y = p.points[0].y;
        for (const auto& pt : p.points) {
            if (pt.x < min_x) min_x = pt.x;
            if (pt.y < min_y) min_y = pt.y;
        }
        std::vector<Point> norm;
        norm.reserve(p.points.size());
        for (const auto& pt : p.points) {
            norm.push_back({ pt.x - min_x, pt.y - min_y });
        }
        std::sort(norm.begin(), norm.end());
        return norm;
    }
    bool operator()(const Polygon& other) const {
        auto other_norm = normalize(other);
        return target_norm == other_norm;
    }
};

struct BothEqualTarget {
    Polygon target;
    BothEqualTarget(const Polygon& t) : target(t) {}
    bool operator()(const Polygon& a, const Polygon& b) const {
        return a == target && b == target;
    }
};

bool parsePolygon(const std::string& str, Polygon& out) {
    std::istringstream iss(str);
    int n;
    if (!(iss >> n)) return false;
    if (n <= 0) return false;
    std::vector<Point> pts;
    pts.reserve(n);
    for (int i = 0; i < n; ++i) {
        std::string token;
        if (!(iss >> token)) return false;
        if (token.size() < 5) return false;
        if (token[0] != '(') return false;
        size_t semicolon = token.find(';');
        if (semicolon == std::string::npos) return false;
        if (token.back() != ')') return false;
        std::string xs = token.substr(1, semicolon - 1);
        std::string ys = token.substr(semicolon + 1, token.size() - semicolon - 2);
        int x, y;
        try {
            x = std::stoi(xs);
            y = std::stoi(ys);
        }
        catch (...) {
            return false;
        }
        pts.push_back({ x, y });
    }
    std::string extra;
    if (iss >> extra) return false;
    out.points = pts;
    return true;
}

void processCommand(const std::string& line, std::vector<Polygon>& polygons) {
    if (line.empty()) return;
    std::istringstream iss(line);
    std::string cmd;
    iss >> cmd;
    if (cmd.empty()) return;
    if (cmd == "AREA") {
        std::string arg;
        iss >> arg;
        if (arg.empty()) {
            std::cout << "<INVALID COMMAND>" << '\n';
            return;
        }

        if (arg == "MEAN") {
            if (polygons.empty()) {
                std::cout << "<INVALID COMMAND>" << '\n';
                return;
            }
            double total = std::accumulate(polygons.begin(), polygons.end(), 0.0, SumArea());
            double mean = total / polygons.size();
            std::cout << std::fixed << std::setprecision(1) << mean << '\n';
            return;
        }

        if (arg == "EVEN") {
            auto pred = std::bind(std::equal_to<int>(),
                std::bind(std::modulus<int>(),
                    std::bind(VertexCount(), std::placeholders::_1),
                    2),
                0);
            double sum = std::accumulate(polygons.begin(), polygons.end(), 0.0, SumAreaIf(pred));
            std::cout << std::fixed << std::setprecision(1) << sum << '\n';
            return;
        }
        if (arg == "ODD") {
            auto pred = std::bind(std::equal_to<int>(),
                std::bind(std::modulus<int>(),
                    std::bind(VertexCount(), std::placeholders::_1),
                    2),
                1);
            double sum = std::accumulate(polygons.begin(), polygons.end(), 0.0, SumAreaIf(pred));
            std::cout << std::fixed << std::setprecision(1) << sum << '\n';
            return;
        }

        try {
            int num = std::stoi(arg);
            if (num <= 0) throw std::invalid_argument("");
            auto pred = std::bind(std::equal_to<int>(),
                std::bind(VertexCount(), std::placeholders::_1),
                num);
            double sum = std::accumulate(polygons.begin(), polygons.end(), 0.0, SumAreaIf(pred));
            std::cout << std::fixed << std::setprecision(1) << sum << '\n';
            return;
        }
        catch (...) {
            std::cout << "<INVALID COMMAND>" << '\n';
            return;
        }
    }

    if (cmd == "MAX") {
        std::string arg;
        iss >> arg;
        if (arg.empty() || polygons.empty()) {
            std::cout << "<INVALID COMMAND>" << '\n';
            return;
        }
        if (arg == "AREA") {
            auto it = std::max_element(polygons.begin(), polygons.end(),
                std::bind(std::less<double>(),
                    std::bind(AreaCalculator(), std::placeholders::_1),
                    std::bind(AreaCalculator(), std::placeholders::_2)));
            if (it == polygons.end()) {
                std::cout << "<INVALID COMMAND>" << '\n';
                return;
            }
            double area = AreaCalculator()(*it);
            std::cout << std::fixed << std::setprecision(1) << area << '\n';
            return;
        }
        if (arg == "VERTEXES") {
            auto it = std::max_element(polygons.begin(), polygons.end(),
                std::bind(std::less<int>(),
                    std::bind(VertexCount(), std::placeholders::_1),
                    std::bind(VertexCount(), std::placeholders::_2)));
            if (it == polygons.end()) {
                std::cout << "<INVALID COMMAND>" << '\n';
                return;
            }
            int v = VertexCount()(*it);
            std::cout << v << '\n';
            return;
        }
        std::cout << "<INVALID COMMAND>" << '\n';
        return;
    }

    if (cmd == "MIN") {
        std::string arg;
        iss >> arg;
        if (arg.empty() || polygons.empty()) {
            std::cout << "<INVALID COMMAND>" << '\n';
            return;
        }
        if (arg == "AREA") {
            auto it = std::min_element(polygons.begin(), polygons.end(),
                std::bind(std::less<double>(),
                    std::bind(AreaCalculator(), std::placeholders::_1),
                    std::bind(AreaCalculator(), std::placeholders::_2)));
            if (it == polygons.end()) {
                std::cout << "<INVALID COMMAND>" << '\n';
                return;
            }
            double area = AreaCalculator()(*it);
            std::cout << std::fixed << std::setprecision(1) << area << '\n';
            return;
        }
        if (arg == "VERTEXES") {
            auto it = std::min_element(polygons.begin(), polygons.end(),
                std::bind(std::less<int>(),
                    std::bind(VertexCount(), std::placeholders::_1),
                    std::bind(VertexCount(), std::placeholders::_2)));
            if (it == polygons.end()) {
                std::cout << "<INVALID COMMAND>" << '\n';
                return;
            }
            int v = VertexCount()(*it);
            std::cout << v << '\n';
            return;
        }
        std::cout << "<INVALID COMMAND>" << '\n';
        return;
    }

    if (cmd == "COUNT") {
        std::string arg;
        iss >> arg;
        if (arg.empty()) {
            std::cout << "<INVALID COMMAND>" << '\n';
            return;
        }
        if (arg == "EVEN") {
            auto pred = std::bind(std::equal_to<int>(),
                std::bind(std::modulus<int>(),
                    std::bind(VertexCount(), std::placeholders::_1),
                    2),
                0);
            int cnt = std::count_if(polygons.begin(), polygons.end(), pred);
            std::cout << cnt << '\n';
            return;
        }
        if (arg == "ODD") {
            auto pred = std::bind(std::equal_to<int>(),
                std::bind(std::modulus<int>(),
                    std::bind(VertexCount(), std::placeholders::_1),
                    2),
                1);
            int cnt = std::count_if(polygons.begin(), polygons.end(), pred);
            std::cout << cnt << '\n';
            return;
        }
        try {
            int num = std::stoi(arg);
            if (num <= 0) throw std::invalid_argument("");
            auto pred = std::bind(std::equal_to<int>(),
                std::bind(VertexCount(), std::placeholders::_1),
                num);
            int cnt = std::count_if(polygons.begin(), polygons.end(), pred);
            std::cout << cnt << '\n';
            return;
        }
        catch (...) {
            std::cout << "<INVALID COMMAND>" << '\n';
            return;
        }
    }

    if (cmd == "RMECHO") {
        size_t pos = line.find(' ');
        if (pos == std::string::npos) {
            std::cout << "<INVALID COMMAND>" << '\n';
            return;
        }
        std::string rest = line.substr(pos + 1);
        size_t start = rest.find_first_not_of(" \t");
        if (start != std::string::npos) rest = rest.substr(start);
        else rest.clear();
        Polygon target;
        if (!parsePolygon(rest, target)) {
            std::cout << "<INVALID COMMAND>" << '\n';
            return;
        }
        std::vector<Polygon> newPolygons;
        std::unique_copy(polygons.begin(), polygons.end(),
            std::back_inserter(newPolygons),
            BothEqualTarget(target));
        int removed = polygons.size() - newPolygons.size();
        polygons.swap(newPolygons);
        std::cout << removed << '\n';
        return;
    }

    if (cmd == "SAME") {
        size_t pos = line.find(' ');
        if (pos == std::string::npos) {
            std::cout << "<INVALID COMMAND>" << '\n';
            return;
        }
        std::string rest = line.substr(pos + 1);
        size_t start = rest.find_first_not_of(" \t");
        if (start != std::string::npos) rest = rest.substr(start);
        else rest.clear();
        Polygon target;
        if (!parsePolygon(rest, target)) {
            std::cout << "<INVALID COMMAND>" << '\n';
            return;
        }
        int cnt = std::count_if(polygons.begin(), polygons.end(), IsCompatibleWith(target));
        std::cout << cnt << '\n';
        return;
    }
    std::cout << "<INVALID COMMAND>" << '\n';
}

int main(int argc, char* argv[]) {
    setlocale(LC_ALL, "RUS");
    if (argc < 2) {
        std::cerr << "Error: file name not specified " << '\n';
        return 1;
    }
    const char* filename = argv[1];
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Failed to open file " << filename << '\n';
        return 1;
    }

    std::vector<Polygon> polygons;
    std::string line;
    while (std::getline(file, line)) {
        Polygon p;
        if (parsePolygon(line, p)) {
            polygons.push_back(p);
        }
    }
    file.close();

    while (std::getline(std::cin, line)) {
        processCommand(line, polygons);
    }

    return 0;
}
