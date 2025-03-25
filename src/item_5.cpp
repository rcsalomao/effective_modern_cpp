#include <boost/type_index.hpp>
#include <functional>
#include <iostream>
#include <memory>
#include <print>
#include <ranges>
#include <unordered_map>
#include <vector>

namespace item_5 {
using boost::typeindex::type_id_with_cvr;
using std::cout;
using std::endl;

template <typename It>
void dwim(It b, It e) {
    while (b != e) {
        typename std::iterator_traits<It>::value_type curr_value = *b;
    }
}

template <typename It>
void auto_dwim(It b, It e) {
    while (b != e) {
        auto curr_value = *b;
    }
}

class Widget {
    int x{0};
    friend bool operator<(const Widget& left, const Widget& right);
};
bool operator<(const Widget& left, const Widget& right) {
    return left.x < right.x;
};

void main() {
    // C++11 faz a introdução do mecanismo 'auto', que traz diversas vantagens
    // frente a necessidade de declaração explícita de tipos.

    // Primeiro, para que 'auto' possa realizar a dedução de tipo, a variável
    // precisa ser explicitamente inicializada. isto impede que tal variável
    // seja declarada e continue não inicializada (o que traz possibilidade de
    // introdução de bugs no executável):
    // int x;    // talvez inicializada ou não. depende do contexto.
    // auto x;   // declaration of variable 'x' with deduced type 'auto'
    //           // requires an initializer.
    auto x{24};  // precisa ser obrigatóriamente inicializada para dedução.

    // 'auto' também facilita substancialmente a dedução e o uso de variáveis
    // cujos tipos acabam se tornando complexos:
    // - ver função 'dwim' e 'auto_dwim'.

    // 'auto' permite que se possa obter os tipos de clausuras e funções
    // 'lambda', que são conhecidos apenas pelo compilador:
    auto f = [](int i) { return i * 3; };
    // ou ainda, criar uma função genérica:
    auto g = [](auto a) { return a * 3; };
    // importante perceber que objetos 'std::function' são fundamentalmente
    // diferentes de 'auto'. Objetos 'std::function' necessitam da definição dos
    // tipos de saída e entrada do objeto invocável, além de potencialmente
    // necessitar de mais memória que uma clausura resultante por 'auto'.
    // Adicionalmente, 'std::function' faz uso de indireção na sua operação, o
    // que pode significar em penalidades de performance.
    std::function<bool(const std::unique_ptr<Widget>&,
                       const std::unique_ptr<Widget>&)>
        h1 = [](const std::unique_ptr<Widget>& p1,
                const std::unique_ptr<Widget>& p2) { return *p1 < *p2; };
    // vs
    auto h2 = [](const std::unique_ptr<Widget>& p1,
                 const std::unique_ptr<Widget>& p2) { return *p1 < *p2; };

    // 'auto' também evita a ocorrência de conversões implícitas indesejáveis de
    // tipos entre operações:
    std::vector<int> v;
    unsigned sz1 =
        v.size();  // o tipo retornado pelo método 'size()' é
                   // 'std::vector<int>::size_type', e portanto, esta expressão
                   // causa uma conversão implícita para 'unsigned int', que
                   // pode resultar em bugs difíceis de capturar.
    // vs
    auto sz2 =
        v.size();  // agora 'sz2' possui o tipo correto retornado pelo método
    // Adicionalmente:
    std::unordered_map<std::string, int> m;
    // ...
    for (const std::pair<std::string, int>& p : m) {
        // ...
        // faça alguma operação com 'p'
        // ...
    }
    // aqui mora um bug: a chave de uma unordered_map é 'const T', e não apenas
    // 'T'. Desta forma, o loop range-for precisa realizar operações de cópia
    // para cada iteração, já que 'std::pair<std::string, int>' !=
    // 'std::pair<const std::string, int>'.
    // Portanto, uma forma de não estar mais propenso à operações não desejadas
    // e bugs com memória é fazer uso de 'auto':
    for (auto& p : m) {
        // ...
        // faça alguma operação com 'p'
        // ...
    }
};
}  // namespace item_5
