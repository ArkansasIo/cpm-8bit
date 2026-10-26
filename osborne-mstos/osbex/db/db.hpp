// osbex/db/db.hpp — OSBEX 2.2 built-in object-oriented database engine
//
// A small, self-contained, OO database that ships inside the OS:
//   - Table  : typed columns, rows as records, OOP interface
//   - Row    : variant cell (int / double / string / bool)
//   - Engine : registry of tables, persistence to the virtual disk (C:)
//   - Config : system configuration stored in the built-in DB ("config" table)
//
// The query layer accepts a friendly SQL-ish subset so .sql bootstrap files
// (see osbex/db/bootstrap.sql, MySQL-compatible) can seed the DB.
#pragma once
#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace osbex::db
{
    // ---------- values ----------
    using Value = std::variant<std::monostate, int64_t, double, std::string, bool>;

    enum class ColType : uint8_t
    {
        Int,
        Double,
        String,
        Bool
    };

    const char *type_name(ColType t);

    struct Column
    {
        std::string name;
        ColType type = ColType::String;
        bool primary_key = false;
    };

    struct Row
    {
        std::vector<Value> cells;
    };

    // ---------- table (OO: knows how to describe, insert, select itself) ----------
    class Table
    {
    public:
        Table() = default;
        explicit Table(std::string name, std::vector<Column> cols);

        const std::string &name() const { return _name; }
        const std::vector<Column> &columns() const { return _cols; }
        const std::vector<Row> &rows() const { return _rows; }
        size_t row_count() const { return _rows.size(); }

        // returns row index or error
        std::optional<std::string> insert(const std::vector<Value> &values);
        std::optional<std::string> insert_from_strings(const std::vector<std::string> &raw);

        // WHERE col op value — ops: = != < > <= >=
        std::vector<const Row *> select(const std::string &where = {}) const;
        bool remove_where(const std::string &where);
        bool update_where(const std::string &where, const std::string &col, const Value &v);

        // persistence (line-based text format on the virtual disk)
        std::string serialize() const;
        static std::unique_ptr<Table> deserialize(const std::string &blob, std::string &err);

    private:
        std::string _name;
        std::vector<Column> _cols;
        std::vector<Row> _rows;
    };

    // ---------- engine (singleton, owns all tables + persistence) ----------
    class Engine
    {
    public:
        static Engine &instance();

        Table *create_table(const std::string &name, const std::vector<Column> &cols, std::string &err);
        Table *find(const std::string &name);
        bool drop_table(const std::string &name);
        std::vector<std::string> table_names() const;

        // load/save all tables in a file (default: C:/osbex.db on the virtual disk)
        bool save(const std::string &path = {}) const;
        bool load(const std::string &path = {});

        // run a tiny SQL-ish script (CREATE TABLE / INSERT INTO / SELECT / DELETE / UPDATE)
        // returns per-statement output lines for display
        std::vector<std::string> execute_script(const std::string &sql);

        // convenience: SELECT * FROM t
        std::string dump_table(const std::string &name, std::string &err) const;

    private:
        Engine() = default;
        std::map<std::string, std::unique_ptr<Table>> _tables;
        std::string _db_path = "osbex.db";
    };

    // ---------- system configuration backed by the DB ----------
    // The OS stores its own configuration here ("config" table) instead of an
    // ad-hoc file — config IS data, queried like anything else.
    class Config
    {
    public:
        static Config &instance();

        // read a key ("" if missing)
        std::string get(const std::string &key) const;
        int get_int(const std::string &key, int fallback = 0) const;
        bool get_bool(const std::string &key, bool fallback = false) const;

        // write a key (upsert)
        void set(const std::string &key, const std::string &value);
        void set_int(const std::string &key, int v);
        void set_bool(const std::string &key, bool v);

        void remove(const std::string &key);
        std::vector<std::pair<std::string, std::string>> all() const;

        void persist(); // Engine::save

    private:
        Config();
        static constexpr const char *kTable = "config";
        static constexpr const char *kDbFile = "config.db";
    };
} // namespace osbex::db
