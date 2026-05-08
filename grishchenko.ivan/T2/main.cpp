#include <iostream>
#include <vector>
#include <string>
#include <complex>
#include <algorithm>
#include <iterator>
#include <iomanip>
#include <limits>

namespace nspace
{
  struct DataStruct
  {
    unsigned long long key1;
    std::complex< double > key2;
    std::string key3;
  };

  struct DelimiterIO
  {
    char exp;
  };

  struct ULLHexIO
  {
    unsigned long long& ref;
  };

  struct ComplexIO
  {
    std::complex< double >& ref;
  };

  struct StringIO
  {
    std::string& ref;
  };

  class iofmtguard
  {
  public:
    iofmtguard(std::basic_ios< char >& s) :
      s_(s),
      fill_(s.fill()),
      precision_(s.precision()),
      fmt_(s.flags())
    {}
    ~iofmtguard()
    {
      s_.fill(fill_);
      s_.precision(precision_);
      s_.flags(fmt_);
    }
  private:
    std::basic_ios< char >& s_;
    char fill_;
    std::streamsize precision_;
    std::basic_ios< char >::fmtflags fmt_;
  };

  std::istream& operator>>(std::istream& in, DelimiterIO&& dest)
  {
    std::istream::sentry sentry(in);
    if (!sentry)
    {
      return in;
    }
    char c = '0';
    in >> c;
    if (in && (std::tolower(c) != std::tolower(dest.exp)))
    {
      in.setstate(std::ios::failbit);
    }
    return in;
  }

  std::istream& operator>>(std::istream& in, ULLHexIO&& dest)
  {
    std::istream::sentry sentry(in);
    if (!sentry)
    {
      return in;
    }
    return in >> std::hex >> dest.ref;
  }

  std::istream& operator>>(std::istream& in, ComplexIO&& dest)
  {
    std::istream::sentry sentry(in);
    if (!sentry)
    {
      return in;
    }
    double re = 0.0;
    double im = 0.0;
    in >> DelimiterIO{ '#' } >> DelimiterIO{ 'c' } >> DelimiterIO{ '(' };
    in >> re >> im >> DelimiterIO{ ')' };
    if (in)
    {
      dest.ref = { re, im };
    }
    return in;
  }

  std::istream& operator>>(std::istream& in, StringIO&& dest)
  {
    std::istream::sentry sentry(in);
    if (!sentry)
    {
      return in;
    }
    return std::getline(in >> DelimiterIO{ '"' }, dest.ref, '"');
  }

  std::istream& operator>>(std::istream& in, DataStruct& dest)
  {
    std::istream::sentry sentry(in);
    if (!sentry)
    {
      return in;
    }
    DataStruct input;
    using sep = DelimiterIO;
    in >> sep{ '(' } >> sep{ ':' };
    for (size_t i = 0; i < 3; ++i)
    {
      std::string key = "";
      std::getline(in, key, ' ');
      if (key == "key1")
      {
        in >> ULLHexIO{ input.key1 } >> sep{ ':' };
      }
      else if (key == "key2")
      {
        in >> ComplexIO{ input.key2 } >> sep{ ':' };
      }
      else if (key == "key3")
      {
        in >> StringIO{ input.key3 } >> sep{ ':' };
      }
      else
      {
        in.setstate(std::ios::failbit);
      }
    }
    in >> sep{ ')' };
    if (in)
    {
      dest = input;
    }
    return in;
  }

  std::ostream& operator<<(std::ostream& out, const DataStruct& src)
  {
    std::ostream::sentry sentry(out);
    if (!sentry)
    {
      return out;
    }
    iofmtguard fmtguard(out);
    out << "(:key1 0x" << std::uppercase << std::hex << src.key1;
    out << ":key2 #c(" << std::fixed << std::setprecision(1) << src.key2.real();
    out << " " << src.key2.imag() << "):key3 \"" << src.key3 << "\":)";
    return out;
  }

  bool compareData(const DataStruct& a, const DataStruct& b)
  {
    if (a.key1 != b.key1)
    {
      return a.key1 < b.key1;
    }
    if (std::abs(a.key2) != std::abs(b.key2))
    {
      return std::abs(a.key2) < std::abs(b.key2);
    }
    return a.key3.length() < b.key3.length();
  }
}

int main()
{
  using namespace nspace;
  std::vector< DataStruct > data;

  while (!std::cin.eof())
  {
    std::copy(
      std::istream_iterator< DataStruct >(std::cin),
      std::istream_iterator< DataStruct >(),
      std::back_inserter(data)
    );
    if (std::cin.fail() && !std::cin.eof())
    {
      std::cin.clear();
      std::cin.ignore(std::numeric_limits< std::streamsize >::max(), '\n');
    }
  }

  std::sort(data.begin(), data.end(), compareData);

  std::copy(
    data.begin(),
    data.end(),
    std::ostream_iterator< DataStruct >(std::cout, "\n")
  );

  return 0;
}
