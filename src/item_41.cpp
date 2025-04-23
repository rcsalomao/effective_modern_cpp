#include <boost/type_index.hpp>
#include <iostream>
#include <memory>
#include <ranges>
#include <type_traits>
#include <vector>

namespace item_41 {
using boost::typeindex::type_id_with_cvr;
using std::cout;
using std::endl;
using std::string;
using std::vector;
using namespace std::literals::string_literals;
using namespace std::literals::string_view_literals;
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

std::string_view to_string(std::string_view s) { return s; }

template <std::ranges::forward_range Rng>
auto stringify(Rng&& seq) {
    auto b = seq | vw::transform([](const auto& a) { return to_string(a); }) |
             vw::join_with(',') | vw::common;
    return "{" + std::string(std::begin(b), std::end(b)) + "}";
};

class Foo {
   public:
    Foo() = default;
    Foo(Foo&& foo) noexcept {
        move_count++;
        x = foo.x;
    };
    Foo(const Foo& foo) {
        copy_count++;
        x = foo.x;
    };
    Foo& operator=(Foo&& foo) noexcept {
        move_count++;
        x = foo.x;
        return *this;
    };
    Foo& operator=(const Foo& foo) {
        copy_count++;
        x = foo.x;
        return *this;
    };
    ~Foo() = default;

    void print_counters() {
        cout << "copy_count: " << copy_count << "  "
             << "move_count: " << move_count << endl;
    }

    int x{0};

   private:
    int move_count{0};
    int copy_count{0};
};

class MoveOnlyFoo {
   public:
    MoveOnlyFoo() = default;
    MoveOnlyFoo(MoveOnlyFoo&& foo) noexcept {
        move_count++;
        x = std::move(foo.x);
    };
    MoveOnlyFoo(const MoveOnlyFoo& foo) = delete;
    MoveOnlyFoo& operator=(MoveOnlyFoo&& foo) noexcept {
        move_count++;
        x = std::move(foo.x);
        return *this;
    };
    MoveOnlyFoo& operator=(const MoveOnlyFoo& foo) = delete;
    ~MoveOnlyFoo() = default;

    void print_counters() {
        cout << "copy_count: " << copy_count << "  "
             << "move_count: " << move_count << endl;
    }

    int x{0};

   private:
    int move_count{0};
    int copy_count{0};
};

class Widget {
   public:
    // Duas funções que operam sob sobrecarga:
    void add_foo_1(const Foo& foo) { foos.push_back(foo); }
    void add_foo_1(Foo&& foo) { foos.push_back(std::move(foo)); }

    // Função 'template' baseada em referência universal:
    template <typename T>
    void add_foo_2(T&& foo) {
        foos.push_back(std::forward<T>(foo));
    }
    // Novamente, para argumentos informados do tipo 'lvalue' a dedução de tipo
    // gera 'T&' e, portanto, acaba resultando numa operação de cópia. Para
    // valores do tipo 'rvalue', têm-se como tipo deduzido 'T&&' e como
    // resultado uma operação de movimentaçao.

    // Função com parâmetro de entrada 'by-value':
    void add_foo_3(Foo foo) { foos.push_back(std::move(foo)); }
    // Neste caso, em tese, deveria-se ter para um argumento do tipo 'lvalue'
    // uma operação de cópia e para um argumento do tipo 'rvalue' uma operação
    // de movimentação, já que seria necessário primeiro construir o parãmetro
    // local. Entretanto, a depender de diversos fatores como compilador,
    // plataforma, arquitetura e contexto, há a possibilidade de realização de
    // processos de otimização e supressão de operações ('copy elision'). Neste
    // caso as operações iniciais de 'copy constructor' ou 'move constructor'
    // foram suprimidas. Por fim, têm-se como resultado apenas uma operação de
    // movimentação do parãmetro 'foo' para dentro do vetor interno.

    void add_foo_3(MoveOnlyFoo move_only_foo) {
        move_only_foos.push_back(std::move(move_only_foo));
    }
    // Novamente, em tese deveriam ter ocorrido 2 operações de movimentação (uma
    // para a construção do parãmetro local e outra para a operação de
    // 'push_back'). Entretanto parece que o compilador está realizando
    // operações de otimização e inibindo determinadas instanciações.

