#include <boost/type_index.hpp>
#include <iostream>
#include <print>
#include <ranges>
#include <vector>

namespace item_1 {
using boost::typeindex::type_id_with_cvr;
using std::cout;
using std::endl;

template <typename T>
void f_1(T& param) {
    std::println("    Tipo de 'T': {}", type_id_with_cvr<T>().pretty_name());
    std::println("    Tipo de 'param': {}",
                 type_id_with_cvr<decltype(param)>().pretty_name());
};

template <typename T>
void f_2(const T& param) {
    std::println("    Tipo de 'T': {}", type_id_with_cvr<T>().pretty_name());
    std::println("    Tipo de 'param': {}",
                 type_id_with_cvr<decltype(param)>().pretty_name());
};

template <typename T>
void f_3(T* param) {
    std::println("    Tipo de 'T': {}", type_id_with_cvr<T>().pretty_name());
    std::println("    Tipo de 'param': {}",
                 type_id_with_cvr<decltype(param)>().pretty_name());
};

template <typename T>
void f_4(T&& param) {
    std::println("    Tipo de 'T': {}", type_id_with_cvr<T>().pretty_name());
    std::println("    Tipo de 'param': {}",
                 type_id_with_cvr<decltype(param)>().pretty_name());
};

template <typename T>
void f_5(T param) {
    std::println("    Tipo de 'T': {}", type_id_with_cvr<T>().pretty_name());
    std::println("    Tipo de 'param': {}",
                 type_id_with_cvr<decltype(param)>().pretty_name());
};

template <typename T, std::size_t N>
constexpr std::size_t array_size(T (&)[N]) noexcept {
    return N;
}

void some_func(int, double) {};

void main() {
    // entendendo a dedução de tipos do mecanismo de "template":
    // O tipo final deduzido (T) pelo mecanismo de template depende não apenas
    // do tipo da expressão a ser deduzida (param), mas como também da definição
    // da função template (ParamType).
    int x{3};
    const int cx = x;
    const int& rx = x;
    const int* px = &x;
    const char* const ptr = "Fun with pointers";
    cout << "Para uma função template do formato:" << endl;
    cout << "template<typename T>" << endl;
    cout << "void f(ParamType param);" << endl;
    cout << "e" << endl;
    cout << "f(arg);" << endl;
    cout << endl;
    cout << "caso 1: ParamType é uma referência ou um ponteiro (T&; T*), mas "
            "não é uma referência universal (T&&)."
         << endl;
    cout << "ParamType é uma referência (T&):" << endl;
    cout << "int arg;" << endl;
    f_1(x);
    cout << "const int arg;" << endl;
    f_1(cx);
    cout << "const int& arg;" << endl;
    f_1(rx);  // a qualidade de referência de 'rx' (&) é ignorada por se tratar
              // de variável constante (const).
    cout << "ParamType é uma referência para uma constante (const T&):" << endl;
    cout << "int arg;" << endl;
    f_2(x);
    cout << "const int arg;" << endl;
    f_2(cx);
    cout << "const int& arg;" << endl;
    f_2(rx);
    cout << "ParamType é um ponteiro (T*):" << endl;
    cout << "int* arg;" << endl;
    f_3(&x);
    cout << "const int* arg;" << endl;
    f_3(px);
    cout << endl;
    cout << "caso 2: ParamType é uma referência universal (T&&)." << endl;
    cout << "A expressão informada é um 'lvalue':" << endl;
    cout << "int arg;" << endl;
    f_4(x);
    cout << "const int arg;" << endl;
    f_4(cx);
    cout << "const int& arg;" << endl;
    f_4(rx);
    cout << "A expressão informada é um 'rvalue':" << endl;
    cout << "int&& arg;" << endl;
    f_4(224);
    cout << endl;
    cout << "caso 3: ParamType não é um ponteiro (T*), nem uma referência (T&)."
         << endl;
    cout << "Desta forma, os parâmetros são passados por valor, realizando-se "
            "cópia (T):"
         << endl;
    // as qualidades de constante são ignoradas, já que agora 'T' e 'param' são
    // cópias dos valores passados, e portanto não são mais constantes.
    cout << "int arg;" << endl;
    f_5(x);
    cout << "const int arg;" << endl;
    f_5(cx);
    cout << "const int& arg;" << endl;
    f_5(rx);
    cout << "const char* const arg;" << endl;
    f_5(ptr);
    cout << endl;
    cout << "No caso do argumento informado ser uma array (a[N]), deve-se "
            "prestar atenção quanto ao 'type decay' que ocorre."
         << endl;
    char name[] = "J. P. Briggs";
    cout << "ParamType é uma referência (T&):" << endl;
    cout << "char name[];" << endl;
    f_1(name);
    cout << "ParamType é passado como valor (T). ocorre 'type decay' para tipo "
            "de ponteiro:"
         << endl;
    cout << "char name[];" << endl;
    f_5(name);
    cout << "Desta forma, é possível se obter o tamanho de uma array por "
            "meio do mecanismo de dedução:"
         << endl;
    cout << "Tamanho de 'name': " << array_size(name) << endl;
    cout << endl;
    cout << "O mesmo processo ocorre quando o argumento informado é uma "
            "função, que pode ter seu tipo dacaído para um ponteiro."
         << endl;
    cout << "ParamType é uma referência (T&):" << endl;
    cout << "void some_func(int, double) {};" << endl;
    f_1(some_func);
    cout << "ParamType é passado como valor (T):" << endl;
    cout << "void some_func(int, double) {};" << endl;
    f_5(some_func);
};
}  // namespace item_1
