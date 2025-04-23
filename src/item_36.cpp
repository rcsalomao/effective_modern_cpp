#include <boost/type_index.hpp>
#include <future>
#include <iostream>
#include <memory>
#include <ranges>
#include <type_traits>
#include <vector>

namespace item_36 {
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

template <typename Func, typename... Args>
auto reall_async(Func&& func, Args&&... args) {
    return std::async(std::launch::async, std::forward<Func>(func),
                      std::forward<Args>(args)...);
}

void main() {
    // A função 'std::async' possui duas modalidades de execução para uma tarefa
    // 'f' informada:
    //
    // - 'std::launch::async': A execução de 'f' deve ser feita de forma
    // assíncrona or meio de uma 'thread' exclusiva.
    //
    // - 'std::launch::deferred': A execução de 'f' acontecerá apenas quando
    // os métodos 'std::future::get()' e 'std:future::wait()' forem invocados
    // pelo objeto 'std::future' retornado de 'std::async'. Quando tais métodos
    // forem invocados, 'f' será executada de forma síncrona, bloqueando a
    // thread de invocação até o térmido do processo de 'f'. Caso os respectivos
    // métodos não sejam invocados, então 'f' não será executada.
    //
    // A modalidade padrão de execução para 'std::async' é 'std::launch::async |
    // std::launch::deferred'. Ou seja, os componentes de gerenciamento de
    // 'threads' da 'STL' possuem a flexibilidade e liberdade de escolher a
    // modalidade mais adequada, a depender do contexto.
    //
    // Como consequência, para uma dada definição:
    // auto future = std::async(f);
    //
    // - Não é possível prever se 'f' será executada em uma 'thread' diferente
    // da 'thread' de invocação dos métodos 'get()' e 'wait()' (causando ou não
    // o bloqueio).
    //
    // - Não é possível prever se 'f' será executada ou não, já que não há
    // garantia de invocação dos métodos 'get()' e 'wait()' ao longo de todo o
    // programa.
    //
    // Ainda, é difícil de conciliar as modalidades de execução com o uso de
    // 'thread_local', já que não é possível prever se tais variáveis serão
    // acessadas ou não por 'f'.
    //
    // Caso se desejável evitar tais situações e ter a garantia de que 'f' seja
    // executada de forma assíncrona, então deve-se realizar a invocação de
    // 'std::async' com a modalidade 'std::launch::async' de forma explícita.
    // Para tanto, pode-se criar uma função genérica para realizar a invocação
    // de forma mais cômoda (1).
    {
        cout << endl;
        cout << "auto future = reall_async([](int i) { return i * 2; }, 3);"
             << endl;
        auto future = reall_async([](int i) { return i * 2; }, 3);
        cout << "future.get(): " << future.get() << endl;
    };
};
}  // namespace item_36
