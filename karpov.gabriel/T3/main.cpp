#include <iostream>
#include <algorithm>
#include <vector>
#include <cstddef>
#include <fstream>
#include <sstream>
#include <iterator>
#include <numeric>
#include <functional>
#include <iomanip>
#include <string>
#include <cmath>

// variant 11

struct Point {
    int x, y;
};

struct Polygon {
    std::vector<Point> points;
};

std::istream& setFailBit(std::istream& in) {
    in.setstate(std::ios::failbit);
    return in;
}

std::istream& operator>>(std::istream& in, Point& dest) {
    std::istream::sentry sentry(in);
    if (!sentry)
        return in;
    char c;
    int x, y;
    in >> c;
    if (c != '(')
        return setFailBit(in);
    in >> x >> c;
    if (c != ';')
        return setFailBit(in);
    in >> y >> c;
    if (c != ')')
        return setFailBit(in);
    dest = {x, y};
    return in;
}

std::istream& operator>>(std::istream& in, Polygon& dest) {
    std::istream::sentry sentry(in);
    if (!sentry)
        return in;
    Polygon tmp;
    std::size_t n;
    in >> n;
    if (!in || n < 3)
        return setFailBit(in);
    tmp.points.resize(n);
    std::copy_n(
        std::istream_iterator<Point>(in),
        n,
        tmp.points.begin()
    );
    if (!in)
        return in;
    dest = std::move(tmp);
    return in;
}

struct GaussSum {
    const std::vector<Point>& p;
    std::size_t n;
    std::size_t i;
    explicit GaussSum(const std::vector<Point>& p)
        : p(p), n(p.size()), i(0) {}
    long long operator()(long long acc, const Point& curr) {
        const Point& next = p[(i + 1) % n];
        long long term =
            static_cast<long long>(curr.x) * next.y -
            static_cast<long long>(next.x) * curr.y;
        ++i;
        return acc + term;
    }
};

double area(const Polygon& poly) {
    const std::vector<Point>& p = poly.points;
    long long sum = std::accumulate(p.begin(), p.end(), 0LL, GaussSum(p));
    return std::abs(sum) / 2.0;
}

struct SumArea {
    double operator()(double acc, const Polygon& p) const {
        return acc + area(p);
    }
};

struct IsEven {
    bool operator()(const Polygon& p) const {
        return p.points.size() % 2 == 0;
    }
};

struct SumIfEven {
    double operator()(double acc, const Polygon& p) const {
        return IsEven()(p) ? acc + area(p) : acc;
    }
};

struct SumIfOdd {
    double operator()(double acc, const Polygon& p) const {
        return !IsEven()(p) ? acc + area(p) : acc;
    }
};

struct AreaIfVertexCount {
    std::size_t n;
    explicit AreaIfVertexCount(std::size_t n) : n(n) {}
    double operator()(double acc, const Polygon& p) const {
        return p.points.size() == n ? acc + area(p) : acc;
    }
};

struct AreaCompare {
    bool operator()(const Polygon& a, const Polygon& b) const {
        return area(a) < area(b);
    }
};

struct NumOfVertexesCompare {
    bool operator()(const Polygon& a, const Polygon& b) const {
        return a.points.size() < b.points.size();
    }
};

struct IsNumOfVertexes {
    std::size_t n;
    explicit IsNumOfVertexes(std::size_t n) : n(n) {}
    bool operator()(const Polygon& p) const {
        return p.points.size() == n;
    }
};

bool operator==(const Point& a, const Point& b) {
    return a.x == b.x && a.y == b.y;
}

bool operator==(const Polygon& a, const Polygon& b) {
    if (a.points.size() != b.points.size())
        return false;
    return std::equal(a.points.begin(), a.points.end(), b.points.begin());
}

struct IsPermutation {
    Polygon target;
    explicit IsPermutation(const Polygon& p) : target(p) {
        std::sort(target.points.begin(), target.points.end(),
            [](const Point& a, const Point& b) {
                return a.x < b.x || (a.x == b.x && a.y < b.y);
            });
    }
    bool operator()(const Polygon& p) const {
        if (p.points.size() != target.points.size())
            return false;
        Polygon sorted = p;
        std::sort(sorted.points.begin(), sorted.points.end(),
            [](const Point& a, const Point& b) {
                return a.x < b.x || (a.x == b.x && a.y < b.y);
            });
        return std::equal(
            target.points.begin(), target.points.end(),
            sorted.points.begin()
        );
    }
};

struct MaxSeqState {
    std::size_t cur;
    std::size_t best;
};

