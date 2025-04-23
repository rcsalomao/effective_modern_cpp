#include <boost/type_index.hpp>
#include <iostream>
#include <memory>
#include <ranges>
#include <type_traits>
#include <vector>

namespace item_34 {
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
    // Após a standard C++11, com a introdução de funções 'lambda', basicamente
    // não há mais motivos para se utilizar 'std::bind'.
    //
    // Os principais fatores que tornam as funções 'lambda' uma opção muito mais
    // favorávevl à 'std::bind' são:
    //
    // - Funções 'lambda' são mais claras e legíveis que objetos 'std::bind'.
    //
    // - Os argumentos informados para a construção dos objetos 'std::bind' são
    // avaliados e copiados para dentro do objeto no momento de compilação.
    // Deve-se tomar cuidado adicional no uso de argumentos que devem ser
    // avaliados em tempo de execução (eg, 'std::chrono::steady_clock::now()').
    //
    // -  Ao contrário de funções 'lambda', não é possível fazer uso de
    // sobrecarga de funções no uso de 'std::bind'. Uma forma de se contornar é
    // com o uso de ponteiro para a função apropriada.
    //
    // - Há maiores oportunidades do compilador realizar processos de otimização
    // (inline por ex.) com funções 'lambda' ao invés de 'std::bind'.
    //
    // - As definições de objetos 'std::bind' são significativamente mais
    // complicadas e inteligíveis que as definições de funções 'lambda'.
    //
    // Pode-se conferir a referência principal para maiores detalhes sobre essa
    // discussão.
    {
        cout << endl;
    };
};
}  // namespace item_34
