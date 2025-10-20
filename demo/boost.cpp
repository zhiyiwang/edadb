/** 
 * @file static.cpp
 * @brief Demonstration of ORM-like pattern for IdbDesign and IdbSlot classes.
 * @version 1.0
 * @date 2025-10-17
 */

#include <cassert>
#include <iostream>
#include <string>
#include <vector>
#include <functional>

#include "edadb.h"

using namespace std;

//////// eda classes ////////////
class IdbSlot {
private:
  long long slot_id_{0};
  std::string layer_name_;
public:
  IdbSlot() = default;
  IdbSlot(long long sid, std::string ln) : slot_id_(sid), layer_name_(std::move(ln)) {}
  IdbSlot(const IdbSlot& other) = default;

  long long get_slot_id() const { return slot_id_; }
  void set_slot_id(long long v) { slot_id_ = v; }
  const std::string& get_layer_name() const { return layer_name_; }
  void set_layer_name(const std::string& s) { layer_name_ = s; }

public:
  void clear() { slot_id_ = 0; layer_name_.clear(); }
  void print() const {
    std::cout << "IdbSlot{slot_id=" << slot_id_
              << ", layer_name=\"" << layer_name_ << "\"}\n";
  }
}; // IdbSlot

class IdbSlotList {
private:
  std::vector<IdbSlot*> slots_;
public:
  ~IdbSlotList() { for (auto* p : slots_) delete p; }
  const int32_t get_num() const { return slots_.size(); }
  std::vector<IdbSlot*>& get_slots() { return slots_; }
  void add(IdbSlot* s) { slots_.push_back(s); }

public:
  void clear() { for (auto* p : slots_) delete p; slots_.clear(); }
  void print() const {
    std::cout << "IdbSlotList with " << slots_.size() << " slots:\n";
    for (const auto* s : slots_) s->print();
  }
}; // IdbSlotList

class IdbDesign {
private:
  long long design_id_{0};
  std::string name_;
  std::string version_;
  long long timestamp_{0};
  IdbSlotList* slot_list_{nullptr};
  // ignore other members
public:
  IdbDesign() : slot_list_(new IdbSlotList) {}
  ~IdbDesign(){ delete slot_list_; }
  long long get_design_id() const { return design_id_; }
  void set_design_id(long long v){ design_id_ = v; }
  const std::string& get_name() const { return name_; }
  void set_name(const std::string& s){ name_ = s; }
  const std::string& get_version() const { return version_; }
  void set_version(const std::string& s){ version_ = s; }
  long long get_timestamp() const { return timestamp_; }
  void set_timestamp(long long v){ timestamp_ = v; }
  void set_slot_list(IdbSlotList* sl){ slot_list_ = sl; }
  IdbSlotList* get_slot_list() { return slot_list_; }

public:
  void clear() {
    design_id_ = 0; name_.clear(); version_.clear(); timestamp_ = 0; slot_list_->clear();
  }
  void print() const {
    std::cout << "IdbDesign{design_id=" << design_id_ << ", name=\"" << name_ << "\""
        << ", version=\"" << version_ << "\"" << ", timestamp=" << timestamp_ << "}\n";
    slot_list_->print();
  }
};



// to/from_row template: fail if not specialized
template <typename T, typename R>
R* to_row(T* obj) {
    static_assert(sizeof(T) != sizeof(T), "to_row not specialized for this type");
    return nullptr;
} // to_row

template <typename T, typename R>
void from_row(const R* row, T* obj) {
    static_assert(sizeof(T) != sizeof(T), "from_row not specialized for this type");
} // from_row


//////// utility macros for row class generation ////////////
// init fields: Type Name = Default
#define DECL_ROW_MEMBER(r, data, E) \
    BOOST_PP_TUPLE_ELEM(5,1,E) BOOST_PP_TUPLE_ELEM(5,0,E) = BOOST_PP_TUPLE_ELEM(5,4,E);

// init vector fields: vector<Type> Name
#define DECL_VEC_MEMBER(r, data, E) \
    std::vector<BOOST_PP_TUPLE_ELEM(4,2,E)*> BOOST_PP_TUPLE_ELEM(4,0,E);

// row->mem = obj->get
#define GEN_TO_ROW_COPY(r, data, E) \
    row->BOOST_PP_TUPLE_ELEM(5,0,E) = obj->BOOST_PP_TUPLE_ELEM(5,2,E)();

// obj->set from row->mem
#define GEN_FROM_ROW_COPY(r, data, E) \
    obj->BOOST_PP_TUPLE_ELEM(5,3,E)(row->BOOST_PP_TUPLE_ELEM(5,0,E));

