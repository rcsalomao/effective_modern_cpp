#include <boost/type_index.hpp>
#include <functional>
#include <iostream>
#include <memory>
#include <ranges>
#include <type_traits>
#include <vector>

namespace item_32 {
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

// Criação de um 'functor' para emular uma clausura (compatível com C++11):
struct Closure {
    using DataType = std::unique_ptr<double>;
    DataType data;

    explicit Closure(DataType&& in) : data{std::move(in)} {}
    explicit Closure(DataType& in) : data{std::move(in)} {}

    double operator()() { return *data; }
};

void main() {
    // Para a definição de funções lambda, é mais interessante realizar 'init
    // capture' para captura de valores e objetos ao invés de 'default capture'.
    // Especialmente nos casos em que se deseja capturar objetos que são
    // 'move-only' (como por exemplo 'std::unique_ptr', 'std::shared_ptr' ou
    // 'std::future').
    //
    // 'init capture' nos permite especificar:
    // - o nome do atributo na clausura gerada pela função lambda.
    //
    // - uma expressão para a inicialização deste atributo.
    {
        cout << endl;
        cout << "auto pw = std::make_unique<double>(42.24);" << endl;
        auto pw = std::make_unique<double>(42.24);
        cout << "*pw = 24.42;" << endl;
        *pw = 24.42;
        cout << "auto f = [pw = std::move(pw)]() { return *pw; };" << endl;
        auto f = [pw = std::move(pw)]() {
            return *pw;
        };  // 'init capture' do ponteiro 'pw' por meio de movimentação.
            // Importante notar que a declaração à esquerda de '=' participa do
            // escopo da clausura, enquanto que a declaração à direita de '='
            // participa do escopo daonde a clausura está sendo definida.
            // Logo '[pw = std::move(pw)]' quer dizer: "criar um atributo 'pw'
            // dentro da clausura, inicializado por meio de 'std::move' sobre a
            // variável local 'pw'".
        cout << "f(): " << f() << endl;
    };
    {
        // exemplo (1):
        cout << endl;
        cout << "auto f = [pw = std::make_unique<double>(42.24)]() {return "
                "*pw;};"
             << endl;
        auto f = [pw = std::make_unique<double>(42.24)]() {
            return *pw;
        };  // Alternativamente, pode-se também realizar a criação do atributo
            // por meio de expressões mais diretas ('std::make_unique<T>' dentro
            // do 'init capture'), caso não seja necessário realizar nenhuma
            // outra operação anterior.
        cout << "f(): " << f() << endl;
    }
    // Outro nome para 'init capture' é 'generalized lambda capture'.
    {
        // Para emular o compartamento de uma clausura, compatível com C++11,
        // pode-se também fazer uso de um 'functor':
        cout << endl;
        cout << "auto f = Closure(std::make_unique<double>(42.01));" << endl;
        auto f = Closure(std::make_unique<double>(42.01));
        cout << "f(): " << f() << endl;
    };
    {
        // Outra forma de emular a funcionalidade de 'init capture', compatível
        // com C++11, seria por meio do uso de 'std::bind' juntamente com
        // 'lambda':
        cout << endl;
        cout << "using namespace std::placeholders;" << endl;
        using namespace std::placeholders;

        cout << "auto pw = std::make_unique<double>(24.02);" << endl;
        auto pw = std::make_unique<double>(24.02);
        cout << "auto f = std::bind(\n"
                "    [](const std::unique_ptr<double>& d, int i) { return *d + "
                "i; },\n"
                "    std::move(pw), _1);"
             << endl;
        auto f = std::bind(
            [](const std::unique_ptr<double>& d, int i) { return *d + i; },
            std::move(pw), _1);
        // Por padrão a clausura resultante é constante ('const') no escopo de
        // sua função lambda, e por isso o argumento 'd' é definido como 'const
        // lvalue ref'.
        // Caso a intenção seja de realizar operações que exijam mutabilidade
        // das variáveis, pode-se definir a função lambda como 'mutable' e
        // remover a qualificação de 'const' do respectivo argumento:
        // auto f = std::bind(
        //     [](std::unique_ptr<double>& d, int i) mutable { return *d + i; },
        //     std::move(pw), _1);

        // 'std::bind' produz um 'function object', em que seu primeiro
        // argumento deve ser um objeto invocável, sucedido pelas argumentos do
        // respectivo objeto.
        //
        // Este 'function object' resultante possui internamente uma cópia de
        // todos os argumentos informados. Para cada argumento do tipo 'lvalue',
        // é gerada uma cópia ('copy constructor'). Para cada argumento do tipo
        // 'rvalue', um novo objeto é criado por meio de 'move constructor'.
        //
        // Neste exemplo, com o uso de 'std::move', um novo objeto 'pw' é criado
        // internamente por meio de seu respectivo 'move constructor'.
        //
        // Quando um objeto resultante de 'std::bind' é invocado, os argumentos
        // armazenados internamente são repassados para o objeto invocável
        // informado originalmente.
        //
        // É por isso que o tipo do argumento a ser informado para a função
        // lambda é do tipo 'const lvalue ref' ao invés de 'rvalue ref', pois
        // estamos levando em consideração o fato de 'std::bind' estar
        // armazenando internamente uma nova cópia (mesmo que realizada por meio
        // de um 'move constructor') do objeto de interesse. Portanto, as
        // operações realizadas em 'd' no escopo da função lambda são referentes
        // à cópia do objeto armazenado dentro do objeto resultante de
        // 'std::bind'.
        //
        // Adicionalmente, pelo fato do objeto 'std::bind' guardar uma cópia de
        // todos os argumentos informados, incluindo a sua respectiva clausura,
        // o tempo de vida da clausura passa à ser o mesmo do objeto resultante
        // de 'std::bind'. Ou seja, é possível tratar os objetos que se
        // encontram dentro do objeto 'std::bind' como se estivessem dentro da
        // clausura.
        //
        cout << "f(3): " << f(3) << endl;
    };
    {
        // Análogamente ao exemplo (1):
        cout << endl;
        cout << "using namespace std::placeholders;" << endl;
        using namespace std::placeholders;

        cout << "auto f = std::bind(\n"
                "    [](const std::unique_ptr<double>& d, int i) { return *d + "
                "i; },\n"
                "    std::make_unique<double>(42.36), _1);"
             << endl;
        auto f = std::bind(
            [](const std::unique_ptr<double>& d, int i) { return *d + i; },
            std::make_unique<double>(42.36),
            _1);  // Criação do ponteiro diretamente na definição do objeto
                  // 'std::bind'.
        cout << "f(3): " << f(3) << endl;
    }
};
}  // namespace item_32
