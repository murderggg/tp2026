#include <algorithm>
#include <cmath>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <numeric>
#include <sstream>
#include <string>
#include <vector>

struct Point
{
  int x;
  int y;
};

struct Polygon
{
  std::vector< Point > points;
};

struct ParsedPolygon
{
  bool correct;
  Polygon polygon;
};

struct Line
{
  std::string text;
};

struct SequenceState
{
  std::size_t current;
  std::size_t maximum;
};

const std::size_t MIN_VERTEX_COUNT = 3;
const std::string INVALID_COMMAND = "<INVALID COMMAND>";

std::istream& operator>>(std::istream& input, Line& line)
{
  std::getline(input, line.text);
  if (!line.text.empty() && line.text.back() == '\r')
  {
    line.text.pop_back();
  }
  return input;
}

std::istream& operator>>(std::istream& input, Point& point)
{
  char left = 0;
  char separator = 0;
  char right = 0;
  int x = 0;
  int y = 0;

  input >> left >> x >> separator >> y >> right;
  if (!input || left != '(' || separator != ';' || right != ')')
  {
    input.setstate(std::ios::failbit);
    return input;
  }

  point = Point{ x, y };
  return input;
}

std::ostream& operator<<(std::ostream& output, const Point& point)
{
  output << '(' << point.x << ';' << point.y << ')';
  return output;
}

bool operator==(const Point& lhs, const Point& rhs)
{
  return lhs.x == rhs.x && lhs.y == rhs.y;
}

bool operator==(const Polygon& lhs, const Polygon& rhs)
{
  return lhs.points == rhs.points;
}

bool isDigit(char symbol)
{
  return symbol >= '0' && symbol <= '9';
}

bool readSize(const std::string& text, std::size_t& value)
{
  if (text.empty() || !std::all_of(text.cbegin(), text.cend(), isDigit))
  {
    return false;
  }

  std::istringstream input(text);
  input >> value;
  return static_cast< bool >(input);
}

bool readVertexCount(const std::string& text, std::size_t& value)
{
  return readSize(text, value) && value >= MIN_VERTEX_COUNT;
}

std::string pointToString(const Point& point)
{
  std::ostringstream output;
  output << point;
  return output.str();
}

struct AddPointToString
{
  std::string operator()(const std::string& result, const Point& point) const
  {
    return result + " " + pointToString(point);
  }
};

std::string polygonToString(const Polygon& polygon)
{
  return std::accumulate(
    polygon.points.cbegin(),
    polygon.points.cend(),
    std::to_string(polygon.points.size()),
    AddPointToString()
  );
}

struct PointReader
{
  std::istream& input;
  bool& correct;

  Point operator()() const
  {
    Point point{ 0, 0 };
    if (!(input >> point))
    {
      correct = false;
    }
    return point;
  }
};

ParsedPolygon parsePolygon(const std::string& line)
{
  std::istringstream input(line);

  std::size_t count = 0;
  input >> count;
  if (!input || count < MIN_VERTEX_COUNT || count > line.size())
  {
    return ParsedPolygon{ false, Polygon{} };
  }

  bool correct = true;
  std::vector< Point > points;
  points.reserve(count);

  std::generate_n(std::back_inserter(points), count, PointReader{ input, correct });

  std::string extra;
  if (!correct || (input >> extra))
  {
    return ParsedPolygon{ false, Polygon{} };
  }

  const Polygon polygon{ points };
  if (polygonToString(polygon) != line)
  {
    return ParsedPolygon{ false, Polygon{} };
  }

  return ParsedPolygon{ true, polygon };
}

std::vector< Polygon > readPolygons(std::istream& input)
{
  std::vector< Line > lines((std::istream_iterator< Line >(input)), std::istream_iterator< Line >());

  std::vector< ParsedPolygon > parsed;
  parsed.reserve(lines.size());
  std::transform(
    lines.cbegin(),
    lines.cend(),
    std::back_inserter(parsed),
    std::bind(parsePolygon, std::bind(&Line::text, std::placeholders::_1))
  );

  std::vector< ParsedPolygon > correct;
  correct.reserve(parsed.size());
  std::copy_if(
    parsed.cbegin(),
    parsed.cend(),
    std::back_inserter(correct),
    std::bind(&ParsedPolygon::correct, std::placeholders::_1)
  );

  std::vector< Polygon > polygons;
  polygons.reserve(correct.size());
  std::transform(
    correct.cbegin(),
    correct.cend(),
    std::back_inserter(polygons),
    std::bind(&ParsedPolygon::polygon, std::placeholders::_1)
  );

  return polygons;
}

