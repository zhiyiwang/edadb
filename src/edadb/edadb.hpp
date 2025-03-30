#pragma once

#include <string>
#include <memory>
#include <set>
#include <type_traits>
#include <cstdint>
#include <cstddef>
#include <memory>
#include <bitset>

#include <boost/mpl/bool.hpp>
#include <boost/mpl/range_c.hpp>
#include <boost/preprocessor.hpp>
#include <boost/fusion/sequence.hpp>
#include <boost/fusion/include/sequence.hpp>
#include <boost/fusion/adapted/struct/adapt_struct.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/algorithm/iteration/for_each.hpp>
#include <boost/fusion/include/for_each.hpp>
#include <boost/fusion/support/pair.hpp>
#include <boost/fusion/include/pair.hpp>
#include <boost/fusion/include/vector.hpp>
#include <boost/fusion/include/algorithm.hpp>
#include <boost/fusion/include/filter_if.hpp>
#include <boost/fusion/include/copy.hpp>
#include <boost/preprocessor/tuple/rem.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/proto/proto.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>



//#include <third_party/fmt/format.hpp>
//#include <third_party/rapidjson/rapidjson.h>
//#include <third_party/rapidjson/document.h>
//#include <third_party/json/json.hpp>
//#include "lib/utils/MD5.hpp"
//#include "lib/edadb/impl/EdadbCppTypeToSQLString.hpp"
// #include <sqlite3.h>
//#include <soci/error.h>
//#include <soci/soci.h>
//#include <soci/sqlite3/soci-sqlite3.h>

#include "CppTypeToDbType.hpp"
#include "TypeMetaData.hpp"
#include "Table4Class.hpp"
#include "SqlStatement.hpp"
#include "DbManager.hpp"
#include "DbMap.hpp"

