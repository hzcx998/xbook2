#include "test.h"

#include <iostream>

using namespace std;

class People{
public:
    People(const char *str) {
        this->name = str;
        printf("call People name:%s\n", this->name);
    }
    ~People() {
        printf("call ~People\n");
    }
private:
    const char *name;
};


int iostream_test(int argc, char *argv[])
{
    int a[] = {1,2,5,6};
    for (int i = 0;i < 4;i ++) {
        cout << a[i] << endl;
    }
    const char *str = "xbook2!";
    cout << 123 << "hello" << str << endl;
    People("jason");

    return 0;
}
