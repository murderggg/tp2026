#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <iterator>
#include <string>
#include <sstream>
#include <cmath>
#include <iomanip>
#include <functional>
#include <cctype>

struct Point {
    int x, y;
    Point(int x = 0, int y = 0) : x(x), y(y) {}
};

bool operator==(const Point& a, const Point& b) {
    return a.x == b.x && a.y == b.y;
}

bool operator!=(const Point& a, const Point& b) {
    return !(a == b);
}

bool operator<(const Point& a, const Point& b) {
    if (a.x != b.x) return a.x < b.x;
    return a.y < b.y;
}

std::ostream& operator<<(std::ostream& out, const Point& p) {
    out << "(" << p.x << ";" << p.y << ")";
    return out;
}

struct Polygon {
    std::vector<Point> points;

    bool operator==(const Polygon& other) const {
        if (points.size() != other.points.size()) return false;
        return std::equal(points.begin(), points.end(), other.points.begin());
    }

    bool operator!=(const Polygon& other) const {
        return !(*this == other);
    }

    size_t getVertexCount() const {
        return points.size();
    }

    double getArea() const {
        if (points.size() < 3) return 0.0;
        std::vector<size_t> indices(points.size());
        std::iota(indices.begin(), indices.end(), 0);

        struct TwiceAreaFunctor {
            const std::vector<Point>& pts;
            TwiceAreaFunctor(const std::vector<Point>& p) : pts(p) {}
            double operator()(double sum, size_t i) const {
                const Point& p1 = pts[i];
                const Point& p2 = pts[(i + 1) % pts.size()];
                return sum + (p1.x * p2.y - p2.x * p1.y);
            }
        };

        double twiceArea = std::accumulate(indices.begin(), indices.end(), 0.0, TwiceAreaFunctor(points));
        return std::abs(twiceArea) / 2.0;
    }

    Point getMinPoint() const {
        if (points.empty()) return Point();
        return *std::min_element(points.begin(), points.end());
    }

    Point getMaxPoint() const {
        if (points.empty()) return Point();
        return *std::max_element(points.begin(), points.end());
    }

    bool isInsideBoundingBox(const Point& minBound, const Point& maxBound) const {
        Point minP = getMinPoint();
        Point maxP = getMaxPoint();
        return minP.x >= minBound.x && minP.y >= minBound.y &&
            maxP.x <= maxBound.x && maxP.y <= maxBound.y;
    }
};

std::ostream& operator<<(std::ostream& out, const Polygon& poly) {
    out << poly.points.size();
    std::ostream_iterator<Point> outIt(out, " ");
    std::copy(poly.points.begin(), poly.points.end(), outIt);
    return out;
}

struct ParsePointFunctor {
    Point operator()(const std::string& token) const {
        if (token.empty() || token.front() != '(' || token.back() != ')') throw std::runtime_error("");
        std::string inner = token.substr(1, token.length() - 2);
        size_t semicolonPos = inner.find(';');
        if (semicolonPos == std::string::npos) throw std::runtime_error("");
        Point p;
        p.x = std::stoi(inner.substr(0, semicolonPos));
        p.y = std::stoi(inner.substr(semicolonPos + 1));
        return p;
    }
};

std::istream& operator>>(std::istream& in, Polygon& poly) {
    std::string line;
    if (!std::getline(in, line)) return in;
    if (line.empty()) return in;

    std::istringstream iss(line);
    int vertexCount;
    if (!(iss >> vertexCount) || vertexCount < 3) {
        in.setstate(std::ios::failbit);
        return in;
    }

    std::vector<Point> points;
    points.resize(vertexCount);

    std::vector<std::string> tokens;
    tokens.reserve(vertexCount);
    std::copy_n(std::istream_iterator<std::string>(iss), vertexCount, std::back_inserter(tokens));

    try {
        std::transform(tokens.begin(), tokens.end(), points.begin(), ParsePointFunctor());
    }
    catch (...) {
        in.setstate(std::ios::failbit);
        return in;
    }

    std::string extra;
    if (iss >> extra) {
        in.setstate(std::ios::failbit);
        return in;
    }

    poly.points = points;
    return in;
}

class InputStreamIterator {
public:
    using iterator_category = std::input_iterator_tag;
    using value_type = Polygon;
    using difference_type = std::ptrdiff_t;
    using pointer = const Polygon*;
    using reference = const Polygon&;

    InputStreamIterator() : in_(nullptr) {}
    explicit InputStreamIterator(std::istream& in) : in_(&in) { ++(*this); }

    reference operator*() const { return value_; }
    pointer operator->() const { return &value_; }

    InputStreamIterator& operator++() {
        if (!in_) return *this;
        while (std::getline(*in_, line_)) {
            if (line_.empty()) continue;
            std::istringstream iss(line_);
            Polygon poly;
            if (iss >> poly) {
                value_ = poly;
                return *this;
            }
        }
        in_ = nullptr;
        return *this;
    }

    InputStreamIterator operator++(int) {
        InputStreamIterator tmp = *this;
        ++(*this);
        return tmp;
    }

