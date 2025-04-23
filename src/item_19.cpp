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

namespace item_19 {
using boost::typeindex::type_id_with_cvr;
using std::cout;
using std::endl;
using std::string;
using std::vector;
namespace rg = std::ranges;
namespace vw = std::views;

template <typename T>
using uptr = std::unique_ptr<T>;

template <typename T, typename... Args>
uptr<T> mu(Args&&... args) {
    return std::make_unique<T>(std::forward<Args>(args)...);
};

using std::to_string;

template <std::ranges::forward_range Rng>
auto stringify(Rng&& seq) {
    auto b = seq | vw::transform([](const auto& a) { return to_string(a); }) |
             vw::join_with(',') | vw::common;
    return "{" + std::string(std::begin(b), std::end(b)) + "}";
};

class Widget {};

class Widget2 : public std::enable_shared_from_this<Widget2> {
    // Para o caso de uma classe de tipo dependente ('Widget2<T>'), deve-se
    // trazer para o escopo da classe a função '(...)::shared_from_this':
    // using std::enable_shared_from_this<Widget2<T>>::shared_from_this;

    // vector<std::shared_ptr<Widget2>>* _v;
    // int _i{0};
    // Widget2(vector<std::shared_ptr<Widget2>>* v, int i) {
    //     _v = v;
    //     _i = i;
    // };

    std::shared_ptr<vector<std::shared_ptr<Widget2>>> _v;
    int _i{0};
    Widget2(std::shared_ptr<vector<std::shared_ptr<Widget2>>>& v, int i) {
        _v = v;
        _i = i;
    };

   public:
    // É necessário que, antes da invocação do método '.do_stuff()', o 'control
    // block' referente à classe 'Widget2' já tenha sido criado. para tal,
    // pode-se qualificar o construtor da classe como privado e criar um método
    // 'factory' ('.create(...)') responsável por isntanciar o objeto na heap e
    // retornar um 'std::shared_ptr' para que se possa trabalhar com o objeto:

