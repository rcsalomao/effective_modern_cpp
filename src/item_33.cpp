#include <boost/type_index.hpp>
#include <iostream>
#include <memory>
#include <ranges>
#include <type_traits>
#include <vector>

namespace item_33 {
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
concept IsInvocable = std::is_invocable_v<Func, Args...>;

void main() {
    // Desde C++14 é possível fazer uso de 'auto' conjutamente com funções
    // 'lambda'. Neste sentido é igualmente possível a definição de funções
    // 'lambda' genéricas e até mesmo funções que executem 'perfect forwarding'.
    {
        cout << endl;

        // captura de argumentos cujos tipos serão deduzidos por meio de 'auto':
        // Também é possível qualificar a função 'lambda' com 'const',
        // 'constexpr', 'noexcept', 'requires', 'mutable', etc.
        auto f = [](auto func, auto&& a)
            requires std::is_invocable_v<decltype(func), decltype(a)>
        { func(std::forward<decltype(a)>(a)); };
        // Por fim, pode-se realizar 'perfect forwarding' por meio da aplicação
        // de 'std::forward<decltype(param)>' sobre o parãmetro que está sendo
        // deduzido com 'auto&&'. 'std::forward<decltype(param)>' realiza
        // exatamente o mesmo processo anteriormente discutido por meio de
        // 'reference collapsing'.

        f([](auto a) { cout << to_string(a) << endl; }, "aldkfj"s);
    };
    {
        cout << endl;

        // Adicionalmente, desde C++14, funções 'lambda' também podem ser
        // variádicas, podendo aceitar um número indeterminado de argumentos:
        auto f = [](auto func, auto&&... a)
            requires std::is_invocable_v<decltype(func), decltype(a)...>
        { func(std::forward<decltype(a)>(a)...); };

        f([](auto a) { cout << to_string(a) << endl; }, "aldkfj"s);
    };
    {
        cout << endl;

        // Por incrível que pareça, isso daqui também é uma função 'lambda':
        auto f = []<typename Func, typename... Args>
            requires IsInvocable<Func, Args...>
        (Func func, Args&&... a) { func(std::forward<Args>(a)...); };

        f([](auto a) { cout << to_string(a) << endl; }, "aldkfj"s);
    };
};
}  // namespace item_33
