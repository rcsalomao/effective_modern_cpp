#include <atomic>
#include <boost/type_index.hpp>
#include <functional>
#include <iostream>
#include <list>
#include <memory>
#include <mutex>
#include <print>
#include <ranges>
#include <syncstream>
#include <thread>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace item_16 {
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
};

class Foo1 {
   public:
    using A = std::vector<double>;

    A calc_a() const {
        // implementação de uma estratégia de 'caching' para evitar o recálculo
        // desnecessário de 'A'
        if (!is_a_valid) {
            cout << "Calculando o valor de 'a'.\n";
            // ... cálculo do valor de 'a':
            a = vw::iota(3, 6) | rg::to<vector<double>>();
            // ... settando a flag para evitar o recálculo de 'a':
            is_a_valid = true;
        }
        return a;
    };

   private:
    // qualificação 'mutable' para que se possa realizar alteração destas
    // variáveis dentro do escopo da função 'const'.
    mutable bool is_a_valid{false};
    mutable A a{};
};
// O problema de 'Foo1' é que num cenário em que diversas 'theads' estejam
// invocando o mesmo método 'calc_a()' de um mesmo objeto 'Foo1', pode-se ter a
// ocorrência de 'data race', com as variáveis 'is_a_valid' e 'a'.
// Desta forma, deve-se tomar cuidado e realizar o correto gerenciamento destas
// variáveis. Uma alternativa seria fazer uso de um 'mutex' para garantir que
// apenas uma 'thread' por vez irá realizar a verificação e o cálculo das
// referidas variáveis:
class Foo2 {
   public:
    using A = std::vector<double>;

    A calc_a() const {
        std::scoped_lock lock{
            m};  // Obtendo uma 'lock' por meio de um 'mutex' deste escopo para
                 // que se possa trabalhar independentemente.
        if (!is_a_valid) {
            cout << "Calculando o valor de 'a'.\n";
            // ... cálculo do valor de 'a'.
            a = vw::iota(3, 6) | rg::to<vector<double>>();
            is_a_valid = true;
        }
        return a;
    };

   private:
    // qualificação 'mutable' para que se possa realizar alteração destas
    // variáveis dentro do escopo da função 'const'.
    mutable bool is_a_valid{false};
    mutable A a{};
    mutable std::mutex
        m;  // Adicionalmente, 'std::mutex' é um tipo 'move-only' (variável que
            // não possui operação de cópia, apenas de movimentação) e portanto
            // 'Foo2' passa a ser também uma classe 'move-only', perdendo sua
            // capacidade de cópia.
};

void main() {
    //  Mesmo em contexto de função ou método 'const', deve-se ter cuidado
    //  quanto à condições de 'data race'. Quando se está realizando operações
    //  de leitura e escrita num contexto de 'multithreading', a qualificação de
    //  'const' no escopo não impede a ocorrência de 'data race' em variáveis
    //  mutáveis.
    {
        cout << endl;
        Foo1 foo{};
        auto f = [](Foo1& obj) {
            std::osyncstream out{cout};
            out << stringify(obj.calc_a()) << endl;
        };
        std::jthread t1{f, std::ref(foo)};
        std::jthread t2{f, std::ref(foo)};
        std::jthread t3{f, std::ref(foo)};
        std::jthread t4{f, std::ref(foo)};
        std::jthread t5{f, std::ref(foo)};
        std::jthread t6{f, std::ref(foo)};
        std::jthread t7{f, std::ref(foo)};
    };
    {
        cout << endl;
        Foo2 foo{};
        auto f = [](Foo2& obj) {
            std::osyncstream out{cout};
            out << stringify(obj.calc_a()) << endl;
        };
        std::jthread t1{f, std::ref(foo)};
        std::jthread t2{f, std::ref(foo)};
        std::jthread t3{f, std::ref(foo)};
        std::jthread t4{f, std::ref(foo)};
        std::jthread t5{f, std::ref(foo)};
        std::jthread t6{f, std::ref(foo)};
        std::jthread t7{f, std::ref(foo)};
    };
    // No mesmo sentido, deve-se fortemente evitar o uso de mais de uma variável
    // 'std::atomic<T>' por contexto de 'thread'. Pode-se acabar realizando
    // 'data race' entre as atribuições das diversas variáveis 'std::atomic<T>'
    // dentre as threads concomitantes.
};
}  // namespace item_16
