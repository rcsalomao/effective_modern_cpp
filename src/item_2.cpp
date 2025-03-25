#include <iostream>
#include <vector>

namespace item_2 {
using std::cout;
using std::endl;

void some_func(int, double) {};

// erro de dedução também pode ocorrer com o uso de 'auto' para dedução de tipos
// de retorno de funções:
// auto create_init_list() { return {1, 2, 3}; }  // erro!

void main() {
    // Entendendo o mecanismo de dedução de 'auto':
    // Quando uma variável é declarada com 'auto', este faz o papel do símbolo
    // 'T' de umaa expressão template (item 1). O especificador do tipo faz o
    // papel de 'ParamType'. De forma geral, o processo de dedução de tipos pelo
    // mecanismo 'auto' segue a mesma lógica que o mecanismo para 'templates',
    // com a ressalva de 1 caso que se trata de uma excessão. seguindo a lógica
    // para 'templates', pode-se classificar os casos em:
    // caso 1: O especificador de tipo é um ponteiro ou uma referência, mas não
    //         é uma referência universal.
    // caso 2: O especificador de tipo é uma referência universal.
    // caso 3: O especificador de tipo não é um ponteiro nem uma referência.
    auto x = 27;         // caso 3
    const auto cx = x;   // caso 3
    const auto& rx = x;  // caso 1
    auto ix = rx;        // caso 3
    auto&& uref1 = x;    // caso 2 (int&)
    auto&& uref2 = cx;   // caso 2 (const int&)
    auto&& uref3 = 47;   // caso 2 (int&&)
    const char name[] = "J. P. Briggs";
    auto arr1 = name;   // (const char *)
    auto& arr2 = name;  // (const char &[13])

    auto func1 = some_func;   // (void (*)(int, double))
    auto& func2 = some_func;  // (void (&)(int, double))

    // o caso em específico que 'auto' se difere de 'template' é no caso do uso
    // de {} para inicialização de variáveis. isto se dá devido à uma regra em
    // especial de dedução de tipos para 'auto'.
    auto x1 = 27;         // (int)
    auto x2(27);          // (int)
    auto x3 = {27};       // (std::initializer_list<int>)
    auto x4{27};          // (int)
    auto x5 = {1, 2, 3};  // (std::initializer_list<int>)
    // Quando um inicializador para uma variável declarada com 'auto' é
    // envolvido com '{}', o tipo deduzido é de 'std::initializer_list<T>'. Se o
    // tipo não puder ser deduzido, o código será rejeitado.
    // auto x5{1, 2, 3};       // erro!
    // auto x5 = {1, 2, 3.0};  // erro!
    //
    // std::vector<int> v;
    // auto reset_v = [&v](const auto&& n) { v = n; };
    // reset_v({1, 2, 3});  // erro!
};
}  // namespace item_2
