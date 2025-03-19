#include <iostream>

class Pra{
private:
    int field1;
    std::string field2;


public:
    inline std::string& outer() { return field2; }
};

// class ShadowBase{
//     public:
//     virtual void get(void* obj) = 0;
//     virtual void set(void* obj) = 0;
// };

// class Pra4DB : public ShadowBase{
// public:
//     std::string field2;
// public:
//     void get(void* obj); // 指针强制类型转换
//     void set(void* obj);
// };

template <typename T> // 上层程序员提供的 其实也可以替换掉external类
class Shadow{
public:
// 根据T的具体内容 要求是原子类型或者vector<原子类型> 不考虑嵌套了
public:
    void get(T* obj);
    void set(T* obj);
};
    

bool writeFuncType(Pra *obj, DbOstream* so){
    // so->write(obj->outer());
    return true;
}
