//Panfilov Ivan, variant 7
#include <iostream>
#include <algorithm>
#include <functional>
#include <vector>
#include <sstream>
#include <string>
#include <fstream>
#include <iomanip>
#include <numeric>
#include <cmath>

struct Point {
    int x, y;
    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }
    bool operator!= (const Point& other) const {
        return !(*this == other);
    }
};

struct Polygon {
    std::vector<Point> points;
    bool operator==(const Polygon& other) const {
        if (points.size() != other.points.size()) return false;
        return std::equal(points.begin(), points.end(), other.points.begin());
    }
    bool operator!=(const Polygon& other) const {
        return !(*this == other);
    }
};

double area(const Polygon& poly) {
    const auto& p = poly.points;
    int n = p.size();
    long long sum = 0;
    for (int i = 0; i < n; ++i) {
        int j = (i + 1) % n;
        sum += static_cast<long long>(p[i].x) * p[j].y -
            static_cast<long long>(p[j].x) * p[i].y;
    }
    return std::abs(sum) / 2.0;
}

struct IsEvenVertexCount {
    bool operator()(const Polygon& p) const {
        return p.points.size() % 2 == 0;
    }
};

struct IsOddVertexCount {
    bool operator()(const Polygon& p) const {
        return p.points.size() % 2 == 1;
    }
};

struct HasVertexCount {
    size_t target;
    HasVertexCount(int t) : target(static_cast<size_t>(t)) {}
    bool operator()(const Polygon& p) const {
        return p.points.size() == target;
    }
};

struct LessArea {
    bool operator()(const Polygon& a, const Polygon& b) const {
        return area(a) < area(b);
    }
};

struct LessVertexCount {
    bool operator()(const Polygon& a, const Polygon& b) const {
        return a.points.size() < b.points.size();
    }
};

bool isSameByTranslation(const Polygon& a, const Polygon& b) {
    if (a.points.size() != b.points.size()) {
        return false;
    }
    if (a.points.empty()) {
        return true;
    }
    size_t n = a.points.size();
    for (size_t start = 0; start < n; ++start) {
        int dx = b.points[start].x - a.points[0].x;
        int dy = b.points[start].y - a.points[0].y;
        bool forwardMatch = true;
        for (size_t i = 0; i < n; ++i) {
            size_t idx = (start + i) % n;
            if (b.points[idx].x - a.points[i].x != dx ||
                b.points[idx].y - a.points[i].y != dy) {
                forwardMatch = false;
                break;
            }
        }
        if (forwardMatch) return true;
        bool backwardMatch = true;
        for (size_t i = 0; i < n; ++i) {
            size_t idx = (start - i + n) % n;
            if (b.points[idx].x - a.points[i].x != dx ||
                b.points[idx].y - a.points[i].y != dy) {
                backwardMatch = false;
                break;
            }
        }
        if (backwardMatch) return true;
    }
    return false;
}

struct IsSamePolygon {
    Polygon target;
    IsSamePolygon(const Polygon& t) : target(t) {}
    bool operator()(const Polygon& p) const {
        return isSameByTranslation(p, target);
    }
};

struct SumAreas {
    double operator()(double sum, const Polygon& p) const {
        return sum + area(p);
    }
};

struct SumEvenAreas {
    double operator()(double sum, const Polygon& p) const {
        if (p.points.size() % 2 == 0) {
            return sum + area(p);
        }
        return sum;
    }
};

struct SumOddAreas {
    double operator()(double sum, const Polygon& p) const {
        if (p.points.size() % 2 == 1) {
            return sum + area(p);
        }
        return sum;
    }
};

struct SumAreasWithVertexCount {
    size_t target;
    SumAreasWithVertexCount(int t) : target(static_cast<size_t>(t)) {}
    double operator()(double sum, const Polygon& p) const {
        if (p.points.size() == target) {
            return sum + area(p);
        }
        return sum;
    }
};

Polygon parsePolygon(const std::string& line) {
    std::istringstream iss(line);
    int n;
    if (!(iss >> n) || n <= 2) {
        return {};
    }
    Polygon poly;
    for (int i = 0; i < n; ++i) {
        char open, semi, close;
        int x, y;
        if (!(iss >> open >> x >> semi >> y >> close)) {
            return {};
        }
        if (open != '(' || semi != ';' || close != ')') {
            return {};
        }
        poly.points.push_back({ x, y });
    }
    std::string extra;
    if (iss >> extra) {
        return {};
    }
    return poly;
}

