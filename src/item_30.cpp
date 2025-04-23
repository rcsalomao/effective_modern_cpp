#include <boost/type_index.hpp>
#include <iostream>
#include <memory>
#include <optional>
#include <ranges>
#include <type_traits>
#include <vector>

namespace item_30 {
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

// Função que realiza 'perfect forwarding' de uma função 'func' com seus
// respectivos argumentos 'args':
template <typename... Args, typename Func>
    requires std::is_invocable_v<Func, Args...>
decltype(auto) fwd(Func func, Args... args) {
    return func(std::forward<Args>(args)...);
}

// Duas funções, de mesmo nome, para simular situação de sobrecarga em
// argumento:
int process_val(int val) { return val; }
int process_val(int val, int priority) {
    int vp = val * priority;
    return vp;
}

// Função 'template' para demonstrar dificuldades no uso de funções genéricas
// como argumento repassado por 'perfect forwarding':
template <typename T>
T work_on_val(T val) {
    return val;
};

class Widget {
   public:
    static const std::size_t min_vals{
        23};  // Declaração de atributo 'static'. Por comodidade é possível
              // também realizar sua inicialização aqui e, a depender de certos
              // compiladores, não há necessidade de se realizar a sua definição
              // para a utilização em certos contextos.
};
// // Atributos do tipo 'static', em tese, precisam ser definidos fora do escopo
// // da classe. A seguir está a respectiva definição (sem a reinicialização,
// // que resultaria em erro por se tratar de uma variável constante):
//
// const std::size_t Widget::min_vals;  // Definição do atributo 'static'

// BitField
struct IPv4Header {
    std::uint32_t version : 4;
    std::uint32_t IHL : 4;
    std::uint32_t DSCP : 6;
    std::uint32_t ECN : 2;
    std::uint32_t total_length : 16;
};

void main() {
    // 'std::forward<T>', introduzido pela standard C++11, permite repassar seus
    // argumentos para outras estruturas de forma condicional, preservando a
    // qualificação original dos respectivos tipos (sejam do tipo 'lvalue ref'
    // ou 'rvalue ref'). Entretanto este mecanismo não é perfeito e, portanto,
    // deve-se tomar conhecimento das situações em que 'std::forward<T>' não
    // consegue realizar sua função.
    //
    // No caso do uso de 'std::forward<T>' para a realização de 'perfect
    // forward', aplica-se 'std::forward<T>' exclusivamente para repassar
    // argumentos do tipo de referência (tanto do tipo 'lvalue ref' como 'rvalue
    // ref'). Excluem-se os argumentos do tipo de 'valor', pois se tratam de
    // cópia, e argumentos do tipo de ponteiro. Por fim, as qualificações de
    // 'const' e 'volatile' dos tipos repassados também devem ser mantidas.
    //
    // Desta forma, para atender as necessidades de 'perfect forwarding', faz-se
    // uso de referência universal ('T&&'), pois esta é capaz de capturar tanto
    // 'lvalue ref' quanto 'rvalue ref', além de também as demais qualificações
    // ('const' e/ou 'volatine').
    //
    // Uma possível implementação pode ser dada por (1). Tal função 'template'
    // recebe um objeto invocável e repassa para este os argumentos subsequentes
    // mantendo as suas respectivas qualificações.
    //
    {
        // Caso comum de 'perfect forwarding' de um conjunto de argumentos para
        // uma função ('f'), por meio de uma função 'template' genérica ('fwd'):
        cout << endl;
        cout << "auto f = [](auto a) { return to_string(a); };" << endl;
        auto f = [](auto a) { return to_string(a); };
        cout << "f(3): " << f(3) << endl;
        cout << "fwd(f, 3): " << fwd(f, 3) << endl;
    };
    // No contexto de 'perfect forward', entende-se como falha quando as funções
    // 'template' e original, para um mesmo conjunto de argumentos, produzem
    // resultados diferentes. A seguir alguns casos são abordados:
    {
        // O primeiro caso é no uso de '{...}', que possui significado à
        // depender do contexto. No caso de conversão implícita e declaração de
        // variável 'auto', por exemplo, '{}' é entendido como
        // 'std::initializer_list<T>'. Entretanto funções 'template' tẽm
        // dificuldade em lidar com a sintaxe '{...}' pois não conseguem
        // realizar a dedução de tipo necessária.
        cout << endl;
        cout << "auto f = [](const std::vector<int>& v){return stringify(v);};"
             << endl;
        auto f = [](const std::vector<int>& v) { return stringify(v); };
        cout << "f({1, 2, 3}): " << f({1, 2, 3}) << endl;
        // fwd(f, {1, 2, 3});  // Erro: a função 'template' não consegue
        //                     // realizar a dedução de tipo de '{1,2,3}'
        //                     // para repassar para 'f'.
        cout << "auto il = {1, 2, 3};" << endl;
        auto il = {1, 2, 3};
        cout << "fwd(f, il): " << fwd(f, il) << endl;
    };
    {
        // O próximo caso é quando ou uso de ponteiros e em especial, o uso de
        // ponteiros nulos ('NULL', '0' ou 'nullptr'). Para os valores 'NULL' e
        // '0' a função 'template' não consegue realizar a correta dedução do
        // tipo e portanto falha e realizar o 'perfect forwarding':
        cout << endl;
        cout << "auto f = [](int* a) {\n  if (a) {\n    return "
                "to_string(*a);\n  } else {\n    return \"\'NULL\' or \'0\' or "
                "\'nullptr\' passed!\"s;\n  }\n};"
             << endl;
        auto f = [](int* a) {
            if (a) {
                return to_string(*a);
            } else {
                return "'NULL' or '0' or 'nullptr' passed!"s;
            }
        };
        cout << "int x{3};" << endl;
        int x{3};
        cout << "f(&x): " << f(&x) << endl;
        cout << "f(NULL): " << f(NULL) << endl;
        cout << "f(0): " << f(0) << endl;
        cout << "f(nullptr): " << f(nullptr) << endl;
        // a função 'f' não tem problemas em lidar com a formas legado de
        // ponteiro nulo ('NULL' e '0').
        cout << "fwd(f, &x): " << fwd(f, &x) << endl;
        // fwd(f, NULL);  // Erro: O tipo deduzido pela função é 'long int'
        // fwd(f, 0);     // Erro: O tipo deduzido pela função é 'int'
        cout << "fwd(f, nullptr): " << fwd(f, nullptr) << endl;
        // Entretanto a função 'fwd' falha quando os ponteiros nulos são 'NULL'
        // e '0'. A única alternativa então é fazer uso de 'nullptr' para
        // indicar a nulidade de determinado ponteiro.
    };
    {
        // Outro caso é no uso de atributos 'static' de classes que, de forma
        // geral, a sua declaração já seria suficiente. Entretanto, no caso de
        // seu uso numa função 'template' que pega seus argumentos por
        // referência, é necessário que esta variável seja definida e alocada em
        // memória.
        cout << endl;
        cout << "auto f = [](std::size_t val) { return to_string(val); };"
             << endl;
        auto f = [](std::size_t val) { return to_string(val); };
        cout << "f(Widget::min_vals): " << f(Widget::min_vals) << endl;
        cout << "fwd(f, Widget::min_vals): "
             << fwd(f,
                    Widget::min_vals)
             << endl;  // No caso do compilador 'gcc' (gcc 15),
                       // aparentemente não há a necessidade de
                       // definição de 'Widget::min_vals', pois
                       // este é capaz de "entender" e fazer uso
                       // do atributo 'static', mesmo com apenas
                       // sua declaração no corpo da classe.
        cout << "auto& mv = Widget::min_vals;" << endl;
        auto& mv = Widget::min_vals;
        cout << "mv: " << to_string(mv) << endl;
        // cout << &Widget::min_vals
        //      << endl;  // Ainda assim, tentar acessar um endereço de memória
        //                // da variável 'Widget::min_vals', da forma que está
        //                // no código (sem uma definição), resulta em erro no
        //                // linker, pois de fato não há uma definição e
        //                // alocação desta variável em memória.
    };
    {
        cout << endl;
        // auto f = [](int func(int), int a) {
        //     return to_string(func(a));
        // };  // sintaxe 'non-pointer'.
        cout << "auto f = [](int (*func)(int), int a) { return "
                "to_string(func(a)); };"
             << endl;
        auto f = [](int (*func)(int), int a) { return to_string(func(a)); };

        cout << "int process_val(int val);" << endl;
        cout << "int process_val(int val, int priority);" << endl;

        cout << "f(process_val, 3): " << f(process_val, 3) << endl;
        // fwd(f, process_val,
        //     3);  // Erro: A função 'template' não conseque realizar
        //          // inferir corretamente qual a versão de 'process_val'
        //          // deve ser utilzada (falha no processo de sobrecarga).

        cout << "using ProcessFuncType = int (*)(int);" << endl;
        using ProcessFuncType = int (*)(int);

        ProcessFuncType process_val_ptr =
            process_val;  // Para ajudar, pode-se criar antecipadamente um
                          // ponteiro para a função desejada (que neste caso
                          // o mecanismo de sobrecarga irá conseguir selecionar
                          // a versão correta) para posteriormente repassar para
                          // 'fwd'.
        cout << "fwd(f, process_val_ptr, 3): " << fwd(f, process_val_ptr, 3)
             << endl;

        // Algo similar ocorre com funções do tipo 'template':
        cout << "template <typename T>\nT work_on_val(T val);" << endl;
        cout << "f(work_on_val, 3): " << f(work_on_val, 3) << endl;
        // fwd(f, work_on_val,
        //     3);  // Novamente 'fwd' não consegue construir a versão correta
        //          // de 'work_on_val<T>' já que também se trata de um função
        //          // 'template' e há poucos elementos para realizar a dedução
        //          // de seu tipo.
        // Uma forma de se lidar com isso é realizar um 'cast' para a versão
        // correta da função 'template' a ser repassada para 'fwd':
        cout << "fwd(f, static_cast<ProcessFuncType>(work_on_val), 3): "
             << fwd(f, static_cast<ProcessFuncType>(work_on_val), 3) << endl;
    };
    {
        // Aparentemente a utilização de atributos de 'bitfields' não é mais uma
        // questão para 'perfect forwarding' (gcc 15).
        auto f = [](std::size_t v) {};
        IPv4Header h;

        f(h.total_length);
        fwd(f, h.total_length);  // Bom, de acordo com a bibliografia de
                                 // referẽncia, esta operação não deveria ser
                                 // possível e resultaria em erro. Aparentemente
                                 // é possível agora se obter 'lvalue ref' de um
                                 // bitfield (gcc 15)?

        // auto& l = h.total_length;  // É proibida a criação de 'non-const ref'
        //                            // de um valor de um bitfield, já que este
        //                            // pode não estar no começo do endereço de
        //                            // memória e seu comportamento, em boa
        //                            // parte, depende da implementação.

        auto l =
            h.total_length;  // Ainda assim é possível realizar a sua cópia.

        fwd(f, l);
        fwd(f, static_cast<std::uint32_t>(h.total_length));
    };
};
}  // namespace item_30