    bool operator==(const InputStreamIterator& other) const {
        return in_ == other.in_;
    }

    bool operator!=(const InputStreamIterator& other) const {
        return !(*this == other);
    }

private:
    std::istream* in_;
    Polygon value_;
    std::string line_;
};

std::vector<std::string> splitArgs(const std::string& line) {
    std::istringstream iss(line);
    std::vector<std::string> args;
    std::copy(std::istream_iterator<std::string>(iss), std::istream_iterator<std::string>(), std::back_inserter(args));
    return args;
}

Polygon polygonFromArgs(const std::vector<std::string>& args) {
    if (args.empty()) throw std::runtime_error("");
    int vertexCount = std::stoi(args[0]);
    if (vertexCount < 3 || args.size() < 1 + static_cast<size_t>(vertexCount)) {
        throw std::runtime_error("");
    }

    std::vector<Point> points;
    points.resize(vertexCount);

    std::transform(args.begin() + 1, args.begin() + 1 + vertexCount, points.begin(), ParsePointFunctor());

    Polygon poly;
    poly.points = points;
    return poly;
}

struct RemoveConsecutiveDuplicates {
    const Polygon& target;
    mutable bool shouldDelete;

    RemoveConsecutiveDuplicates(const Polygon& t) : target(t), shouldDelete(false) {}

    bool operator()(const Polygon& p) const {
        if (p == target) {
            if (shouldDelete) {
                return true;
            }
            shouldDelete = true;
            return false;
        }
        shouldDelete = false;
        return false;
    }
};

struct MinPointFunctor {
    Point operator()(const Point& a, const Point& b) const {
        return Point(std::min(a.x, b.x), std::min(a.y, b.y));
    }
};

