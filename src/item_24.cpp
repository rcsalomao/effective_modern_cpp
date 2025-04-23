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

namespace item_24 {
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

class Widget {};

template <typename T>
void f(T&& param) {
    cout << type_id_with_cvr<decltype(param)>().pretty_name() << endl;
};  // Não é 'rvalue ref' (Ocorrência de dedução de tipo e
    // 'reference collapsing').

template <typename T>
void f2(std::vector<T>&& param) {
    cout << type_id_with_cvr<decltype(param)>().pretty_name() << endl;
};  // É 'rvalue ref' (não há dedução de tipo, nem de 'reference collapsing').

template <typename T>
void f3(const T&& param) {
    cout << type_id_with_cvr<decltype(param)>().pretty_name() << endl;
};  // É 'rvalue ref'. A presença de 'const' já é o suficiente para inibir o
    // mecanismo de dedução de tipo.

template <class T>
class Vector {
   public:
    Vector() {};
    void push_back(T&&) {
    };  // É 'rvalue ref', já que não há a presença do mecanismo de type
        // deduction para o método 'push_back' específicamente.
    template <class... Args>
    void emplace_back(Args&&... args) {
    };  // É 'universal ref', já que se trata de um método template, com a
        // presença do mecanismo de dedução de tipo. Os tipos dos parãmetros
        // 'args' são independentes do tipo 'T' da classe 'Vector<T>'. Todo
        // processo de invocação do método 'emplace_back' resulta na necessidade
        // de se deduzir o tipo dos argumentos 'args'.
};

void main() {
    // Até certo ponto, é interessante criar o conceito de 'referência
    // universal', distinguível de 'referência rvalue', para melhor entendimento
    // do código e do fluxo de informação resultante.
    {
        cout << endl;
        auto f = [](Widget&&) {};  // É 'rvalue ref' (não há dedução de tipo,
                                   // nem de 'reference collapsing').
        Widget&& var1 = Widget();  // É 'rvalue ref' (não há dedução de tipo,
                                   // nem de 'reference collapsing').
        auto&& var2 = var1;  // Não é 'rvalue ref' (Ocorrência de dedução de
                             // tipo e 'reference collapsing').
    };
    // O principal ponto de distinção entre 'rvalue ref' e 'universal ref' é se
    // há a presença do mecanismo de dedução de tipo, seja por meio de
    // 'templates' ou 'auto', e da ocorrência de 'reference collapsing'.
    //
    // De fato, 'T&&' ou 'auto&&' podem ser tanto 'rvalue ref' OU 'lvalue ref',
    // a depender do mecanismo de dedução de tipo e de 'reference collapsing'.
    // 'T&&' e 'auto&&' são justamente os dois contextos em que a referência
    // universal surge e é o inicializador para tal referência que determinará
    // de qual tipo esta será ('rvalue' ou 'lvalue').
    //
    // Já nos casos de 'Widget&&' e 'std::vector<T>&&' não há necessidade de se
    // realizar dedução de tipos, e portanto, tratam-se apenas de 'rvalue ref'.

    {
        cout << endl;
        cout << "Widget w;" << endl;
        Widget w;
        cout << "f(w): ";
        f(w);
        cout << "f(std::move(w)): ";
        f(std::move(w));
        cout << "f(Widget{}): ";
        f(Widget{});
    };
    {
        cout << endl;
        cout << "std::vector<Widget> vw;" << endl;
        std::vector<Widget> vw;
        // f2(vw);  // error: cannot bind rvalue reference of type
        //          // ‘std::vector<item_24::Widget>&&’ to lvalue of type
        //          // ‘std::vector<item_24::Widget>’
        cout << "f2(std::move(vw)): ";
        f2(std::move(vw));
    };
    {
        cout << endl;
        cout << "Widget w;" << endl;
        Widget w;
        // f3(w);  // error: cannot bind rvalue reference of type ‘const
        //         // item_24::Widget&&’ to lvalue of type ‘item_24::Widget’
        cout << "f3<Widget&>(w): ";
        f3<Widget&>(w);
        cout << "f3(std::move(w)): ";
        f3(std::move(w));
    };
    {
        cout << endl;
        Widget w;
        Vector<Widget> vw;
        // vw.push_back(w);  // erro: rvalue reference to type 'item_24::Widget'
        //                   // cannot bind to lvalue of type 'Widget'
        vw.push_back(std::move(w));
    };
    {
        // Ainda, é possível definir funções lambda com referências universais:
        cout << endl;
        cout << "auto g = [](auto&& func, auto&&... args) {\n"
                "std::forward<decltype(func)>(func)("
                "std::forward<decltype(args)>(args)...);\n};"
             << endl;
        auto g = [](auto&& func, auto&&... args) {
            std::forward<decltype(func)>(func)(
                std::forward<decltype(args)>(args)...);
        };  // invocação de 'func' com os argumentos 'args'.

        cout << "g(f3<Widget>, Widget{}): ";
        g(f3<Widget>, Widget{});
        cout << "Widget w;" << endl;
        Widget w;
        cout << "g(f3<Widget&>, w): ";
        g(f3<Widget&>, w);
        cout << "g(f3<Widget>, std::move(w)): ";
        g(f3<Widget>, std::move(w));
    };
};
}  // namespace item_24
