/**
 * @brief test the soci interface
 * https://soci.sourceforge.net/doc/master/interfaces/
 */
#include <vector>
#include <iostream>
#include <soci/soci.h>
#include <soci/sqlite3/soci-sqlite3.h>



int test_sugar_interface(void) {
    try {
        // create session: connect to SQLite database
        soci::session sql(soci::sqlite3, "sugar.db");

        // create table
        sql << "CREATE TABLE IF NOT EXISTS person (id INTEGER PRIMARY KEY, name TEXT, age INTEGER)";

        // insert records
        // each statement will be prepared and executed separately
        sql << "INSERT INTO person (name, age) VALUES ('Alice', 30)";
        sql << "INSERT INTO person (name, age) VALUES ('Bob', 25)";

        // query records
        // session::prepare to prepare a statement
        // rowset to iterate over the result set
        soci::rowset<soci::row> rs = sql.prepare << "SELECT id, name, age FROM person";

        // iterate each row and get the value by column name
        // the name is defined when creating the table
        printf("Get Value by Column Name:\n");
        for (const auto& row : rs) {
            int id = row.get<int>("id");
            std::string name = row.get<std::string>("name");
            int age = row.get<int>("age");

            std::cout << "ID: " << id << ", Name: " << name << ", Age: " << age << std::endl;
        }

        // iterate each row and get the value by column index
        // the index is the order of the columns in the SELECT statement
        printf("Get Value by Column Index:\n");
        // reset the rowset to query again
        rs = sql.prepare << "SELECT id, name, age FROM person";
        for (const auto& row : rs) {
            int id = row.get<int>(0);
            std::string name = row.get<std::string>(1);
            int age = row.get<int>(2);

            std::cout << "ID: " << id << ", Name: " << name << ", Age: " << age << std::endl;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
} // test_suagr_interface


int test_core_interface(void) {
    try {
        // create session: connect to SQLite database
        soci::session sql(soci::sqlite3, "core.db");

        // sugar interface to create table 
        sql << "CREATE TABLE IF NOT EXISTS person (id INTEGER PRIMARY KEY, name TEXT, age INTEGER)";

        uint64_t batch_size = 20;

        {
            std::cout << std::endl;
            std::cout << "Bulk insert operations using non-null indicator" << std::endl;
            std::vector<int> ids = {1, 2, 3};
            std::vector<soci::indicator> ids_ind = {soci::i_ok, soci::i_ok, soci::i_ok};
            std::vector<std::string> names = {"Alice", "Bob", "Charlie"};
            std::vector<soci::indicator> names_ind = {soci::i_ok, soci::i_ok, soci::i_ok};
            std::vector<int> ages = {30, 25, 35};
            std::vector<soci::indicator> ages_ind = {soci::i_ok, soci::i_ok, soci::i_ok};

            soci::statement st(sql);
            st.exchange(soci::use(ids, ids_ind));  
            st.exchange(soci::use(names, names_ind));
            st.exchange(soci::use(ages, ages_ind));
            st.alloc();
            st.prepare("INSERT INTO person (id, name, age) VALUES (:id, :name, :age)");
            st.define_and_bind();
            st.execute(true);
            std::cout << std::endl;
        }

        {
            std::cout << std::endl;
            std::cout << "Bulk read operations WO Indicator:" << std::endl;
            std::vector<int> ids;
            std::vector<std::string> names;
            std::vector<int> ages;
            ids.resize  (batch_size);
            names.resize(batch_size);
            ages.resize (batch_size);

            soci::statement st(sql);
            st.exchange(soci::into(ids));
            st.exchange(soci::into(names));
            st.exchange(soci::into(ages));

            st.alloc();
            st.prepare("SELECT id, name, age FROM person");
            st.define_and_bind();
            st.execute(true);

            for (size_t i = 0; i < ids.size(); ++i) {
                std::cout << "ID: " << ids[i] << ", Name: " << names[i] << ", Age: " << ages[i];
                std::cout << std::endl;
            }
            std::cout << std::endl;
        }

        {
            std::cout << std::endl;
            std::cout << "Bulk insert operations using null indicator" << std::endl;
            std::vector<int> ids = {4, 5, 6};
            std::vector<soci::indicator> ids_ind = {soci::i_ok, soci::i_ok, soci::i_ok};
            std::vector<std::string> names = {"David", "Eve", "Frank"};
            std::vector<soci::indicator> names_ind = {soci::i_null, soci::i_ok, soci::i_null};
            std::vector<int> ages = {40, 45, 50};
            std::vector<soci::indicator> ages_ind = {soci::i_ok, soci::i_null, soci::i_null};

            soci::statement st(sql);
            st.exchange(soci::use(ids, ids_ind));  // bind the vector to the statement
            st.exchange(soci::use(names, names_ind));
            st.exchange(soci::use(ages, ages_ind));
            st.alloc();
            st.prepare("INSERT INTO person (id, name, age) VALUES (:id, :name, :age)");
            st.define_and_bind();
            st.execute(true);
            std::cout << std::endl;
        }

        {
            std::cout << std::endl;
            std::cout << "Bulk read operations W Indicator:" << std::endl;
            std::vector<int> ids;
            std::vector<soci::indicator> ids_ind;
            std::vector<std::string> names;
            std::vector<soci::indicator> names_ind;
            std::vector<int> ages;
            std::vector<soci::indicator> ages_ind;
            ids  .resize(batch_size), ids_ind  .resize(batch_size);
            names.resize(batch_size), names_ind.resize(batch_size);
            ages .resize(batch_size), ages_ind .resize(batch_size);

            soci::statement st(sql);
            st.exchange(soci::into(ids, ids_ind));
            st.exchange(soci::into(names, names_ind));
            st.exchange(soci::into(ages, ages_ind));

            st.alloc();
            st.prepare("SELECT id, name, age FROM person");
            st.define_and_bind();
            st.execute(true);

            // print indicator value to string
            auto ind_to_string = [](soci::indicator ind) -> std::string {
                switch (ind) {
                    case soci::i_ok:        return "i_ok"   ;
                    case soci::i_null:      return "i_null" ;
                    case soci::i_truncated: return "i_trunc";
                    default:                return "unknown";
                }
            };
            for (size_t i = 0; i < ids.size(); ++i) {
                std::cout << "Iter Index: [" << i << "]  ";  
                std::cout << "ID: "   << ids[i]   << ", ind: " << ind_to_string(ids_ind[i])   << "  ";
                std::cout << "Name: " << names[i] << ", ind: " << ind_to_string(names_ind[i]) << "  ";
                std::cout << "Age: "  << ages[i]  << ", ind: " << ind_to_string(ages_ind[i]);
                std::cout << std::endl;
            }
        }

        {
            std::cout << std::endl;
            std::cout << "Bulk read operations with predicate:" << std::endl;
            std::string sql_str = "SELECT id, name, age FROM person WHERE age > :age and name IS NOT NULL"; 
            std::cout << "SQL: " << sql_str << std::endl;

            std::vector<int> ids;
            std::vector<soci::indicator> ids_ind;
            std::vector<std::string> names;
            std::vector<soci::indicator> names_ind;
            std::vector<int> ages;
            std::vector<soci::indicator> ages_ind;
            ids.resize  (batch_size), ids_ind  .resize(batch_size);   
            names.resize(batch_size), names_ind.resize(batch_size);
            ages.resize (batch_size), ages_ind .resize(batch_size);

            soci::statement st(sql);
            st.exchange(soci::into(ids, ids_ind));
            st.exchange(soci::into(names, names_ind));
            st.exchange(soci::into(ages, ages_ind));

            int age_threshold = 30;
            st.exchange(soci::use(age_threshold));

            st.alloc();
            st.prepare(sql_str);
            st.define_and_bind();
            st.execute(true);

            // print indicator value to string
            auto ind_to_string = [](soci::indicator ind) -> std::string {
                switch (ind) {
                    case soci::i_ok:        return "i_ok"   ;
                    case soci::i_null:      return "i_null" ;
                    case soci::i_truncated: return "i_trunc";
                    default:                return "unknown";
                }
            };
            for (size_t i = 0; i < ids.size(); ++i) {
                std::cout << "Iter Index: [" << i << "]  ";  
                std::cout << "ID: "   << ids[i]   << ", ind: " << ind_to_string(ids_ind[i])   << "  ";
                std::cout << "Name: " << names[i] << ", ind: " << ind_to_string(names_ind[i]) << "  ";
                std::cout << "Age: "  << ages[i]  << ", ind: " << ind_to_string(ages_ind[i]);
                std::cout << std::endl;
            }
            std::cout << std::endl;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}


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
            Person bind_person;
            soci::statement st = (sql.prepare << sql_str, soci::into(bind_person));
            st.execute(false); // fetch all records
            while (st.fetch()) {
                persons.push_back(bind_person);
            }

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




int main(void) {
//    // test the soci interface
//    test_sugar_interface();

//    // test the core interface
//    test_core_interface();

    // test the soci orm interface
    test_soci_orm();


    return 0;
}

