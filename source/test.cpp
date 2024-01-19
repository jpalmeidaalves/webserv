#include <iostream>
#include <map>

class MyClass {
  private:
    /* data */
  public:
    MyClass(/* args */){};
    ~MyClass(){};
    static std::map<int, int> _nbs;

    static void add(int a, int b) { MyClass::_nbs[a] = b; }
    static int get(int a) {
        try {
            return MyClass::_nbs[a];
        } catch (std::exception &e) {
            (void)e;
            return -1;
        }

        // std::map<int, int>::iterator it;
        // for (it = MyClass::_nbs.begin(); it != MyClass::_nbs.end(); it++) {
        //     if (it->first == a)
        //         return it->second;
        // }

        // return -1;
    }
};

std::map<int, int> MyClass::_nbs;

int main() {

    MyClass::add(2, 4);
    MyClass::add(3, 6);
    MyClass::add(4, 8);
    MyClass::add(5, 12);
    MyClass::add(6, 623);
    MyClass::add(7, 33);
    MyClass::add(8, 44);
    MyClass::add(9, 55);
    MyClass::add(10, 77);
    MyClass::add(11, 88);
    MyClass::add(12, 4);
    MyClass::add(13, 6);
    MyClass::add(14, 8);
    MyClass::add(15, 12);
    MyClass::add(16, 623);
    MyClass::add(17, 33);
    MyClass::add(18, 44);
    MyClass::add(19, 55);
    MyClass::add(110, 77);
    MyClass::add(111, 88);

    std::cout << "2 has " << MyClass::get(2) << std::endl;
    std::cout << "3 has " << MyClass::get(3) << std::endl;
    std::cout << "8 has " << MyClass::get(8) << std::endl;
    std::cout << "8 has " << MyClass::get(11) << std::endl;
    std::cout << "8 has " << MyClass::get(7) << std::endl;
    std::cout << "8 has " << MyClass::get(15) << std::endl;
    std::cout << "8 has " << MyClass::get(19) << std::endl;
    std::cout << "8 has " << MyClass::get(60) << std::endl;
    std::cout << "8 has " << MyClass::get(55) << std::endl;

    return 0;
}
