#include <boost/type_index.hpp>
#include <chrono>
#include <functional>
#include <iostream>
#include <list>
#include <memory>
#include <print>
#include <ranges>
#include <type_traits>
#include <unordered_map>
#include <variant>
#include <vector>

namespace item_27 {
using boost::typeindex::type_id_with_cvr;
using std::cout;
using std::endl;
using std::string;
using std::vector;
namespace rg = std::ranges;
namespace vw = std::views;

template <typename T, typename D = std::default_delete<T>>
using up = std::unique_ptr<T, D>;

template <typename T>
using sp = std::shared_ptr<T>;

template <typename T, typename... Args>
up<T> mu(Args&&... args) {
    return std::make_unique<T>(std::forward<Args>(args)...);
};

template <typename T, typename... Args>
sp<T> ms(Args&&... args) {
    return std::make_shared<T>(std::forward<Args>(args)...);
};

using std::to_string;

template <std::ranges::forward_range Rng>
auto stringify(Rng&& seq) {
    auto b = seq | vw::transform([](const auto& a) { return to_string(a); }) |
             vw::join_with(',') | vw::common;
    return "{" + std::string(std::begin(b), std::end(b)) + "}";
};

vector<string> names{};

// Implementação de 'Tag Dispatch':
//
vector<string> name_from_idx{
    "Dante",
    "Vergil",
    "Nero",
};
void log_and_add_impl(int idx, std::true_type) {
    auto now = std::chrono::system_clock::now();
    cout << std::format("name: {} | time: {}", name_from_idx[idx], now) << endl;
    names.emplace_back(name_from_idx[idx]);
}

template <typename T>
void log_and_add_impl(T&& name, std::false_type) {
    // cout << type_id_with_cvr<decltype(name)>().pretty_name() << endl;
    auto now = std::chrono::system_clock::now();
    cout << std::format("name: {} | time: {}", name, now) << endl;
    names.emplace_back(std::forward<T>(name));
}

// Uma função com a interface geral para o usuário é definida de tal forma que
// possa aceitar qualquer argumento informado. Na sua implemetação, esta função
// repassa o argumento para a função específica que será escolhida por meio de
// sobrecarga. É neste ponto que as 'tags' conduzirão o compilador na escolha da
// função desejada.
template <typename T>
void log_and_add(T&& name) {
    // o segundo argumento é uma 'tag' que retornará 'true' caso o argumento for
    // do tipo 'integral' e 'false' caso contrário:
    log_and_add_impl(std::forward<T>(name),
                     std::is_integral<std::remove_reference_t<T>>());
}

template <typename T, typename S>
concept UniversalRef = !std::is_base_of_v<S, std::decay_t<T>> &&
                       !std::is_integral<std::remove_reference_t<T>>();

class Foo {  // (1)
   public:
    // Faz-se uso de metaprogramação para condicionalmente desativar o
    // construtor com referência universal, caso o tipo do argumento de entrada
    // seja de um tipo indesejado ou necessite outra implementação mais
    // especializada:
    //
    // template <typename T, typename = std::enable_if_t<
    //                           !std::is_base_of_v<Foo, std::decay_t<T>> &&
    //                           !std::is_integral<std::remove_reference_t<T>>()>>
    // Foo(T&& name) : name{name} {
    //     // Pode-se realizar verificação prévia se o argumento informado pode
    //     ser
    //     // utilzado para a construção do atributo de tipo 'std::string':
    //     static_assert(
    //         std::is_constructible_v<std::string, T>,
    //         "Parâmetro 'name' não pode ser usado para a construção de "
    //         "variável de tipo 'std::string'.");
    //
    //     cout << "universal reference constructor" << endl;
    // };
    template <UniversalRef<Foo> T>
    Foo(T&& name) : name{name} {
        // Pode-se realizar verificação prévia se o argumento informado pode ser
        // utilzado para a construção do atributo de tipo 'std::string':
        static_assert(
            std::is_constructible_v<std::string, T>,
            "Parâmetro 'name' não pode ser usado para a construção de "
            "variável de tipo 'std::string'.");

        cout << "universal reference constructor" << endl;
    };

