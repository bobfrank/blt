#if 0
g++ dynamic_scoping.cpp -o dynamic_scoping -std=c++11
./dynamic_scoping
exit
#endif

#include <iostream>
#include <unordered_map>
#include <stack>
#include <string>

template<typename T>
class DS {
public:
    DS(const std::string& name, const T& value);
    ~DS();
    static const T& get(const std::string& name);
private:
    static std::unordered_map<std::string, std::stack<const T*> > s_globals;
    const std::string m_name;
};

template<typename T>
std::unordered_map<std::string, std::stack<const T*> > DS<T>::s_globals;

template<typename T>
DS<T>::DS(const std::string& name, const T& value) : m_name(name) {
    s_globals[name].push(&value);
}

template<typename T>
DS<T>::~DS() {
    s_globals[m_name].pop();
}

template<typename T>
const T& DS<T>::get(const std::string& name) {
    // should throw an exception saying stack is empty if name was never defined
    return *(s_globals[name].top());
}

// and some example usage
void g() {
    std::cout << "a=" << DS<int>::get("a") << std::endl;
}

void f() {
    DS<int> a("a", 12);
    g();
}

int main() {
    DS<int> a("a", 10);
    f();
    g();
}
