#include <iostream>

template<typename T>
class Test{
    public:
    void print();
};

template<>
class Test<int>{
    public:
    void print();
};

template<>
class Test<double>{
    public:
    void print();
};

template<typename T>
void Test<T>::print(){
    std::cout<<typeid(T).name()<< " hello\n";
}

int main(){
    Test<int> t_int;
    Test<double> t_double;
    t_int.print();
    t_double.print();
    return 0;
}