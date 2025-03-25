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

namespace item_10 {
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

template <typename E>
constexpr auto to_u_type(E enumerator) noexcept {
    return static_cast<std::underlying_type_t<E>>(enumerator);
}

void main() {
    // Em relação aos 'enums', deve-se dar preferência no uso de 'scoped enums'
    // (ou 'enum classes') ao invés de 'enums' convencionais (ou 'unscoped
    // enums'). 'enum classes' não 'vazam' os seus respectivos nomes para o
    // escopo adjascente, além de trazer vantagens adicionais.
    cout << endl;
    {
        enum Color { black, white, red };
        cout << "enum Color { black, white, red };" << endl;
        auto f = [](std::size_t x) { return x * 2; };
        Color c = red;
        if (c < 14.5) {  // aviso: comparison of enumeration type 'Color' with
                         // floating-point type 'double' is deprecated. ou seja,
                         // enum clássico é passível de conversão implícita.
            cout << "... if (c < 14.5) ..." << endl;
            cout << "auto f = [](std::size_t x) { return x * 2; }; f(c): "
                 << f(c) << endl;
        }
    }

    cout << endl;
    {
        enum class Color { black, white, red };
        auto f = [](std::size_t x) { return x * 2; };
        Color c = Color::red;
        // if (c < 14.5) {  // erro: não é permitida a conversão implícita
        //     cout << "... if (c < 14.5) ..." << endl;
        //     cout << "auto f = [](std::size_t x) { return x * 2; }; f(c): "
        //          << f(c) << endl;  // erro: 'no matching function ...'
        // }
    }

    cout << endl;
    {
        // o tipo padrão de um 'scoped enum' é 'int'. Entretanto, é possível
        // definir um tipo diferente:
        enum class Color { black, white, red };
        cout << "Tipo do 'enum class Color': "
             << type_id_with_cvr<std::underlying_type_t<Color>>().pretty_name()
             << endl;
        enum class Status : unsigned short { good, ok, bad };
        cout << "Tipo do 'enum class Status': "
             << type_id_with_cvr<std::underlying_type_t<Status>>().pretty_name()
             << endl;
    }

    cout << endl;
    {
        // uma utilidade dos enums é o mapeamento de informações de tuplas para
        // recuperação dos seus valores:
        using UserInfo =
            std::tuple<std::string, std::size_t>;  // (nome, reputação)
        enum class UserInfoFields { name, reputation };

        UserInfo uinfo{"Jon Doe", 5};
        cout << "std::tuple<std::string, std::size_t> uinfo{'Jon Doe', 5}:"
             << endl;
        auto name1 = std::get<0>(uinfo);
        auto reputation1 = std::get<1>(uinfo);
        cout << "name1: " << name1 << endl;
        cout << "reputation1: " << reputation1 << endl;
        auto name2 = std::get<to_u_type(UserInfoFields::name)>(uinfo);
        auto reputation2 =
            std::get<to_u_type(UserInfoFields::reputation)>(uinfo);
        cout << "name2: " << name2 << endl;
        cout << "reputation2: " << reputation2 << endl;
    }
};
}  // namespace item_10
