#include <boost/type_index.hpp>
#include <iostream>
#include <print>
#include <ranges>
#include <vector>

namespace item_3 {
using boost::typeindex::type_id_with_cvr;
using std::cout;
using std::endl;

class Widget {};
bool f(const Widget& w) {
    std::println("decltype(w): {}",
                 type_id_with_cvr<decltype(w)>().pretty_name());
    return true;
};

struct Point {
    int x;
    int y;
};

template <typename T>
class my_vector {
   public:
    T& operator[](std::size_t index);
};

void authenticate_user();
template <typename Container, typename Index>
// auto auth_and_access(Container& c, Index i)
//     -> decltype(c[i]) {  // forma alternativa de definição (trailing return)
// auto auth_and_access(
//     Container& c,
//     Index i) {  // esta forma não está correta, pois o mecanismo 'auto'
//                 // (similar ao caso de expressões 'template') poderá remover
//                 // as qualidades do tipo, como a referência por exemplo.
decltype(auto) auth_and_access(Container& c, Index i) {
    authenticate_user();
    return c[i];  // 'decltype(auto) faz com que a dedução de tipo de
                  // retorno seja feita pelo mecanismo de 'decltype' ao
                  // invés de 'auto'. garante-se que o tipo de restorno seja
                  // exatamente o mesmo do tipo retornado pelo 'operador
                  // []'. conserva-se as qualidades do tipo, como a referência,
                  // ponteiro ou constância.
}
// para permitir que a função 'auth_and_access' possa operar com valores
// 'rvalue', deve-se fazer as seguintes atualizações:
template <typename Container, typename Index>
decltype(auto) auth_and_access(Container&& c, Index i) {
    authenticate_user();
    return std::forward<Container>(c)[i];
}

void main() {
    // 'decltype(expr)', de forma geral, informa o tipo da expressão 'expr'.
    // alguns casos em que decltype papagueia o exato tipo da expressão
    // informada:
    const int i = 0;
    std::println("decltype(i): {}",
                 type_id_with_cvr<decltype(i)>().pretty_name());
    std::println("decltype(f): {}",
                 type_id_with_cvr<decltype(f)>().pretty_name());
    Widget w;
    f(w);
    std::println("decltype(f(w)): {}",
                 type_id_with_cvr<decltype(f(w))>().pretty_name());

    std::println("decltype(Point::x): {}",
                 type_id_with_cvr<decltype(Point::x)>().pretty_name());
    std::println("decltype(Point::y): {}",
                 type_id_with_cvr<decltype(Point::y)>().pretty_name());

    my_vector<int> v;
    std::println("decltype(v): {}",
                 type_id_with_cvr<decltype(v)>().pretty_name());
    std::println("decltype(v[0]): {}",
                 type_id_with_cvr<decltype(v[0])>().pretty_name());

    // um dos principais usos de 'decltype' é a especificação de tipos de
    // variáveis que dependem dos tipos de outros parâmetros, como no caso
    // de expressões 'template'. Ver as funções template 'auth_and_access'.
    //
    // para exemplificar a diferença enter 'decltype' e 'auto':
    const Widget& cw = w;
    auto my_widget_1 = cw;            // tipo de my_widget_1 é (Widget)
    decltype(auto) my_widget_2 = cw;  // tipo de my_widget_2 é (const Widget&)
    std::println("decltype(my_widget_1): {}",
                 type_id_with_cvr<decltype(my_widget_1)>().pretty_name());
    std::println("decltype(my_widget_2): {}",
                 type_id_with_cvr<decltype(my_widget_2)>().pretty_name());

    // Por fim, deve-se tomar cuidado com um detalhe respectivo a 'decltype'. De
    // forma geral 'decltype' retorna o tipo do nome informado.
    // Entretanto, para o caso de expressões mais complicadas, como no caso do
    // uso de parênteses '()', o tipo resultante passa a ser uma referência para
    // um 'lvalue' (T&).
    int x{24};
    std::println("decltype(x): {}",
                 type_id_with_cvr<decltype(x)>().pretty_name());
    std::println("decltype((x)): {}",
                 type_id_with_cvr<decltype((x))>().pretty_name());
    // Detalhe especialmente importante quando utilizado em conjunto com
    // 'decltype(auto)' para definir o tipo de retorno de uma função, por
    // exemplo.
};
}  // namespace item_3