struct GaussSum
{
  const Polygon& polygon;
  std::size_t index;

  long long operator()(long long result, const Point& point)
  {
    const Point& next = polygon.points[(index + 1) % polygon.points.size()];
    ++index;
    return result + static_cast< long long >(point.x) * next.y - static_cast< long long >(next.x) * point.y;
  }
};

double getArea(const Polygon& polygon)
{
  const long long doubleArea = std::accumulate(
    polygon.points.cbegin(),
    polygon.points.cend(),
    0LL,
    GaussSum{ polygon, 0 }
  );
  return std::abs(static_cast< double >(doubleArea)) / 2.0;
}

std::size_t getVertexCount(const Polygon& polygon)
{
  return polygon.points.size();
}

struct AddArea
{
  double operator()(double result, const Polygon& polygon) const
  {
    return result + getArea(polygon);
  }
};

struct AddEvenArea
{
  double operator()(double result, const Polygon& polygon) const
  {
    return polygon.points.size() % 2 == 0 ? result + getArea(polygon) : result;
  }
};

struct AddOddArea
{
  double operator()(double result, const Polygon& polygon) const
  {
    return polygon.points.size() % 2 != 0 ? result + getArea(polygon) : result;
  }
};

struct AddAreaWithVertexCount
{
  std::size_t vertexCount;

  double operator()(double result, const Polygon& polygon) const
  {
    return polygon.points.size() == vertexCount ? result + getArea(polygon) : result;
  }
};

struct AreaLess
{
  bool operator()(const Polygon& lhs, const Polygon& rhs) const
  {
    return getArea(lhs) < getArea(rhs);
  }
};

struct VertexCountLess
{
  bool operator()(const Polygon& lhs, const Polygon& rhs) const
  {
    return lhs.points.size() < rhs.points.size();
  }
};

struct HasLessArea
{
  double area;

  bool operator()(const Polygon& polygon) const
  {
    return getArea(polygon) < area;
  }
};

struct AddToSequence
{
  Polygon sample;

  SequenceState operator()(SequenceState state, const Polygon& polygon) const
  {
    state.current = polygon == sample ? state.current + 1 : 0;
    state.maximum = std::max(state.maximum, state.current);
    return state;
  }
};

std::string makeDoubleAnswer(double value)
{
  std::ostringstream output;
  output << std::fixed << std::setprecision(1) << value;
  return output.str();
}

std::string makeSizeAnswer(std::size_t value)
{
  return std::to_string(value);
}

bool startsWith(const std::string& line, const std::string& prefix)
{
  return line.find(prefix) == 0;
}

std::string commandParameter(const std::string& line, const std::string& prefix)
{
  return line.substr(prefix.size());
}

std::string makeAreaAnswer(const std::vector< Polygon >& polygons, const std::string& parameter)
{
  if (parameter == "EVEN")
  {
    return makeDoubleAnswer(std::accumulate(polygons.cbegin(), polygons.cend(), 0.0, AddEvenArea()));
  }

  if (parameter == "ODD")
  {
    return makeDoubleAnswer(std::accumulate(polygons.cbegin(), polygons.cend(), 0.0, AddOddArea()));
  }

  if (parameter == "MEAN")
  {
    if (polygons.empty())
    {
      return INVALID_COMMAND;
    }
    return makeDoubleAnswer(std::accumulate(polygons.cbegin(), polygons.cend(), 0.0, AddArea()) / polygons.size());
  }

  std::size_t vertexCount = 0;
  if (!readVertexCount(parameter, vertexCount))
  {
    return INVALID_COMMAND;
  }

  return makeDoubleAnswer(std::accumulate(
    polygons.cbegin(),
    polygons.cend(),
    0.0,
    AddAreaWithVertexCount{ vertexCount }
  ));
}

std::string makeMaxAnswer(const std::vector< Polygon >& polygons, const std::string& parameter)
{
  if (polygons.empty())
  {
    return INVALID_COMMAND;
  }

  if (parameter == "AREA")
  {
    return makeDoubleAnswer(getArea(*std::max_element(polygons.cbegin(), polygons.cend(), AreaLess())));
  }

  if (parameter == "VERTEXES")
  {
    return makeSizeAnswer(std::max_element(polygons.cbegin(), polygons.cend(), VertexCountLess())->points.size());
  }

  return INVALID_COMMAND;
}