    vector<Foo> foos;
    vector<MoveOnlyFoo> move_only_foos;
};

void main() {
    // No contexto de submissão de argumentos para funções, há diversas
    // possibilidades e consequências. Neste sentido é interessante realizar uma
    // análise mais detalhada sobre os processos de cópia, movimentação e
    // construção dos objetos envolvidos. A seguir estão alguns exemplos e
    // contagens de quantas operações de cópia e movimentação são realizadas
    // para cada definição das funções 'add_foo_i':
    {
        // Para o caso das funções sob sobrecarga 'const lvalue ref' e 'rvalue
        // ref':
        cout << endl;
        Widget w{};
        Foo foo{};

        w.add_foo_1(foo);
        w.foos[0].print_counters();
        // copy_count: 1  move_count: 0

        w.add_foo_1(Foo{});
        w.foos[1].print_counters();
        // copy_count: 0  move_count: 1
    };
    {
        // Para o caso da função template com referência universal:
        cout << endl;
        Widget w{};
        Foo foo{};

        w.add_foo_2(foo);
        w.foos[0].print_counters();
        // copy_count: 1  move_count: 0

        w.add_foo_2(Foo{});
        w.foos[1].print_counters();
        // copy_count: 0  move_count: 1
    };
    {
        // Para o caso da função com parâmeto com tipo 'by-value':
        cout << endl;
        Widget w{};
        Foo foo{};

        w.add_foo_3(foo);
        w.foos[0].print_counters();
        // copy_count: 0  move_count: 1

        w.add_foo_3(Foo{});
        w.foos[1].print_counters();
        // copy_count: 0  move_count: 1

        // Provavelmente o compilador está realizando operações de otimização
        // ('copy elision').
    };
    {
        // Para o caso da função com parâmeto com tipo 'by-value' e objeto do
        // tipo 'move-only':
        cout << endl;
        Widget w{};
        MoveOnlyFoo foo{};

        // w.add_foo_3(
        //     foo);  // erro: 'foo', por ser do tipo 'MoveOnlyFoo', não
        //            // possui construtor e atribuidor de cópia. Desta forma
        //            // não há função que consiga atender o requisito definido.
        w.add_foo_3(std::move(foo));
        w.move_only_foos[0].print_counters();
        // copy_count: 0  move_count: 1

        w.add_foo_3(MoveOnlyFoo{});
        w.move_only_foos[1].print_counters();
        // copy_count: 0  move_count: 1

        // Provavelmente o compilador está realizando operações de otimização
        // ("move elision").
    };

    // Percebe-se pelos resultados dos 3 grupos de funções que é interessante
    // passar argumentos por valor quando já se espera a ocorrência de operações
    // de cópia no mesmo e estes objetos sejam baratos de se mover.
    // Adicionalmente, pode-se ainda ser agraciado por operações de otimização
    // pelo compilador ('copy elision').

    // É possível elencar como desvantagens de parâmetros 'by-value':
    //
    // - Inabilidade de lidar com objetos que não possuem 'copy construtor' e/ou
    // 'copy assignment', já que argumentos 'by-value' exigem a presença de
    // operadores de cópia para funcionar.
    //
    // - É interessante apenas para parâmetros que são baratos para se mover, já
    // que geralmente (a menos que o compilador realize operações de otimização)
    // funções com parâmetros 'by-value' tendem a realizar mais operações de
    // movimentação no objeto.
    //
    // - É interessante apenas em situações que obrigatoriamente exigem a cópia
    // dos parâmetros. Do contrário, o informe de parâmetros por referência
    // passa a ser mais vantajoso.
    //
    // - Na situação de realização de atribuição dentro de uma função,
    // parâmetros passados 'by-value' podem acabar gerando alocação e
    // desalocação de memória adicionais, em comparação com parâmetros por
    // referência.
    //
    // Adicionalmente, deve-se ter em mente que estes custos adicionais podem ir
    // se compondo ao longo da cadeia de funções e objetos sendo construídos e
    // manipulados durante todo o tempo de vida do programa.
    //
    // Por fim, a informação de argumentos para funções 'by-value' ainda pode
    // acarretar em problemas sérios como 'slicing' de tipos definidos pelo
    // usuário.
};
}  // namespace item_41
