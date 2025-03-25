#include <atomic>
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

namespace item_13 {
using boost::typeindex::type_id_with_cvr;
using std::cout;
using std::endl;
using std::string;
using std::vector;
namespace rg = std::ranges;
namespace vw = std::views;

template <std::ranges::forward_range Rng>
auto stringify(Rng&& seq) {
    using std::to_string;

    auto b = seq | vw::transform([](const auto& a) { return to_string(a); }) |
             vw::join_with(',') | vw::common;
    return "{" + std::string(std::begin(b), std::end(b)) + "}";
}

void main() {
    // No que diz respeito aos iteradores, prefere-se fazer uso de
    // 'const_iterators' ao invés de 'iterators' normais.
    // Desde a standard C++14, a fim de se obter um código maximalmente
    // genérico, pode-se utilizar as funções livres 'std::cbegin()' e
    // 'std::cend()' para obter, respectivamente, os iteradores do começo e do
    // fim do container:
    cout << endl;
    vector<int> v = vw::iota(0, 10) | rg::to<vector<int>>();
    cout << "original 'v': " << stringify(v) << endl;
    // auto it = rg::find(v, 4);
    auto it = std::find(std::cbegin(v), std::cend(v), 4);
    if (it != std::cend(v)) {
        // v.insert(it, 24);
        v.insert(it, {24, 42});
    }
    cout << "modified 'v': " << stringify(v) << endl;
};
}  // namespace item_13
