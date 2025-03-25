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

namespace item_12 {
using boost::typeindex::type_id_with_cvr;
using std::cout;
using std::endl;
using std::string;
using std::vector;
namespace rg = std::ranges;
namespace vw = std::views;

using std::to_string;

template <std::ranges::forward_range Rng>
auto stringify(Rng&& seq) {
    auto b = seq | vw::transform([](const auto& a) { return to_string(a); }) |
             vw::join_with(',') | vw::common;
    return "{" + std::string(std::begin(b), std::end(b)) + "}";
}

// Neste caso, nenhum dos métodos definidos em 'Derived1' fazem 'override' nos
// métodos de 'Base', pois não cumprem as 6 condições necessárias:
class Base1 {
   public:
    virtual void mf1() const { cout << "Base" << endl; };
    virtual void mf2(int) { cout << "Base" << endl; };
    virtual void mf3() & { cout << "Base" << endl; };
    void mf4() { cout << "Base" << endl; };
    virtual ~Base1() = default;
};
class Derived1 : public Base1 {
   public:
    virtual void mf1() { cout << "Derived" << endl; };  // método não é 'const'
    virtual void mf2(unsigned int) {
        cout << "Derived" << endl;
    };  // diferença nos tipos dos parâmetros de entrada 'int' != 'unsigned int'
    virtual void mf3() && { cout << "Derived" << endl; };  // '&' != '&&'
    void mf4() {
        cout << "Derived" << endl;
    };  // ambos métodos não são definidos como 'virtual'
};

// para que se possa realizar 'override', as 6 condições precisam ser atendidas:
class Base2 {
   public:
    virtual void mf1() const { cout << "Base" << endl; };
    virtual void mf2(int) { cout << "Base" << endl; };
    virtual void mf3() & { cout << "Base" << endl; };
    virtual void mf4() { cout << "Base" << endl; };
    virtual ~Base2() = default;
};
class Derived2 : public Base2 {
   public:
    virtual void mf1() const { cout << "Derived" << endl; };
    virtual void mf2(int) { cout << "Derived" << endl; };
    virtual void mf3() & { cout << "Derived" << endl; };
    // virtual void mf4() { ... };
    void mf4() {
        cout << "Derived" << endl;
    };  // Não há problema em identificar como 'virtual'.
        // É apenas necessário para o método da classe 'Base2'.
};

class Foo {
   public:
    Foo() { cout << "Foo default constructor" << endl; }
    Foo(const Foo&) { cout << "Foo copy constructor" << endl; }
    Foo& operator=(const Foo&) {
        cout << "Foo copy assignment" << endl;
        return *this;
    }
    Foo(Foo&&) { cout << "Foo move constructor" << endl; }
    Foo& operator=(Foo&&) {
        cout << "Foo move assignment" << endl;
        return *this;
    }
};

class Widget {
   public:
    using DataType = Foo;
    Widget() { cout << "Widget default constructor" << endl; };

    DataType& data() & {
        cout << "DataType& data() & {...}" << endl;
        return value;
    }
    DataType data() && {
        cout << "DataType data() && {...}" << endl;
        return std::move(value);
    }

   private:
    DataType value{};
};

void main() {
    // Para que um método de uma classe derivada possa fazer 'override' num
    // método de uma classe base 6 condições precisam ser obedecidas:
    // - o método da classe base precisa ser 'virtual'.
    //
    // - nome dos métodos da classe base e derivada devem ser idênticos (exceto
    // para o caso de 'destructors').
    //
    // - os tipos dos parâmetros de ambos os métodos devem ser idênticos.
    //
    // - os atributos de 'const' de ambos os métodos devem ser idênticos.
    //
    // - o tipos dos valores de retornos de ambos os métodos devem ser
    // compatíveis.
    //
    // - para ambos os métodos, os qualificadores de referência devem ser
    // idênticos.

    cout << endl;
    // std::unique_ptr<Base1> b2 = std::make_unique<Derived1>();
    Base1* b1 = new Derived1{};
    cout << "Base1* b1 = new Derived1{}; b1->mf1(): " << endl;
    b1->mf1();

    cout << endl;
    Base2* b2 = new Derived2{};
    cout << "Base2* b2 = new Derived2{}; b2->mf1(): " << endl;
    b2->mf1();

    // outra consequência ao se utilizar 'override' é no momento de se
    // reconhecer a extensão do impacto ao se realizar alterações na interface
    // dos métodos da classe base. Ao se realizar alguma mudança na interface
    // do método, o compilador já irá emitir avisos de erro para cada classe
    // derivada que falhar em dar 'override' na classe base.

    // As palavras chave 'override' e 'final' são contextuais. Possuem sentido
    // na linguagem apenas após a definição da assinatura de uma função. Outro
    // lugar não possui nenhum significado especial. Métodos e funções com nome
    // de 'override' continuam inafetados.

    // Ainda sobre os qualificadores de referência para funções membro, estes
    // permitem realizar distinções e especificações sobre qual método será
    // invocado a depender do tipo do objeto '*this'.

    // O objeto 'w' de tipo 'Widget' é um 'l-value' com nome e endereço de
    // memória. Logo, faz uso do método '.data() &':
    cout << endl;
    cout << "Widget w{}: " << endl;
    Widget w{};
    cout << "auto vals1 = w.data(): " << endl;
    auto vals1 = w.data();

    // O objeto de tipo 'Widget' é um 'r-value', temporário e sem nome. Logo,
    // faz uso do método '.data() &&':
    cout << endl;
    cout << "auto vals2 = Widget{}.data(): " << endl;
    auto vals2 = Widget{}.data();
};
}  // namespace item_12
