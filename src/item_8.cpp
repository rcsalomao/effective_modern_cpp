#include <atomic>
#include <boost/type_index.hpp>
#include <functional>
#include <iostream>
#include <memory>
#include <print>
#include <ranges>
#include <unordered_map>
#include <vector>

namespace item_8 {
using boost::typeindex::type_id_with_cvr;
using std::cout;
using std::endl;
using std::vector;
using namespace std::ranges;

template <std::ranges::forward_range Rng>
auto stringify(Rng&& seq) {
    auto b = seq |
             views::transform([](const auto& a) { return std::to_string(a); }) |
             views::join_with(',') | views::common;
    return "{" + std::string(std::begin(b), std::end(b)) + "}";
}

void f(int i) { cout << "f(int i): valor de 'i': " << i << endl; }
void f(bool b) { cout << "f(bool i): valor de 'b': " << b << endl; }
void f(void* p) { cout << "f(void* p): valor de 'p': " << p << endl; }

struct Widget {};

double f1(std::unique_ptr<Widget> upw) { return 2.4; }
double f2(std::unique_ptr<Widget>& upw) { return 2.4; }
double f3(Widget* pw) { return 4.2; }

template <typename FuncType, typename PtrType>
void call_f(FuncType func, PtrType ptr) {
    return func(ptr);
}

void main() {
    cout << endl;
    // Deve-se dar preferência à nullptr, ao invés de '0' ou 'NULL'. Em C++ '0'
    // é um tipo integral, podendo ser convertido para um ponteiro nulo
    // dependendo do contexto (ao se tentar usar um '0' aonde apenas um ponteiro
    // pode ser utilizado), entretanto esta é uma conversão implícita não muito
    // interessante. Infelizmente 'NULL' também é do tipo integral ('int' ou
    // 'long', a depender da implementação) e sofre do mesmo problema do uso de
    // '0' para representar ponteiros nulos.
    // Para tanto, deve-se utilizar 'nullptr', pondendo ser entendido como de
    // tipo ponteiro (também não o é, mas possui um mecanismo que o faz ser
    // entendido como um ponteiro para todos os tipos). tais diferenças são
    // importantes no momento de resolução de sobrecarga, por exemplo:
    f(0);
    // f(NULL);  // erro: f(NULL) é ambíguo entre as declarações {'f(int)',
    //           // 'f(bool)' e/ou 'f(void*)'}.
    f(false);
    f(nullptr);

    cout << endl;
    // outra característica é o problema de conversão implícita entre '0' e NULL
    // para outros tipos em chamadas de funções:
    std::unique_ptr<Widget> w = std::make_unique<Widget>();
    f1(0);     // não causa erro. Conversão implícita :c
    f1(NULL);  // não causa erro. Conversão implícita :c
    // f2(nullptr);  // erro
    cout << f2(w) << endl;
    cout << f3(w.get()) << endl;
    f3(nullptr);
    // No caso de 'templates', o processo de dedução de tipo à partir dos
    // argumentos informados acaba por provocar erros de compilação devido aos
    // tipos diferentes entre os argumentos ('int' ou 'long') e os parâmetros
    // das funções ('std::unique_ptr<T>'):
    // call_f(f1, 0);     // erro: no viable conversion from 'int' to
    //                    // 'std::unique_ptr<item_8::Widget>'
    // call_f(f1, NULL);  // erro: no viable conversion from 'long' to
    //                    // 'std::unique_ptr<item_8::Widget>'
    // call_f(f2, 0);     // non-const lvalue reference to type
    //                    // 'std::unique_ptr<item_8::Widget>'
    //                    // cannot bind to a value of
    //                    // unrelated type 'int'
    // call_f(f2, NULL);  // non-const lvalue reference to type
    //                    // 'std::unique_ptr<item_8::Widget>'
    //                    // cannot bind to a value of
    //                    // unrelated type 'long'
    // call_f(f3, 0);  // cannot initialize a parameter of type 'item_8::Widget
    //                 // *' with an lvalue of type 'int'
    // call_f(f3, NULL);  // cannot initialize a parameter of type
    //                    // 'item_8::Widget *' with an lvalue of type 'int'
};
}  // namespace item_8
