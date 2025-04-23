#include <boost/type_index.hpp>
#include <iostream>
#include <list>
#include <memory>
#include <ranges>
#include <type_traits>
#include <vector>

namespace item_42 {
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

class Widget {
   public:
    Widget();
    Widget(Widget&&) = default;
    Widget(const Widget&) = default;
    Widget& operator=(Widget&&) = default;
    Widget& operator=(const Widget&) = default;
    ~Widget();

   private:
};

Widget::Widget() {}

Widget::~Widget() {}

void main() {
    // Nos mais diversos containers disponíveis pela 'STL', pode-se encontrar os
    // métodos 'push_back', 'emplace_back' e variações. Estes métodos diferem
    // entre si em comportamento e, portanto, utilização. 'push_back' consiste
    // em uma operação clássica de inserção de um elemento num container.
    // Enquanto que 'emplace_back' tenta realizar a construção e instanciação de
    // determinado elemento dentro do respectivo objeto container.
    {
        // Considerando o simples caso de utilização do método 'push_back':
        cout << endl;
        vector<string> vs;

        vs.push_back(
            "xyzzy");  // "xyzzy" é um 'string literal' (const char[6]). Como o
                       // tipo do argumento não é exatamente o tipo do argumento
                       // da função ('std::string'), o compilador deve realizar
                       // conversão de tipos. Para tal é criado um objeto
                       // 'std::string' temporário, que então é informado para o
                       // método 'push_back'.
        vs.push_back(string{"xyzzy"});  // Definição equivalente à anterior,
                                        // enaltecendo a necessidade de criação
                                        // do objeto 'std::string' temporário.

        cout << "vs: " << stringify(vs) << endl;
        // Percebe-se, portanto, que ao não utilizar o tipo exato para o método
        // 'push_back', deve-se realizar duas construções. A primeira é a
        // construção do objeto temporário ('default constructor') do tipo
        // apropriado. O mecanismo de sobrecarga irá então escolher a versão
        // 'push_back(T&&);' pelo fato do argumento informado ser uma variável
        // 'rvalue'. Por causa disso, no momento de inserção do elemento no
        // container, será realizada outra operação de construção ('move
        // constructor') para a inserção do elemento no container. Por fim, o
        // objeto previamente criado é destruído.
        //
        // A fim de se evitar a instanciação desnecessária de um objeto
        // temporário para realizar operações de inserção, a 'STL' provém o
        // método 'emplace_back'. 'emplace_back' é capaz de realizar a
        // construção do elemento desejado já no seu local de destino dentro do
        // respectivo objeto container.
    };
    {
        // Agora um caso de utilização do método 'emplace_back':
        cout << endl;
        vector<string> vs;

        vs.emplace_back(
            "xyzzy");  // Realiza a construção de um objeto 'std::string'
                       // diretamente dentro do objeto container 'vs', sem a
                       // necessidade de objetos temporários.

        // 'emplace_back' faz uso de 'perfect forwarding' para realizar a
        // operação de inserção do elemento, recebendo quaisquer argumentos
        // para tal.

        vs.emplace_back(
            10, 'a');  // Neste caso, 'emplace_back' irá construir um objeto
                       // 'std::string' com os argumentos '10' e 'x' (uma string
                       // composta por 10 caracteres 'x') no seu local de
                       // destino dentro do container.

        cout << "vs: " << stringify(vs) << endl;
    };

    // 'emplace_back' está presente em qualquer container padrão que suporte
    // 'push_back'. Análogamente, todo container que possua o método
    // 'push_front', também possui suporte para 'emplace_front'. Qualquer
    // container que possua o método 'insert' (todo container com exceção de
    // 'std::forward_list' e 'std::arrar') também possui o método 'emplace'.
    // Containers associativos possuem o método 'emplace_hint' para complementar
    // o método 'insert' e 'std::forward_list' possui o método 'emplace_after'.

    // Métodos de inserção ('insert', 'push_back', etc) têm como argumento um
    // objeto para realizar a operação de inserção. Já os métodos de 'emplace'
    // recebem os argumentos necessários para a construção do respectivo objeto
    // dentro do container.

    {
        // Caso o argumento informado para o método 'emplace' seja do tipo do
        // próprio objeto a ser construído, têm-se então a invocação de um
        // construtor ('copy construtor' ou 'move construtor') a depender do
        // caso. Desta forma os métodos 'insert' e 'emplace' realizam, de forma
        // geral, a mesma operação.

        cout << endl;
        vector<string> vs;

        string name{"Dante"};

        vs.push_back(name);  // realização de construção por cópia à partir do
                             // objeto 'name' no final do container 'vs'.
        vs.emplace_back(name);  // idem.

        cout << "vs: " << stringify(vs) << endl;
    };

    // A priori, os métodos 'emplace' são os mais interessantes e performáticos
    // pois são capazes de realizar a construção do objeto no seu local de
    // destino sem fazer uso de instâncias temporárias. Entretanto, há
    // determinadas situações em que os métodos de inserção acabam sendo mais
    // vantajosos.

    // Das situações em que os métodos 'emplace' costumam ser mais vantajosos:
    //
    // - O elemento a ser adicionado no container é construído e não atribuído:
    // A principal vantagem de 'emplace' em relação à 'insert' é a capacidade de
    // construção do elemento diretamente no seu local de destino. Caso não seja
    // possível realizar essa construção, então 'emplace' e 'insert' se tornam
    // indistinguíveis. Containers baseados em nós quase sempre fazem uso de
    // construção para adicionar novos valores. Os únicos containers que não são
    // baseados em nós são 'std::vector', 'std::deque', 'std::string' e
    // 'std::array'. Nestes ainda é possível fazer uso de 'emplace_back' para
    // realizar a construção do novo elemento na posição final do container.
    // Adicionalmente, 'std::deque' possui também o método 'emplace_front'.
    //
    // - O tipo dos argumentos informados são diferentes do tipo dos elementos
    // contidos pelo container: Caso os tipos do argumento seja o mesmo dos
    // elementos contidos pelo objeto container, então os métodos de inserção
    // não precisarão criar variáveis temporárias e portanto terão performance
    // similar aos métodos 'emplace'.
    //
    // - É improvável a rejeição do novo elemento pelo objeto container por se
    // tratar de um valor duplicado: No caso de ser necessário a verificação de
    // duplicidade de valores dos elementos contidos num container, o método
    // 'emplace' irá inicialmente construir um nó com o valor a ser adicionado e
    // compará-lo com todos os outros elementos do container. Caso este seja um
    // elemento único, então o nó será adicionado. Do contrário, este nó será
    // destruído e a operação de 'emplace' é cancelada. Logo, têm-se nestes
    // casos desperdício com operações de criação e destruição de objetos.

    // Outra questão sobre o uso dos métodos 'emplace' diz respeito ao
    // gerenciamento de memória. Deve-se prestar atenção quanto ao uso do método
    // 'emplace' junto com funções de criação de 'smart pointers' por exemplo. A
    // questão principal reside na possibilidade de ocorrência de exceções
    // durante o processo de instanciação do ponteiro.
    {
        cout << endl;
        std::list<std::shared_ptr<Widget>> ptrs;

        auto kill_Widget = [](Widget* pw) {
            delete pw;
        };  // Destruidor particular para 'Widget'.

        // ptrs.push_back(std::shared_ptr<Widget>(new Widget, kill_Widget));
        ptrs.push_back(
            {new Widget,
             kill_Widget});  // 'push_back' irá exigir a criação de um elemento
                             // 'std::shared_ptr<Widget>' temporário (um
                             // 'default constructor' com uma função própria de
                             // desalocação) para que possa realizar sua
                             // operação (no caso um 'move construtor').

        ptrs.emplace_back(
            new Widget,
            kill_Widget);  // 'emplace_back' é capaz de realizar a construção do
                           // novo elemento 'std::shared_ptr<Widget>'
                           // diretamente dentro do container.

        // A grande questão neste caso é caso ocorra alguma problema durante a
        // realização da operação de inserção do novo elemento
        // ('std::shared_ptr<Widget>').
        //
        // Considera-se a sequẽncia de eventos para o método 'push_back':
        //
        // - Um objeto 'std::shared_ptr<Widget>' é construído para guardar o
        // ponteiro resultante de 'new Widget'.
        //
        // - 'push_back' irá capturar este objeto temporário por referência
        // ('rvalue ref').
        //
        // - Ocorre uma exceção 'out-of-memory' durante o processo de alocação
        // de memória para o novo nó do container.
        //
        // - Enquanto a exceção propaga para fora do método 'push_back', o
        // objeto temporário é destruído, liberando automaticamente o seu
        // respectivo recurso ('Widget').
        //
        // Logo, nestas condições o método 'push_back' se comporta de forma
        // segura, previnindo problemas de vazamento de memória e afins.
        //
        // Agora, a mesma consideração de eventos com o método 'emplace_back':
        //
        // - O ponteiro cru de 'new Widget' é repassado por 'perfect forwarding'
        // para o ponto de criação do novo nó da lista.
        //
        // - Ocorre uma exceção 'out-of-memory' durante o processo de alocação
        // de memória para o novo nó do container.
        //
        // - Enquanto a exceção propaga para fora de 'emplace_back', o ponteiro
        // cru é destruído. Desta maneira se perde a única forma de acessar o
        // objeto 'Widget' alocado na heap, resultando em vazamento de memória.
        //
        // A capacidade dos 'smart pointers' de conseguirem realizar o correto
        // gerenciamento de seu recurso reside no fato de obterem a posse do
        // ponteiro cru de seu respectivo objeto o mais imediatamente possível.
        // Neste caso o método 'emplace_back' retarda essa posse para um momento
        // posterior para possibilitar a construção do objeto final. O que abre
        // brechas para ocorrência de exceções e vazamento de memória.
        //
        // Neste caso, a forma mais correta seria realizar o processo de
        // instanciação do 'smart pointer' em separado com o processo de
        // inserção/'emplace' deste elemento no container:

        auto spw1 = std::shared_ptr<Widget>(new Widget, kill_Widget);
        ptrs.push_back(std::move(spw1));

        auto spw2 = std::shared_ptr<Widget>(new Widget, kill_Widget);
        ptrs.emplace_back(std::move(spw1));

        // Ou seja, quando se trata de containers de objetos que devem realizar
        // gerenciamento de memória, deve-se se ter um cuidade adicional. Nas
        // últimas 2 utilizações, ambos os métodos 'insert' e 'emplace' se
        // comportam de forma muito similar.

        // Outra consideração a ser feita é sobre o tipo de inicialização
        // utilizado pelos métodos 'insert' e 'emplace'. Métodos 'insert'
        // utilizam 'copy initialization', com a sintaxe 'T var = expr;'. Já os
        // métodos 'emplace' fazem uso de 'direct initialization', com a sintaxe
        // 'T var{expr};'.
        // A questão é que os tipos diferentes de inicialização, também possuem
        // diferenças no seu comportamento que, por consequẽncia, resultam em
        // diferenças entre os métodos 'insert' e 'emplace'.
        // Para inicializações do tipo 'copy initialization' não é permitido o
        // uso de construtores do tipo 'explicit'. Já para inicializações do
        // tipo 'direct initialization', é permitido o uso de construtores do
        // tipo 'explicit'. O que por consequẽncia significa que os métodos de
        // 'insert' não fazem uso de 'explict constructors', enquanto que os
        // métodos 'emplace' fazem.
        // Por fim, ao se fazer uso de métodos 'emplace', deve-se prestar um
        // maior cuidado com o tipo dos argumentos informados, pois até os
        // construtores do tipo 'explict' poderão ser utilizados pelo
        // compilador.
    };
};
}  // namespace item_42