std::string makeMinAnswer(const std::vector< Polygon >& polygons, const std::string& parameter)
{
  if (polygons.empty())
  {
    return INVALID_COMMAND;
  }

  if (parameter == "AREA")
  {
    return makeDoubleAnswer(getArea(*std::min_element(polygons.cbegin(), polygons.cend(), AreaLess())));
  }

  if (parameter == "VERTEXES")
  {
    return makeSizeAnswer(std::min_element(polygons.cbegin(), polygons.cend(), VertexCountLess())->points.size());
  }

  return INVALID_COMMAND;
}

std::string makeCountAnswer(const std::vector< Polygon >& polygons, const std::string& parameter)
{
  if (parameter == "EVEN")
  {
    return makeSizeAnswer(static_cast< std::size_t >(std::count_if(
      polygons.cbegin(),
      polygons.cend(),
      std::bind(
        std::equal_to< std::size_t >(),
        std::bind(std::modulus< std::size_t >(), std::bind(getVertexCount, std::placeholders::_1), 2),
        0
      )
    )));
  }

  if (parameter == "ODD")
  {
    return makeSizeAnswer(static_cast< std::size_t >(std::count_if(
      polygons.cbegin(),
      polygons.cend(),
      std::bind(
        std::equal_to< std::size_t >(),
        std::bind(std::modulus< std::size_t >(), std::bind(getVertexCount, std::placeholders::_1), 2),
        1
      )
    )));
  }

  std::size_t vertexCount = 0;
  if (!readVertexCount(parameter, vertexCount))
  {
    return INVALID_COMMAND;
  }

  return makeSizeAnswer(static_cast< std::size_t >(std::count_if(
    polygons.cbegin(),
    polygons.cend(),
    std::bind(
      std::equal_to< std::size_t >(),
      std::bind(getVertexCount, std::placeholders::_1),
      vertexCount
    )
  )));
}

std::string makeLessAreaAnswer(const std::vector< Polygon >& polygons, const std::string& parameter)
{
  const ParsedPolygon parsed = parsePolygon(parameter);
  if (!parsed.correct)
  {
    return INVALID_COMMAND;
  }

  return makeSizeAnswer(static_cast< std::size_t >(std::count_if(
    polygons.cbegin(),
    polygons.cend(),
    HasLessArea{ getArea(parsed.polygon) }
  )));
}

std::string makeMaxSeqAnswer(const std::vector< Polygon >& polygons, const std::string& parameter)
{
  const ParsedPolygon parsed = parsePolygon(parameter);
  if (!parsed.correct)
  {
    return INVALID_COMMAND;
  }

  const SequenceState answer = std::accumulate(
    polygons.cbegin(),
    polygons.cend(),
    SequenceState{ 0, 0 },
    AddToSequence{ parsed.polygon }
  );

  return makeSizeAnswer(answer.maximum);
}

std::string makeAnswer(const std::vector< Polygon >& polygons, const std::string& line)
{
  if (startsWith(line, "AREA "))
  {
    return makeAreaAnswer(polygons, commandParameter(line, "AREA "));
  }
  if (startsWith(line, "MAX "))
  {
    return makeMaxAnswer(polygons, commandParameter(line, "MAX "));
  }
  if (startsWith(line, "MIN "))
  {
    return makeMinAnswer(polygons, commandParameter(line, "MIN "));
  }
  if (startsWith(line, "COUNT "))
  {
    return makeCountAnswer(polygons, commandParameter(line, "COUNT "));
  }
  if (startsWith(line, "LESSAREA "))
  {
    return makeLessAreaAnswer(polygons, commandParameter(line, "LESSAREA "));
  }
  if (startsWith(line, "MAXSEQ "))
  {
    return makeMaxSeqAnswer(polygons, commandParameter(line, "MAXSEQ "));
  }

  return INVALID_COMMAND;
}

void printAnswers(const std::vector< Polygon >& polygons, std::istream& input, std::ostream& output)
{
  std::vector< Line > commands((std::istream_iterator< Line >(input)), std::istream_iterator< Line >());

  std::vector< std::string > answers;
  answers.reserve(commands.size());
  std::transform(
    commands.cbegin(),
    commands.cend(),
    std::back_inserter(answers),
    std::bind(makeAnswer, std::cref(polygons), std::bind(&Line::text, std::placeholders::_1))
  );

  std::copy(answers.cbegin(), answers.cend(), std::ostream_iterator< std::string >(output, "\n"));
}

int main(int argc, char* argv[])
{
  if (argc != 2)
  {
    std::cerr << "Error: filename is not specified\n";
    return 1;
  }

  std::ifstream input(argv[1]);
  if (!input)
  {
    std::cerr << "Error: cannot open file\n";
    return 1;
  }

  const std::vector< Polygon > polygons = readPolygons(input);
  printAnswers(polygons, std::cin, std::cout);

  return 0;
}
