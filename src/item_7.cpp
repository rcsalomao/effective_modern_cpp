#include <atomic>
#include <boost/type_index.hpp>
#include <functional>
#include <iostream>
#include <memory>
#include <print>
#include <ranges>
#include <unordered_map>
#include <vector>

namespace item_7 {
using boost::typeindex::type_id_with_cvr;
using std::cout;
using std::endl;
using std::vector;
using namespace std::ranges;

using std::to_string;

template <std::ranges::forward_range Rng>
auto stringify(Rng&& seq) {
    auto b = seq |
             views::transform([](const auto& a) { return std::to_string(a); }) |
             views::join_with(',') | views::common;
    return "{" + std::string(std::begin(b), std::end(b)) + "}";
}

class Widget {
   public:
    int v;
    Widget(int i) : v{i} {
        cout << "Variável Widget: Inicialização direta com valor: " << v
             << endl;
    };
    Widget(const Widget& w) : v{w.v} {
        cout << "Variável Widget: Inicialização por cópia com valor: " << v
             << endl;
    };
    Widget(const Widget&& w) : v{std::move(w.v)} {
        cout << "Variável Widget: Inicialização por movimentação com valor: "
             << v << endl;
    };
    Widget& operator=(const Widget& w) {
        v = w.v;
        cout << "Variável Widget: Atribuição por cópia com valor: " << v
             << endl;
        return *this;
    };
    Widget& operator=(const Widget&& w) {
        v = std::move(w.v);
        cout << "Variável Widget: Atribuição por movimentação com valor: " << v
             << endl;
        return *this;
    };

   private:
    // '{}' também pode ser utilizado para especificar inicialização padrão para
    // membros 'data' não estáticos. o que não é possível com a sintaxe '()'.
    int x{0};
    int y = 0;
    // int z(0);  // erro!
};

class Foo1 {
    int i{};
    bool b{};

   public:
    Foo1() { cout << "Foo: construtor default ()" << endl; };
    Foo1(int i) : i{i} { cout << "Foo: construtor (int i)" << endl; };
    Foo1(int i, bool b) : i{i}, b{b} {
        cout << "Foo: construtor (int i, bool b)" << endl;
    };
};

class Foo2 {
    int i{};
    bool b{};
    std::vector<double> vd;

   public:
    Foo2() { cout << "Foo: construtor default ()" << endl; };
    Foo2(int i) : i{i} { cout << "Foo: construtor (int i)" << endl; };
    Foo2(int i, bool b) : i{i}, b{b} {
        cout << "Foo: construtor (int i, bool b)" << endl;
    };
    Foo2(std::initializer_list<double> id) : vd{id} {
        cout << "Foo: construtor (std::initializer_list<double> id)" << endl;
    };
};

template <typename T, typename... Ts>
void do_some_work1(Ts&&... params) {
    T objeto_local(std::forward<Ts>(params)...);
    cout << stringify(objeto_local) << endl;
}

template <typename T, typename... Ts>
void do_some_work2(Ts&&... params) {
    T objeto_local{std::forward<Ts>(params)...};
    cout << stringify(objeto_local) << endl;
}

void main() {
    // C++ permite especificar diversas formas de inicialização de variáveis.
    // Ainda mais com a introdução da inicialização por '{}'.
    // alguns exemplos:
    cout << endl;
    Widget a(1);              // inicialização direta por '()'
    Widget b = 2;             // inicialização direta após '='
    Widget c{3};              // inicialização direta por '{}'
    Widget d = {4};           // inicialização direta utilizando '=' e '{}'
    Widget e{d};              // inicialização por cópia
    Widget f = d;             // inicialização por cópia
    f = c;                    // atribuição por cópia
    Widget g{std::move(b)};   // inicialização por movimentação
    Widget h = std::move(g);  // inicialização por movimentação
    h = std::move(a);         // atribuição por movimentação

    // dá-se o nome de 'inicialização universal' a sintaxe de inicialização por
    // '{}'. Esta permite uma sintaxe de inicialização mais uniforme e a novas
    // possibilidades:
    vector<int> v1{1, 2, 3, 4, 5};  // definição e inicialização de 'v1' no
                                    // mesmo ponto por meio de '{}'

    // Ainda, Inicialização por '{}' impede a conversão implícita entre os tipos
    // próprios:
    double x{};
    double y{};
    // int soma1{x + y};  // erro: type 'double' cannot be narrowed to 'int' in
    //                    // initializer list
    // já a inicialização por '()' ou '=' não faz a verificação de conversão
    // entre tipos:
    int soma2(x + y);   // sem erro; valor da expressão é truncado para um 'int'
    int soma3 = x + y;  // sem erro; valor da expressão é truncado para um 'int'

    // o uso de '{}' para inicialização também impede o incorreto parseamento de
    // uma expressão:
    double z1(10.0);  // definição de variável 'z1' do tipo double, inicializada
                      // com valor 10
    double z2();  // declaração de uma função de nome 'z2' que retorna um valor
                  // do tipo 'double'.
    double z3{};  // definição de uma variável 'z3' do tipo 'double' com valor
                  // 'default' ('default constructor').

    // Entretanto, o mecanismo de funcionamento por detrás de '{}'
    // ('std::initializar_list<T>') pode resultar em comportamentos inesperados:
    cout << endl;
    auto i = {1, 2, 3, 4};  // tipo de 'i': std::initializar_list<int>.
    Foo1 j1a;               // construtor padrão
    Foo1 j1b{};             // construtor padrão
    Foo1 j2a(3);            // construtor (int i)
    Foo1 j2b{3};            // construtor (int i)
    Foo1 j3a(2, true);      // construtor (int i, bool b)
    Foo1 j3b{2, true};      // construtor (int i, bool b)
    cout << endl;
    Foo2 j4a(3);        // construtor (int i)
    Foo2 j4b{3};        // construtor (std::initializar_list<double> id)
    Foo2 j5a(2, true);  // construtor (int i, bool b)
    Foo2 j5b{2, true};  // construtor (std::initializar_list<double> id)

    // o construtor '(std::initializar_list<T>)' acaba roubando a preferência no
    // mommento de resolução sobre qual construtor será invocado. mesmo quando
    // os tipos dos argumentos casam perfeitamente com qualquer outro
    // construtor, 'std::initializar_list<T>' acaba ganhando a preferência e
    // sendo invocado ainda que seja necessário realizar conversões de tipos.
    // Neste caso, os outros construtores serão considerados para resolução
    // somente quando não há a possibilidade de se utilizar o construtor
    // 'std::initializar_list<T>'.
    // Considerando as características deste mecanismo, deve-se prestar atenção
    // e cuidado com casos específicos, especialmente devido à interfaces e
    // decisões de design legado:
    vector<int> v2a(4, 20);  // criação de um vetor de tamanho 4, com seus
                             // respectivos itens inicializados com valor 20.
    vector<int> v2b{
        4, 20};  // criação de um vetor de tamanho 2, com seus itens
                 // inicializados com os valores '4' e '20', respectivamente.
    // outro ponto de atenção é na criação de funções genéricas 'template'. ver
    // as funções 'do_some_work1()' e 'do_some_work2()'. caso se deseje aplicar
    // a função 'do_some_work1()'/'do_some_work2()' à uma variável do tipo
    // 'std::vector', deve-se usar internamente inicialização do tipo '()' ou
    // '{}'?
    do_some_work1<std::vector<int>>(4, 20);
    do_some_work2<std::vector<int>>(4, 20);
};
}  // namespace item_7