// row->vec.push_back to_row(obj->vec.elems)
#define GEN_TO_ROW_VEC_COPY(r, data, E) \
    { \
        auto& src = obj->BOOST_PP_TUPLE_ELEM(4,1,E); \
        row->BOOST_PP_TUPLE_ELEM(4,0,E).reserve(src.size()); \
        for (auto* s : src) { \
            row->BOOST_PP_TUPLE_ELEM(4,0,E).push_back(to_row<BOOST_PP_TUPLE_ELEM(4,3,E), BOOST_PP_TUPLE_ELEM(4,2,E)>(s)); \
        } \
    }

// obj->vec.elems = from_row(row elem, obj elem)
#define GEN_FROM_ROW_VEC_COPY(r, data, E) \
    { \
      auto& dst = obj->BOOST_PP_TUPLE_ELEM(4,1,E); \
      assert(dst.empty()); \
      dst.reserve(row->BOOST_PP_TUPLE_ELEM(4,0,E).size()); \
      for (auto* rp : row->BOOST_PP_TUPLE_ELEM(4,0,E)) { \
          auto* elem = new BOOST_PP_TUPLE_ELEM(4,3,E)(); \
          from_row<BOOST_PP_TUPLE_ELEM(4,3,E), BOOST_PP_TUPLE_ELEM(4,2,E)>(rp, elem); \
          dst.push_back(elem); \
      } \
    }



// ---------- ORM-like row classes ----------

//-// ================== IdbSlot to/from_row ==================
//-template<> inline IdbSlotRow* to_row<IdbSlot, IdbSlotRow>(IdbSlot* obj) {
//-  if (!obj) return nullptr;
//-  auto* row = new IdbSlotRow();
//-  BOOST_PP_SEQ_FOR_EACH(GEN_TO_ROW_COPY, ~, IDB_SLOT_FIELDS)
//-  return row;
//-}
//-template<> inline void from_row<IdbSlot, IdbSlotRow>(const IdbSlotRow* row, IdbSlot* obj) {
//-  if (!row || !obj) return;
//-  BOOST_PP_SEQ_FOR_EACH(GEN_FROM_ROW_COPY, ~, IDB_SLOT_FIELDS)
//-}
//-
//-// ================== IdbDesign to/from_row ==================
//-template<> inline IdbDesignRow* to_row<IdbDesign, IdbDesignRow>(IdbDesign* obj) {
//-  if (!obj) return nullptr;
//-  auto* row = new IdbDesignRow();
//-  BOOST_PP_SEQ_FOR_EACH(GEN_TO_ROW_COPY, ~, IDB_DESIGN_SCALAR_FIELDS)
//-  BOOST_PP_SEQ_FOR_EACH(GEN_TO_ROW_VEC_COPY, ~, IDB_DESIGN_VEC_FIELDS)
//-  return row;
//-}
//-template<> inline void from_row<IdbDesign, IdbDesignRow>(const IdbDesignRow* row, IdbDesign* obj) {
//-  if (!row || !obj) return;
//-  BOOST_PP_SEQ_FOR_EACH(GEN_FROM_ROW_COPY, ~, IDB_DESIGN_SCALAR_FIELDS)
//-  BOOST_PP_SEQ_FOR_EACH(GEN_FROM_ROW_VEC_COPY,  ~, IDB_DESIGN_VEC_FIELDS)
//-}


// scalar only to_row / from_row
#define DEFINE_TO_FROM_ROW_SCALAR(T, R, SCALAR_SEQ)                              \
template<> inline R* to_row<T,R>(T* obj) {                                       \
  if (!obj) return nullptr;                                                      \
  auto* row = new R();                                                           \
  BOOST_PP_SEQ_FOR_EACH(GEN_TO_ROW_COPY, ~, SCALAR_SEQ)                          \
  return row;                                                                    \
}                                                                                \
template<> inline void from_row<T,R>(const R* row, T* obj) {                     \
  if (!row || !obj) return;                                                      \
  BOOST_PP_SEQ_FOR_EACH(GEN_FROM_ROW_COPY, ~, SCALAR_SEQ)                        \
}

// scalar + vector to_row / from_row
#define DEFINE_TO_FROM_ROW_SCALAR_VEC(T, R, SCALAR_SEQ, VEC_SEQ)                 \
template<> inline R* to_row<T,R>(T* obj) {                                       \
  if (!obj) return nullptr;                                                      \
  auto* row = new R();                                                           \
  BOOST_PP_SEQ_FOR_EACH(GEN_TO_ROW_COPY, ~, SCALAR_SEQ)                          \
  BOOST_PP_SEQ_FOR_EACH(GEN_TO_ROW_VEC_COPY,  ~, VEC_SEQ)                        \
  return row;                                                                    \
}                                                                                \
template<> inline void from_row<T,R>(const R* row, T* obj) {                     \
  if (!row || !obj) return;                                                      \
  BOOST_PP_SEQ_FOR_EACH(GEN_FROM_ROW_COPY, ~, SCALAR_SEQ)                        \
  BOOST_PP_SEQ_FOR_EACH(GEN_FROM_ROW_VEC_COPY,  ~, VEC_SEQ)                      \
}



