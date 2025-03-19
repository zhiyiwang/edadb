namespace edadb
{
    template <>
    struct CppTypeToDbType<IdbPort>
    {
        static const DbTypes ret = DbTypes::kComposite;
    };
    template <>
    struct IsComposite<IdbPort> : boost::mpl::bool_<true>
    {
    };
    template <>
    struct TypeMetaData<IdbPort>
    {
        using TupType = boost::fusion::vector<std::add_pointer<std::remove_reference<std::remove_cv<decltype(IdbPort ::_name)>::type>::type>::type>;
        using TupTypePairType = boost::fusion::vector<boost::fusion::pair<edadb::StripQualifiersAndMakePointer<decltype(IdbPort ::_name)>::type, std::string>>;
        using T = IdbPort;
        inline static auto tuple_type_pair() -> TupTypePairType const &
        {
            static const TupTypePairType t{boost::fusion::make_pair<edadb::StripQualifiersAndMakePointer<decltype(IdbPort ::_name)>::type>("_name")};
            return t;
        }
        inline static std::string const &class_name()
        {
            static std::string const class_name = "IdbPort";
            return class_name;
        }
        inline static std::string const &table_name()
        {
            static std::string const table_name = "port_table";
            return table_name;
        }
        inline static const std::vector<std::string> &member_names()
        {
            static const std::vector<std::string> names = {"_name"};
            return names;
        }
        inline static const std::vector<std::string> &column_names()
        {
            static const std::vector<std::string> names = {"_name"};
            return names;
        }
        inline static TupType getVal(IdbPort *obj) { return TupType(&obj->_name); }
    };
};
namespace edadb
{
    template <typename IdbPort>
    class DbMapAll
    {
    private:
        std::string table_name;
        DbMap<IdbPort> db_map;
        DbMapT2T<IdbPort, typename std::remove_reference<decltype(IdbPort::_layer_shapes)>::type::value_type> _layer_shapes_dbmap;
        DbMapT2T<IdbPort, typename std::remove_reference<decltype(IdbPort::_vias)>::type::value_type> _vias_dbmap;

    public:
        bool connectToDb(const std::string &db_connect_str) { return DbBackend::i().connect(db_connect_str); }
        bool createTable(std::string tab_name)
        {
            table_name = tab_name;
            return db_map.createTable(table_name);
        }
        bool createTable__layer_shapes()
        {
            std::string arr_tab_name = "IdbPort._layer_shapes";
            return _layer_shapes_dbmap.createTable(arr_tab_name, table_name);
        }
        bool createTable__vias()
        {
            std::string arr_tab_name = "IdbPort._vias";
            return _vias_dbmap.createTable(arr_tab_name, table_name);
        }
        bool createTableAll(std::string tab_name) { return createTable(tab_name) && createTable__layer_shapes() && createTable__vias(); }
        bool insertToDb(IdbPort *obj) { return db_map.insertToDb(obj); }
        bool insertToDb__layer_shapes(IdbPort *obj1, typename std::remove_reference<decltype(IdbPort::_layer_shapes)>::type::value_type *obj2) { return _layer_shapes_dbmap.insertToDb(obj1, obj2); }
        bool insertToDb__vias(IdbPort *obj1, typename std::remove_reference<decltype(IdbPort::_vias)>::type::value_type *obj2) { return _vias_dbmap.insertToDb(obj1, obj2); }
        bool insertToDbAll(IdbPort *obj)
        {
            try
            {
                insertToDb(obj);
                for (auto i : obj->_layer_shapes)
                {
                    insertToDb__layer_shapes(obj, &i);
                }
                for (auto i : obj->_vias)
                {
                    insertToDb__vias(obj, &i);
                };
                return true;
            }
            catch (std::exception const &e)
            {
                std::cerr << "insertToDbAll: " << e.what() << "\n";
                return false;
            }
        }
        bool deleteFromDb(IdbPort *obj) { return db_map.deleteFromDb(obj); }
        bool deleteFromDb__layer_shapes(IdbPort *obj1, typename std::remove_reference<decltype(IdbPort::_layer_shapes)>::type::value_type *obj2) { return _layer_shapes_dbmap.deleteFromDb(obj1, obj2); }
        bool deleteFromDb__vias(IdbPort *obj1, typename std::remove_reference<decltype(IdbPort::_vias)>::type::value_type *obj2) { return _vias_dbmap.deleteFromDb(obj1, obj2); }
        bool deleteFromDbAll(IdbPort *obj)
        {
            try
            {
                deleteFromDb(obj);
                for (auto i : obj->_layer_shapes)
                {
                    deleteFromDb__layer_shapes(obj, &i);
                }
                for (auto i : obj->_vias)
                {
                    deleteFromDb__vias(obj, &i);
                };
                return true;
            }
            catch (std::exception const &e)
            {
                std::cerr << "deleteFromDbAll: " << e.what() << "\n";
                return false;
            }
        }
        bool updateDb(IdbPort *obj) { return db_map.updateDb(obj); }
        bool updateDb__layer_shapes(IdbPort *obj1, typename std::remove_reference<decltype(IdbPort::_layer_shapes)>::type::value_type *obj2) { return _layer_shapes_dbmap.updateDb(obj1, obj2); }
        bool updateDb__vias(IdbPort *obj1, typename std::remove_reference<decltype(IdbPort::_vias)>::type::value_type *obj2) { return _vias_dbmap.updateDb(obj1, obj2); }
        bool updateDbAll(IdbPort *obj)
        {
            try
            {
                updateDb(obj);
                for (auto i : obj->_layer_shapes)
                {
                    updateDb__layer_shapes(obj, &i);
                }
                for (auto i : obj->_vias)
                {
                    updateDb__vias(obj, &i);
                };
                return true;
            }
            catch (std::exception const &e)
            {
                std::cerr << "updateDbAll: " << e.what() << "\n";
                return false;
            }
        }
        bool selectFromDb(std::vector<IdbPort> *vec, std::string where_str = "") { return db_map.selectFromDb(vec, where_str); }
        bool selectFromDb__layer_shapes(std::vector<typename std::remove_reference<decltype(IdbPort::_layer_shapes)>::type::value_type> *vec, std::string where_str = "") { return _layer_shapes_dbmap.selectFromDb(vec, where_str); }
        bool selectFromDb__vias(std::vector<typename std::remove_reference<decltype(IdbPort::_vias)>::type::value_type> *vec, std::string where_str = "") { return _vias_dbmap.selectFromDb(vec, where_str); }
        template <typename O>
        std::string stringVal(O val) { return std::to_string(val); }
        std::string stringVal(std::string val)
        {
            std::string str = "\'" + val + "\'";
            return str;
        }
        bool selectFromDbAll(std::vector<IdbPort> *vec, std::string where_str = "")
        {
            try
            {
                selectFromDb(vec, where_str);
                for (auto &i : *vec)
                {
                    const auto vecs = TypeMetaData<IdbPort>::tuple_type_pair();
                    const auto vals = TypeMetaData<IdbPort>::getVal(&i);
                    auto &first_pair = boost::fusion::at_c<0>(vecs);
                    std::string arr_where_str = TypeMetaData<IdbPort>::class_name() + "_" + first_pair.second + " = " + stringVal(*(boost::fusion::at_c<0>(vals)));
                    selectFromDb__layer_shapes(&i._layer_shapes, arr_where_str);
                    selectFromDb__vias(&i._vias, arr_where_str);
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