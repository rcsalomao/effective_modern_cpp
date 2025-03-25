#include <atomic>
#include <boost/type_index.hpp>
#include <functional>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <print>
#include <ranges>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace item_17 {
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

class Foo {
   public:
    // constructor
    Foo() = default;

    // copy constructor
    Foo(const Foo& other) = default;  // 'default' indica que é para utilizar a
                                      // implementação padrão para a operação
    // copy assignment
    Foo& operator=(const Foo& other) = default;

    // move constructor
    Foo(Foo&& other) = default;
    // move assignment
    Foo& operator=(Foo&& other) = default;

    // destructor
    ~Foo() = default;
};

class Bar1 {
   public:
    Bar1() = default;

   private:
    std::map<int, std::string> values;
};  // Nesta classe, os métodos especiais são implícitamente criados ('copy',
    // 'move' e 'destructor'), permitindo a realização das respectivas operações
    // caso necessárias.

class Bar2 {
   public:
    Bar2() = default;
    ~Bar2() = default;

   private:
    std::map<int, std::string> values;
};  // Entretanto, a definição do 'destructor' '~Bar2()' também impede a criação
    // implícita das operações de movimentação ('move constructor' e 'move
    // assignment'), o que pode gerar grande impacto de performance. Operações
    // que anteriormente resultavam na movimentação do recurso, agora resulta na
    // cópia do recurso que a classe tem posse.

class Spam {
    template <typename T>
    Spam(const T&){};

    template <typename T>
    Spam& operator=(const T&) {
        return *this;
    };
};  // a presença de métodos 'template' não impede a criação implícita dos
    // métodos especiais, pois não o são.

void main() {
    // À partir de C++11, o compilador, quando se faz necessário e a depender de
    // determinadas condições, é capaz de criar métodos especiais para prover
    // funcionalidades de 'destructor', 'copy constructor', 'copy assignment',
    // 'move constructor', 'move assignment'.
    // As interfaces de tais métodos estão demonstradas na classe 'Foo'.
    // Tais métodos são implícitamente 'public' e 'inline', além de não serem de
    // natureza 'virtual', com a excessão do 'destructor' de uma classe derivada
    // de uma classe base com método 'destructor' virtual.
    // Tais métodos realizam a cópia, movimentação ou destruição de cada um dos
    // atributos não estáticos do objeto, assim como também dos atributos da
    // classe base. Caso a operação de movimentação não seja suportada para
    // algum destes atributos, será realizada a operação de cópia no seu lugar.
    //
    // Dependendo de certas condições, a criação implícita destas operações é
    // desativada:
    // - As operações de cópia são independentes entre si ('copy constructor' e
    // 'copy assignment'). A definição de uma, não impede a construção implícita
    // da outra.
    //
    // - As operações de movimentação são dependentes entre si ('move
    // constructor' e 'move assignment'). A definição de uma, acaba impedindo a
    // construção implícita da outra.
    //
    // - A definição de pelo menos uma das operações de cópia ('constructor' ou
    // 'assignment') irá impedir a construção implícita das operações de
    // movimentação ('constructor' e 'assignment').
    //
    // - De forma análoga, a definição de pelo menos uma operação de
    // movimentação ('move constructor' ou 'move assignment') irá impedir a
    // criação implícita de ambas operações de cópia ('copy constructor' e 'copy
    // assignment').
    //
    // - a definição de um 'destructor' específico para a classe, impede a
    // criação implícita dos operações de movimentação ('move constructor' e
    // 'move assignment').
    //
    // - a definição de um 'destructor' específico para a classe NÃO impede a
    // criação implícita dos operações de cópia ('copy constructor' e 'copy
    // assignment').
    // Portanto, caso a classe necessite de implementação própria de quaisquer
    // um dos 'copy constructor', 'copy assignment' ou 'destructor',
    // constitui-se como boa prática a definição dos 3 ('Rule Of Three').
    //
    // De forma resumida, as operações de movimentação serão
    // construídas implícitamente, quando necessário, apenas na concretização
    // destas 3 condições:
    // - Nenhuma definição de operação de cópia.
    // - Nenhuma definição de operação de movimentação.
    // - Nenhuma definição de um 'destructor'.
    //
    // Por fim, a definição de métodos 'template' não impedem a criação
    // implícita dos métodos especiais ('copy', 'move' ou 'destructor').
    {
        cout << endl;
    };
};
}  // namespace item_17
