#include <core/bio.h>

namespace arcaie
{

void P_write_map(byte_buf &buf, const binary_map &map);
void P_write_array(byte_buf &buf, const binary_array &arr);

void P_write_primitive(byte_buf &buf, const binary_value &v)
{
    buf.write<byte>((byte)v.type);

    switch (v.type)
    {
    case P_bincvt::BYTE:
        buf.write<byte>(v.cast<byte>());
        break;
    case P_bincvt::SHORT:
        buf.write<short>(v.cast<short>());
        break;
    case P_bincvt::INT:
        buf.write<int>(v.cast<int>());
        break;
    case P_bincvt::LONG:
        buf.write<long>(v.cast<long>());
        break;
    case P_bincvt::FLOAT:
        buf.write<float>(v.cast<float>());
        break;
    case P_bincvt::DOUBLE:
        buf.write<double>(v.cast<double>());
        break;
    case P_bincvt::STRING_C:
        buf.write_string(v.cast<std::string>());
        break;
    case P_bincvt::BOOL:
        buf.write<bool>(v.cast<bool>());
        break;
    case P_bincvt::MAP:
        P_write_map(buf, v.cast<binary_map>());
        break;
    case P_bincvt::ARRAY:
        P_write_array(buf, v.cast<binary_array>());
        break;
    case P_bincvt::BUF:
        buf.write_byte_buf(v.cast<byte_buf>());
        break;
    default:
        break;
    }
}

void P_write_map(byte_buf &buf, const binary_map &map)
{
    for (auto &kv : map.data)
    {
        std::string str = kv.first;
        binary_value v = kv.second;

        P_write_primitive(buf, v);
        buf.write_string(str);
    }

    buf.write<byte>((byte)P_bincvt::MAP_ENDV);
}

void P_write_array(byte_buf &buf, const binary_array &arr)
{
    buf.write<size_t>(arr.size());
    for (auto &bv : arr.data)
        P_write_primitive(buf, bv);
}

binary_map P_read_map(byte_buf &buf);
binary_array P_read_array(byte_buf &buf);

binary_value P_read_primitive(byte_buf &buf)
{
    P_bincvt id = (P_bincvt)buf.read<byte>();

    if (id == P_bincvt::MAP_ENDV)
        return {P_bincvt::MAP_ENDV, std::any(0)};

    switch (id)
    {
    case P_bincvt::BYTE:
        return binary_value::make(buf.read<byte>());
    case P_bincvt::SHORT:
        return binary_value::make(buf.read<short>());
    case P_bincvt::INT:
        return binary_value::make(buf.read<int>());
    case P_bincvt::LONG:
        return binary_value::make(buf.read<long>());
    case P_bincvt::FLOAT:
        return binary_value::make(buf.read<float>());
    case P_bincvt::DOUBLE:
        return binary_value::make(buf.read<double>());
    case P_bincvt::STRING_C:
        return binary_value::make(buf.read_string());
    case P_bincvt::BOOL:
        return binary_value::make(buf.read<bool>());
    case P_bincvt::MAP:
        return binary_value::make(P_read_map(buf));
    case P_bincvt::ARRAY:
        return binary_value::make(P_read_array(buf));
    case P_bincvt::BUF:
        return binary_value::make(buf.read_byte_buf());
    }

    arcthrow(ARC_FATAL, "unknown binary id.");
}

binary_map P_read_map(byte_buf &buf)
{
    binary_map map;
    while (true)
    {
        binary_value bv = P_read_primitive(buf);

        if (bv.type == P_bincvt::MAP_ENDV)
            break;

        std::string str = buf.read_string();
        map.data[str] = bv;
    }
    return map;
}

binary_array P_read_array(byte_buf &buf)
{
    size_t size = buf.read<size_t>();
    binary_array arr;
    while (size-- > 0)
        arr.data.push_back(P_read_primitive(buf));
    return arr;
}

binary_map bio_read_buf(byte_buf &v)
{
    return P_read_map(v);
}

byte_buf bio_write_buf(const binary_map &map)
{
    byte_buf buf;
    P_write_map(buf, map);
    return buf;
}

binary_map bio_read(const path_handle &path)
{
    byte_buf buf = byte_buf(io_read_bytes(path, io_compression_level::DCMP_READ));
    return bio_read_buf(buf);
}

void bio_write(const binary_map &map, const path_handle &path)
{
    io_write_bytes(path, bio_write_buf(map).to_vector(), io_compression_level::OPTIMAL);
}

class P_binparser
{
  private:
    std::string input;
    size_t pos;

    void P_skipspace()
    {
        while (pos < input.size() && std::isspace(input[pos]))
            pos++;
    }

    char P_cur_ch()
    {
        return (pos < input.size()) ? input[pos] : '\0';
    }

    char P_nxt()
    {
        pos++;
        return P_cur_ch();
    }

    binary_value P_parse_value();
    binary_value P_parse_bool();
    binary_value P_parse_num();
    std::string P_parse_key();
    binary_value P_parse_str();
    binary_value P_parse_arr();
    binary_value P_parse_map();

  public:
    P_binparser(const std::string &str) : input(str), pos(0)
    {
        P_skipspace();
        while (P_cur_ch() != '{')
            P_nxt();
    }

