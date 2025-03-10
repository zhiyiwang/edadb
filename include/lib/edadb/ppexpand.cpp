namespace edadb
{
    template <>
    struct CppTypeToDbType<IdbSites>
    {
        static const DbTypes ret = DbTypes::kComposite;
    };
    template <>
    struct IsComposite<IdbSites> : boost::mpl::bool_<true>
    {
    };
    template <>
    struct TypeMetaData<IdbSites>
    {
        using TupType = boost::fusion::vector<std::add_pointer<std::remove_reference<std::remove_cv<decltype(IdbSites ::name)>::type>::type>::type>;
        using TupTypePairType = boost::fusion::vector<boost::fusion::pair<edadb::StripQualifiersAndMakePointer<decltype(IdbSites ::name)>::type, std::string>>;
        using T = IdbSites;
        inline static auto tuple_type_pair() -> TupTypePairType const &
        {
            static const TupTypePairType t{boost::fusion::make_pair<edadb::StripQualifiersAndMakePointer<decltype(IdbSites ::name)>::type>("name")};
            return t;
        }
        inline static std::string const &class_name()
        {
            static std::string const class_name = "IdbSites";
            return class_name;
        }
        inline static std::string const &table_name()
        {
            static std::string const table_name = "IdbSites_all_table";
            return table_name;
        }
        inline static const std::vector<std::string> &member_names()
        {
            static const std::vector<std::string> names = {"name"};
            return names;
        }
        inline static const std::vector<std::string> &column_names()
        {
            static const std::vector<std::string> names = {"name"};
            return names;
        }
        inline static TupType getVal(IdbSites *obj) { return TupType(&obj->name); }
    };
};
namespace edadb
{
    template <typename IdbSites>
    class DbMapAll
    {
    private:
        std::string table_name;
        DbMap<IdbSites> db_map;
        DbMapT2T<IdbSites, typename std::remove_reference<decltype(IdbSites::idbsite_array)>::type::value_type> idbsite_array_dbmap;
        DbMapT2T<IdbSites, typename std::remove_reference<decltype(IdbSites::array2_array)>::type::value_type> array2_array_dbmap;

