#include <boost/type_index.hpp>
#include <functional>
#include <iostream>
#include <list>
#include <memory>
#include <print>
#include <ranges>
#include <stdexcept>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace item_18 {
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

using std::to_string;

template <std::ranges::forward_range Rng>
auto stringify(Rng&& seq) {
    auto b = seq | vw::transform([](const auto& a) { return to_string(a); }) |
             vw::join_with(',') | vw::common;
    return "{" + std::string(std::begin(b), std::end(b)) + "}";
};

// Um uso comum para 'std::unique_ptr' é na aplicação do padrão de 'Factory':
class Foo {
   private:
    int _a;

   public:
    Foo(int a) : _a{a} {};
    virtual ~Foo() =
        default;  // 'destructor' deve ser 'virtual' para que se possa
                  // efetivamente invocar o 'destructor' do tipo derivado.
    int get_a() { return _a; }
};

class A : public Foo {
   public:
    using Foo::Foo;  // usando o construtor da classe base.
    // A(int i) : Foo(i) {};
    int get_a() { return Foo::get_a(); }
};

class B : public Foo {
   public:
    B(int i) : Foo(3 * i) {};
    int get_a() { return Foo::get_a(); }
};

enum class Types { A, B };

// função 'Factory' realiza a alocação do recurso na 'heap' e por fim retorna um
// ponteiro para este recurso:
template <typename... Args>
uptr<Foo> make_foo_derivates_1(Types t, Args&&... args) {
    if (t == Types::A) {
        return mu<A>(args...);
    } else if (t == Types::B) {
        // return mu<B>(args...);
        return std::make_unique<B>(std::forward<Args>(args)...);
    } else {
        throw std::runtime_error("Unknown type supplied.");
    };
}

// Caso seja necessário utilizar uma função específica para a destruição do
// rescuso, pode-se informá-la no construtor de 'std::unique_ptr':
auto custom_del = [](Foo* f) {
    cout << "Calling custom deleter." << endl;
    delete f;
};
template <typename... Args>
auto make_foo_derivates_2(Types t, Args&&... args) {
    std::unique_ptr<Foo, decltype(custom_del)> ptr(
        nullptr,
        custom_del);  // Definindo o ponteiro inicialmente como 'nullptr', para
                      // posteriormente, a depender de 'Types::A|B', setar o
                      // ponteiro para o objeto correto.
    if (t == Types::A) {
        // ptr.reset(new A(std::forward<Args>(args)...));
        return std::unique_ptr<Foo, decltype(custom_del)>(
            new A(std::forward<Args>(args)...),
            custom_del);  // Pode-se, alternativamente, construir diretamente o
                          // ponteiro com os argumentos necessários.
    } else if (t == Types::B) {
        ptr.reset(new B(std::forward<Args>(args)...));
    } else {
        throw std::runtime_error("Unknown type supplied.");
    };
    return ptr;  // retorna o ponteiro do objeto criado.
}

void process_foo_ptr_1(Foo* f) {
    cout << "fi->get-a(): " << f->get_a() << endl;
};
void process_foo_ptr_2(auto& f) {
    cout << "fi->get-a(): " << f->get_a() << endl;
};

void main() {
    // Pode-se entender o 'std::unique_ptr' como um objeto, que se comporta tal
    // qual um ponteiro, mas possui a responsabilidade de realizar o
    // gerenciamento de memória do recurso ao qual está atrelado. No momento em
    // que um objeto do tipo 'std:unique_ptr' sai de escopo e é destruído, este
    // também realiza a destruição do recurso que está em sua posse.
    // 'std::unique_ptr' não possui operações de cópia, apenas operações de
    // movimentação. Por padrão, a operação de destruíção do recurso involve a
    // invocação da instrução 'delete' sobre o ponteiro interno atrelado a este
    // recurso.
    {
        // Aplicação do padrão de função 'Factory':
        cout << endl;
        cout << "uptr<Foo> f1 = make_foo_derivates(Types::A, 8);" << endl;
        uptr<Foo> f1 = make_foo_derivates_1(Types::A, 8);
        cout << "f1->get_a(): " << f1->get_a() << endl;
        cout << "uptr<Foo> f2 = make_foo_derivates(Types::B, 8);" << endl;
        uptr<Foo> f2 = make_foo_derivates_1(Types::B, 8);
        cout << "f2->get_a(): " << f2->get_a() << endl;
    };
    {
        // Uso de padrão de função 'Factory' com 'std::unique_ptr' com função
        // própria 'deleter' para seus respectivos recursos. Importante notar
        // que a função 'deleter' faz parte do tipo do ponteiro:
        cout << endl;
        cout << "uptr<Foo> f1 = make_foo_derivates_2(Types::A, 8);" << endl;
        std::unique_ptr<Foo, decltype(custom_del)> f1 =
            make_foo_derivates_2(Types::A, 8);
        cout << "f1->get_a(): " << f1->get_a() << endl;
        cout << "uptr<Foo> f2 = make_foo_derivates_2(Types::B, 8);" << endl;
        auto f2 = make_foo_derivates_2(Types::B, 8);
        cout << "f2->get_a(): " << f2->get_a() << endl;
    };
    {
        cout << endl;
        cout << "uptr<Foo> f1 = make_foo_derivates_2(Types::A, 8);" << endl;
        std::shared_ptr<Foo> f1 = make_foo_derivates_2(
            Types::A,
            8);  // outra vantagem de 'std::unique_ptr' é a conversão direta
                 // para 'std::shared_ptr' caso seja necessário. Por isso que
                 // 'std::unique_ptr' é interessante como valor de retorno de
                 // funções 'Factory', pois podem ser diretamente utilizados
                 // para outro contexto ('shared');
        cout << "f1->get_a(): " << f1->get_a() << endl;
        // também é possível utilizar 'std::unique_ptr' com funções que aceitam
        // ponteiros 'raw':
        process_foo_ptr_1(f1.get());
        process_foo_ptr_2(f1);  // pegar o ponteiro por referência direta.
    }
};
}  // namespace item_18