    binary_value P_parse()
    {
        P_skipspace();
        return P_parse_value();
    }
};

binary_value P_binparser::P_parse_value()
{
    P_skipspace();
    char c = P_cur_ch();

    if (c == 'n')
        arcthrow(ARC_FATAL, "cannot use a null value");
    if (c == 't' || c == 'f')
        return P_parse_bool();
    if (c == '"')
        return P_parse_str();
    if (c == '[')
        return P_parse_arr();
    if (c == '{')
        return P_parse_map();
    if (c == '-' || (c >= '0' && c <= '9'))
        return P_parse_num();

    arcthrow(ARC_FATAL, "unexpected character at position " + pos);
}

binary_value P_binparser::P_parse_bool()
{
    if (input.substr(pos, 4) == "true")
    {
        pos += 4;
        return binary_value::make(true);
    }
    if (input.substr(pos, 5) == "false")
    {
        pos += 5;
        return binary_value::make(false);
    }
    arcthrow(ARC_FATAL, "expected boolean at position " + pos);
}

binary_value P_binparser::P_parse_num()
{
    size_t start = pos;

    if (P_cur_ch() == '-')
        P_nxt();

    while (pos < input.size() && std::isdigit(input[pos]))
        P_nxt();

    if (P_cur_ch() == '.')
    {
        P_nxt();
        while (pos < input.size() && std::isdigit(input[pos]))
            P_nxt();
    }

    if (P_cur_ch() == 'e' || P_cur_ch() == 'E')
    {
        P_nxt();
        if (P_cur_ch() == '+' || P_cur_ch() == '-')
            P_nxt();
        while (pos < input.size() && std::isdigit(input[pos]))
            P_nxt();
    }

    std::string numStr = input.substr(start, pos - start);
    double value = std::stod(numStr);
    return binary_value::make(value);
}

std::string P_binparser::P_parse_key()
{
    std::string result;
    while (pos < input.size() && P_cur_ch() != '=')
    {
        char ch = P_cur_ch();
        if (!std::isspace(ch))
            result += ch;
        P_nxt();
    }
    return result;
}

binary_value P_binparser::P_parse_str()
{
    if (P_cur_ch() != '"')
        arcthrow(ARC_FATAL, "expected '\"' at position " + pos);
    P_nxt();

    std::string result;
    while (pos < input.size() && P_cur_ch() != '"')
    {
        char c = P_cur_ch();

        if (c == '\\')
        {
            P_nxt();
            c = P_cur_ch();
            switch (c)
            {
            case '"':
                result += '"';
                break;
            case '\\':
                result += '\\';
                break;
            case '/':
                result += '/';
                break;
            case 'b':
                result += '\b';
                break;
            case 'f':
                result += '\f';
                break;
            case 'n':
                result += '\n';
                break;
            case 'r':
                result += '\r';
                break;
            case 't':
                result += '\t';
                break;
            case 'u':
                P_nxt();
                for (int i = 0; i < 4 && pos < input.size(); i++)
                {
                    P_nxt();
                }
                result += '?';
                break;
            default:
                result += c;
                break;
            }
        }
        else
            result += c;
        P_nxt();
    }

    if (P_cur_ch() != '"')
        arcthrow(ARC_FATAL, "unterminated std::stringat position " + pos);
    P_nxt();

    return binary_value::make(result);
}

binary_value P_binparser::P_parse_arr()
{
    if (P_cur_ch() != '[')
        arcthrow(ARC_FATAL, "expected '[' at position " + pos);
    P_nxt();

    binary_array result;
    P_skipspace();

    if (P_cur_ch() == ']')
    {
        P_nxt();
        return binary_value::make(result);
    }

    while (pos < input.size())
    {
        result.data.push_back(P_parse_value());

        P_skipspace();
        if (P_cur_ch() == ']')
            break;
        if (P_cur_ch() != ',')
            arcthrow(ARC_FATAL, "expected ',' or ']' in array at position " + pos);
        P_nxt();
        P_skipspace();
    }

    if (P_cur_ch() != ']')
        arcthrow(ARC_FATAL, "unterminated array at position " + pos);
    P_nxt();

    return binary_value::make(result);
}

binary_value P_binparser::P_parse_map()
{
    if (P_cur_ch() != '{')
        arcthrow(ARC_FATAL, "expected '{' at position " + pos);
    P_nxt();

    binary_map result;
    P_skipspace();

    if (P_cur_ch() == '}')
    {
        P_nxt();
        return binary_value::make(result);
    }

    while (pos < input.size())
    {
        P_skipspace();
        std::string key = P_parse_key();
        P_skipspace();
        if (P_cur_ch() != '=')
            arcthrow(ARC_FATAL, "expected '=' after object key at position " + pos);
        P_nxt();

        binary_value value = P_parse_value();
        result.data[key] = value;

        P_skipspace();

        if (P_cur_ch() == '}')
            break;
        if (P_cur_ch() != ',')
            arcthrow(ARC_FATAL, "expected ',' or '}' in object at position " + pos);

        P_nxt();
    }

    if (P_cur_ch() != '}')
        arcthrow(ARC_FATAL, "unterminated object at position " + pos);
    P_nxt();

    return binary_value::make(result);
}

binary_map bio_read_langd(const path_handle &path)
{
    binary_value result = P_binparser(io_read_str(path)).P_parse();
    if (result.type != P_bincvt::MAP)
        arcthrow(ARC_FATAL, "binary root is not an object");
    return result.cast<binary_map>();
}

} // namespace arcaie
