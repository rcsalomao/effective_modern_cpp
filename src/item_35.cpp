#include <boost/type_index.hpp>
#include <chrono>
#include <future>
#include <iostream>
#include <memory>
#include <mutex>
#include <numeric>
#include <ranges>
#include <thread>
#include <type_traits>
#include <vector>

namespace item_35 {
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

using namespace std::chrono_literals;

void do_async_work(int& out, std::mutex& m) {
    // auto time = std::chrono::seconds{2};
    auto time = 100ms;
    std::this_thread::sleep_for(time);
    {
        std::scoped_lock lock{m};
        out += 42;
    }
};

int do_async_work(std::size_t& cnt, std::mutex& m) {
    // auto time = std::chrono::seconds{2};
    auto time = 100ms;
    std::this_thread::sleep_for(time);
    {
        std::scoped_lock lock{m};
        cnt++;
    }
    return 42;
};

void main() {
    // Caso seja necessário executar uma função qualquer de forma assíncrona
    // (neste caso a função 'do_async_work'), pode-se, basicamente, realizar a
    // execução de 2 maneiras: operando e gerenciando diretamente as respectivas
    // threads ('thread-based design') ou fazendo uso do padrão de projeto
    // baseado em atividades ('task-based design').
    // Em termos de C++, isto significa fazer uso de
    // 'std::thread'/'std::jthread' ou 'std::async'/'std::future<T>',
    // respectivamente.
    {
        // Exemplo de execução de código assíncrono por meio de 'threads':
        cout << endl;

        cout << "void do_async_work(int& out, std::mutex& m);" << endl;
        void do_async_work(int& out, std::mutex& m);

        cout << "int res{0};" << endl;
        int res{0};
        cout << "std::mutex m{};" << endl;
        std::mutex m{};

        cout << "std::jthread t1{static_cast<void (*)(int&, "
                "std::mutex&)>(do_async_work),\n"
                "    std::ref(res), std::ref(m)};"
             << endl;
        std::jthread t1{static_cast<void (*)(int&, std::mutex&)>(do_async_work),
                        std::ref(res), std::ref(m)};
        cout << "std::jthread t2{static_cast<void (*)(int&, "
                "std::mutex&)>(do_async_work),\n"
                "    std::ref(res), std::ref(m)};"
             << endl;
        std::jthread t2{static_cast<void (*)(int&, std::mutex&)>(do_async_work),
                        std::ref(res), std::ref(m)};

        cout << "t1.join()" << endl;
        t1.join();
        cout << "t2.join()" << endl;
        t2.join();

        cout << "res: " << res << endl;
    };
    {
        // Exemplo de execução de código assíncrono por meio de 'std::async':
        cout << endl;

        cout << "int do_async_work(std::size_t& cnt, std::mutex& m);" << endl;
        int do_async_work(std::size_t& cnt, std::mutex& m);

        cout << "size_t cnt{0};" << endl;
        std::size_t cnt{0};
        cout << "std::mutex m{};" << endl;
        std::mutex m{};

        cout << "vector<std::future<int>> futures;\n"
                "for (int _ : vw::iota(0, 4)) {\n"
                "    futures.push_back(std::async(\n"
                "        static_cast<int (*)(std::size_t&, "
                "std::mutex&)>(do_async_work),\n"
                "        std::ref(cnt), std::ref(m)));\n"
                "}"
             << endl;
        vector<std::future<int>> futures;
        for (int _ : vw::iota(0, 4)) {
            futures.push_back(std::async(
                static_cast<int (*)(std::size_t&, std::mutex&)>(do_async_work),
                std::ref(cnt), std::ref(m)));
        }

        cout << "int res = std::accumulate(\n"
                "    std::begin(futures), std::end(futures), 0,\n"
                "    [](int i, std::future<int>& b) { return i + b.get(); });"
             << endl;
        int res = std::accumulate(
            std::begin(futures), std::end(futures), 0,
            [](int i, std::future<int>& b) { return i + b.get(); });

        cout << "res: " << res << "  " << "cnt: " << cnt << endl;
    };
    // Na abordagem do tipo 'thread-based', deve-se realizar o gerenciamento
    // manual das respectivas threads (sua instanciação e destruição), enquanto
    // que na abordagem 'task-based' o foco está mais orientado na atividade em
    // si, com o gerenciamento da respectiva thread sendo realizado de forma
    // automática por 'std::async'.
    //
    // A forma canônica de se informar e recuperar valores pela abordagem
    // 'thread-based' é por meio de referências de variáveis 'input' e 'output',
    // que terão seus valores alterados pela tarefa executada pela thread.
    // Enquanto que por meio da abordagem 'task-based' se obtém um objeto do
    // tipo 'std::future<T>', no qual é possível se obter, em um momento
    // posterior, o valor calculado de forma assíncrona.
    //
    // Caso a função a ser executada assíncronamente emita uma exceção,
    // 'std::future<T>' possui a capacidade de repassar e/ou informá-la.
    // Enquanto que no caso das 'std::thread's, o programa é terminado por meio
    // de uma chamada à 'std::terminate'.
    //
    // 'software threads' são um recurso limitado, e caso haja a tentativa de se
    // criar um número de 'threads' acima do máximo permitido, uma exceção do
    // tipo 'std::system_error' é lançada. Este mecanismo ocorre mesmo se a
    // função em específico for qualificada como 'noexcept'. Existem, claro,
    // mecanismos e padrões para se evitar este tipo de ocorrência.
    //
    // Outra questão que precisa ser considerada é a de 'oversubscription', ou
    // seja, a instanciação e criação de mais tarefas e 'software threads' do
    // que 'hardware threads'. Nesta situação, o sistema operacional precisa
    // realizar o fatiamento do tempo de execução das threads para poder revezar
    // a execução das tarefas no número de 'hardware threads' disponíveis. Tal
    // revezamento pode ser prejudicial pois resulta em 'context switch',
    // gerando aumento no 'overhead' de gerenciamento das respectivas 'threads',
    // invalidação de cache, 'poluição' do cache local. Entretanto o
    // entendimento e consequente minimização de 'oversubscription' é algo bem
    // complicado pois depende de diversos fatores (se determinado ponto do
    // programa é 'i/o-bound' ou 'cpu-bound', o real custo do 'context switch',
    // uso efetivo do cache da CPU, etc).
    //
    // O uso de 'std::async' repassa estas considerações e responsabilidades
    // para a implementação da 'STL'. Logo, atualmente, o comportamento de
    // 'std::async' é totalmente dependente da sua implementação, variando entre
    // 'gcc', 'clang' e 'msvc', por exemplo. A depender do 'vendor',
    // 'std::async' pode gerar uma nova 'thread' para cada tarefa ou fazer uso
    // de uma 'threadpool'. Por causa disso, também é possível atingir o
    // limite máximo de 'threads' permitido. 'std::async' também pode ter este
    // compartamento alterado por meio de diretrizes ('std::launch::async' ou
    // 'std::launch::deferred'), que serão posteriormente melhor detelhadas.
    // Ainda, outro detalhe importante à saber, é o fato de que
    // 'std::future<T>::get()' realiza o bloqueio de execução da 'thread' que
    // invoca o método até o fim da execução da respectiva tarefa. Portanto, no
    // caso de 'std::future's que dependam do resultado de outra ou da
    // própria 'std::future', há o risco de ocorrência de 'deadlock' do
    // programa.
    //
    // 'std::thread' é mais indicado para os casos em que:
    //
    // - É necessário acessar a API subjascente de implementação da plataforma,
    // como no caso do uso de afinidades e prioridades de processos.
    //
    // - É necessário realizar a otimização de uso de 'threads' da aplicação,
    // como no caso de máquinas com perfil e características fixas de hardware,
    // ou programas com perfil de execução bem definidos.
    //
    // - Há a necessidade de se implementar tecnologia de 'threading' além da
    // API disponível pela linguagem C++, como no caso de 'threadpool's em
    // plataformas que não as fornecem.
    //
};
}  // namespace item_35
