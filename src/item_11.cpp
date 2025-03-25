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

namespace item_11 {
using boost::typeindex::type_id_with_cvr;
using std::cout;
using std::endl;
using std::string;
using std::vector;
namespace rg = std::ranges;
namespace vw = std::views;

using std::to_string;

template <std::ranges::forward_range Rng>
auto stringify(Rng&& seq) {
    auto b = seq | vw::transform([](const auto& a) { return to_string(a); }) |
             vw::join_with(',') | vw::common;
    return "{" + std::string(std::begin(b), std::end(b)) + "}";
}

// forma de suprimir os 'copy constructor' e 'copy assignment' segundo C++98.
class Foo {
   private:  // declaração das funções como privadas e não as definindo.
    Foo(const Foo&);
    Foo& operator=(const Foo&);
};
// usuários são impedidos de chamar estas funções, e se algum código ainda tiver
// acesso à elas (seja por meio de outras funções membro da classe ou funções do
// tipo 'friend' de outra classe), processo de linkagem irá falhar devido a
// falta de definição.

// forma de suprimir os 'copy constructor' e 'copy assignment' segundo C++11.
class Bar {
   public:
    Bar(const Bar&) = delete;
    Bar& operator=(const Bar&) = delete;
};
// por meio de 'delete', os referidos métodos estão de fato suprimidos, não
// sendo gerados e acusando erro em tempo de compilação.

// Outra característica é que 'delete' pode ser aplicado para qualquer
// função, não apenas à membros de classes.
// versão que pode sofrer problemas com conversão implícita:
bool is_lucky(int number) { return static_cast<bool>(number); };
// versão com sobrecarga específica para suprimir conversão implícita:
bool is_lucky2(int number) { return static_cast<bool>(number); };
bool is_lucky2(char number) = delete;
bool is_lucky2(bool number) = delete;
bool is_lucky2(double number) = delete;

// definição de função template que trabalha com ponteiros.
template <typename T>
void process_pointer(T* ptr) {
    cout << *ptr << endl;
};
// Entretanto, pode-se desejar suprimir o uso desta função para casos de
// ponteiros do tipo 'void*' e 'char*'. Desta forma definem-se versões
// específicas destes tipos para aplicar 'delete'.
template <>
void process_pointer<void>(void* ptr) = delete;
template <>
void process_pointer<char>(char* ptr) = delete;
// e mais versões específicas para 'const void*', 'const char*', 'const volatile
// void*', ...

void main() {
    cout << endl;
    // Uma das decisões mais importantes no que diz respeito à arquitetura é
    // sobre as interfaces abstratas das entidades que compõem o programa. Neste
    // sentido, deve-se dar preferência à sintaxe '= delete' para identificar
    // funções que não devem ser implementadas e utilizadas, ao invés de
    // simplesmente definí-las como funções de escopo privado.

    // Isto é especialmente importante para o caso de funções membro especiais,
    // criadas automaticamente, de acordo com a necessidade, tais como 'copy
    // constructor', 'copy assignment', 'move constructor', 'move assignment'.
    // Caso deseja-se suprimir o uso de determinada função, deve-se de forma
    // geral identificá-las com '= delete' para que a linguagem não às gere e o
    // usuário não às utilize.

    // no caso trivial da função 'is_lucky', o argumento passado pode ser do
    // tipo 'int', mas também pode ser utilizado 'char', 'bool' ou 'double',
    // pois C++ irá realizar conversão implícita para atender o tipo do
    // argumento.
    is_lucky(0);
    is_lucky('c');
    is_lucky(true);
    is_lucky(3.5);
    // Já para a função 'is_lucky2', com as versões de sobrecarga suprimidas, as
    // tentativas de conversão implícita geram erro:
    is_lucky2(0);
    // is_lucky2('c');   // erro: call to deleted function 'is_lucky2'
    // is_lucky2(true);  // erro: call to deleted function 'is_lucky2'
    // is_lucky2(3.5);   // erro: call to deleted function 'is_lucky2'

    // Outra vantagem é que 'delete' pode ser utilizado para suprimir a
    // instânciação de funções 'template' não desejados:
    int x1 = 3;
    int* y1 = &x1;
    void* y2 = &x1;
    char x2 = 'c';
    char* y3 = &x2;
    process_pointer(y1);
    // process_pointer(y2);  // erro: call to deleted function 'process_pointer'
    // process_pointer(y3);  // erro: call to deleted function 'process_pointer'
    //
    // Ainda, especializações de funções 'template' devem ser definidas no mesmo
    // nível de escopo. Não é possível criar especializações dentro de uma
    // classe ('class scope') como uma função 'private' por exemplo. Caso se
    // deseje realizar especificação de um método, deve-se realizar fora da
    // classe, no mesmo nível de escopo ('namespace scope').
};
}  // namespace item_11