// macros to generate ORM-like code for the classes
#define IDB_SLOT_FIELDS \
    ((slot_id_,   long long,    get_slot_id,    set_slot_id, 0)) \
    ((layer_name_, std::string, get_layer_name, set_layer_name, "")) 


// scalar members: use get
#define IDB_DESIGN_SCALAR_FIELDS \
    ((design_id,  long long,    get_design_id,    set_design_id, 0)) \
    ((name,       std::string,  get_name,         set_name, "")) \
    ((version,    std::string,  get_version,      set_version, "")) \
    ((timestamp,  long long,    get_timestamp,    set_timestamp, 0))

#define IDB_DESIGN_VEC_FIELDS \
    ((slot_list,  get_slot_list()->get_slots(),  IdbSlotRow,     IdbSlot))



struct IdbSlotRow {
    BOOST_PP_SEQ_FOR_EACH(DECL_ROW_MEMBER, ~, IDB_SLOT_FIELDS)
};

struct IdbDesignRow {
    BOOST_PP_SEQ_FOR_EACH(DECL_ROW_MEMBER, ~, IDB_DESIGN_SCALAR_FIELDS)
    BOOST_PP_SEQ_FOR_EACH(DECL_VEC_MEMBER, ~, IDB_DESIGN_VEC_FIELDS)
};



DEFINE_TO_FROM_ROW_SCALAR(IdbSlot,   IdbSlotRow,   IDB_SLOT_FIELDS)
DEFINE_TO_FROM_ROW_SCALAR_VEC(IdbDesign, IdbDesignRow,
        IDB_DESIGN_SCALAR_FIELDS, IDB_DESIGN_VEC_FIELDS)


// edadb table mapping from row members to database columns
TABLE4CLASS(IdbSlotRow, "slot_tab", (slot_id_, layer_name_))
TABLE4CLASS_WVEC(IdbDesignRow, "design_tab", (design_id, name, version, timestamp), (slot_list))



// utility functions
template <typename T, typename R>
int scanTable(edadb::DbMap<R>& dbm) {
  edadb::DbMapReader<R>* rd = nullptr;
  T got;
  R apt;
  while (edadb::read2Scan<R>(rd, dbm, &apt) > 0) {
    from_row(&apt, &got);
    got.print();
  }
  assert(rd == nullptr);
  return 0;
} // scanTable

// T: eda class, R: row class
template<typename T, typename R>
int scanTable(void) {
    edadb::DbMap<R> dbm;
    dbm.init();
    return scanTable<T, R>(dbm);
} // scanTable



int main() {
  IdbDesign d;
  d.set_design_id(42);
  d.set_name("top");
  d.set_version("5.8");
  d.set_timestamp(1699999999LL);

  d.get_slot_list()->add(new IdbSlot(1001, "M1"));
  d.get_slot_list()->add(new IdbSlot(1002, "M2"));


  std::cout << "DbMap<IdbDesignRow>" << std::endl;
  edadb::DbMap<IdbDesignRow> design_map;

  // init database
  std::string conn_param = "design_slot.db";
  std::cout << "[DbMap Init]" << std::endl;
  if (!edadb::initDatabase(conn_param)) {
      std::cerr << "DbMap::init failed" << std::endl;
      return 1;
  }
  std::cout << std::endl << std::endl;

  // create table
  std::cout << "[DbMap CreateTable]" << std::endl;
  if (!edadb::createTable(design_map)) {
      std::cerr << "DbMap::createTable failed" << std::endl;
      return 1; 
  }
  std::cout << std::endl << std::endl;

  // insert object
  IdbDesignRow *design_row = to_row<IdbDesign, IdbDesignRow>(&d);
  std::cout << "[DbMap Insert]" << std::endl;
  if (!edadb::insertObject(design_map, design_row)) {
      std::cerr << "DbMap::insertObject failed" << std::endl;
      return 1;
  }
  std::cout << std::endl << std::endl;

  // scan table
  std::cout << "[DbMap Scan without DbMap]" << std::endl;
  scanTable<IdbDesign, IdbDesignRow>();
  std::cout << std::endl << std::endl;

  return 0;
}
