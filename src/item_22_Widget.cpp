#include "item_22_Widget.hpp"

#include <vector>
using std::vector;

// Definição do tipo 'Impl':
struct Widget::Impl {
    string name;
    vector<double> data;
};

// Definições dos construtures, destruidor e métodos quaisquer:
Widget::Widget(string name) : pImpl(std::make_unique<Impl>(name)) {}
Widget::~Widget() =
    default;  // Definição do destruidor precisa ser realizada após a definição
              // do 'Impl' para que o compilador possa ter a informação completa
              // do tipo a ser destruído pelo ponteiro 'std::unique_ptr<Impl>'.
              // Neste caso, a definição padrão 'default' é suficiente. Vale
              // lembrar que para o caso de 'std::shared_ptr' não há a
              // necessidade de se ter o tipo completamente definido, já que o
              // destructor não faz parte do tipo do ponteiro. Desta forma o
              // destruidor implícitamente definido pelo compilador lá na
              // declaração do tipo 'Widget' já seria o suficiente.
              // 'std::shared_ptr' não possui problemas com tipos incompletos.

// 'Widget' possui como atributo um 'smart pointer' do tipo 'std::unique_ptr',
// que é um tipo que não possui operadores de cópia, apenas de movimentação.
// Desta forma, por padrão, 'Widget' também passa a ser um tipo 'move-only'. É
// então necessário realizar a definição dos operadores de cópia caso se deseje
// realizar estes tipos de operações:
Widget::Widget(const Widget& other)
    : pImpl(std::make_unique<Impl>(*other.pImpl)) {};
Widget& Widget::operator=(const Widget& other) {
    *pImpl = *other.pImpl;
    return *this;
};

// Com a definição de um destruidor próprio, por padrão, a construção implícita
// dos operadores de movimentação é inibida. Desta forma é necessário também
// realizar a definição dos operadores de movimentação. Neste caso, a
// implementação padrão, por meio da sintaxe 'default', já é o suficiente:
Widget::Widget(Widget&& other) = default;
Widget& Widget::operator=(Widget&& other) = default;
// Implementação dos operadores sem o uso da sintaxe 'default':
// Widget::Widget(Widget&& other) : pImpl(std::move(other.pImpl)) {};
// Widget& Widget::operator=(Widget&& other) {
//     pImpl = std::move(other.pImpl);
//     return *this;
// };

string Widget::get_name() { return pImpl->name; }
