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

namespace item_15 {
using boost::typeindex::type_id_with_cvr;
using std::cout;
using std::endl;
using std::string;
using std::vector;
namespace rg = std::ranges;
namespace vw = std::views;

template <typename T>
using uptr = std::unique_ptr<T>;

template <typename T, typename... Args>
uptr<T> mu(Args&&... args) {
    return std::make_unique<T>(std::forward<Args>(args)...);
};

template <std::ranges::forward_range Rng>
auto stringify(Rng&& seq) {
    using std::to_string;

    auto b = seq | vw::transform([](const auto& a) { return to_string(a); }) |
             vw::join_with(',') | vw::common;
    return "{" + std::string(std::begin(b), std::end(b)) + "}";
};

// exemplo de função que pode ser executada em tempo de compilação, caso seus
// argumentos tenham valores conhecidos em tempo de compilação:
constexpr int pow(const int base,
                  const int exp) noexcept {  // noexcept não é obrigatório.
    auto result{1};
    for (int i = 0; i < exp; ++i) {
        result *= base;
    }
    return result;
}

class Point {
   public:
    // construtores podem ser 'constexpr'.
    constexpr Point(double x_val = 0, double y_val = 0) noexcept
        : x{x_val}, y{y_val} {};

    // assim como também os 'getters'
    constexpr double x_value() const noexcept { return x; }
    constexpr double y_value() const noexcept { return y; }

    // e 'setters'
    constexpr void set_x_value(double new_x) noexcept { x = new_x; }
    constexpr void set_y_value(double new_y) noexcept { y = new_y; }

    // com essas definições, é possível então fazer uso da classe 'Point' em
    // contextos que exijam processamento em tempo de compilação.

   private:
    double x;
    double y;
};

// dados pontos 'p1' e 'p2', conhecidos em tempo de compilação, esta função
// também será executada em tempo de compilação:
constexpr Point midpoint(const Point& p1, const Point& p2) noexcept {
    return {(p1.x_value() + p2.x_value()) / 2,
            (p1.y_value() + p2.y_value()) / 2};
}

// podendo definir 'setters' como 'constexpr', estes também podem ser utilizados
// em contexto de execução em tempo de compilação:
constexpr Point reflection(const Point& p) noexcept {
    Point res{};  // criando um ponto 'non-const' para posterior modificação
    res.set_x_value(-p.x_value());  // invocando os respectivos 'setters'
    res.set_y_value(-p.y_value());
    return res;
}

void main() {
    // 'constexpr' é um mecanismo muito interessante sob diversos pontos de
    // vista.
    // Quando utilizado na definição de um objeto, acaba funcionando como
    // uma declaração 'const', indicando ao compilador que aquele objeto é
    // constante (pode ser alocado num espaço de memória 'read-only') e possui
    // valor conhecido em tempo de compilação. Desta forma pode-se utilizá-lo em
    // contextos tais como instanciação de 'templates', definição de
    // 'std::array<T, N>', valores de enumeradores, especificações de
    // alinhamento, dentre outros.

    // int sz;
    // constexpr auto array_size_1 =
    //     sz;  // erro: constexpr variable 'array_size_1' must be initialized
    //          // by a constant expression
    // std::array<int, sz> data1;  // erro: non-type template argument is not
    //                             // a constant expression
    //
    constexpr auto array_size_2 =
        10;  // ok, '10' é uma constante conhecida em tempo de compilação.
    std::array<int, array_size_2> data2;  // ok, 'array_size_2' é uma constexpr.
    //
    // int sz;
    // const auto array_size_1a = sz;  // ok, 'array_size_1a' é
    //                                 // uma cópia de 'sz'
    // std::array<int, array_size_1a>
    //     data3;  // erro: valor de 'array_size_1a' não é conhecido em
    //             // tempo de compilação.

    // De forma geral, todo objeto 'constexpr' é 'const', mas nem todo objeto
    // 'const' é 'constexpr'. Ou seja, um objeto 'constexpr' é um objeto 'const'
    // mais especializado.

    // O resultado de 'constexpr' para funções depende da qualificação dos
    // argumentos de entrada para a respectiva função. Tais funções irão
    // produzir constantes em tempo de compilação somente quando forem invocadas
    // com argumentos de valor conhecido em tempo de compilação. Do contrário,
    // se tais funções forem invocadas com argumentos cujos valores são apenas
    // conhecidos em tempo de executação, então estas funções retornarão valores
    // em tempo de execução.
    // Desta forma, é possível definir funções e expressões que podem ser
    // utilizadas nos mais diversos contextos. Tanto em tempo de execução, como
    // em contextos que exigem o conhecimentos de valores, constantes, em tempo
    // de compilação.
    //
    cout << endl;

    // {
    //     auto n{3};
    //     std::array<int, pow(2, n)>
    //         res;  // erro: resultado de 'pow' não é sabido em tempo de
    //               // compilação, já que 'n' também não o é ('constexpr').
    // }
    {
        const auto n{3};
        std::array<int, pow(2, n)> res;
    }
    {
        constexpr auto n{3};
        std::array<int, pow(2, n)> res;
    }

    constexpr Point p1{
        9.4,
        24.7};  // construtor 'constexpr' é executado em tempo de compilação
    constexpr Point p2{28.9, 5.3};

    constexpr auto mid = midpoint(
        p1, p2);  // cálculo do ponto 'mid' é realizado em tempo de compilação.
    cout << "'mid': " << "{" << mid.x_value() << ", " << mid.y_value() << "}"
         << endl;

    constexpr auto reflected_point =
        reflection(mid);  // cálculo do ponto 'reflected_point' também é
                          // realizado em tempo de compilação.
    cout << "reflected 'mid': " << "{" << reflected_point.x_value() << ", "
         << reflected_point.y_value() << "}" << endl;

    // Por fim, é importante notar que assim como 'noexcept', 'constexpr' faz
    // parte da interface da função/classe/programa. Desta forma deve-se
    // realizar a adoção deste mecanismo de forma cautelosa, pois posterior
    // alteração pode quebrar código que faça uso desta interface.
};
}  // namespace item_15
