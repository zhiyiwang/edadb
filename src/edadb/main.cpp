#include <iostream>
#include <soci/soci.h>
#include <soci/sqlite3/soci-sqlite3.h>

int main() {
    try {
        // 连接到SQLite数据库（如果不存在，则创建数据库文件）
        soci::session sql(soci::sqlite3, "test.db");

        // 创建一个表格
        sql << "CREATE TABLE IF NOT EXISTS person (id INTEGER PRIMARY KEY, name TEXT, age INTEGER)";

        // 插入数据
        sql << "INSERT INTO person (name, age) VALUES ('Alice', 30)";
        sql << "INSERT INTO person (name, age) VALUES ('Bob', 25)";

        // 查询数据
        soci::rowset<soci::row> rs = (sql.prepare << "SELECT id, name, age FROM person");

        for (const auto& row : rs) {
            int id = row.get<int>("id");
            std::string name = row.get<std::string>("name");
            int age = row.get<int>("age");

            std::cout << "ID: " << id << ", Name: " << name << ", Age: " << age << std::endl;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}

