#include <boost/type_index.hpp>
#include <deque>
#include <functional>
#include <future>
#include <iostream>
#include <memory>
#include <ranges>
#include <thread>
#include <type_traits>
#include <vector>

namespace item_38 {
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

void main() {
    // Pode-se entender que 'std::thread' em estado 'joinable' e
    // 'std::future<T>' instanciado no modo 'std::launch::async' possuem
    // uma relação similar com suas respectivas threads de execução (são
    // basicamente 'handles' destas threads).
    //
    // O processo de destruição de um objeto 'std::thread' resulta no término
    // imediato do programa por meio de 'std::terminate'. Entretanto, o processo
    // de destruição de um objeto 'std::future<T>' pode resultar num 'join'
    // implícito, num 'detach' implícito ou em nenhum dos dois casos.
    //
    // Pode-se entender melhor o objeto 'std:future<T>' como uma ponta de um
    // canal de comunicação entre o ponto de invocação ('std::future<T>') e o
    // ponto de execução ('std::promise'). Ambos pontos podem estar em threads e
    // momentos distintos, realizando o compartilhamento de informação por meio
    // de um objeto de estado compartilhado ('shared state objectt'), alocado na
    // heap.
    //
    // Considerando o design do canal de comunicação entre 'std::future' e
    // 'std::promise' por meio de um objeto compartilhado, pode-se então
    // construir o modelo mental sobre seu processo de destruição. O
    // comportamento padrão de destruição de um objeto 'std::future' é o da
    // simples destruição do próprio objeto 'std::future' e de seus respectivos
    // atributos (sem execução de 'join', 'detach' ou qualquer outra operação).
    // Somente quando as 3 condições detalhadas à seguir forem verdadeiras é que
    // o objeto 'std::future' irá executar uma operação implícita de 'join':
    //
    // - O objeto 'std::future' faz referência à um objeto de estado
    // compartilhado criado por meio de uma chamada 'std::async'.
    //
    // - A política de execução da tarefa é definida como 'std::launch::async'.
    // Seja por especificação da chamada, ou por definição automática do
    // sistema.
    //
    // - O objeto 'std::future' correspondente é o último objeto que faz
    // referência ao objeto de estado compartilhado (mais relevante no caso de
    // objetos 'std::shared_future').
    //
    // A priori não há como saber, por meio de sua API, se um objeto
    // 'std::future' qualquer é o último objeto que faz referência à um objeto
    // compartilhado de estado, o que resultaria num 'join' implícito e bloqueio
    // da thread de execução durante a sua destruição.
    //
    // 'std::async' não é a única função capaz de gerar objetos 'std::future'. A
    // função 'std::packaged_task<T>' é capaz de envelopar um objeto invocável e
    // retornar um objeto 'std::shared_future<T>' para posterior obtenção do
    // resultado. E como tal, objetos 'std::shared_future' não são
    // capazes de bloquear a thread de execução durante a sua destruição.
    {
        // exemplo de uso de um objeto 'std::packaged_task<T>':
        cout << endl;
        cout << "std::packaged_task<int(int, int)> p{[](int a, int b){return a "
                "* b;}};"
             << endl;
        std::packaged_task<int(int, int)> p{[](int a, int b) {
            return a * b;
        }};  // 'std::packaged_task' envelopa um objeto invocável.
        cout << "auto sf = p.get_future();" << endl;
        auto sf =
            p.get_future();  // obtenção de um objeto 'std::shared_future<T>'
                             // para uso posterior.
        cout << "std::jthread t{std::move(p)};" << endl;
        std::jthread t{std::move(p), 2, 12};
        // movimentação do objeto 'std::packaged_task' para dentro de
        // um objeto 'std::thread' (não é possível realizar cópia de
        // um objeto 'std::packaged_task').
        //
        // ...
        //
        // À partir deste ponto há 3 possibilidades:
        //
        // - Nada acontece: Sem a invocação do método 'get()' do objeto
        // 'std::shared_future', não há a necessidade de se realizar a execução
        // do correspondente 'std::packaged_task'. O objeto 'std::thread' irá
        // permanecer no estado 'joinable'.
        //
        // - Operação de 'join' é realizada em 't': A invocação do método 'get'
        // do objeto 'std::shared_future', exige a execução do objeto
        // 'std::packaged_task' e por consequência a operação 'join' no objeto
        // 'std::thread'. O objeto 'std::thread' passa a ficar no estado
        // 'unjoinable'.
        //
        // - Operação de 'detach' é realizada em 't': Neste caso não há a
        // necessidade de se realizar a operação de 'detach' para o objeto
        // 'std::shared_future' no seu método 'destructor', pois o ponto de
        // invocação já irá tomar conta disso por meio do objeto
        // 'std::packaged_task'.

        cout << "1" << endl;
        cout << "sf.get(): " << sf.get()
             << endl;  // A execução do objeto invocável ocorre neste momento.
        cout << "2" << endl;

        // Ou seja, para objetos 'std::future' gerados por meio da função
        // 'std::packaged_task', não há a necessidade de se realizar tratamento
        // especial para a respectiva operação de destruição, pois esta já é
        // tratada de forma adequada por meio de 'std::packaged_task' e
        // 'std::thread'.
    };
    {
        cout << endl;
        cout << "std::deque<std::move_only_function<void()>> q;" << endl;
        std::deque<std::move_only_function<void()>> q;
        cout << "auto f = [](int a, int b) { return a * b; };" << endl;
        auto f = [](int a, int b) { return a * b; };
        cout << "std::packaged_task<\n"
                "    typename std::invoke_result_t<decltype(f), int, int>()>\n"
                "    p{[&f]() { return f(2, 12); }};"
             << endl;
        std::packaged_task<
            typename std::invoke_result_t<decltype(f), int, int>()>
            p{[&f]() { return f(2, 12); }};
        cout << "auto sf = p.get_future();" << endl;
        auto sf = p.get_future();
        cout << "q.emplace_back(std::move(p));" << endl;
        q.emplace_back(std::move(p));
        cout << "std::jthread t{std::move(q.front())};" << endl;
        std::jthread t{std::move(q.front())};
        cout << "(1)" << endl;
        cout << "sf.get(): " << sf.get() << endl;
        cout << "(2)" << endl;
    };
    {
        using Task = std::function<void()>;
        cout << endl;
        int a{0};
        auto pm_ptr = std::make_shared<std::promise<void>>();
        auto ft = pm_ptr->get_future();
        std::deque<Task> q;
        auto f = [&a, pm_ptr]() {
            a = 3;
            pm_ptr->set_value();
        };
        q.emplace_back(std::forward<decltype(f)>(f));
        std::jthread t{[&q] {
            Task task;
            task = std::move(q.front());
            q.pop_front();
            task();
        }};
        ft.wait();
        cout << a << endl;
    };
    {
        using Task = std::move_only_function<void()>;
        cout << endl;
        int a{0};
        std::promise<void> pm;
        auto ft = pm.get_future();
        std::deque<Task> q;
        auto f = [&a, pm = std::move(pm)]() mutable {
            a = 3;
            pm.set_value();
        };
        q.emplace_back(std::forward<decltype(f)>(f));
        std::jthread t{[&q] {
            Task task;
            task = std::move(q.front());
            q.pop_front();
            task();
        }};
        ft.wait();
        cout << a << endl;
    };
    {
        using Task = std::function<void()>;
        cout << endl;
        int a{0};
        std::promise<void> pm;
        auto ft = pm.get_future();
        std::deque<Task> q;
        auto f = [&a, &pm]() mutable {
            a = 3;
            pm.set_value();
        };
        q.emplace_back(std::forward<decltype(f)>(f));
        std::jthread t{[&q] {
            Task task;
            task = std::move(q.front());
            q.pop_front();
            task();
        }};
        ft.wait();
        cout << a << endl;
    };
};
}  // namespace item_38
