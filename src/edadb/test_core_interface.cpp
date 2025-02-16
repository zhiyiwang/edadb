/**
 * @file test_core_interface.cpp
 * @brief test the soci core interface
 * https://soci.sourceforge.net/doc/master/interfaces/
 */

#include <vector>
#include <iostream>
#include <soci/soci.h>
#include <soci/sqlite3/soci-sqlite3.h>

/**
 * @brief test the soci core interface
 * https://soci.sourceforge.net/doc/master/interfaces/
 */
int test_core_interface(void) {
    try {
        // create session: connect to SQLite database
        soci::session sql(soci::sqlite3, "core.intf.db");

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