    // static std::shared_ptr<Widget2> create(vector<std::shared_ptr<Widget2>>*
    // v, int i) {
    static std::shared_ptr<Widget2> create(
        std::shared_ptr<vector<std::shared_ptr<Widget2>>>& v, int i) {
        return std::shared_ptr<Widget2>(new Widget2{v, i});
    }
    void do_stuff() {
        // _v->emplace_back(
        //     this);  // problematáco: se simplesmente retornar um
        //             // ponteiro 'raw', resultará na construção de um
        //             // 'std::shared_ptr' com um novo 'control block' separado
        //             // para cada chamada da função (mesmo que seja de um
        //             // mesmo objeto), o que resultará em erros de memória
        //             // posteriores.

        // _v->emplace_back(
        //     this->shared_from_this());  // Para o caso da classe ser um tipo
        //                                 // dependente ('Widget2<T>').
        _v->emplace_back(shared_from_this());
    }
    int get_i() { return _i; }
};

void main() {
    // 'std::shared_ptr' é um ponteiro que tem a responsabilidade de
    // gerenciamento do seu recurso, mas, ao contrário de 'std::unique_ptr' que
    // tem a posse exclusiva, este realiza posse compartilhada entre todas os
    // objetos 'std::shared_ptr' que gerenciam o mesmo recurso.
    {
        cout << endl;
        // tal qual 'std::unique_ptr', 'std::shared_ptr' também pode ser
        // instanciado com função própria para realização da operação de
        // 'delete':

        auto log_del_1 = [](Widget* w) {
            cout << "Custom deleter 1 for Widget." << endl;
            delete w;
        };
        auto log_del_2 = [](Widget* w) {
            cout << "Custom deleter 2 for Widget." << endl;
            delete w;
        };

        // Entretanto, a função responsável pela desalocação da memória do
        // recurso não faz parte do tipo resultante. Desta forma,
        // 'std::shared_ptr' passa a ser um tipo de ponteiro mais flexível.

        std::unique_ptr<Widget, decltype(log_del_1)> upw1{new Widget,
                                                          log_del_1};
        std::unique_ptr<Widget, decltype(log_del_2)> upw2{new Widget,
                                                          log_del_2};
        cout << "type_id_with_cvr<decltype(upw1)>().pretty_name(): " << endl;
        cout << type_id_with_cvr<decltype(upw1)>().pretty_name() << endl;
        cout << "type_id_with_cvr<decltype(upw2)>().pretty_name(): " << endl;
        cout << type_id_with_cvr<decltype(upw2)>().pretty_name() << endl;
        cout << "std::is_same<decltype(upw1), decltype(upw2)>(): "
             << std::boolalpha << std::is_same<decltype(upw1), decltype(upw2)>()
             << endl;

        cout << endl;

        std::shared_ptr<Widget> spw1{new Widget, log_del_1};
        std::shared_ptr<Widget> spw2{new Widget, log_del_2};
        cout << "type_id_with_cvr<decltype(spw1)>().pretty_name(): " << endl;
        cout << type_id_with_cvr<decltype(spw1)>().pretty_name() << endl;
        cout << "type_id_with_cvr<decltype(spw2)>().pretty_name(): " << endl;
        cout << type_id_with_cvr<decltype(spw2)>().pretty_name() << endl;
        cout << "std::is_same<decltype(spw1), decltype(spw2)>(): "
             << std::boolalpha << std::is_same<decltype(spw1), decltype(spw2)>()
             << endl;

        cout << endl;

        // ponteiros 'std::shared_ptr', mesmo com 'destructors' diferentes,
        // podem ser armazenados em estruturas homogêneas:
        vector<std::shared_ptr<Widget>> vw{spw1, spw2};

        std::shared_ptr<Widget> spw3;
        // 'copy assignment':
        spw3 = spw1;
        // 'copy constructor':
        std::shared_ptr<Widget> spw4{spw3};

        cout << "No fim do escopo e momento de destruição dos ponteiros "
                "'std::shared_ptr', estes automaticamente invocarão as funções "
                "'destructors' dos seus respectivos recursos:"
             << endl;
    };
    // Para 'std::shared_ptr', as regras de criação do 'control block' são:
    //
    // - 'std::make_shared' sempre cria um 'control block'.
    //
    // - Um 'control block' sempre é criado quando um 'std::shared_ptr' é
    // construído à partir de um 'std::unique_ptr'.
    //
    // - Quando o construtor de 'std::shared_ptr' é invocado à partir de um
    // ponteiro 'raw'.
    {
        // auto pw = new Widget;  // ponteiro 'raw'.
        // std::shared_ptr<Widget> spw1(
        //     pw);  // realiza a criação de um 'control block' para '*pw'.
        // std::shared_ptr<Widget> spw2(
        //     pw);  // realiza a criação de um SEGUNDO 'control block' para o
        //           // mesmo objeto '*pw'.

        // no momento de se realizar a destruição dos ponteiros, e portanto dos
        // respectivos recursos, o que acontecerá é que será invocado 'delete'
        // duas vezes para o exato mesmo endereço de memória (objeto),
        // resultando em erro de memória.
    }
    {
        // Exemplo em que o objeto é responsável por gerar ponteiros dele mesmo
        // para outras estruturas de dados (no caso, um vetor de ponteiros
        // 'std::shared_ptr<Widget2>').
        cout << endl;
        auto svspw = std::make_shared<vector<std::shared_ptr<Widget2>>>();
        cout << "svspw = std::make_shared<vector<std::shared_ptr<Widget2>>>();"
             << endl;
        auto w1 = Widget2::create(svspw, 24);
        auto w2 = Widget2::create(svspw, 42);
        auto w3 = Widget2::create(svspw, 8);
        cout << "svspw.size(): " << svspw->size() << endl;
        w1->do_stuff();  // realiza alguma operação e insere um ponteiro de si
                         // mesmo no vetor de ponteiros 'svspw'.
        w2->do_stuff();
        w3->do_stuff();
        cout << "w1->do_stuff(); w2->do_stuff(); w3->do_stuff();" << endl;
        cout << "svspw.size(): " << svspw->size() << endl;
        cout << "*svspw | vw::transform([](auto& a) { return a->get_i(); }): "
             << stringify(*svspw |
                          vw::transform([](auto& a) { return a->get_i(); }))
             << endl;
    }
};
}  // namespace item_19
