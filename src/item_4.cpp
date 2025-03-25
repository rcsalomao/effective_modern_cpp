#include <boost/type_index.hpp>
#include <iostream>
#include <print>
#include <ranges>
#include <vector>

namespace item_4 {
using boost::typeindex::type_id_with_cvr;
using std::cout;
using std::endl;

template <typename T>
class TD;

template <typename T>
void f(const T& param) {
    std::println("Tipo de 'T': {}", type_id_with_cvr<T>().pretty_name());
    std::println("Tipo de 'param': {}",
                 type_id_with_cvr<decltype(param)>().pretty_name());
};

auto create_vector() { return std::vector<double>{1, 2, 3}; };

void main() {
    // Sobre algumas técnicas para se realizar a inspeção de tipos em c++.
    //
    // - uso de IDEs e, atualmente, LSPs:
    // A depender da IDE ou LSP, este pode fornecer informações em tempo real
    // sobre o código trabalhado e respectivos tipos.
    const int ans = 42;
    auto x = ans;   // int x
    auto y = &ans;  // const int *y

    // - Diagnóstico de erro do compilador:
    // Pode-se também fazer uso de objetos 'tempalte' para gerar erros que podem
    // nos informar os tipos das variáveis utilizadas na sua instanciação
    // TD<decltype(x)> x_type;  // implicit instantiation of undefined template
    //                          // 'item_4::TD<int>'
    // TD<decltype(y)> y_type;  // implicit instantiation of undefined template
    //                          // 'item_4::TD<const int *>'
    // - Output em tempo de execução:
    // Também é possível realizer a escrita em 'cout', por exemplo, do tipo da
    // variável desejada. Para tanto, recomenda-se o uso de uma biblioteca da
    // 'boost' para tal fim, pois esta já contempla casos especiais e é capaz de
    // realizar o 'demangling' dos nomes internos dos símbolos compilados.
    std::println("Tipo de 'x': {}",
                 type_id_with_cvr<decltype(x)>().pretty_name());
    std::println("Tipo de 'y': {}",
                 type_id_with_cvr<decltype(y)>().pretty_name());
    // ainda, 'type_id_with_cvr' é capaz de lidar com situações não triviais
    // como:
    const auto vw = create_vector();
    if (!vw.empty()) {
        f(&vw[0]);  // função para escrita do tipo da variável desejada.
    }
    // como comparação, o LSP da máquina pode ter dificuldades de lidar
    // corretamente com situações não triviais e informar o tipo errado ou
    // incompleto:
    const auto& b = &vw[0];
};
}  // namespace item_4
