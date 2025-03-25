#include <boost/type_index.hpp>
#include <functional>
#include <iostream>
#include <memory>
#include <print>
#include <ranges>
#include <unordered_map>
#include <vector>

namespace item_6 {
using boost::typeindex::type_id_with_cvr;
using std::cout;
using std::endl;

void main() {
    // Embora 'auto' traga diversas vantagens em termos de sintaxe, redução de
    // complexidade, performance e redução de bugs, ainda assim o mecanismo não
    // é isento de falhas. Para tanto, quando necessário, emprega-se o uso de
    // tipagem explícita quando 'auto' resolve realizar dedução para tipos
    // indesejáveis.
    std::vector<bool> v{true, false, false, true};
    bool a1 = v[3];  // tipo de a1: bool
    auto a2 = v[3];  // tipo de a2: std::_Bit_reference
    cout << "bool a1 = v[3]; tipo de a1: "
         << type_id_with_cvr<decltype(a1)>().pretty_name() << endl;
    cout << "auto a2 = v[3]; tipo de a2: "
         << type_id_with_cvr<decltype(a2)>().pretty_name() << endl;
    // Neste caso, assim como em várias outras ocasiões, é que o operador '[]'
    // para um 'std::vector<bool>' retorna um objeto 'proxy' que age como um
    // valor booleano para fins práticos.
    // O problema aqui é que o mecanismo 'auto' não funciona muito bem na
    // presença de classes 'proxy', que de forma geral costumam passar
    // despercebidas.

    // Para evitar que 'auto' acabe deduzindo erroneamente o tipo de uma
    // variável para uma classe 'proxy', pode-se fazer uso de 'idioma de
    // inicialização explícita de tipo':
    auto a3 = static_cast<bool>(v[3]);  // tipo de a3: bool
    cout << "auto a3 = static_cast<bool>(v[3]); tipo de a3: "
         << type_id_with_cvr<decltype(a3)>().pretty_name() << endl;

    // também é possível utilzar esse idioma para casos em que se deseja,
    // deliberadamente, realizar a conversão de tipos para um tipo desejado.
    auto calc_epsilon = []() { return 3.0; };
    auto e1 = calc_epsilon();                      // tipo de e1: bool.
    float e2 = calc_epsilon();                     // tipo de e2: float.
    auto e3 = static_cast<float>(calc_epsilon());  // tipo de e3: float.
};
}  // namespace item_6
