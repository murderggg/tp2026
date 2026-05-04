#include <iostream>
#include <vector>
#include <string>
#include <iterator>
#include <algorithm>
#include <cctype>
#include <sstream>

struct DataStruct
{
    unsigned long long key1;
    char key2;
    std::string key3;
};

struct DelimiterIO
{
    char exp;
};

std::istream& operator>>(std::istream& in, DelimiterIO&& dest)
{
    std::istream::sentry sentry(in, true);
    if (!sentry)
    {
        return in;
    }
    char c = '0';
    in.get(c);
    if (in && c != dest.exp)
    {
        in.setstate(std::ios::failbit);
    }
    return in;
}

struct CharLiteralIO
{
    char& ref;
};

std::istream& operator>>(std::istream& in, CharLiteralIO&& dest)
{
    std::istream::sentry sentry(in, true);
    if (!sentry)
    {
        return in;
    }
    char c;
    in >> DelimiterIO{ '\'' } >> c >> DelimiterIO{ '\'' };
    if (in)
    {
        dest.ref = c;
    }
    return in;
}

struct StringIO
{
    std::string& ref;
};

std::istream& operator>>(std::istream& in, StringIO&& dest)
{
    std::istream::sentry sentry(in, true);
    if (!sentry)
    {
        return in;
    }

    char c;
    in.get(c);
    if (c != '"')
    {
        in.setstate(std::ios::failbit);
        return in;
    }

    dest.ref.clear();
    while (in.get(c) && c != '"')
    {
        dest.ref.push_back(c);
    }

    if (!in)
    {
        in.setstate(std::ios::failbit);
        return in;
    }
    return in;
}

struct IdentifierIO
{
    std::string& ref;
};

std::istream& operator>>(std::istream& in, IdentifierIO&& dest)
{
    std::istream::sentry sentry(in, true);
    if (!sentry)
    {
        return in;
    }

    dest.ref.clear();
    char c = '0';
    while (in.get(c) && (std::isalpha(static_cast<unsigned char>(c)) ||
        std::isdigit(static_cast<unsigned char>(c))))
    {
        dest.ref.push_back(c);
    }

    if (dest.ref.empty())
    {
        in.setstate(std::ios::failbit);
    }
    return in;
}

struct UnsignedOctalIO
{
    unsigned long long& ref;
};

std::istream& operator>>(std::istream& in, UnsignedOctalIO&& dest)
{
    std::istream::sentry sentry(in, true);
    if (!sentry)
    {
        return in;
    }

    unsigned long long value = 0;
    char c;
    in >> std::ws;
    c = in.peek();
    if (c != '0')
    {
        in.setstate(std::ios::failbit);
        return in;
    }
    in.get();

    while (in.get(c) && c >= '0' && c <= '7')
    {
        value = value * 8 + (c - '0');
    }
    if (!in && !in.eof())
    {
        in.setstate(std::ios::failbit);
        return in;
    }
    in.unget();

    if (in)
    {
        dest.ref = value;
    }
    return in;
}

std::istream& operator>>(std::istream& in, DataStruct& dest)
{
    std::istream::sentry sentry(in);
    if (!sentry)
    {
        return in;
    }
    while (std::isspace(in.peek()))
    {
        in.get();
    }

    if (in.peek() != '(')
    {
        in.setstate(std::ios::failbit);
        return in;
    }

    DataStruct temp;
    bool key1_set = false;
    bool key2_set = false;
    bool key3_set = false;

    in >> DelimiterIO{ '(' };
    if (!in)
    {
        return in;
    }

    while (in && in.peek() != ')')
    {
        if (in.peek() == ':')
        {
            in.get();
        }

        std::string field_name;
        in >> IdentifierIO{ field_name };
        if (!in)
        {
            in.setstate(std::ios::failbit);
            return in;
        }

        if (in.peek() == ' ')
        {
            in.get();
        }

        if (field_name == "key1" && !key1_set)
        {
            in >> UnsignedOctalIO{ temp.key1 };
            key1_set = true;
        }
        else if (field_name == "key2" && !key2_set)
        {
            in >> CharLiteralIO{ temp.key2 };
            key2_set = true;
        }
        else if (field_name == "key3" && !key3_set)
        {
            in >> StringIO{ temp.key3 };
            key3_set = true;
        }
        else
        {
            in.setstate(std::ios::failbit);
            return in;
        }

        if (!in)
        {
            return in;
        }

        if (in.peek() == ':')
        {
            in.get();
        }
        else if (in.peek() != ')')
        {
            in.setstate(std::ios::failbit);
            return in;
        }
    }

    in >> DelimiterIO{ ')' };

    if (in && key1_set && key2_set && key3_set)
    {
        dest = temp;
    }
    else
    {
        in.setstate(std::ios::failbit);
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

    out << "(:key1 0" << std::oct << src.key1 << std::dec
        << ":key2 '" << src.key2
        << "':key3 \"" << src.key3 << "\":)";

    return out;
}

bool compareDataStruct(const DataStruct& a, const DataStruct& b)
{
    if (a.key1 != b.key1)
    {
        return a.key1 < b.key1;
    }
    if (a.key2 != b.key2)
    {
        return a.key2 < b.key2;
    }
    return a.key3.length() < b.key3.length();
}

int main()
{
    std::vector<DataStruct> data;
    std::string line;

    while (std::getline(std::cin, line))
    {
        bool onlyWhitespace = true;
        for (char c : line)
        {
            if (!std::isspace(static_cast<unsigned char>(c)))
            {
                onlyWhitespace = false;
                break;
            }
        }
        if (onlyWhitespace)
        {
            continue;
        }

        std::istringstream lineStream(line);
        std::copy(
            std::istream_iterator<DataStruct>(lineStream),
            std::istream_iterator<DataStruct>(),
            std::back_inserter(data)
        );
    }

    std::sort(data.begin(), data.end(), compareDataStruct);

    std::copy(
        data.begin(),
        data.end(),
        std::ostream_iterator<DataStruct>(std::cout, "\n")
    );

    return 0;
}
