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
    // Entendendo a dedução de tipos do mecanismo de "template":
    // Para uma função template do formato:
    //
    // template<typename T>
    // void f(ParamType param);
    //
    // e
    //
    // f(expr);
    //
    // Durante o processo de compilação, o compilador utiliza a expressão 'expr'
    // para deduzir os tipos de 'T' e de 'ParamType'. De forma geral, estes
    // tipos são diferentes, já que 'ParamType' com frequência contém
    // qualificadores como 'const', 'volatine' e 'ref'.
    // O tipo final de 'T' depende não apenas do tipo da expressão 'expr'
    // informada, mas como também da definição de 'ParamType'.
    //
    // Para melhor entendimento, pode-se categorizar em 3 contextos:
    // - Caso 1: A expressão é uma referência ou um ponteiro.
    // - Caso 2: A expressão é uma referẽncia universal.
    // - Caso 3: A expressão não é uma referência nem ponteiro, mas sim um valor
    //
    cout << endl;
    int x{3};
    const int cx = x;
    const int& rx = x;
    const int* px = &x;
    const char* const ptr = "Fun with pointers";
    cout << "Para uma função template do formato:" << endl;
    cout << "template<typename T>" << endl;
    cout << "void f(ParamType param);" << endl;
    cout << "e" << endl;
    cout << "f(expr);" << endl;
    //
    // Caso 1:
    cout << endl;
    cout << "caso 1: ParamType é uma referência ou um ponteiro (T&; T*), mas "
            "não é uma referência universal (T&&)."
         << endl;
    // Neste caso:
    // 1: Se a expressão for uma referência, deve-se ignorar a referência.
    // 2: Realizar 'pattern-match' em 'ParamType' para deduzir o tipo de 'T'.
    //
    cout << "ParamType é uma referência (T&):" << endl;
    cout << "int expr;" << endl;
    f_1(x);
    cout << "const int expr;" << endl;
    f_1(cx);
    cout << "const int& expr;" << endl;
    f_1(rx);
    // Percebe-se que mesmo 'ParamType' sendo uma referência, 'T' continua
    // possuindo a qualificação de 'const' das expressões informadas.
    // Adicionalmente, a qualificação de referência de 'rx' é devidamente
    // ignorada, ficando presente apenas no tipo de 'ParamType'.
    cout << "ParamType é uma referência para uma constante (const T&):" << endl;
    cout << "int expr;" << endl;
    f_2(x);
    cout << "const int expr;" << endl;
    f_2(cx);
    cout << "const int& expr;" << endl;
    f_2(rx);
    // Agora, não muito diferentemente do caso anterior, a qualificação de
    // 'const' não está mais presente no tipo de 'T', mas apenas no tipo de
    // 'ParamType', devido ao 'pattern-match'.
    cout << "ParamType é um ponteiro (T*):" << endl;
    cout << "int* expr;" << endl;
    f_3(&x);
    cout << "const int* expr;" << endl;
    f_3(px);
    //
    // Caso 2:
    cout << endl;
    cout << "caso 2: ParamType é uma referência universal (T&&)." << endl;
    // Neste caso:
    // 1: Se 'expr' for um 'lvalue', então ambos 'T' e 'ParamType' serão
    // deduzidos como 'lvalue ref'. Esta é a única situação em que 'T' pode ser
    // deduzido como uma referência. 'ParamType' é deduzido como 'lvalue ref'
    // devido ao mecanismo de 'reference collapsing' (E& && -> E&).
    // 2: Se 'expr' for um 'rvalue', então deve-se aplicar o mesmo procedimento
    // do caso 1: ignorar a parte da referência e executar 'pattern-match'.
    //
    cout << "A expressão informada é um 'lvalue':" << endl;
    cout << "int expr;" << endl;
    f_4(x);
    cout << "const int expr;" << endl;
    f_4(cx);
    cout << "const int& expr;" << endl;
    f_4(rx);
    cout << "A expressão informada é um 'rvalue':" << endl;
    cout << "int&& expr;" << endl;
    f_4(224);
    //
    // Caso 3:
    cout << endl;
    cout << "caso 3: ParamType não é um ponteiro (T*), nem uma referência (T& "
            "ou T&&)."
         << endl;
    cout << "Desta forma, os parâmetros são passados por valor, realizando-se "
            "cópia (T):"
         << endl;
    // Neste caso:
    // 1: Se 'expr' for uma referência, deve-se ignorar a parte da referência.
    // 2: Se, após ignorar a qualificação de referência, 'expr' for 'const'
    // e/ou 'volatile', deve-se também ignorar estas qualificações.
    //
    // As qualidades de 'const', 'ref' e 'volatile' são ignoradas, já que agora
    // 'T' e 'ParamType' são cópias dos valores passados e, portanto, não são
    // mais constantes, voláteis, nem referências.
    cout << "int expr;" << endl;
    f_5(x);
    cout << "const int expr;" << endl;
    f_5(cx);
    cout << "const int& expr;" << endl;
    f_5(rx);
    // Deve-se prestar atenção, pois apenas o nível de 'const' e/ou 'volatile'
    // mais superficial é ignorado (a qualificação 'const' da expressão de fato
    // que está sendo informada). No caso de um ponteiro constante para um valor
    // também contante, apenas a qualificação do ponteiro é que é ignorada. A
    // qualificação do valor apontado se mantém.
    cout << "const char* const expr;" << endl;
    // const T* const ptr;
    // ^^^^^    ^^^^^
    //   │        └─> qualificação do ponteiro.
    //   └─> qualificação do valor apontado.
    f_5(ptr);

    cout << endl;
    cout << "No caso da expressão informada ser uma array (a[N]), deve-se "
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
    cout << "O mesmo processo ocorre quando a expressão informada é uma "
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
