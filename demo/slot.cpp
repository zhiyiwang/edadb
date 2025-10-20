/** 
 * @file static.cpp
 * @brief Demonstration of ORM-like pattern for IdbDesign and IdbSlot classes.
 * @version 1.0
 * @date 2025-10-17
 */

#include <iostream>
#include <string>
#include <vector>
#include <functional>

#include "edadb.h"

using namespace std;

// ---------- the real classes ----------
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



// Utility function to convert between object and row
template <typename T, typename R>
R* to_row(T* obj) {
    static_assert(sizeof(T) != sizeof(T), "to_row not specialized for this type");
    return nullptr;
} // to_row

template <typename T, typename R>
void from_row(const R* row, T* obj) {
    static_assert(sizeof(T) != sizeof(T), "from_row not specialized for this type");
} // from_row



// ---------- hard code ORM-like row classes ----------
struct IdbSlotRow {
    long long    slot_id_ = 0;
    std::string  layer_name_;
}; // IdbSlotRow

template <>
IdbSlotRow* to_row(IdbSlot* obj) {
    if (obj == nullptr) return nullptr;
    IdbSlotRow* row = new IdbSlotRow();
    row->slot_id_ = obj->get_slot_id();
    row->layer_name_ = obj->get_layer_name();
    return row;
} // to_row

template <>
void from_row(const IdbSlotRow* row, IdbSlot* obj) {
    if (obj == nullptr || row == nullptr) return;
    obj->set_slot_id(row->slot_id_);
    obj->set_layer_name(row->layer_name_);
} // from_row


struct IdbDesignRow {
    long long    design_id  = 0;
    std::string  name;
    std::string  version;
    long long    timestamp  = 0;
  
    std::vector<IdbSlotRow*> slot_list;
}; // IdbDesignRow

template <>
IdbDesignRow* to_row(IdbDesign* obj) {
    if (obj == nullptr) return nullptr;
    IdbDesignRow* row = new IdbDesignRow();
    row->design_id = obj->get_design_id();
    row->name = obj->get_name();
    row->version = obj->get_version();
    row->timestamp = obj->get_timestamp();

    IdbSlotList* sl = obj->get_slot_list();
    vector<IdbSlot*> &obj_slot_list = sl->get_slots();
    row->slot_list.reserve(obj_slot_list.size());
    for (auto* sa: obj_slot_list) 
        row->slot_list.push_back(to_row<IdbSlot, IdbSlotRow>(sa));

    return row;
} // to_row

template <>
void from_row(const IdbDesignRow* row, IdbDesign* obj) {
    if (obj == nullptr || row == nullptr) return;
    obj->set_design_id(row->design_id);
    obj->set_name(row->name);
    obj->set_version(row->version);
    obj->set_timestamp(row->timestamp);

    IdbSlotList* sl = obj->get_slot_list();
    vector<IdbSlot*> &obj_slot_list = sl->get_slots();
    obj_slot_list.clear();
    for (auto* sa: row->slot_list) {
        IdbSlot* s = new IdbSlot();
        from_row(sa, s);
        obj_slot_list.push_back(s);
    }
} // from_row



// edadb table mapping
TABLE4CLASS(IdbSlotRow, "slot_tab", (slot_id_, layer_name_))
TABLE4CLASS_WVEC(IdbDesignRow, "design_tab", 
    (design_id, name, version, timestamp), (slot_list))



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
