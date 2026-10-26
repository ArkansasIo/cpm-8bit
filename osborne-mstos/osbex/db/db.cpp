// osbex/db/db.cpp — implementation of the built-in OSBEX database engine
#include "db.hpp"
#include <algorithm>
#include <cctype>
#include <charconv>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace osbex::db
{
// thin wrappers over <cctype> � IntelliSense misresolves std::tolower et al
// through <cctype> on some MSVC STL layouts, which cascades into bogus
// 'type name is not allowed' errors; tiny local helpers keep it clean.
namespace
{
    inline int lower_c(int c) { return c >= 'A' && c <= 'Z' ? c + 32 : c; }
    inline int upper_c(int c) { return c >= 'a' && c <= 'z' ? c - 32 : c; }
    inline int space_c(int c) { return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '\v'; }
}
    const char *type_name(ColType t)
    {
        switch (t)
        {
        case ColType::Int:
            return "INT";
        case ColType::Double:
            return "DOUBLE";
        case ColType::Bool:
            return "BOOL";
        default:
            return "STRING";
        }
    }

    static std::string lower(std::string s)
    {
        for (char &c : s)
            c = char(lower_c(static_cast<unsigned char>(c)));
        return s;
    }
    static std::string upper(std::string s)
    {
        for (char &c : s)
            c = char(upper_c(static_cast<unsigned char>(c)));
        return s;
    }
    static std::vector<std::string> split_ws(const std::string &s)
    {
        std::vector<std::string> out;
        std::istringstream ss(s);
        std::string w;
        while (ss >> w)
            out.push_back(w);
        return out;
    }
    static std::string val_to_str(const Value &v)
    {
        if (std::holds_alternative<int64_t>(v))
            return std::to_string(std::get<int64_t>(v));
        if (std::holds_alternative<double>(v))
        {
            std::ostringstream ss;
            ss << std::get<double>(v);
            return ss.str();
        }
        if (std::holds_alternative<bool>(v))
            return std::get<bool>(v) ? "true" : "false";
        if (std::holds_alternative<std::string>(v))
            return std::get<std::string>(v);
        return {};
    }
    static Value str_to_val(ColType t, const std::string &raw)
    {
        switch (t)
        {
        case ColType::Int:
        {
            int64_t n = 0;
            std::from_chars(raw.data(), raw.data() + raw.size(), n);
            return n;
        }
        case ColType::Double:
        {
            try
            {
                return std::stod(raw);
            }
            catch (...)
            {
                return double(0);
            }
        }
        case ColType::Bool:
        {
            const std::string l = lower(raw);
            return l == "true" || l == "1" || l == "yes";
        }
        default:
            return raw;
        }
    }

    // ---------- WHERE evaluation ----------
    static bool cmp_values(const Value &a, const std::string &op, const Value &b)
    {
        // numeric compare when both numeric, else string compare
        auto num = [](const Value &v) -> std::optional<double>
        {
            if (auto p = std::get_if<int64_t>(&v))
                return double(*p);
            if (auto p = std::get_if<double>(&v))
                return *p;
            if (auto p = std::get_if<bool>(&v))
                return *p ? 1.0 : 0.0;
            return std::nullopt;
        };
        const auto na = num(a), nb = num(b);
        if (na && nb)
        {
            const double x = *na, y = *nb;
            if (op == "=")
                return x == y;
            if (op == "!=" || op == "<>")
                return x != y;
            if (op == "<")
                return x < y;
            if (op == ">")
                return x > y;
            if (op == "<=")
                return x <= y;
            if (op == ">=")
                return x >= y;
            return false;
        }
        const std::string x = val_to_str(a), y = val_to_str(b);
        if (op == "=")
            return x == y;
        if (op == "!=" || op == "<>")
            return x != y;
        if (op == "<")
            return x < y;
        if (op == ">")
            return x > y;
        if (op == "<=")
            return x <= y;
        if (op == ">=")
            return x >= y;
        return false;
    }

    // parse "col op value" (value may be quoted)
    static bool parse_where(const std::string &where, const std::vector<Column> &cols,
                            int *col_idx, std::string *op, std::string *raw_val)
    {
        const std::vector<std::string> toks = split_ws(where);
        if (toks.size() < 3)
            return false;
        for (size_t i = 0; i < cols.size(); ++i)
        {
            if (lower(cols[i].name) == lower(toks[0]))
            {
                *col_idx = int(i);
                break;
            }
        }
        if (*col_idx < 0)
            return false;
        *op = toks[1];
        // value = rest, trimmed of quotes
        std::string v = where;
        // find position after second token
        size_t pos = 0;
        int tok = 0;
        while (tok < 2 && pos < v.size())
        {
            while (pos < v.size() && space_c(static_cast<unsigned char>(v[pos])))
                ++pos;
            while (pos < v.size() && !space_c(static_cast<unsigned char>(v[pos])))
                ++pos;
            ++tok;
        }
        if (pos < v.size())
        {
            v = v.substr(pos);
            while (!v.empty() && space_c(static_cast<unsigned char>(v.back())))
                v.pop_back();
            if (v.size() >= 2 && (v.front() == '\'' || v.front() == '"') && v.back() == v.front())
                v = v.substr(1, v.size() - 2);
            *raw_val = v;
            return true;
        }
        return false;
    }

    // ---------- Table ----------
    Table::Table(std::string name, std::vector<Column> cols)
        : _name(std::move(name)), _cols(std::move(cols)) {}

    std::optional<std::string> Table::insert(const std::vector<Value> &values)
    {
        if (values.size() != _cols.size())
            return "expected " + std::to_string(_cols.size()) + " values, got " + std::to_string(values.size());
        Row r;
        r.cells = values;
        // enforce primary key uniqueness
        for (size_t ci = 0; ci < _cols.size(); ++ci)
        {
            if (_cols[ci].primary_key)
            {
                for (const Row &e : _rows)
                    if (cmp_values(e.cells[ci], "=", r.cells[ci]))
                        return "duplicate primary key '" + _cols[ci].name + "'";
            }
        }
        _rows.push_back(std::move(r));
        return std::nullopt;
    }

    std::optional<std::string> Table::insert_from_strings(const std::vector<std::string> &raw)
    {
        if (raw.size() != _cols.size())
            return "expected " + std::to_string(_cols.size()) + " values, got " + std::to_string(raw.size());
        std::vector<Value> vals;
        vals.reserve(raw.size());
        for (size_t i = 0; i < raw.size(); ++i)
            vals.push_back(str_to_val(_cols[i].type, raw[i]));
        return insert(vals);
    }

    std::vector<const Row *> Table::select(const std::string &where) const
    {
        std::vector<const Row *> out;
        for (const Row &r : _rows)
            out.push_back(&r);
        if (where.empty())
            return out;
        int ci = -1;
        std::string op, rv;
        if (!parse_where(where, _cols, &ci, &op, &rv))
            return out; // bad where -> empty filter, caller sees all; keep simple
        std::vector<const Row *> filtered;
        const Value needle = str_to_val(_cols[ci].type, rv);
        for (const Row *r : out)
            if (cmp_values(r->cells[ci], op, needle))
                filtered.push_back(r);
        return filtered;
    }

    bool Table::remove_where(const std::string &where)
    {
        int ci = -1;
        std::string op, rv;
        if (!parse_where(where, _cols, &ci, &op, &rv))
            return false;
        const Value needle = str_to_val(_cols[ci].type, rv);
        const size_t before = _rows.size();
        _rows.erase(std::remove_if(_rows.begin(), _rows.end(), [&](const Row &r)
                                   { return cmp_values(r.cells[ci], op, needle); }),
                    _rows.end());
        return _rows.size() != before;
    }

    bool Table::update_where(const std::string &where, const std::string &col, const Value &v)
    {
        int ci = -1;
        std::string op, rv;
        if (!parse_where(where, _cols, &ci, &op, &rv))
            return false;
        int target = -1;
        for (size_t i = 0; i < _cols.size(); ++i)
            if (lower(_cols[i].name) == lower(col))
            {
                target = int(i);
                break;
            }
        if (target < 0)
            return false;
        const Value needle = str_to_val(_cols[ci].type, rv);
        bool changed = false;
        for (Row &r : _rows)
            if (cmp_values(r.cells[ci], op, needle))
            {
                r.cells[target] = v;
                changed = true;
            }
        return changed;
    }

    std::string Table::serialize() const
    {
        std::ostringstream ss;
        ss << "TABLE " << _name << "\n";
        for (const Column &c : _cols)
            ss << "COL " << c.name << " " << type_name(c.type)
               << (c.primary_key ? " PK" : "") << "\n";
        for (const Row &r : _rows)
        {
            ss << "ROW";
            for (const Value &v : r.cells)
                ss << " " << val_to_str(v);
            ss << "\n";
        }
        ss << "END\n";
        return ss.str();
    }

    std::unique_ptr<Table> Table::deserialize(const std::string &blob, std::string &err)
    {
        std::istringstream ss(blob);
        std::string line;
        std::unique_ptr<Table> t;
        while (std::getline(ss, line))
        {
            if (line.empty())
                continue;
            const std::vector<std::string> toks = split_ws(line);
            if (toks[0] == "TABLE" && toks.size() >= 2)
                t = std::make_unique<Table>(toks[1], std::vector<Column>{});
            else if (toks[0] == "COL" && t && toks.size() >= 3)
            {
                Column c;
                c.name = toks[1];
                const std::string ty = upper(toks[2]);
                c.type = ty == "INT" ? ColType::Int : ty == "DOUBLE" ? ColType::Double
                                                  : ty == "BOOL"     ? ColType::Bool
                                                                     : ColType::String;
                c.primary_key = toks.size() >= 4 && toks[3] == "PK";
                t->_cols.push_back(c);
            }
            else if (toks[0] == "ROW" && t)
                t->insert_from_strings(std::vector<std::string>(toks.begin() + 1, toks.end()));
            else if (toks[0] == "END")
                return t;
        }
        err = "unterminated table";
        return nullptr;
    }

    // ---------- Engine ----------
    Engine &Engine::instance()
    {
        static Engine e;
        return e;
    }

    Table *Engine::create_table(const std::string &name, const std::vector<Column> &cols, std::string &err)
    {
        if (cols.empty())
        {
            err = "no columns";
            return nullptr;
        }
        if (_tables.count(name))
        {
            err = "table already exists: " + name;
            return nullptr;
        }
        auto t = std::make_unique<Table>(name, cols);
        Table *raw = t.get();
        _tables.emplace(name, std::move(t));
        save();
        return raw;
    }

    Table *Engine::find(const std::string &name)
    {
        auto it = _tables.find(name);
        return it == _tables.end() ? nullptr : it->second.get();
    }

    bool Engine::drop_table(const std::string &name)
    {
        const bool removed = _tables.erase(name) > 0;
        if (removed)
            save();
        return removed;
    }

    std::vector<std::string> Engine::table_names() const
    {
        std::vector<std::string> out;
        for (const auto &kv : _tables)
            out.push_back(kv.first);
        return out;
    }

    bool Engine::save(const std::string &path) const
    {
        std::ofstream f(path.empty() ? _db_path : path, std::ios::binary | std::ios::trunc);
        if (!f)
            return false;
        for (const auto &kv : _tables)
            f << kv.second->serialize();
        return true;
    }

    bool Engine::load(const std::string &path)
    {
        std::ifstream f(path.empty() ? _db_path : path, std::ios::binary);
        if (!f)
            return false;
        std::ostringstream ss;
        ss << f.rdbuf();
        const std::string blob = ss.str();
        // split into TABLE ... END chunks
        size_t pos = 0;
        while (true)
        {
            const size_t start = blob.find("TABLE ", pos);
            if (start == std::string::npos)
                break;
            const size_t end = blob.find("\nEND\n", start);
            const size_t chunk_end = end == std::string::npos ? blob.size() : end + 5;
            std::string err;
            auto t = Table::deserialize(blob.substr(start, chunk_end - start), err);
            if (t)
                _tables.insert_or_assign(t->name(), std::move(t));
            pos = chunk_end;
        }
        return true;
    }

    std::vector<std::string> Engine::execute_script(const std::string &sql)
    {
        std::vector<std::string> out;
        std::istringstream ss(sql);
        std::string stmt;
        while (std::getline(ss, stmt, ';'))
        {
            // strip comments and whitespace
            const size_t dash = stmt.find("--");
            if (dash != std::string::npos)
                stmt = stmt.substr(0, dash);
            std::string work = stmt;
            while (!work.empty() && space_c(static_cast<unsigned char>(work.front())))
                work.erase(0, 1);
            while (!work.empty() && space_c(static_cast<unsigned char>(work.back())))
                work.pop_back();
            if (work.empty())
                continue;

            std::string u = upper(work);
            if (u.rfind("CREATE TABLE", 0) == 0)
            {
                // CREATE TABLE name (col TYPE [PK], col TYPE, ...)
                const size_t p1 = work.find('('), p2 = work.rfind(')');
                if (p1 == std::string::npos || p2 == std::string::npos)
                {
                    out.push_back("ERR: malformed CREATE TABLE");
                    continue;
                }
                std::string tname = work.substr(12, p1 - 12);
                while (!tname.empty() && space_c(static_cast<unsigned char>(tname.back())))
                    tname.pop_back();
                while (!tname.empty() && space_c(static_cast<unsigned char>(tname.front())))
                    tname.erase(0, 1);
                std::vector<Column> cols;
                std::istringstream cols_ss(work.substr(p1 + 1, p2 - p1 - 1));
                std::string cdef;
                while (std::getline(cols_ss, cdef, ','))
                {
                    const std::vector<std::string> ct = split_ws(cdef);
                    if (ct.size() < 2)
                        continue;
                    Column c;
                    c.name = ct[0];
                    const std::string ty = upper(ct[1]);
                    c.type = ty.rfind("INT", 0) == 0 ? ColType::Int : ty.rfind("DOUBLE", 0) == 0 || ty.rfind("FLOAT", 0) == 0 ? ColType::Double
                                                                  : ty.rfind("BOOL", 0) == 0 || ty.rfind("BOOLEAN", 0) == 0   ? ColType::Bool
                                                                                                                              : ColType::String;
                    for (size_t k = 2; k < ct.size(); ++k)
                        if (upper(ct[k]) == "PRIMARY" || upper(ct[k]) == "PK")
                            c.primary_key = true;
                    cols.push_back(c);
                }
                std::string err;
                if (!create_table(tname, cols, err))
                    out.push_back("ERR: " + err);
                else
                    out.push_back("OK: created table " + tname + " (" + std::to_string(cols.size()) + " columns)");
            }
            else if (u.rfind("INSERT INTO", 0) == 0)
            {
                // INSERT INTO name VALUES (v1, v2, ...)
                const size_t vp = u.find("VALUES");
                if (vp == std::string::npos)
                {
                    out.push_back("ERR: INSERT needs VALUES");
                    continue;
                }
                std::string tname = work.substr(11, vp - 11);
                while (!tname.empty() && space_c(static_cast<unsigned char>(tname.back())))
                    tname.pop_back();
                while (!tname.empty() && space_c(static_cast<unsigned char>(tname.front())))
                    tname.erase(0, 1);
                Table *t = find(tname);
                if (!t)
                {
                    out.push_back("ERR: no such table: " + tname);
                    continue;
                }
                const size_t p1 = work.find('(', vp), p2 = work.rfind(')');
                if (p1 == std::string::npos || p2 == std::string::npos)
                {
                    out.push_back("ERR: INSERT needs (...)");
                    continue;
                }
                std::vector<std::string> raw;
                std::istringstream vss(work.substr(p1 + 1, p2 - p1 - 1));
                std::string v;
                while (std::getline(vss, v, ','))
                {
                    while (!v.empty() && space_c(static_cast<unsigned char>(v.front())))
                        v.erase(0, 1);
                    while (!v.empty() && space_c(static_cast<unsigned char>(v.back())))
                        v.pop_back();
                    if (v.size() >= 2 && (v.front() == '\'' || v.front() == '"') && v.back() == v.front())
                        v = v.substr(1, v.size() - 2);
                    raw.push_back(v);
                }
                if (auto err = t->insert_from_strings(raw))
                    out.push_back("ERR: " + *err);
                else
                {
                    save();
                    out.push_back("OK: 1 row inserted into " + tname);
                }
            }
            else if (u.rfind("SELECT", 0) == 0)
            {
                // SELECT * FROM t [WHERE ...]
                size_t from = u.find("FROM");
                if (from == std::string::npos)
                {
                    out.push_back("ERR: SELECT needs FROM");
                    continue;
                }
                std::string rest = work.substr(from + 4);
                while (!rest.empty() && space_c(static_cast<unsigned char>(rest.front())))
                    rest.erase(0, 1);
                std::string tname = rest, where;
                const size_t wp = lower(rest).find("where");
                if (wp != std::string::npos)
                {
                    tname = rest.substr(0, wp);
                    while (!tname.empty() && space_c(static_cast<unsigned char>(tname.back())))
                        tname.pop_back();
                    where = rest.substr(wp + 5);
                    while (!where.empty() && space_c(static_cast<unsigned char>(where.front())))
                        where.erase(0, 1);
                }
                std::string err;
                std::string dump = dump_table(tname, err);
                if (!err.empty())
                {
                    out.push_back("ERR: " + err);
                    continue;
                }
                if (!where.empty())
                {
                    Table *t = find(tname);
                    const auto rows = t->select(where);
                    std::ostringstream rs;
                    rs << tname << ": " << rows.size() << " row(s)";
                    for (const Row *r : rows)
                    {
                        rs << "\n  ";
                        for (const Value &v : r->cells)
                            rs << val_to_str(v) << " | ";
                    }
                    dump = rs.str();
                }
                out.push_back(dump);
            }
            else if (u.rfind("DELETE FROM", 0) == 0)
            {
                std::string tname = work.substr(11), where;
                const size_t wp = lower(tname).find("where");
                if (wp != std::string::npos)
                {
                    where = tname.substr(wp + 5);
                    tname = tname.substr(0, wp);
                }
                while (!tname.empty() && space_c(static_cast<unsigned char>(tname.back())))
                    tname.pop_back();
                while (!tname.empty() && space_c(static_cast<unsigned char>(tname.front())))
                    tname.erase(0, 1);
                Table *t = find(tname);
                if (!t)
                {
                    out.push_back("ERR: no such table: " + tname);
                    continue;
                }
                const bool ok = t->remove_where(where);
                save();
                out.push_back(ok ? "OK: rows deleted" : "OK: nothing matched");
            }
            else if (u.rfind("DROP TABLE", 0) == 0)
            {
                std::string tname = work.substr(10);
                while (!tname.empty() && space_c(static_cast<unsigned char>(tname.front())))
                    tname.erase(0, 1);
                out.push_back(drop_table(tname) ? "OK: dropped " + tname : "ERR: no such table: " + tname);
            }
            else
            {
                out.push_back("ERR: unsupported statement: " + work.substr(0, 24));
            }
        }
        return out;
    }

    std::string Engine::dump_table(const std::string &name, std::string &err) const
    {
        const Table *t = nullptr;
        for (const auto &kv : _tables)
            if (lower(kv.first) == lower(name))
            {
                t = kv.second.get();
                break;
            }
        if (!t)
        {
            err = "no such table: " + name;
            return {};
        }
        std::ostringstream ss;
        ss << t->name() << " (" << t->row_count() << " rows)\n";
        for (const Column &c : t->columns())
            ss << "  " << c.name << " [" << type_name(c.type) << (c.primary_key ? ", PK" : "") << "]\n";
        for (const Row &r : t->rows())
        {
            ss << "  ";
            for (const Value &v : r.cells)
                ss << val_to_str(v) << " | ";
            ss << "\n";
        }
        return ss.str();
    }

    // ---------- Config ----------
    Config::Config()
    {
        Engine::instance().load(kDbFile);
        if (!Engine::instance().find(kTable))
        {
            std::string err;
            Engine::instance().create_table(kTable,
                                            {{"key", ColType::String, true},
                                             {"value", ColType::String, false}},
                                            err);
        }
    }

    Config &Config::instance()
    {
        static Config c;
        return c;
    }

    Table *config_table()
    {
        return Engine::instance().find("config");
    }

    std::string Config::get(const std::string &key) const
    {
        const Table *t = Engine::instance().find(kTable);
        if (!t)
            return {};
        const auto rows = t->select("key = " + key);
        if (rows.empty())
            return {};
        return val_to_str(rows[0]->cells[1]);
    }

    int Config::get_int(const std::string &key, int fallback) const
    {
        const std::string v = get(key);
        if (v.empty())
            return fallback;
        try
        {
            return std::stoi(v);
        }
        catch (...)
        {
            return fallback;
        }
    }

    bool Config::get_bool(const std::string &key, bool fallback) const
    {
        const std::string v = get(key);
        if (v.empty())
            return fallback;
        return v == "true" || v == "1" || v == "yes";
    }

    void Config::set(const std::string &key, const std::string &value)
    {
        Table *t = Engine::instance().find(kTable);
        if (!t)
            return;
        if (!t->update_where("key = " + key, "value", Value(value)))
        {
            // key may not exist — upsert
            if (auto err = t->insert_from_strings({key, value}))
            {
                t->remove_where("key = " + key);
                (void)t->insert_from_strings({key, value});
            }
        }
        persist();
    }

    void Config::set_int(const std::string &key, int v) { set(key, std::to_string(v)); }
    void Config::set_bool(const std::string &key, bool v) { set(key, v ? "true" : "false"); }

    void Config::remove(const std::string &key)
    {
        if (Table *t = Engine::instance().find(kTable))
        {
            t->remove_where("key = " + key);
            persist();
        }
    }

    std::vector<std::pair<std::string, std::string>> Config::all() const
    {
        std::vector<std::pair<std::string, std::string>> out;
        if (const Table *t = Engine::instance().find(kTable))
            for (const Row &r : t->rows())
                out.emplace_back(val_to_str(r.cells[0]), val_to_str(r.cells[1]));
        return out;
    }

    void Config::persist()
    {
        Engine::instance().save(kDbFile);
    }
} // namespace osbex::db
