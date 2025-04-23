#include <boost/type_index.hpp>
#include <concepts>
#include <functional>
#include <iostream>
#include <list>
#include <memory>
#include <print>
#include <ranges>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace item_25 {
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

class Widget {
   public:
    Widget() {};

    // 'rvalue ref':
    Widget(Widget&& other)
        : name(std::move(other.name)), p(std::move(other.p)) {}

    // template <typename T>
    //     requires std::is_convertible_v<T, string>
    template <std::convertible_to<std::string_view> T>
    void forward_set_name(T&& new_name) {
        cout << type_id_with_cvr<decltype(new_name)>().pretty_name() << endl;
        name = std::forward<T>(new_name);
    }

    template <std::convertible_to<std::string_view> T>
    void move_set_name(T&& new_name) {
        cout << type_id_with_cvr<decltype(new_name)>().pretty_name() << endl;
        name = std::move(
            new_name);  // cuidado! Caso a dedução de tipo resulte que
                        // 'new_name' seja uma 'lvalue ref', então 'std::move'
                        // irá alterar a varíavel correspondente ao argumento,
                        // movimentando seu recurso e possívelmente gerando UB.
    }

    void set_name_2(const std::string& new_name) {
        cout << "set_name_2(const std::string& new_name);" << endl;
        name = new_name;
    }
    void set_name_2(std::string&& new_name) {
        cout << "set_name_2(std::string&& new_name);" << endl;
        name = std::move(new_name);
    }

    string get_name() { return name; }

   private:
    string name;
    std::shared_ptr<string> p;
};

struct Foo {
    int x;

    Foo(int i) : x{i} { cout << "default constructor" << endl; };

    Foo(const Foo& f) {
        cout << "copy constructor" << endl;
        x = f.x;
    }
    Foo& operator=(const Foo& f) {
        cout << "copy assignment" << endl;
        x = f.x;
        return *this;
    }

    Foo(Foo&& f) {
        cout << "move constructor" << endl;
        x = std::move(f.x);
    }
    Foo& operator=(Foo&& f) {
        cout << "move assignment" << endl;
        x = std::move(f.x);
        return *this;
    }
};

template <typename T>
T pass_foo(T&& f) {
    cout << type_id_with_cvr<decltype(f)>().pretty_name() << endl;
    return std::forward<T>(f);
}

Foo make_foo_1(int i) {
    Foo f{i};
    return f;  // cópia de 'f' para o valor de retorno. De forma geral os
               // compiladores mais proeminentes tentarão realizar 'RVO' para
               // suprimir esta operação de cópia ('copy elision'), fazendo com
               // que o objeto já seja construído no local de memória de
               // destino. Caso ainda assim não seja possível realizar RVO,
               // então a última instrução serão tratada como se fosse 'return
               // std::move(f);'.
}

Foo make_foo_2(int i) {
    Foo f{i};
    return std::move(f);  // warn: moving a local object in a return statement
                          // prevents copy elision. A instrução de movimentação
                          // por meio de 'std::move' impede com que o compilador
                          // possa realizar 'copy elision'.
}

Foo make_foo_3(Foo f) {
    f.x = 42;
    return f;  // A implementação desta função não é cadidata para RVO,
               // entretanto a última instrução será tratada como se fosse:
               // 'return std::move(f);'.
}