int rmecho(std::vector<Polygon>& polygons, const Polygon& pol) {
    std::vector<Polygon> res;
    int count = 0;
    size_t i = 0;
    while (i < polygons.size()) {
        if (polygons[i] == pol) {
            size_t j = i + 1;
            while (j < polygons.size() && polygons[j] == pol) {
                j++;
            }
            count += static_cast<int>(j - i - 1);
            res.push_back(polygons[i]);
            i = j;
        }
        else {
            res.push_back(polygons[i]);
            i++;
        }
    }
    polygons = res;
    return count;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "ERROR: Invalid number of parameters\n";
        return 1;
    }
    std::ifstream file(argv[1]);
    if (!file.is_open()) {
        std::cerr << "ERROR: cannot open file " << argv[1] << "\n";
        return 1;
    }
    std::vector<Polygon> polygons;
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        Polygon pol = parsePolygon(line);
        if (!pol.points.empty()) {
            polygons.push_back(pol);
        }
    }
    file.close();
    std::string command;
    std::cout << std::fixed << std::setprecision(1);
    while (std::getline(std::cin, command)) {
        if (command.empty()) continue;
        size_t start = command.find_first_not_of(" \t");
        if (start == std::string::npos) continue;
        size_t end = command.find_last_not_of(" \t");
        command = command.substr(start, end - start + 1);
        if (command.find("RMECHO") == 0) {
            std::string strPol = command.substr(6);
            Polygon target = parsePolygon(strPol);
            if (target.points.empty()) {
                std::cout << "<INVALID COMMAND>\n";
                continue;
            }
            int removedCount = rmecho(polygons, target);
            std::cout << removedCount << "\n";
        }
        else if (command.find("SAME") == 0) {
            std::string strPol = command.substr(4);
            Polygon target = parsePolygon(strPol);
            if (target.points.empty()) {
                std::cout << "<INVALID COMMAND>\n";
                continue;
            }
            int count = std::count_if(polygons.begin(), polygons.end(), IsSamePolygon(target));
            std::cout << count << "\n";
        }
        else if (command.find("AREA") == 0) {
            std::string param = command.substr(5);
            param.erase(0, param.find_first_not_of(" \t"));
            param.erase(param.find_last_not_of(" \t") + 1);
            if (param == "EVEN") {
                double sum = std::accumulate(polygons.begin(), polygons.end(), 0.0,
                    SumEvenAreas());
                std::cout << sum << "\n";
            }
            else if (param == "ODD") {
                double sum = std::accumulate(polygons.begin(), polygons.end(), 0.0,
                    SumOddAreas());
                std::cout << sum << "\n";
            }
            else if (param == "MEAN") {
                if (polygons.empty()) {
                    std::cout << "<INVALID COMMAND>\n";
                    continue;
                }
                double sum = std::accumulate(polygons.begin(), polygons.end(), 0.0, SumAreas());
                std::cout << sum / polygons.size() << "\n";
            }
            else {
                try {
                    int numVertices = std::stoi(param);
                    if (numVertices < 3) {
                        std::cout << "<INVALID COMMAND>\n";
                    }
                    else {
                        double sum = std::accumulate(polygons.begin(), polygons.end(), 0.0,
                            SumAreasWithVertexCount(numVertices));
                        std::cout << sum << "\n";
                    }
                }
                catch (...) {
                    std::cout << "<INVALID COMMAND>\n";
                }
            }
        }
        else if (command.find("MAX") == 0) {
            std::string param = command.substr(4);
            param.erase(0, param.find_first_not_of(" \t"));
            param.erase(param.find_last_not_of(" \t") + 1);
            if (polygons.empty()) {
                std::cout << "<INVALID COMMAND>" << "\n";
                continue;
            }
            if (param == "AREA") {
                std::vector<Polygon>::iterator maxIt = std::max_element(polygons.begin(),
                    polygons.end(), LessArea());
                std::cout << area(*maxIt) << "\n";
            }
            else if (param == "VERTEXES") {
                std::vector<Polygon>::iterator maxIt = std::max_element(polygons.begin(),
                    polygons.end(), LessVertexCount());
                std::cout << maxIt->points.size() << "\n";
            }
            else {
                std::cout << "<INVALID COMMAND>" << "\n";
            }
        }
        else if (command.find("MIN") == 0) {
            std::string param = command.substr(4);
            param.erase(0, param.find_first_not_of(" \t"));
            param.erase(param.find_last_not_of(" \t") + 1);
            if (polygons.empty()) {
                std::cout << "<INVALID COMMAND>" << "\n";
                continue;
            }
            if (param == "AREA") {
                std::vector<Polygon>::iterator minIt = std::min_element(polygons.begin(),
                    polygons.end(), LessArea());
                std::cout << area(*minIt) << "\n";
            }
            else if (param == "VERTEXES") {
                std::vector<Polygon>::iterator minIt = std::min_element(polygons.begin(),
                    polygons.end(), LessVertexCount());
                std::cout << minIt->points.size() << "\n";
            }
            else {
                std::cout << "<INVALID COMMAND>\n";
            }
        }
        else if (command.find("COUNT") == 0) {
            std::string param = command.substr(6);
            param.erase(0, param.find_first_not_of(" \t"));
            param.erase(param.find_last_not_of(" \t") + 1);

            if (param == "EVEN") {
                int count = std::count_if(polygons.begin(), polygons.end(),
                    IsEvenVertexCount());
                std::cout << count << "\n";
            }
            else if (param == "ODD") {
                int count = std::count_if(polygons.begin(), polygons.end(),
                    IsOddVertexCount());
                std::cout << count << "\n";
            }
            else {
                try {
                    int numVertices = std::stoi(param);
                    if (numVertices < 3) {
                        std::cout << "<INVALID COMMAND>\n";
                    }
                    else {
                        int count = std::count_if(polygons.begin(), polygons.end(),
                            HasVertexCount(numVertices));
                        std::cout << count << "\n";
                    }
                }
                catch (...) {
                    std::cout << "<INVALID COMMAND>\n";
                }
            }
        }
        else {
            std::cout << "<INVALID COMMAND>\n";
        }
    }
    return 0;
}
