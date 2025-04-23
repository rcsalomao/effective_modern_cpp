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

namespace item_23 {
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

template <typename T>
decltype(auto) move2(T&& param) {
    // Se o argumento informado for um 'lvalue', então T é deduzido como 'T&'.
    // Se o argumento informado for um 'rvalue', então T é deduzido como 'T'.
    using ReturnType =
        std::remove_reference_t<T>&&;  // 'type alias' para um tipo
                                       // 'rvalue ref' ('T&&').
    return static_cast<ReturnType>(
        param);  // 'cast' incondicional de 'param' para um tipo 'T&&'.
}

class String {
   public:
    // ...
    String() = default;
    String(const String&) { cout << "copy constructor!"; };  // copy ctor
    String(String&&) { cout << "move constructor!"; };       // move ctor
    // ...
};  // namespace item_23

// Entretanto, deve-se ter cuidado no momento de realizar as operações de
// 'std::move', pois em determinados contextos é possível obter resultados
// diferentes do esperado:
class Annotation {
   public:
    // No caso deste construtor, a operação que será realizada de fato é uma
    // operação de cópia.
    explicit Annotation(const String& text) : value(std::move(text)) {}
    // Este construtor tenta inicializar o atributo 'value' por meio de uma
    // operação de movimentação sobre o objeto 'text'. Entretanto o argumento
    // 'text' é definido como um 'const lvalue'. Mesmo com a tentativa de
    // realizar um 'cast' sobre 'text' para 'rvalue ref' por meio da função
    // 'std::move', o qualificador de 'const' deve ser mantido, resultando num
    // argumento do tipo 'const rvalue ref' ('const String&&') a ser passado
    // pelo construtor de 'value' da classe 'String'.
    // O problema é que 'String' não pode utilizar o 'move constructor', pois
    // este não garante a propriedade de 'const' no seu argumento de entrada
    // ('String(String&& other){...}').
    // Por fim, resta fazer uso do 'copy constructor', já que neste é permitido
    // que o argumento de entrada 'const lvalue&' possa realizar 'bind' em
    // 'const rvalue ref', pois desta forma as qualificações 'const' são
    // mantidas.

    // Caso seja necessário evitar o uso do 'copy constructor', deve-se definir
    // o argumento de entrada sem o qualificador 'const', para que então o 'move
    // constructor' de 'String' seja selecionado de fato:
    // explicit Annotation(String& text) : value(std::move(text)) {}
    // ou:
    // explicit Annotation(String&& text) : value(std::move(text)) {}

   private:
    String value;
};
// Como regra prática, pode-se ter que caso seja desejável a realização de
// operações de movimentação num objeto, então não se deve qualificar o mesmo
// como 'const'.

template <typename T>
    requires std::is_base_of_v<String, std::remove_reference_t<T>>
auto dispatch_args_to_String(T&& arg) {
    return std::forward<T>(arg);
}

void main() {
    // Apesar do nome, 'std::move' é basicamente uma função template que realiza
    // um 'cast' incondicional para um tipo 'rvalue'. Desta forma, abrindo a
    // possibilidade do compilador realizar uma operação de movimentação no
    // respectivo objeto.
    {
        cout << endl;
        cout << "Annotation annot{String{}}: ";
        Annotation annot{String{}};
        cout << endl;
    };
    {
        cout << endl;
        cout << "String texto{};" << endl;
        String texto{};
        cout << "Annotation annot{texto}: ";
        Annotation annot{texto};
        cout << endl;
    };

    // Análogamente à função 'std::move', 'std::forward<T>' também é uma função
    // template que realiza 'cast'. Entretato, neste caso a operação de 'cast' é
    // condicional. Caso o argumento de entrada seja um 'lvalue',
    // 'std::forward<T>' realiza um 'cast' para 'lvalue ref'. Já se o argumento
    // for 'rvalue', então o 'cast' será para 'rvalue ref'.
    // O principal uso desta função é realizar a passagem, ou condução, dos
    // argumentos informados para outra função/operação, mantendo intacta a
    // tipagem destes argumentos.
    // O mecanismo que permite o funcionamento de 'std::forward' é o mecanismo
    // de 'reference collapsing'.
    {
        cout << endl;
        cout << "dispatch_args_to_String(String{}): ";
        dispatch_args_to_String(String{});
        cout << endl;
    };
    {
        cout << endl;
        cout << "String texto{};" << endl;
        cout << "dispatch_args_to_String(texto): ";
        String texto{};
        dispatch_args_to_String(texto);
        cout << endl;
    }
};
}  // namespace item_23
