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

#include "item_22_Widget.hpp"

namespace item_22 {
using boost::typeindex::type_id_with_cvr;
using std::cout;
using std::endl;
using std::string;
using std::vector;
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

template <std::ranges::forward_range Rng>
auto stringify(Rng&& seq) {
    auto b = seq | vw::transform([](const auto& a) { return to_string(a); }) |
             vw::join_with(',') | vw::common;
    return "{" + std::string(std::begin(b), std::end(b)) + "}";
};

void main() {
    // Um dos principais usos para 'std::unique_ptr<T, D>' é na implementação do
    // padrão 'Pimpl'. Tal padrão consiste no uso de um atributo interno,
    // definido como um ponteiro, para outro objeto que possui a implementação
    // e os recursos de fato. A vantagem está no fato de que há uma separação
    // clara dentre a declaração (geralmente num arquivo cabeçalho '.hpp' e
    // utilizado exaustivamente pelo usuário) e a definição (realizada em
    // arquivo fonte '.cpp' e compilado numa unidade em separado). Tal separação
    // também possibilita reduções nos tempos de compilações, pois é necessário
    // apenas recompilar os arquivos que foram de fato alterados.
    // Neste sentido, o uso de 'smart pointers' ('std::unique_ptr' e
    // 'std::shared_ptr') é bastante vantajoso pois estes são capazes de
    // realizar o gerenciamento automático de seus respectivos recursos.
    {
        cout << endl;
        Widget w{"lakdsfj"};
        cout << w.get_name() << endl;
        // Widget w2{w}; // copy constructor
        Widget w2{std::move(w)};  // move constructor
        cout << w2.get_name() << endl;
        // cout << w.get_name()
        //      << endl;  // erro: segfault. tentativa de acessar recurso que já
        //                // foi previamente movido para outro nome.
    };
};
}  // namespace item_22
