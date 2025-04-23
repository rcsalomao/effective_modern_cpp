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

namespace item_21 {
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

class Widget {};

int calc_priority(int&& arg) {
    int prio = 42 / arg;
    try {
        return prio;
    } catch (...) {
        throw std::runtime_error("Algum erro na função 'calc_priority'.");
    };
}

void process_widget(auto&& ptr, int priority) {};

void main() {
    // Junto com os 'smart pointers', a 'stl' também provém funções específicas
    // para realizar a construção destes ponteiros, 'std::make_unique' e
    // 'std::make_shared'.
    {
        cout << endl;
        // A forma com que estas funções são invocadas é:
        auto uptr = std::make_unique<double>(42);
        cout << "auto uptr = std::make_unique<double>(42);" << endl;
        cout << "tipo de 'uptr': "
             << type_id_with_cvr<decltype(uptr)>().pretty_name() << endl;
        auto sptr = std::make_shared<double>(24);
        cout << "auto sptr = std::make_shared<double>(24);" << endl;
        cout << "tipo de 'sptr': "
             << type_id_with_cvr<decltype(sptr)>().pretty_name() << endl;
        // Como vantagem, percebe-se que a sintaxe é mais compacta, necessitando
        // declarar o tipo do objeto apenas 1 vez.
    };
    {
        // Outra vantagem, e talvez a mais importante, é o fato de que as
        // funções de construção do objeto ('new T') e de construção do ponteiro
        // correspondente ('std::unique_ptr<T>{}') acontecem no mesmo local, na
        // função 'std::make_unique' ou 'std::make_shared'. Evita-se desta forma
        // potencial vazamento de memória a depender da situação:

        // A seguinte função, na forma que está sendo utilizada, pode acabar
        // resultando em vazamento de memória. O compilador possui a liberdade
        // de rearranjar a ordem das operações de uma mesma instrução. Neste
        // caso, antes que se possa realizar a avaliação da função
        // 'process_widget', deve-se primeiro realizar o cálculo da prioridade
        // pela função 'calc_priority', o instanciamento do objeto 'new
        // Widget{}' e a criação do ponteiro 'std::shared_ptr<Widget>'.
        // Entretanto, pode acontecer do compilador ordenar a execução das 3
        // operações na seguinte ordem: 'new Widget' > 'calc_priority' >
        // 'std::shared_ptr'. Caso aconteça algum erro em 'calc_priority' que
        // resulte numa exception ou erro, acabaria impedindo a criação do
        // ponteiro 'std::shared_ptr' que daria acesso ao recurso do objeto
        // 'Widget' recém criado. Desta forma resultando em vazamento de
        // memória.
        process_widget(std::shared_ptr<Widget>(new Widget{}),
                       calc_priority(24));
        // O uso da função 'std::make_shared<Widget>' forçaria com que a
        // operação de criação do objeto 'Widget' e do seu correspondente
        // ponteiro aconteçam de forma conjunta. Evitando-se assim o problema
        // anteriormente abordado.
        process_widget(std::shared_ptr<Widget>(), calc_priority(24));
        // Outra forma de se evitar que os processos de criação do objeto e de
        // seu ponteiro sejam interrompidos é realizando as operações de forma
        // sequencial:
        auto sp = std::shared_ptr<Widget>(
            new Widget{});  // criação do objeto 'Widget' e de seu ponteiro
                            // 'std::shared_ptr' de forma isolada numa mesma
                            // instrução.
        process_widget(std::move(sp), calc_priority(24));
        // O mesmo caso se aplica para 'std::unique_ptr' e 'std::make_unique'.
    };
    {
        // Em comparação com 'std::unique_ptr'/'std::shared_ptr',
        // 'std::make_shared'/'std::make_unique' também são capazes de resultar
        // em código mais eficiente. No processo de instanciar o objeto e seu
        // ponteiro, 'std::make_shared'/'std::make_unique' realizam apenas 1
        // alocação de memória (1 'control block' para gerenciar tanto o objeto
        // como o ponteiro), ao contrário de 'std::shared_ptr'/'std::unique_ptr'
        // que realizam 2 alocações de memória (1 para o objeto e 1 para o
        // 'control block').
    };
    {
        cout << endl;
        // Entretanto, 'std::shared_ptr'/'std::unique_ptr' possuem desvantagens
        // frente ao uso direto do construtor dos respectivos ponteiros.
        // 'std::make_shared'/'std::make_unique' não permitem a descrição de
        // função alocadora própria. Para tal é necessário utilizar a própria
        // sintaxe do construtor do 'smart pointer':
        auto widget_deleter = [](Widget* pw) {};
        std::unique_ptr<Widget, decltype(widget_deleter)> upw(new Widget{},
                                                              widget_deleter);
        std::shared_ptr<Widget> spw(new Widget{}, widget_deleter);

        // Outra característica, é que as funções geradoras utilizam a sintaxe
        // '()' para a construção do respectivo objeto, ao invés da sintaxe '{}'
        // (inicialização universal):
        auto upv = std::make_unique<std::vector<int>>(
            10, 20);  // criação de um vetor de 10 posições do tipo 'int',
                      // inicializadas com o valor de 20.
        cout << "auto upv = std::make_unique<std::vector<int>>(10, 20);"
             << endl;
        cout << "upv->size(): " << upv->size() << endl;
        // Caso se deseje utilizar 'braced initialization' e
        // 'std::initializer_list', deve-se criar a lista em instrução separada
        // para posteriormente invocar as funções geradoras:
        auto init_list = {10, 20};
        auto spv = std::make_shared<std::vector<int>>(init_list);
        cout << "auto init_list = {10, 20};" << endl;
        cout << "auto spv = std::make_shared<std::vector<int>>(init_list);"
             << endl;
        cout << "spv->size(): " << spv->size() << endl;

        // Ainda sobre 'std::make_shared', o seu processo de alocação de memória
        // resulta em apenas 1 alocação de memória (o 'control block') que é
        // responsável por gerenciar o objeto instanciado, o número de ponteiros
        // do tipo 'std::shared_ptr' e também o número de ponteiros do tipo
        // 'std::weak_ptr'. Desta forma, no momento de realizar a operação de
        // 'delete' e liberação de memória, esta somente poderá ser efetivada
        // quando o 'control block' for liberado. Neste caso isto ocorrerá
        // quando o número de 'str::shared_ptr' chegar a 0 E o número de
        // 'std::weak_ptr' também chegar a 0.
        // Diferentemente, o processo do construtor 'std::shared_ptr' realiza 2
        // alocações de memória, uma responsável pelo objeto e outra responsável
        // pelo 'control block'. Desta forma a operação de 'delete' do objeto e
        // respectiva liberação de memória acontece de forma independente da
        // quantidade de ponteiros 'std::shared_ptr' e 'std::weak_ptr' ainda
        // presentes.
    }
};
}  // namespace item_21