    public:
        bool connectToDb(const std::string &db_connect_str) { return DbBackend::i().connect(db_connect_str); }
        bool createTable(std::string tab_name)
        {
            table_name = tab_name;
            return db_map.createTable(table_name);
        }
        bool createTable_idbsite_array()
        {
            std::string arr_tab_name = "IdbSites.idbsite_array";
            return idbsite_array_dbmap.createTable(arr_tab_name, table_name);
        }
        bool createTable_array2_array()
        {
            std::string arr_tab_name = "IdbSites.array2_array";
            return array2_array_dbmap.createTable(arr_tab_name, table_name);
        }
        bool createTableAll(std::string tab_name) { return createTable(tab_name) && createTable_idbsite_array() && createTable_array2_array(); }
        bool insertToDb(IdbSites *obj) { return db_map.insertToDb(obj); }
        bool insertToDb_idbsite_array(IdbSites *obj1, typename std::remove_reference<decltype(IdbSites::idbsite_array)>::type::value_type *obj2) { return idbsite_array_dbmap.insertToDb(obj1, obj2); }
        bool insertToDb_array2_array(IdbSites *obj1, typename std::remove_reference<decltype(IdbSites::array2_array)>::type::value_type *obj2) { return array2_array_dbmap.insertToDb(obj1, obj2); }
        bool insertToDbAll(IdbSites *obj)
        {
            try
            {
                insertToDb(obj);
                for (auto i : obj->IdbSites)
                {
                    insertToDb_IdbSites(obj, &i);
                }
                for (auto i : obj->idbsite_array)
                {
                    insertToDb_idbsite_array(obj, &i);
                };
                return true;
            }
            catch (std::exception const &e)
            {
                std::cerr << "insertToDbAll: " << e.what() << "\n";
                return false;
            }
        }
        bool deleteFromDb(IdbSites *obj) { return db_map.deleteFromDb(obj); }
        bool deleteFromDb_idbsite_array(IdbSites *obj1, typename std::remove_reference<decltype(IdbSites::idbsite_array)>::type::value_type *obj2) { return idbsite_array_dbmap.deleteFromDb(obj1, obj2); }
        bool deleteFromDb_array2_array(IdbSites *obj1, typename std::remove_reference<decltype(IdbSites::array2_array)>::type::value_type *obj2) { return array2_array_dbmap.deleteFromDb(obj1, obj2); }
        bool deleteFromDbAll(IdbSites *obj)
        {
            try
            {
                deleteFromDb(obj);
                for (auto i : obj->idbsite_array)
                {
                    deleteFromDb_idbsite_array(obj, &i);
                }
                for (auto i : obj->array2_array)
                {
                    deleteFromDb_array2_array(obj, &i);
                };
                return true;
            }
            catch (std::exception const &e)
            {
                std::cerr << "deleteFromDbAll: " << e.what() << "\n";
                return false;
            }
        }
        bool updateDb(IdbSites *obj) { return db_map.updateDb(obj); }
        bool updateDb_idbsite_array(IdbSites *obj1, typename std::remove_reference<decltype(IdbSites::idbsite_array)>::type::value_type *obj2) { return idbsite_array_dbmap.updateDb(obj1, obj2); }
        bool updateDb_array2_array(IdbSites *obj1, typename std::remove_reference<decltype(IdbSites::array2_array)>::type::value_type *obj2) { return array2_array_dbmap.updateDb(obj1, obj2); }
        bool updateDbAll(IdbSites *obj)
        {
            try
            {
                updateDb(obj);
                for (auto i : obj->idbsite_array)
                {
                    updateDb_idbsite_array(obj, &i);
                }
                for (auto i : obj->array2_array)
                {
                    updateDb_array2_array(obj, &i);
                };
                return true;
            }
            catch (std::exception const &e)
            {
                std::cerr << "updateDbAll: " << e.what() << "\n";
                return false;
            }
        }
        bool selectFromDb(std::vector<IdbSites> *vec, std::string where_str = "") { return db_map.selectFromDb(vec, where_str); }
        bool selectFromDb_idbsite_array(std::vector<typename std::remove_reference<decltype(IdbSites::idbsite_array)>::type::value_type> *vec, std::string where_str = "") { return idbsite_array_dbmap.selectFromDb(vec, where_str); }
        bool selectFromDb_array2_array(std::vector<typename std::remove_reference<decltype(IdbSites::array2_array)>::type::value_type> *vec, std::string where_str = "") { return array2_array_dbmap.selectFromDb(vec, where_str); }
        template <typename O>
        std::string stringVal(O val) { return std::to_string(val); }
        std::string stringVal(std::string val)
        {
            std::string str = "\'" + val + "\'";
            return str;
        }
        bool selectFromDbAll(std::vector<IdbSites> *vec, std::string where_str = "")
        {
            try
            {
                selectFromDb(vec, where_str);
                for (auto &i : *vec)
                {
                    const auto vecs = TypeMetaData<IdbSites>::tuple_type_pair();
                    const auto vals = TypeMetaData<IdbSites>::getVal(&i);
                    auto &first_pair = boost::fusion::at_c<0>(vecs);
                    std::string arr_where_str = TypeMetaData<IdbSites>::class_name() + "_" + first_pair.second + " = " + stringVal(*(boost::fusion::at_c<0>(vals)));
                    selectFromDb_idbsite_array(&i.idbsite_array, arr_where_str);
                    selectFromDb_array2_array(&i.array2_array, arr_where_str);
                    ;
                }
                return true;
            }
            catch (std::exception const &e)
            {
                std::cerr << "selectFromDbAll: " << e.what() << "\n";
                return false;
            }
        }
    };
};