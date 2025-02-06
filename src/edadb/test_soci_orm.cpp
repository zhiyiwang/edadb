/**
 * @file test_soci_orm.cpp
 * @brief test the soci orm interface
 */

#include <vector>
#include <iostream>
#include <soci/soci.h>
#include <soci/sqlite3/soci-sqlite3.h>


// test the soci orm interface
// https://soci.sourceforge.net/doc/master/types/#object-relational-mapping
struct Person {
    int id;
    std::string first_name;
    std::string last_name;
    int age;
    int temp_id; // temnporary id, do not store in database

public:
    void to_string(void) {
        std::cout << "ID: " << id 
                << ", First Name: " << first_name
                << ", Last Name: " << last_name
                << ", Age: " << age;
        std::cout << std::endl;
    }
};


namespace soci
{
    // soci::type_conversion template specialization for Person
    // map ONE Person instance to/from a row in the database
    template<>
    struct type_conversion<Person> {
        typedef values base_type;
        
        static void from_base(values const& v, indicator /* ind */, Person& p) {
            p.id         = v.get<int>("id");
            p.first_name = v.get<std::string>("first_name");
            p.last_name  = v.get<std::string>("last_name");
            p.age        = v.get<int>("age");
        }

        static void to_base(const Person& p, values& v, indicator& ind) {
            v.set("id", p.id);
            v.set("first_name", p.first_name);
            v.set("last_name", p.last_name);
            v.set("age", p.age);
            ind = i_ok;
        } 
    }; // type_conversion<Person>
} // namespace soci


int test_soci_orm (void) {
    try {
        // create session: connect to SQLite database
        soci::session sql(soci::sqlite3, "orm.db");

        // create table
        sql << "CREATE TABLE IF NOT EXISTS person (id INTEGER PRIMARY KEY, first_name TEXT, last_name TEXT, age INTEGER)";


        uint64_t batch_size = 20;
        std::string insert_sql = "INSERT INTO person (id, first_name, last_name, age) VALUES (:id, :first_name, :last_name, :age)";
        std::string select_sql = "SELECT id, first_name, last_name, age FROM person";

        {
            std::cout << "Insert one record using ORM" << std::endl;
            Person p = {1, "Alice", "Smith", 30, 100};
            
            soci::statement st(sql);
            st.exchange(soci::use(p)); // bind by object instead of values
            st.alloc();
            st.prepare(insert_sql);
            st.define_and_bind();
            st.execute(true);
            std::cout << std::endl;
        }

        {
            std::cout << "Read one record using ORM" << std::endl;
            Person p;
            soci::statement st(sql);
            st.exchange(soci::into(p));
            st.alloc();
            st.prepare(select_sql);
            st.define_and_bind();
            st.execute(true);

            p.to_string();
        }

        {
            std::cout << "Bulk insert operations using ORM" << std::endl;
            std::vector<Person> persons = {
                {2, "Bob", "Johnson", 25, 200},
                {3, "Charlie", "Brown", 35, 300}
            };

            Person bind_person;
            soci::statement st = (sql.prepare << insert_sql, soci::use(bind_person));
            for (auto &p : persons) {
                bind_person = p;
                st.execute(true);
            }
            std::cout << std::endl;
        }

        {
            std::cout << "Bulk read operations using ORM:" << std::endl;
            std::string sql_str = "SELECT id, age, first_name, last_name FROM person";
            std::cout << "SQL: " << sql_str << std::endl;

            std::vector<Person> persons;
#if 0
            {
                // execute and fetch records one by one
                Person bind_person;
                soci::statement st = (sql.prepare << sql_str, soci::into(bind_person));
                st.execute(false); // fetch all records
                while (st.fetch()) {
                    persons.push_back(bind_person);
                }
            }
#else
            {
                // prepare and fetch all records into vector
                soci::rowset<Person> rs = (sql.prepare << sql_str);
                persons.assign(rs.begin(), rs.end());
            }
#endif

            for (auto &p : persons) {
                p.to_string();
            } 
            std::cout << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}   