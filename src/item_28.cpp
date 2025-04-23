#include <boost/type_index.hpp>
#include <iostream>
#include <memory>
#include <ranges>
#include <type_traits>
#include <vector>

namespace item_28 {
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
    // No momento que se passa a realizar operações e manipulações com tipos,
    // surge a necessidade de realizar o tratamento de determinadas situações.
    // Uma destas situações se refere à resolução de referência de referências
    // (tanto nos casos de 'lvalue ref' como de 'rvalue ref'). O mecanismo que
    // permite esta resolução é chamado de 'reference collapsing'.
    //
    // Este mecanismo em especial é de uso exclusivo do compilador. Para o
    // usuário não é permitido definir referências de referências, mas ao
    // compilador é permitido realizar a operação de 'reference collapsing'.
    //
    // Há um total de 4 combinações de referências possíveis:
    // 1: lvalue para lvalue.
    // 2: lvalue para rvalue.
    // 3: rvalue para lvalue.
    // 4: rvalue para rvalue.
    //
    // A regra de resolução não é complicada:
    // Se ambas as referências forem do tipo 'rvalue ref', então o tipo
    // resultante também será 'rvalue ref'. Do contrário será do tipo 'lvalue
    // ref'.
    //
    // Desta forma, tem-se para as 4 combinações previamente definidas:
    // 1: (E&)& -> E&
    // 2: (E&)&& -> E&
    // 3: (E&&)& -> E&
    // 4: (E&&)&& -> E&&
    //
    // O mecanismo de 'reference collapsing' é o mecanismo responsável pelo
    // funcionamento da função 'std::forward<T>'. Este mecanismo pode ocorrer em
    // 4 contextos:
    // - No momento de instanciação de uma função/classe 'template'.
    //
    // - No mommento de formação do tipo de uma variável 'auto'.
    //
    // - No momento de formação e uso de 'typedefs' e 'alias' (que podem ser
    // definidos dentro de estruturas 'template' e, portanto, passíveis
    // 'reference collapsing').
    //
    // - No uso de 'decltype', nas situações em que há a ocorrência de
    // referências de referências.
};
}  // namespace item_28
