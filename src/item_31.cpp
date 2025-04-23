#include <boost/type_index.hpp>
#include <functional>
#include <iostream>
#include <memory>
#include <ranges>
#include <type_traits>
#include <vector>

namespace item_31 {
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
   private:
    int val{5};

   public:
    // Neste caso, a função 'lambda' acaba pegando, implícitamente, o ponteiro
    // 'this' que representa o objeto instanciado pela classe para que se possa
    // ter acesso ao atributo 'val'.
    template <typename C>
    void add_function_1(C& container) {
        cout << val << endl;                             // == 'this->val'
        container.push_back([=]() { return 3 * val; });  // == 'this->val'
        // Agora é possível notar que a função 'lambda' realizou a cópia do
        // ponteiro 'this' e não do atributo 'val', como se havia pensado. Desta
        // forma, com a invocação da respectiva função 'lambda' após a
        // destruição do objeto 'Widget' instanciado, tem-se 'UB' devido ao
        // 'dangling pointer'.
    };

    template <typename C>
    void add_function_2(C& container) {
        cout << val << endl;

        // Uma forma alternativa de se realizar a cópia do valor desejado e
        // ainda fazendo o uso de captura default por valor:
        // auto val_copy = val;
        // container.push_back([=]() { return 3 * val_copy; });

        // Adicionalmente, é ainda mais vantajoso definir uma variável local da
        // função 'lambda' e instanciá-la por meio de uma cópia do valor
        // desejado ('[val = val]').
        container.push_back([val_local = val]() { return 3 * val_local; });
    };
};