struct MaxSeqAccumulator {
    const Polygon& target;
    explicit MaxSeqAccumulator(const Polygon& p) : target(p) {}
    MaxSeqState operator()(MaxSeqState s, const Polygon& p) const {
        if (p == target) {
            ++s.cur;
            s.best = std::max(s.best, s.cur);
        } else {
            s.cur = 0;
        }
        return s;
    }
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cout << "ERROR. No file name provided\n";
        return 1;
    }
    std::ifstream file(argv[1]);
    if (!file) {
        std::cout << "ERROR: file cannot be opened\n";
        return 1;
    }
    std::vector<Polygon> polygons;
    std::string raw;
    while (std::getline(file, raw)) {
        if (raw.empty())
            continue;
        std::istringstream iss(raw);
        Polygon poly;
        if (iss >> poly) {
            iss >> std::ws;
            if (iss.eof())
                polygons.push_back(std::move(poly));
        }
    }

    std::cout << std::fixed << std::setprecision(1);

    std::string command;
    while (std::getline(std::cin, command)) {
        if (command.empty())
            continue;
        std::istringstream iss(command);
        std::string main_cmd;
        iss >> main_cmd;
        if (main_cmd == "AREA") {
            std::string sub_cmd;
            iss >> sub_cmd;
            if (iss >> std::ws && !iss.eof()) {
                std::cout << "<INVALID COMMAND>\n";
                continue;
            }
            if (sub_cmd == "EVEN") {
                double res = std::accumulate(
                    polygons.begin(), polygons.end(), 0.0, SumIfEven()
                );
                std::cout << res << "\n";
            } else if (sub_cmd == "ODD") {
                double res = std::accumulate(
                    polygons.begin(), polygons.end(), 0.0, SumIfOdd()
                );
                std::cout << res << "\n";
            } else if (sub_cmd == "MEAN") {
                if (polygons.empty()) {
                    std::cout << "<INVALID COMMAND>\n";
                    continue;
                }
                double sum = std::accumulate(
                    polygons.begin(), polygons.end(), 0.0, SumArea()
                );
                std::cout << sum / polygons.size() << "\n";
            } else {
                try {
                    std::size_t num = std::stoul(sub_cmd);
                    if (num < 3) {
                        std::cout << "<INVALID COMMAND>\n";
                        continue;
                    }
                    double res = std::accumulate(
                        polygons.begin(), polygons.end(), 0.0, AreaIfVertexCount(num)
                    );
                    std::cout << res << "\n";
                } catch (...) {
                    std::cout << "<INVALID COMMAND>\n";
                }
            }
        } else if (main_cmd == "MAX") {
            std::string sub_cmd;
            iss >> sub_cmd;
            if (iss >> std::ws && !iss.eof()) {
                std::cout << "<INVALID COMMAND>\n";
                continue;
            }
            if (polygons.empty()) {
                std::cout << "<INVALID COMMAND>\n";
                continue;
            }
            if (sub_cmd == "AREA") {
                auto it = std::max_element(
                    polygons.begin(), polygons.end(), AreaCompare()
                );
                std::cout << area(*it) << "\n";
            } else if (sub_cmd == "VERTEXES") {
                auto it = std::max_element(
                    polygons.begin(), polygons.end(), NumOfVertexesCompare()
                );
                std::cout << it->points.size() << "\n";
            } else {
                std::cout << "<INVALID COMMAND>\n";
            }
        } else if (main_cmd == "MIN") {
            std::string sub_cmd;
            iss >> sub_cmd;
            if (iss >> std::ws && !iss.eof()) {
                std::cout << "<INVALID COMMAND>\n";
                continue;
            }
            if (polygons.empty()) {
                std::cout << "<INVALID COMMAND>\n";
                continue;
            }
            if (sub_cmd == "AREA") {
                auto it = std::min_element(
                    polygons.begin(), polygons.end(), AreaCompare()
                );
                std::cout << area(*it) << "\n";
            } else if (sub_cmd == "VERTEXES") {
                auto it = std::min_element(
                    polygons.begin(), polygons.end(), NumOfVertexesCompare()
                );
                std::cout << it->points.size() << "\n";
            } else {
                std::cout << "<INVALID COMMAND>\n";
            }
        } else if (main_cmd == "COUNT") {
            std::string sub_cmd;
            iss >> sub_cmd;
            if (iss >> std::ws && !iss.eof()) {
                std::cout << "<INVALID COMMAND>\n";
                continue;
            }
            if (sub_cmd == "EVEN") {
                std::size_t cnt = std::count_if(
                    polygons.begin(), polygons.end(), IsEven()
                );
                std::cout << cnt << "\n";
            } else if (sub_cmd == "ODD") {
                std::size_t cnt = std::count_if(
                    polygons.begin(), polygons.end(),
                    std::bind(std::logical_not<bool>(), std::bind(IsEven(), std::placeholders::_1))
                );
                std::cout << cnt << "\n";
            } else {
                try {
                    std::size_t num = std::stoul(sub_cmd);
                    if (num < 3) {
                        std::cout << "<INVALID COMMAND>\n";
                        continue;
                    }
                    std::size_t cnt = std::count_if(
                        polygons.begin(), polygons.end(), IsNumOfVertexes(num)
                    );
                    std::cout << cnt << "\n";
                } catch (...) {
                    std::cout << "<INVALID COMMAND>\n";
                }
            }
        } else if (main_cmd == "PERMS") {
            Polygon poly;
            iss >> poly;
            if (!iss || (iss >> std::ws && !iss.eof())) {
                std::cout << "<INVALID COMMAND>\n";
                continue;
            }
            std::size_t cnt = std::count_if(
                polygons.begin(), polygons.end(), IsPermutation(poly)
            );
            std::cout << cnt << "\n";
        } else if (main_cmd == "MAXSEQ") {
            Polygon poly;
            iss >> poly;
            if (!iss || (iss >> std::ws && !iss.eof())) {
                std::cout << "<INVALID COMMAND>\n";
                continue;
            }
            if (polygons.empty()) {
                std::cout << "0\n";
                continue;
            }
            MaxSeqState res = std::accumulate(
                polygons.begin(), polygons.end(),
                MaxSeqState{0, 0},
                MaxSeqAccumulator(poly)
            );
            std::cout << res.best << "\n";
        } else {
            std::cout << "<INVALID COMMAND>\n";
        }
    }

    return 0;
}
