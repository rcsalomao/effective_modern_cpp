#include <atomic>
#include <boost/type_index.hpp>
#include <functional>
#include <iostream>
#include <list>
#include <memory>
#include <print>
#include <ranges>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace item_14 {
using boost::typeindex::type_id_with_cvr;
using std::cout;
using std::endl;
using std::string;
using std::vector;
namespace rg = std::ranges;
namespace vw = std::views;

template <typename T>
using uptr = std::unique_ptr<T>;

template <typename T, typename... Args>
uptr<T> mu(Args&&... args) {
    return std::make_unique<T>(std::forward<Args>(args)...);
};

template <std::ranges::forward_range Rng>
auto stringify(Rng&& seq) {
    using std::to_string;

    auto b = seq | vw::transform([](const auto& a) { return to_string(a); }) |
             vw::join_with(',') | vw::common;
    return "{" + std::string(std::begin(b), std::end(b)) + "}";
}

void foo1() noexcept {};  // mais otimizável. sintaxe de C++11.
void foo2() throw() {};   // menos otimizável. sintaxe de C++98.
void foo3() {};           // menos otimizável.

template <typename T, size_t N>
void swap_array_like(T (&a)[N], T (&b)[N]) noexcept(noexcept(swap(*a, *b))){
    // ...
};
template <typename T1, typename T2>
struct pair {
    T1 first;
    T2 second;

    void swap_pair_like(pair& p) noexcept(noexcept(swap(first, p.first)) &&
                                          noexcept(swap(second, p.second))) {
        // ...
    };
};

void main() {
    // O que interessa saber para um função, no que diz respeito às exceções, é
    // se ela emite ou não uma exceção. Tal comportamento faz parte do projeto
    // de interface do programa. Não há resposta objetivamente correta para uma
    // interface baseada em exceções ou não. No fim, a interface (e por
    // consequência se uma função emite ou não exceções) irá ditar como o
    // usuário fará uso destas funções/biblioteca.
    //
    // Ainda, outro ponto importante é que compiladores podem produzir código
    // mais otimizado para funções que não emitem exceções, definidas pela
    // sintaxe 'noexcept'. Já que se uma determinada função não irá emitir
    // exceções, então não há a necessidade de se guardar informações e
    // instruções sobre a pilha de processos de determinada função.
    //
    // Adicionalmente, algumas operações básicas da 'stl', quando dectam a
    // disponibilidade de funções 'noexcept', são capazes de realizar operações
    // alternativas mais eficientes, tal como 'std::vector<T>::push_back()'. O
    // método 'push_back()' verifica inicialmente se para o tipo da variável que
    // ele possui há a definição de operação de 'move noexpect'. Caso positivo,
    // então o método realiza a operação de movimentação, ao invés da operação
    // convencional de cópia. Caso contrário, então 'push_back' realiza a cópia
    // do elemento para dentro do container. Outras funções operam de forma
    // similar, tais como 'std::vector<T>::reserve', 'std::deque::insert', etc).
    //
    // De forma análoga, as funções 'std::swap', que são fundamentais para
    // diversos algoritmos da 'stl', possuem comportamento 'noexcept'
    // condicional. 'std::swap' será definida como 'noexcept' caso também
    // existam versões 'swap noexcept' para os respectivos tipos a serem
    // operados. ver 'swap_*_like'.
    //
    // Por padrão, 'destructors' e funções responsáveis pela desalocação de
    // memória (tanto as definidas pelo usuário, como as automáticamente
    // geradas) são 'noexcept'.
    //
    // Por fim, deve-se ter cuidado e atenção ao se definir uma função como
    // 'noexcept' pois este se trata de um comprometimento de que tal objeto não
    // irá emitir exceções. Trata-se portanto de uma desisão de projeto e
    // arquitetura do programa que irá influênciar diretamente no comportamento
    // e em como tal função/biblioteca será utilizado.

    cout << endl;
};
}  // namespace item_14
