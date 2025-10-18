/** 
 * @file slot.cpp
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

// ---------- hard code ORM-like adapter classes ----------
struct IdbSlotAdapter {
public:
    // IdbSlot columns
    long long    slot_id_ = 0;
    std::string  layer_name_;
public:
    IdbSlotAdapter() = default;
    ~IdbSlotAdapter() = default;

    int setObj(IdbSlot* s) {
        if (s == nullptr) return -1;
        s->set_slot_id(slot_id_);
        s->set_layer_name(layer_name_);
        return 0;
    }

    int getObj(IdbSlot* s) {
        if (s == nullptr) return -1;
        slot_id_ = s->get_slot_id();
        layer_name_ = s->get_layer_name();
        return 0;
    }
};

struct IdbDesignAdapter {
public:
    // IdbDesign columns
    long long    design_id  = 0;
    std::string  name;
    std::string  version;
    long long    timestamp  = 0;
  
    // ignore IdbSlotList:
    // directly reference the real container (for ORM's _WVEC usage)
    std::vector<IdbSlotAdapter*> slot_list;

public:
    IdbDesignAdapter() = default;
    ~IdbDesignAdapter() {
        for (auto* s : slot_list) { delete s; }
        slot_list.clear();
    }

    /**
     * set data from adapter (storage columns) to the real object
     * @return 0 success, else failure
     */ 
    int setObj(IdbDesign* obj) {
        if (obj == nullptr) return -1;

        obj->set_design_id(design_id);
        obj->set_name(name);
        obj->set_version(version);
        obj->set_timestamp(timestamp);

        IdbSlotList* sl = obj->get_slot_list();
        vector<IdbSlot*> &obj_slot_list = sl->get_slots();
        assert(obj_slot_list.empty());

        for (auto* sa: slot_list) {
            IdbSlot* s = new IdbSlot();
            sa->setObj(s);
            obj_slot_list.push_back(s);
        }

        return 0;
    } // setObj

    /**
     * get data from the real object to the adapter (storage columns)
     * @return 0 success, else failure
     */
    int getObj(IdbDesign* obj) {
        design_id = obj->get_design_id();
        name = obj->get_name();
        version = obj->get_version();
        timestamp = obj->get_timestamp();

        IdbSlotList* sl = obj->get_slot_list();
        vector<IdbSlot*> &obj_slot_list = sl->get_slots();
        for (auto* sa: obj_slot_list) {
            IdbSlotAdapter* s_adapter = new IdbSlotAdapter();
            s_adapter->getObj(sa);
            slot_list.push_back(s_adapter);
        }

        return 0;
    } // getObj
};


// edadb table mapping
TABLE4CLASS(IdbSlotAdapter, "slot_tab", (slot_id_, layer_name_))
TABLE4CLASS_WVEC(IdbDesignAdapter, "design_tab", 
    (design_id, name, version, timestamp), (slot_list))



// utility functions
template <typename T, typename A>
int scanTable(edadb::DbMap<A>& dbm) {
  edadb::DbMapReader<A>* rd = nullptr;
  T got;
  A apt;
  while (edadb::read2Scan<A>(rd, dbm, &apt) > 0) {
    apt.setObj(&got);
    got.print();
  }
  assert(rd == nullptr);
  return 0;
} // scanTable

// T: store class, A: adapter class
template<typename T, typename A>
int scanTable(void) {
    edadb::DbMap<A> dbm;
    dbm.init();
    return scanTable<T, A>(dbm);
} // scanTable



int main() {
  IdbDesign d;
  d.set_design_id(42);
  d.set_name("top");
  d.set_version("5.8");
  d.set_timestamp(1699999999LL);

  d.get_slot_list()->add(new IdbSlot(1001, "M1"));
  d.get_slot_list()->add(new IdbSlot(1002, "M2"));


  std::cout << "DbMap<IdbDesignAdapter>" << std::endl;
  edadb::DbMap<IdbDesignAdapter> design_map;

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
  IdbDesignAdapter design_adapter;
  design_adapter.getObj(&d);
  std::cout << "[DbMap Insert]" << std::endl;
  if (!edadb::insertObject(design_map, &design_adapter)) {
      std::cerr << "DbMap::insertObject failed" << std::endl;
      return 1;
  }
  std::cout << std::endl << std::endl;

  // scan table
  std::cout << "[DbMap Scan without DbMap]" << std::endl;
  scanTable<IdbDesign, IdbDesignAdapter>();
  std::cout << std::endl << std::endl;

  return 0;
}