void main() {
    // No contexto de funções 'lambda', existem dois modos de captura 'default':
    // 'por referência' ou 'por valor'. Captura padrão por referência pode
    // resultar em 'dangling references'. Já a captura padrão por valor também
    // pode resultar em 'dangling pointer' (como no caso do ponteiro 'this'),
    // indicando, portanto, que 'lambdas' com captura padrão por valor não são
    // tão isoladas quanto se imagina.
    {
        // No caso de captura padrão por referência, há a possibilidade do
        // objeto, cuja referência está sendo utilizada no contexto da função
        // 'lambda', acabar sendo destruído (seja deliberadamente ou pelo fim do
        // escopo'). Desta forma a referência acaba se tornando uma 'dangling
        // reference', resultando em 'undefined behaviour' (UB).
        cout << endl;
        cout << "vector<std::function<int()>> functions;" << endl;
        vector<std::function<int()>> functions;
        cout << "auto add_funtion = [&functions]() {\n"
                "    int a1{5};\n"
                "    cout << \"a1: \" << a1 << endl;\n"
                "    functions.push_back([&]() {\n"
                "        return a1;\n"
                "    });\n"
                "};"
             << endl;
        auto add_funtion = [&functions]() {
            int a1{5};  // variável definida em escopo local de 'add_function'.
            cout << "a1: " << a1 << endl;
            functions.push_back([&]() {  // Captura 'default' por referência.
                return a1;
            });  // criação e adição de função lambda no vetor de funções
                 // definido em escopo superior.
        };
        cout << "add_funtion();" << endl;
        add_funtion();  // Execução do trecho de código definido para
                        // 'add_funtion'. Com o fim do respectivo escopo, a
                        // variável local 'a1' é destruída e resta apenas uma
                        // 'dangling reference' junto com a função lambda que
                        // foi adicionada ao vetor 'functions'.
        cout << "functions[0](): " << functions[0]() << endl;  // UB.

        // Usar referências em funções 'lambda' não é um problema por si só. O
        // problema é quando a função lambda possui um tempo de vida maior que
        // os objetos cujas referências foram capturadas. Mas se a função lambda
        // cumprir seu papel no mesmo tempo ou antes do tempo de vida dos
        // objetos referenciados, então não haverá problema. Tem-se como exemplo
        // o uso de funções lambda junto com as funções de algoritmos da 'stl'.
    };
    {
        // Uma maneira de contornar este problema, é realizar captura padrão por
        // valor:
        cout << endl;
        cout << "vector<std::function<int()>> functions;" << endl;
        vector<std::function<int()>> functions;
        cout << "auto add_funtion = [&functions]() {\n"
                "    int a1{5};\n"
                "    cout << \"a1: \" << a1 << endl;\n"
                "    functions.push_back([=]() {\n"
                "        return a1;\n"
                "    });\n"
                "};"
             << endl;
        auto add_funtion = [&functions]() {
            int a1{5};  // variável definida em escopo local de 'add_function'.
            cout << "a1: " << a1 << endl;
            functions.push_back([=]() {  // Captura 'default' por valor.
                return a1;
            });  // criação e adição de função lambda no vetor de funções
                 // definido em escopo superior.
        };
        cout << "add_funtion();" << endl;
        add_funtion();  // Execução do trecho de código definido para
                        // 'add_funtion'. Com o fim do respectivo escopo, a
                        // variável local 'a1' é destruída. Entretanto não
                        // haverá problema pois o valor do objeto original foi
                        // copiado inteiramente para dentro do escopo da função
                        // lambda adicionada à 'functions'.
        cout << "functions[0](): " << functions[0]() << endl;  // UB.
    };
    {
        // Já no caso de captura padrão por valor, o problema surge quando se
        // realiza a cópia de um ponteiro. Este ponteiro pode, em algum momento
        // qualquer ao longo do tempo de vida da função lambda, se tornar um
        // 'dangling pointer' (ter seu recurso deletado por algum outro
        // processo).
        cout << endl;
        cout << "vector<std::function<int()>> functions;" << endl;
        vector<std::function<int()>> functions;
        cout << "template <typename C>\n"
                "void Widget::add_function_1(C& container) {\n"
                "    cout << val << endl;\n"
                "    container.push_back([=]() { return 3 * val; });\n"
                "};"
             << endl;
        cout << "{\n"
                "    auto w = make_unique<Widget>();\n"
                "    w->add_function_1(functions);\n"
                "}"
             << endl;
        {
            auto w = mu<Widget>();
            w->add_function_1(functions);
        }
        cout << "functions[0](): " << functions[0]() << endl;  // UB.
        // Com a destruição do objeto 'Widget' instanciado, uma tentativa de
        // acessar seu valor (ou seja, seu ponteiro 'this') por meio da
        // função 'lambda' em 'functions' gera UB.
    };
    {
        // Uma forma de contornar é realizando a devida cópia do valor
        // pretendido para dentro do escopo da função 'lambda'. Desta forma se
        // evita de correr o risco de realizar cópia não intencional de seu
        // ponteiro.
        cout << endl;
        cout << "vector<std::function<int()>> functions;" << endl;
        vector<std::function<int()>> functions;
        cout << "template <typename C>\n"
                "void Widget::add_function_2(C& container) {\n"
                "    cout << val << endl;\n"
                "    container.push_back([val = val]() { return 3 * val; });\n"
                "};"
             << endl;
        cout << "{\n"
                "    auto w = make_unique<Widget>();\n"
                "    w->add_function_2(functions);\n"
                "}"
             << endl;
        {
            auto w = mu<Widget>();
            w->add_function_2(functions);
        }
        cout << "functions[0](): " << functions[0]() << endl;  // UB.
    };
    {
        // Por fim, deve-se prestar atenção para o caso de variáveis do tipo
        // 'static'. Mesmo com a captura padrão por valor, não é possível
        // realizar a cópia de tais variáveis para dentro de uma função
        // 'lambda'. Qualquer alteração no valor destas variáveis, irá refletir
        // no comportamento de quaisquer funções 'lambda' que estiverem fazendo
        // uso das mesmas.
        cout << endl;
        cout << "static int val{3};" << endl;
        static int val{3};
        cout << "auto f = [=]() { return val; };" << endl;
        auto f = [=]() { return val; };
        cout << "f(): " << f() << endl;
        cout << "++val;" << endl;
        ++val;
        cout << "f(): " << f() << endl;
        cout << "++val;" << endl;
        ++val;
        cout << "++val;" << endl;
        ++val;
        cout << "f(): " << f() << endl;
        // Em termos práticos é como se a função 'lambda' tivesse pego a
        // variável do tipo 'static' por referência.
    };
};
}  // namespace item_31
