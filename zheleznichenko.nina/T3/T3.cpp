#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <functional>
#include <iterator>
#include <iomanip>
#include <cmath>

struct Point {
  int x, y;
};

struct Polygon {
  std::vector<Point> points;
};

struct AreaPartSum {
  const std::vector<Point>& pts;
  double& sum_ref;
  AreaPartSum(const std::vector<Point>& p, double& s) : pts(p), sum_ref(s) {}
  bool operator()(const Point& p1) {
    size_t i = &p1 - &pts[0];
    const Point& p2 = pts[(i + 1) % pts.size()];
    sum_ref += (static_cast<double>(p1.x) * p2.y -
      static_cast<double>(p2.x) * p1.y);
    return true;
  }
};

double getArea(const Polygon& p) {
  if (p.points.size() < 3) return 0.0;
  double total = 0.0;
  std::count_if(p.points.begin(), p.points.end(),
    AreaPartSum(p.points, total));
  return std::abs(total) / 2.0;
}

struct CheckRightAngle {
  const std::vector<Point>& pts;
  CheckRightAngle(const std::vector<Point>& p) : pts(p) {}
  bool operator()(const Point& b) const {
    size_t i = &b - &pts[0];
    const Point& a = pts[(i == 0) ? pts.size() - 1 : i - 1];
    const Point& c = pts[(i + 1) % pts.size()];
    long long v1x = a.x - b.x, v1y = a.y - b.y;
    long long v2x = c.x - b.x, v2y = c.y - b.y;
    return (v1x * v2x + v1y * v2y == 0);
  }
};

struct HasRightAngle {
  bool operator()(const Polygon& p) const {
    return std::any_of(p.points.begin(), p.points.end(),
      CheckRightAngle(p.points));
  }
};

struct IsSizeEven {
  bool operator()(const Polygon& p) { return p.points.size() % 2 == 0; }
};
struct IsSizeOdd {
  bool operator()(const Polygon& p) { return p.points.size() % 2 != 0; }
};
struct IsSizeN {
  size_t n;
  IsSizeN(size_t _n) : n(_n) {}
  bool operator()(const Polygon& p) { return p.points.size() == n; }
};

struct SumArea {
  double& total;
  SumArea(double& t) : total(t) {}
  bool operator()(const Polygon& p) {
    total += getArea(p);
    return true;
  }
};

struct GetAreaValue {
  double operator()(const Polygon& p) { return getArea(p); }
};
struct GetVertexCount {
  double operator()(const Polygon& p) {
    return static_cast<double>(p.points.size());
  }
};

std::istream& readPoint(std::istream& is, Point& p) {
  char c1, c2, c3;
  is >> std::skipws >> c1 >> p.x >> c2 >> p.y >> c3;
  if (is && (c1 != '(' || c2 != ';' || c3 != ')')) {
    is.setstate(std::ios::failbit);
  }
  return is;
}

struct PointGenerator {
  std::istream& is;
  PointGenerator(std::istream& i) : is(i) {}
  Point operator()() {
    Point p;
    if (!(readPoint(is, p))) throw std::runtime_error("err");
    return p;
  }
};

bool parsePolygon(const std::string& line, Polygon& poly) {
  if (line.empty()) return false;
  std::istringstream iss(line);
  size_t n;
  if (!(iss >> n) || n < 3) return false;
  poly.points.clear();
  try {
    std::generate_n(std::back_inserter(poly.points), n, PointGenerator(iss));
  }
  catch (...) { return false; }
  std::string extra;
  return !(iss >> extra);
}

struct PointLess {
  bool operator()(const Point& a, const Point& b) const {
    return a.x < b.x || (a.x == b.x && a.y < b.y);
  }
};
struct PointEqual {
  bool operator()(const Point& a, const Point& b) const {
    return a.x == b.x && a.y == b.y;
  }
};

