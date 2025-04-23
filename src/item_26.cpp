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
#include <utility>
#include <vector>

namespace item_26 {
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

void log_and_add_1(const string& name) {  // (1)
    auto now = std::chrono::system_clock::now();
    cout << std::format("name: {} | time: {}", name, now) << endl;
    names.emplace_back(name);
}

template <typename T>  // (2)
void log_and_add_2(T&& name) {
    // cout << type_id_with_cvr<decltype(name)>().pretty_name() << endl;
    auto now = std::chrono::system_clock::now();
    cout << std::format("name: {} | time: {}", name, now) << endl;
    names.emplace_back(std::forward<T>(name));
}

template <typename T>  // (2)
void log_and_add_3(T&& name) {
    // cout << type_id_with_cvr<decltype(name)>().pretty_name() << endl;
    auto now = std::chrono::system_clock::now();
    cout << std::format("name: {} | time: {}", name, now) << endl;
    names.emplace_back(std::forward<T>(name));
}

vector<string> name_from_idx{
    "Dante",
    "Vergil",
    "Nero",
};
void log_and_add_3(int idx) {
    auto now = std::chrono::system_clock::now();
    cout << std::format("name: {} | time: {}", name_from_idx[idx], now) << endl;
    names.emplace_back(name_from_idx[idx]);
}

class Foo {
   public:
    template <typename T>
    Foo(T&& name) : name{name} {
        cout << "universal reference constructor" << endl;
    };

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

