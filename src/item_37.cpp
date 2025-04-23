#include <boost/type_index.hpp>
#include <iostream>
#include <memory>
#include <ranges>
#include <thread>
#include <type_traits>
#include <vector>

namespace item_37 {
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

// Classe RAII para gerenciamento de 'std::thread' (1):
class ThreadRAII {
   public:
    enum class DtorAction { join, detach };

    // O construtor aceita apenas argumentos do tipo 'rvalue', pois não é
    // possível copiar objetos 'std::thread'.
    ThreadRAII(std::thread&& t, DtorAction a) : action(a), t(std::move(t)) {};

    // Podemos definir operadores de movimentação sem problemas:
    ThreadRAII(ThreadRAII&&) = default;
    ThreadRAII& operator=(ThreadRAII&&) = default;

    // Antes que se possa realizar a destruição do objeto 'ThreadRAII', deve-se
    // verificar se o objeto 'std::thread' interno está no estado 'joinable'.
    // Caso positivo, deve-se realizar uma das operações de 'join' ou 'detach'
    // para que o programa não seja imediatamente terminado.
    ~ThreadRAII() {
        if (t.joinable()) {
            if (action == DtorAction::join) {
                t.join();
            } else {
                t.detach();
            }
        }
    };

    // Função 'getter' para se ter acesso á thread interna do objeto
    // 'ThreadRAII', de forma análoga ao método 'get()' de 'smart pointers' por
    // exemplo.
    std::thread& get() { return t; }

   private:
    // A ordem de declaração das variáveis importa, pois é a ordem em que tais
    // variáveis serão inicializadas (ainda mais importante caso uma variável
    // dependa do valor de outra).
    // Portanto, é mais interessante declarar 'std::thread' como uma das últimas
    // variáveis, já que neste momento todas as outras variáveis já terão sido
    // instanciadas.
    DtorAction action;
    std::thread t;
};

void main() {
    // Os objetos 'std::thread' podem se encontrar em dois estados:
    // 'joinable' ou 'unjoinable'.
    //
    // Uma 'std::thread' em estado 'joinable' corresponde à:
    //
    // - Uma 'std::thread' que está, ou poderá estar, em execução.
    //
    // - Uma 'std::thread' bloqueada ou esperando para execução.
    //
    // - 'Threads' que já realizaram sua tarefa por completo.
    //
    // Já uma 'std::thread' em estado 'unjoinable' corresponde à:
    //
    // - 'Default-constructed std::threads': Tais objetos não possuem uma função
    // atrelada para execução.
    //
    // - Objetos 'std::thread' que foram movidos: O resultado de uma operação
    // de movimentação é que a respectiva thread de execução atrelada à um
    // objeto 'std::thread' agora estará atrelada à outro objeto 'std::thread'.
    //
    // - Objetos 'std::thread' que foram resumidos ('joined'): Após uma operação
    // de 'join', um objeto 'std::thread' não corresponde mais à uma thread de
    // execução.
    //
    // - Objetos 'std::thread' que tenham sido desacoplados: A operação de
    // 'detach' corta a ligação entre o objeto 'std::thread' e sua
    // correspondente thread de execução.
    //
    // Se um 'destructor' for invocado para um objeto 'std::thread' em estado
    // 'joinable', então a execução do programa será imediatamente terminada. As
    // outras duas alternativas (com consequências mais severas) seriam um
    // 'join' implícito incondicional ou uma operação de 'detach' implícita,
    // resultando em uma thread de execução capaz de alterar o estado de memória
    // do programa em si.
    //
    // Uma alternativa para se evitar que o 'destructor' seja invocado durante a
    // execução de uma 'thread' é o uso do padrão RAII (1).
    {
        // exemplo de uso de 'ThreadRAII':
        cout << endl;
        cout << "int a{3};" << endl;
        int a{3};
        cout << "ThreadRAII t = ThreadRAII{std::thread{[&a] { a *= 2; }},\n"
                "                          ThreadRAII::DtorAction::join};"
             << endl;
        ThreadRAII t = ThreadRAII{std::thread{[&a] { a *= 2; }},
                                  ThreadRAII::DtorAction::join};
        cout << "a: " << a << endl;
    };
    // Por fim, a standard C++20 introduziu o objeto 'std::jthread' que,
    // da mesma forma que os 'smart pointers', realiza o gerenciamento
    // automático da respectiva thread de execução. No caso, o 'destructor' do
    // objeto 'std::jthread' realiza automaticamente a operação de 'join()' da
    // correspondente thread de execução.
};
}  // namespace item_37