struct MaxPointFunctor {
    Point operator()(const Point& a, const Point& b) const {
        return Point(std::max(a.x, b.x), std::max(a.y, b.y));
    }
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Error: filename not specified" << std::endl;
        return 1;
    }

    std::ifstream file(argv[1]);
    if (!file.is_open()) {
        std::cerr << "Error: cannot open file " << argv[1] << std::endl;
        return 1;
    }

    std::vector<Polygon> polygons;
    InputStreamIterator inputBegin(file);
    InputStreamIterator inputEnd;
    std::copy(inputBegin, inputEnd, std::back_inserter(polygons));
    file.close();

    using namespace std::placeholders;
    std::string cmdLine;

    while (std::getline(std::cin, cmdLine)) {
        if (cmdLine.empty()) continue;

        std::vector<std::string> tokens = splitArgs(cmdLine);
        if (tokens.empty()) {
            std::cout << "<INVALID COMMAND>" << std::endl;
            continue;
        }

        const std::string& command = tokens[0];

        try {
            if (command == "AREA") {
                if (tokens.size() < 2) throw std::runtime_error("");
                const std::string& param = tokens[1];

                if (param == "EVEN") {
                    double sum = std::accumulate(polygons.begin(), polygons.end(), 0.0,
                        std::bind(std::plus<double>(),
                            _1,
                            std::bind(
                                std::multiplies<double>(),
                                std::bind(&Polygon::getArea, _2),
                                std::bind(std::equal_to<size_t>(),
                                    std::bind(std::modulus<size_t>(),
                                        std::bind(&Polygon::getVertexCount, _2),
                                        2),
                                    0))));
                    std::cout << std::fixed << std::setprecision(1) << sum << std::endl;
                }
                else if (param == "ODD") {
                    double sum = std::accumulate(polygons.begin(), polygons.end(), 0.0,
                        std::bind(std::plus<double>(),
                            _1,
                            std::bind(
                                std::multiplies<double>(),
                                std::bind(&Polygon::getArea, _2),
                                std::bind(std::not_equal_to<size_t>(),
                                    std::bind(std::modulus<size_t>(),
                                        std::bind(&Polygon::getVertexCount, _2),
                                        2),
                                    0))));
                    std::cout << std::fixed << std::setprecision(1) << sum << std::endl;
                }
                else if (param == "MEAN") {
                    if (polygons.empty()) throw std::runtime_error("");
                    double sum = std::accumulate(polygons.begin(), polygons.end(), 0.0,
                        std::bind(std::plus<double>(), _1, std::bind(&Polygon::getArea, _2)));
                    double mean = sum / polygons.size();
                    std::cout << std::fixed << std::setprecision(1) << mean << std::endl;
                }
                else {
                    int targetCount = std::stoi(param);
                    double sum = std::accumulate(polygons.begin(), polygons.end(), 0.0,
                        std::bind(std::plus<double>(),
                            _1,
                            std::bind(
                                std::multiplies<double>(),
                                std::bind(&Polygon::getArea, _2),
                                std::bind(std::equal_to<size_t>(),
                                    std::bind(&Polygon::getVertexCount, _2),
                                    targetCount))));
                    std::cout << std::fixed << std::setprecision(1) << sum << std::endl;
                }
            }
            else if (command == "MAX") {
                if (tokens.size() < 2) throw std::runtime_error("");
                if (polygons.empty()) throw std::runtime_error("");
                const std::string& param = tokens[1];

                if (param == "AREA") {
                    auto it = std::max_element(polygons.begin(), polygons.end(),
                        std::bind(std::less<double>(),
                            std::bind(&Polygon::getArea, _1),
                            std::bind(&Polygon::getArea, _2)));
                    std::cout << std::fixed << std::setprecision(1) << it->getArea() << std::endl;
                }
                else if (param == "VERTEXES") {
                    auto it = std::max_element(polygons.begin(), polygons.end(),
                        std::bind(std::less<size_t>(),
                            std::bind(&Polygon::getVertexCount, _1),
                            std::bind(&Polygon::getVertexCount, _2)));
                    std::cout << it->getVertexCount() << std::endl;
                }
                else {
                    std::cout << "<INVALID COMMAND>" << std::endl;
                }
            }
            else if (command == "MIN") {
                if (tokens.size() < 2) throw std::runtime_error("");
                if (polygons.empty()) throw std::runtime_error("");
                const std::string& param = tokens[1];

                if (param == "AREA") {
                    auto it = std::min_element(polygons.begin(), polygons.end(),
                        std::bind(std::less<double>(),
                            std::bind(&Polygon::getArea, _1),
                            std::bind(&Polygon::getArea, _2)));
                    std::cout << std::fixed << std::setprecision(1) << it->getArea() << std::endl;
                }
                else if (param == "VERTEXES") {
                    auto it = std::min_element(polygons.begin(), polygons.end(),
                        std::bind(std::less<size_t>(),
                            std::bind(&Polygon::getVertexCount, _1),
                            std::bind(&Polygon::getVertexCount, _2)));
                    std::cout << it->getVertexCount() << std::endl;
                }
                else {
                    std::cout << "<INVALID COMMAND>" << std::endl;
                }
            }
            else if (command == "COUNT") {
                if (tokens.size() < 2) throw std::runtime_error("");
                const std::string& param = tokens[1];

                if (param == "EVEN") {
                    size_t cnt = std::count_if(polygons.begin(), polygons.end(),
                        std::bind(std::equal_to<size_t>(),
                            std::bind(std::modulus<size_t>(),
                                std::bind(&Polygon::getVertexCount, _1),
                                2),
                            0));
                    std::cout << cnt << std::endl;
                }
                else if (param == "ODD") {
                    size_t cnt = std::count_if(polygons.begin(), polygons.end(),
                        std::bind(std::not_equal_to<size_t>(),
                            std::bind(std::modulus<size_t>(),
                                std::bind(&Polygon::getVertexCount, _1),
                                2),
                            0));
                    std::cout << cnt << std::endl;
                }
                else {
                    int targetCount = std::stoi(param);
                    size_t cnt = std::count_if(polygons.begin(), polygons.end(),
                        std::bind(std::equal_to<size_t>(),
                            std::bind(&Polygon::getVertexCount, _1),
                            targetCount));
                    std::cout << cnt << std::endl;
                }
            }
            else if (command == "RMECHO") {
                if (tokens.size() < 2) throw std::runtime_error("");
                std::vector<std::string> args(tokens.begin() + 1, tokens.end());
                Polygon target = polygonFromArgs(args);

                RemoveConsecutiveDuplicates counter(target);
                size_t removed = std::count_if(polygons.begin(), polygons.end(), counter);

                std::vector<Polygon> temp;
                RemoveConsecutiveDuplicates filter(target);
                std::remove_copy_if(polygons.begin(), polygons.end(), std::back_inserter(temp), filter);
                polygons = temp;

                std::cout << removed << std::endl;

                std::copy(polygons.begin(), polygons.end(), std::ostream_iterator<Polygon>(std::cout, "\n"));
            }
            else if (command == "INFRAME") {
                if (tokens.size() < 2) throw std::runtime_error("");
                std::vector<std::string> args(tokens.begin() + 1, tokens.end());
                Polygon target = polygonFromArgs(args);

                if (polygons.empty()) {
                    std::cout << "<INVALID COMMAND>" << std::endl;
                    continue;
                }

                Point globalMin = std::accumulate(polygons.begin(), polygons.end(), polygons[0].getMinPoint(),
                    std::bind(MinPointFunctor(), _1, std::bind(&Polygon::getMinPoint, _2)));

                Point globalMax = std::accumulate(polygons.begin(), polygons.end(), polygons[0].getMaxPoint(),
                    std::bind(MaxPointFunctor(), _1, std::bind(&Polygon::getMaxPoint, _2)));

                bool result = target.isInsideBoundingBox(globalMin, globalMax);
                std::cout << (result ? "<TRUE>" : "<FALSE>") << std::endl;
            }
            else {
                std::cout << "<INVALID COMMAND>" << std::endl;
            }
        }
        catch (const std::exception&) {
            std::cout << "<INVALID COMMAND>" << std::endl;
        }
    }

    return 0;
}