   private:
    std::string name;
};

// class DerivedFoo : public Foo {
//    public:
//     // Os construtores de 'Foo', invocados dentro dos construtores 'copy' e
//     // 'move' de 'DerivedFoo', irão ser o 'default constructor' dado pela
//     // função template com referência universal. No caso, a função tentará
//     // instanciar o atributo 'nome' com o argumento de tipo 'DerivedFoo', o
//     // que resultará em erro:
//     DerivedFoo(const DerivedFoo& r) : Foo(r) {};
//     DerivedFoo(DerivedFoo&& r) : Foo(std::move(r)) {};
// };

void main() {
    // Deve-se ter um cuidado especial ao se tentar realizar sobrecarga em
    // funções que fazem uso de referência universal nos seus argumentos.
    {
        // inicialmente podemos supor a função (1), cujo argumento de entrada é
        // definido como 'const lvalue ref', e 3 chamadas possíveis:
        cout << endl;

        string outer_name{"Darlan"};

        log_and_add_1(
            outer_name);  // O parâmetro 'name' da função está atrelado à
                          // 'outer_name'. Já que 'name' é um 'const lvalue
                          // ref', este é copiado para dentro do vetor 'names'.
                          // Não é possível evitar essa operação de cópia, já
                          // que um valor 'lvalue' ('outer_name') foi passado
                          // para dentro de 'names'.

        log_and_add_1(string{
            "Persephone"});  // Agora 'name' está atrelado à uma váriável
                             // temporária 'rvalue' ('string{"Persephone"}').
                             // Como 'name' é um 'const lvalue ref', este também
                             // é copiado para dentro do vetor 'names'. Havendo
                             // a necessidade de criação de variável temporária,
                             // percebe-se aí uma possibilidade de se tentar
                             // definir uma operação de movimentação, ao invés
                             // de cópia.

        log_and_add_1(
            "Paty");  // Neste caso, há uma conversão implícita da variável de
                      // tipo 'const char *' ("Paty") para uma variável
                      // temporária do tipo 'std::string'. Logo após 'name' se
                      // atrela á esta variável temporária ('rvalue'), para
                      // posterior inserção no vetor 'names'. Novamente, essa
                      // operação de inserção se dá por cópia. Da mesma forma
                      // que com 'string{"Persephone"}', há aí uma possibilidade
                      // de se tentar definir uma operação de movimentação ao
                      // invés de cópia.
    };
    {
        // De forma análoga ao caso anterior, redefine-se a função 'log_and_add'
        // em (2) por meio de referência universal:
        cout << endl;

        string outer_name{"Darlene"};

        log_and_add_2(outer_name);  // Da mesma forma que em (1), 'name' é
                                    // deduzido como um 'lvalue ref' do tipo
                                    // 'std::string&' e, portanto, é copiado
                                    // para dentro de 'names'.

        log_and_add_2(string{
            "Persephone"});  // Agora 'name' é deduzido como um 'rvalue
                             // ref' do tipo 'std::string&&' e desta forma é
                             // movimentado para dentro de 'names'.

        log_and_add_2(
            "Paty");  // De forma similar, 'name' é deduzido como
                      // 'rvalue ref' do tipo 'const char (&) [5]' sendo
                      // repassado diretamente para o método '.emplace_back()'
                      // de 'names', que irá realizar a construção do objeto do
                      // tipo 'std::string' diretamente sem o uso de variáveis
                      // temporárias.
    };
    {
        cout << endl;

        string outer_name{"Darlene"};

        log_and_add_3(
            outer_name);  // utilizará 'log_and_add_3(T&& name) {...};'

        log_and_add_3(string{
            "Persephone"});  // utilizará 'log_and_add_3(T&& name) {...};'

        log_and_add_3("Paty");  // utilizará 'log_and_add_3(T&& name) {...};'

        log_and_add_3(2);  // utilizará 'log_and_add_3(int idx) {...};'

        // short name_idx{0};
        // log_and_add_3(
        //     name_idx);  // error: no matching function for call to
        //                 // ‘construct_at(std::__cxx11::basic_string<char>*&,
        //                 // short int&)’ | std::construct_at(__p,
        //                 // std::forward<_Args>(__args)...);
        //                 //
        //                 // Ou seja, embora os tipos 'short' e 'int' sejam
        //                 // tipos integrais, eles são diferentes e portanto
        //                 // a função template 'log_and_add_3' com referência
        //                 // universal acaba sendo escolhida para gerar a
        //                 // especialização para o tipo 'short'. Neste caso a
        //                 // presença de uma função template com referência
        //                 // universal inibe o mecanismo de conversão implícita
        //                 // de tipos ('short' para 'int').
    };
    // Percebe-se que as funções com argumentos definidos como referências
    // universais são as mais propensas a serem escolhidas para execução.
    // Justamente pelo fato do mecanismo de dedução de tipo conseguir produzir,
    // na grande maioria das vezes, a tipagem exata, inibindo outros mecanismos
    // (conversão implícita por exemplo).
    {
        // Mesmo problema ocorre no caso do uso de referências universais em
        // 'default constructors' de classes. Em muitas situações o 'default
        // constructor' com referência universal acaba sendo preferencialmente
        // escolhido ao invés dos outros construtores ('copy', 'move' e
        // 'overloads'), pois inibe conversões implícitas necessárias:
        cout << endl;
        Foo f1{"Vergil"};  // Default constructor ('Foo(T&& name);').
        Foo f2{2};         // 'int' specialized constructor ('Foo(int idx);').
        // Foo f3{f1};  // erro: Acaba chamando do 'default constructor' ao
        //              // invés do 'copy constructor'. O tipo real do argumento
        //              // informado é 'lvalue ref', enquanto que o 'copy
        //              // constructor' exige 'const lvalue ref'. Desta forma o
        //              // 'default constructor' acaba tendo a preferência por
        //              // ser capaz de deduzir uma versão exata para
        //              // 'lvalue ref', o que acaba resultando em erro
        //              // posteriormente.

        // É possível forçar o uso do 'copy constructor' adicionando a
        // qualificação de 'const' ao tipo do argumento informado:
        // Foo f3(const_cast<const decltype(f1)&>(f1));
        Foo f3(std::as_const(f1));
    };
    // Por fim, percebe-se que tentar realizar 'overload' em funções template
    // que possuem refências universais resulta na maioria das vezes com a
    // função template sendo escolhida com frequẽncia maior que a esperada.
    // Construtores com 'perefct-forward' são especialmente problemáticos pois
    // geralmente resultam em melhores 'matches' que 'copy constructors' para
    // 'lvalues' não constantes.
};
}  // namespace item_26