    // Construtor especializado para argumentos de tipo 'integral':
    Foo(int idx) : name{name_from_idx[idx]} {
        cout << "'int' specialized constructor" << endl;
    };

    Foo(const Foo& f) {
        cout << "copy constructor" << endl;
        name = f.name;
    }
    Foo(Foo&& f) {
        cout << "move constructor" << endl;
        name = std::move(f.name);
    }

    string get_name() { return name; }

   private:
    std::string name;
};

class DerivedFoo : public Foo {
   public:
    using Foo::Foo;
    DerivedFoo(const DerivedFoo& r) : Foo(r) {};
    DerivedFoo(DerivedFoo&& r) : Foo(std::move(r)) {};
};

void main() {
    // O uso de referências universais pode ser problemático quando se precisa
    // realizar sobrecarga com funções de tipos específicos, pois a referência
    // universal irá em vários casos inibir conversões de tipo implícitas
    // desejáveis. A seguir, são discutidas algumas formas de se lidar com essa
    // característica.
    //
    // A primeira alternativa trivial é não fazer uso de sobrecarga.
    // Sem sobrecarga, não há mais problemas com 'overload' errado.
    // Infelizmente, desta forma, para cada tipo diferente de argumentos, há
    // a necessidade de se criar uma nova função com um novo nome.
    //
    // Outra alternativa é abandonar a idéia de utilizar função template com
    // referência universal e empregar apenas função com argumento de entrada do
    // tipo 'const lvalue ref'. A desvantagem é que a implementação possa não
    // ser tão performática com a possibilidade de repassar os argumentos e
    // evitar operações de cópia, mas desta forma é possível fazer uso de
    // 'overloads' adicionais e melhor previsibilidade do comportamento geral.
    //
    // Quando há previsão da realização de operação de cópia nos argumentos, uma
    // possibilidade interessante é definir as funções com argumentos passados
    // por valor ('lvalue'). Passar por valor passa a ser interessante quando o
    // argumento for barato de se realizar a cópia e/ou movimentação. Ao se
    // realizar a cópia, tem-se um objeto independente dentro do escopo da
    // função, que pode ser livremente movimentado para outras variáveis.
    // Quando o tipo do argumento informado for um 'lvalue', então ocorre uma
    // operação de 'copy constructor', mas se o argumento for um 'rvalue', então
    // é realizada uma operação de 'move constructor'. Novamente, tem-se uma
    // melhor previsibilidade sobre o comportamento de sobrecarga.
    {
        // Mais uma possibilidade seria fazer uso de algo denominado como
        // 'tag dispatch':
        cout << endl;
        string outer_name{"Darlene"};

        log_and_add(
            outer_name);  // utilizará 'log_and_add_impl(T&& name) {...};'
        log_and_add(string{
            "Persephone"});     // utilizará 'log_and_add_impl(T&& name) {...};'
        log_and_add("Paty");    // utilizará 'log_and_add_impl(T&& name) {...};'
        log_and_add(2);         // utilizará 'log_and_add_impl(int idx) {...};'
        log_and_add(short{0});  // utilizará 'log_and_add_impl(int idx) {...};'
    };
    // Por fim, pode-se fazer uso de 'template metaprogramming' para uma
    // melhor condução do fluxo de execução (1):
    {
        cout << endl;
        cout << "Foo f1{\"Dante\"};" << endl;
        Foo f1{"Dante"};
        cout << "f1.get_name(): " << f1.get_name() << endl;
        cout << "Foo f2{2};" << endl;
        Foo f2{2};
        cout << "f2.get_name(): " << f2.get_name() << endl;
        cout << "DerivedFoo df1{\"Vergil\"};" << endl;
        DerivedFoo df1{"Vergil"};
        cout << "df1.get_name(): " << df1.get_name() << endl;
        cout << "DerivedFoo df2{df1};" << endl;
        DerivedFoo df2{df1};
        cout << "df2.get_name(): " << df2.get_name() << endl;
    };
};
}  // namespace item_27