void main() {
    // A definição de argumentos de tipo 'Widget&&', por exemplo, indica que tal
    // argumento é elegível para operação de movimentação. Adicionalmente,
    // exprime uma intenção no uso de tal objeto. Ver 'move constructor' de
    // 'Widget'.
    //
    // De forma análoga, a definição de argumento do tipo 'T&&' numa função
    // template, por exemplo, indica que tal argumento se trata de uma
    // referência universal, podendo resultar numa 'rvalue ref' ou 'lvalue ref',
    // a depender do inicializador utilizado. Ver o método 'set_name' da classe
    // 'Widget'.
    //
    // Desta maneira, ao se tratar com 'rvalues', deve-se fazer uso da função
    // 'std::move' para realizar o 'cast' incondicional do objeto para 'rvalue'
    // e, ao se tratar de referências universais, deve-se utilizar a função
    // 'std::forward<T>' para realizar o 'cast' condicional do objeto para
    // 'lvalue ref' ou 'rvalue ref'.
    //
    // Ainda, deve-se evitar utilizar a função 'std::move' sobre referências
    // universais, pois é bem possível acabar realizando operação de
    // movimentação sobre uma 'lvalue ref', o que resultaria em problemas
    // futuros graves.
    //
    {
        cout << endl;
        cout << "std::string name = Ariele;" << endl;
        string name = "Ariele";
        cout << "Widget w;" << endl;
        Widget w;
        cout << "w.forward_set_name(name);" << endl;
        w.forward_set_name(name);
        cout << "w.get_name(): " << w.get_name() << endl;
        cout << "name: " << name << endl;
    };
    {
        cout << endl;
        cout << "Widget w;" << endl;
        Widget w;
        cout << "w.forward_set_name(string{\"Vergil\"});" << endl;
        w.forward_set_name(string{"Vergil"});
        cout << "w.get_name(): " << w.get_name() << endl;
    };
    {
        cout << endl;
        cout << "Widget w;" << endl;
        Widget w;
        cout << "w.move_set_name(\"Dante\");" << endl;
        w.move_set_name("Dante");
        cout << "w.get_name(): " << w.get_name() << endl;
    };
    {
        cout << endl;
        cout << "std::string name = Fulano;" << endl;
        string name = "Fulano";
        cout << "Widget w;" << endl;
        Widget w;
        cout << "w.move_set_name(name);" << endl;
        w.move_set_name(name);
        cout << "w.get_name(): " << w.get_name() << endl;
        cout << "name: " << name
             << endl;  // name == "". A invocação do método 'move_set_name'
                       // realizou uma operação de movimentação num objeto de
                       // tipo 'lvalue ref', modificando a variável 'nome' de
                       // escopo superior empregada como argumento do método.
    };
    {
        cout << endl;
        cout << "Foo _ = pass_foo(Foo{4});" << endl;
        Foo _ = pass_foo(Foo{4});
    };
    {
        cout << endl;
        cout << "Foo f{4};" << endl;
        Foo f{4};
        cout << "Foo _ = pass_foo(f);" << endl;
        Foo _ = pass_foo(f);
    };
    {
        cout << endl;
        cout << "Widget w;" << endl;
        Widget w;
        // w.set_name_2(
        //     "Algum Nome");  // Neste caso há ainda a conversão implícita de
        //                     // 'const char*' (ou 'literal string') para o
        //                     // tipo 'std::string' definido como argumento
        //                     // de entrada na função. Desta forma há a
        //                     // alocação e criação de objeto temporário para
        //                     // posterior 'bind' do 'rvalue ref'.
        cout << "w.set_name_2(string{\"Arandela\"});" << endl;
        w.set_name_2(
            string{"Arandela"});  // Entretanto, este overload acaba
                                  // implícitamente criando um objeto temporário
                                  // para posterior movimentação/atribuição e
                                  // realizando sua destruíção posteriomente.
                                  // Resultando em operações de criação e
                                  // destruição adicionais em comparação com a
                                  // implementação com referência universal.
        cout << "w.get_name(): " << w.get_name() << endl;
    };
    // Em alguns casos, a estratégia de abandonar o uso de referência universal
    // e realizar a implementação apenas por meio de sobrecarga com 'const
    // lvalue ref' e 'rvalue ref' não é sustentável. Desta forma seriam
    // necessárias 2^n implementações para contemplar os 'n' argumentos
    // informados para cada função. Para funções com número significativo de
    // argumentos, o uso de referência universal é basicamente o único tipo de
    // implementação viável.
    //
    // Ainda, o uso de 'std::move' pode ser prejudicial nas situações em que há
    // a possibilidade do compilador efetuar 'copy elision'. Tal procedimento de
    // otimização (Return Value Optimization - RVO) está presente, pelo menos,
    // nos compiladores mais proeminentes. Trata-se da possibilidade de, no
    // momento de retorno de um objeto de uma função, suprimir a operação de
    // cópia ('copy elision') e já realizar a construção/retorno do objeto no
    // local de memória de destino, evitando-se assim operações de cópia e
    // movimentação. Entretanto, se por acaso for feita instrução no código para
    // realizar 'std::move' sobre o valor de retorno, então o compilador não
    // terá a possiblidade de realizar o procedimento de 'copy elision', desta
    // forma tornando o executável menos performático.
    {
        cout << endl;
        cout << "Foo f = make_foo_1(24);" << endl;
        Foo f = make_foo_1(24);
        cout << "f.x: " << f.x << endl;
    };
    {
        cout << endl;
        cout << "Foo f{0};" << endl;
        Foo f{0};
        cout << "f = make_foo_1(24);" << endl;
        f = make_foo_1(24);
        cout << "f.x: " << f.x << endl;
    };
    {
        cout << endl;
        cout << "Foo f = make_foo_2(36);" << endl;
        Foo f = make_foo_2(36);
        cout << "f.x: " << f.x << endl;
    };
    {
        cout << endl;
        cout << "Foo f{0};" << endl;
        Foo f{0};
        cout << "f = make_foo_2(36);" << endl;
        f = make_foo_2(36);
        cout << "f.x: " << f.x << endl;
    };
    {
        cout << endl;
        cout << "Foo f = make_foo_3(0);" << endl;
        Foo f = make_foo_3(0);
        cout << "f.x: " << f.x << endl;
    };
    {
        cout << endl;
        cout << "Foo f = make_foo_3(Foo{0});" << endl;
        Foo f = make_foo_3(Foo{0});
        cout << "f.x: " << f.x << endl;
    };
    {
        cout << endl;
        cout << "Foo f0{0};" << endl;
        Foo f0{0};
        cout << "Foo f1 = make_foo_3(f0);" << endl;
        Foo f1 = make_foo_3(f0);
        cout << "f1.x: " << f1.x << endl;
    };
};
}  // namespace item_25
