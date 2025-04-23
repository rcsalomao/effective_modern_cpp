#include <boost/type_index.hpp>
#include <iostream>
#include <memory>
#include <ranges>
#include <type_traits>
#include <vector>

namespace item_29 {
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
    // A standard C++11 introduziu o que se chama de 'move semantics'. Por meio
    // desta nova funcionalidade é possível evitar a realização desnecessária de
    // operações de cópia em diversos contextos. Tal funcionalidade se dá pela
    // substituição dessas operações de cópia por operações de movimentação do
    // recurso atrelado à determinado objeto (seja ele 'lvalue' ou 'rvalue').
    //
    // Embora tal funcionalidade traga como principal vantagem o ganho de
    // performance, é interessante se familiarizar com as situações em que a
    // operação de movimentação não seja tão performática ou presente. Citam-se
    // a seguir alguns casos exemplares:
    //
    // A standard C++11 redefiniu a 'stl' para que seus tipos possam fazer uso e
    // aproveitar as novas vantagens da operação de movimentação. Entretanto há
    // a possibilidade de se deparar com bases de código legado que ainda não
    // foram refatoradas e não possuem a 'api' necessária para a realização de
    // movimentações. Embora o compilador ainda possa tentar criar operadores
    // implícitos de movimentação ('copy' e/ou 'assignment'), este mecanismo é
    // inibido devido aos seguintes fatores:
    //
    // -  Quando já exitirem definições de operadores de cópia ('constructors'
    // ou 'atribuição'), movimentação ou 'destructors'.
    //
    // - A presença de atributos de classe com operadores de movimentação
    // desativados (por meio de 'delete').
    //
    // - Quando a classe base possui operadores de movimentação desativados (por
    // meio de 'delete').
    //
    // Tudos os tipos de containers da 'stl' suportam as operações de
    // movimentação. Entretanto esta operação não possui a mesma eficiência para
    // todos os tipos de container. Este é o caso de 'std::array', que nada mais
    // é do que um 'array' convencional envelopado com uma interface da 'stl'.
    // No caso de 'std::array', a operação de movimentação irá instanciar um
    // segundo objeto 'std::array' e realizar a movimentação de cada um dos
    // elementos do primeiro objeto para o segundo. Portanto, sua complexidade
    // de execução é linear (O(n)).
    //
    // Outro caso relevante é sobre 'std::string'. A nível de implementação, é
    // comum os compiladores empregarem 'small string optmization' (SSO). Tal
    // otimização, orientada para pequenas 'strings' de aproximadamente 15
    // caracteres, permite que operações de cópia sejam tão performáticas quanto
    // movimentação. Tal otimização faz com que o objeto 'std::string' seja
    // alocado num 'buffer' e não mais na 'heap'.
    //
    // Outra situação está no fato que mesmo com a presença de difinição de
    // operadores de movimentação, ainda assim o compilador é forçado a executar
    // a correspondente operação de cópia. Algumas operações em containers da
    // 'stl', para manter a retrocompatibilidade com C++98, devem garantir a não
    // ocorrẽncia de exceções durante a movimentação (item 14). Como
    // consequência, mesmo se um tipo possuir sua defininição de movimentação e
    // seu objeto estiver sendo utilizado em um contexto que possa fazer uso
    // deste método, o mesmo somente poderá ser invocado caso seja declarado
    // como 'noexcept'.
    //
    // Em resumo, os cenários em que a semântica de movimentação não é capaz de
    // trazer resultados positivos são:
    //
    // - O tipo do objeto a ser movimentado não possui definição de operação de
    // movimentação, resultando em sua cópia.
    //
    // - A operação de movimentação não é tão mais performática quanto a
    // correspondente operação de cópia para um determinado objeto.
    //
    // - A depender do contexto de utilização do objeto, não é possível invocar
    // a operação de movimentação (caso o respectivo método não tenha sido
    // definido como 'noexcept').
    //
    // De forma geral, estas considerações são importantes no contexto de
    // implementação de funções genéricas, aonde não há completo conhecimento
    // dos tipos e situações de seu uso. Também é o caso de código 'instável',
    // no qual há mudança frequente nas características dos tipos dos objetos
    // utilizados.
    //
    // Já no caso em que se tem bom conhecimento dos tipos e situações de uso,
    // então se torna significantemente mais fácil de se prever e fazer uso da
    // semântica de movimentação para obtenção de ganhos em performance.
    //
    {
        cout << endl;
    };
};
}  // namespace item_29