struct IsPermutation {
  Polygon target;
  IsPermutation(Polygon t) : target(t) {
    std::sort(target.points.begin(), target.points.end(), PointLess());
  }
  bool operator()(const Polygon& p) {
    if (p.points.size() != target.points.size()) return false;
    std::vector<Point> temp = p.points;
    std::sort(temp.begin(), temp.end(), PointLess());
    return std::equal(temp.begin(), temp.end(),
      target.points.begin(), PointEqual());
  }
};

int main(int argc, char* argv[]) {
  if (argc < 2) return 1;
  std::ifstream file(argv[1]);
  std::vector<Polygon> data;
  std::string line;
  while (std::getline(file, line)) {
    Polygon p;
    if (parsePolygon(line, p)) data.push_back(p);
  }

  std::string cmd;
  while (std::cin >> cmd) {
    if (cmd == "AREA") {
      std::string arg; std::cin >> arg;
      std::vector<Polygon> sel;
      if (arg == "EVEN") {
        std::copy_if(data.begin(), data.end(),
          std::back_inserter(sel), IsSizeEven());
      }
      else if (arg == "ODD") {
        std::copy_if(data.begin(), data.end(),
          std::back_inserter(sel), IsSizeOdd());
      }
      else if (arg == "MEAN") {
        if (data.empty()) { std::cout << "<INVALID COMMAND>\n"; continue; }
        sel = data;
      }
      else {
        try {
          size_t n = std::stoul(arg);
          if (n < 3) throw std::exception();
          std::copy_if(data.begin(), data.end(),
            std::back_inserter(sel), IsSizeN(n));
        }
        catch (...) { std::cout << "<INVALID COMMAND>\n"; continue; }
      }
      double sum = 0;
      std::count_if(sel.begin(), sel.end(), SumArea(sum));
      std::cout << std::fixed << std::setprecision(1);
      std::cout << (arg == "MEAN" ? sum / data.size() : sum) << "\n";
    }
    else if (cmd == "MAX") {
      std::string arg; std::cin >> arg;
      if (data.empty()) { std::cout << "<INVALID COMMAND>\n"; continue; }
      if (arg == "AREA") {
        std::vector<double> res(data.size());
        std::transform(data.begin(), data.end(), res.begin(), GetAreaValue());
        std::cout << std::fixed << std::setprecision(1)
          << *std::max_element(res.begin(), res.end()) << "\n";
      }
      else if (arg == "VERTEXES") {
        std::vector<double> res(data.size());
        std::transform(data.begin(), data.end(), res.begin(), GetVertexCount());
        std::cout << std::defaultfloat << std::setprecision(0)
          << *std::max_element(res.begin(), res.end()) << "\n";
      }
      else std::cout << "<INVALID COMMAND>\n";
    }
    else if (cmd == "COUNT") {
      std::string arg; std::cin >> arg;
      if (arg == "EVEN") {
        std::cout << std::count_if(data.begin(), data.end(), IsSizeEven()) << "\n";
      }
      else if (arg == "ODD") {
        std::cout << std::count_if(data.begin(), data.end(), IsSizeOdd()) << "\n";
      }
      else {
        try {
          size_t n = std::stoul(arg);
          if (n < 3) throw std::exception();
          std::cout << std::count_if(data.begin(), data.end(), IsSizeN(n)) << "\n";
        }
        catch (...) { std::cout << "<INVALID COMMAND>\n"; }
      }
    }
    else if (cmd == "PERMS") {
      std::string rest; std::getline(std::cin, rest);
      Polygon target;
      if (parsePolygon(rest, target)) {
        std::cout << std::count_if(data.begin(), data.end(),
          IsPermutation(target)) << "\n";
      }
      else std::cout << "<INVALID COMMAND>\n";
    }
    else if (cmd == "RIGHTSHAPES") {
      std::cout << std::count_if(data.begin(), data.end(),
        HasRightAngle()) << "\n";
    }
    else {
      std::cout << "<INVALID COMMAND>\n";
      std::cin.ignore(10000, '\n');
    }
  }
  return 0;
}
