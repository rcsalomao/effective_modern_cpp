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

namespace item_20 {
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

class Foo {
   public:
    int x;
};

void main() {
    // Para além de 'std::unique_ptr' (posse exclusiva do recurso) e
    // 'std::shared_ptr' (posse compartilhada do recurso), a 'stl' também provém
    // 'std::weak_ptr' que cumpre papel específico não coberto pelos dois
    // ponteiros anteriores. 'std::weak_ptr' trata-se de um 'smart pointer',
    // intimamente atrelado à 'std::shared_ptr', mas que não é resposável por
    // realizar o gerenciamento de memória (não possui a posse do recurso ao
    // qual está apontando). Sua principal vantagem, ao contrário do ponteiro
    // 'raw', é poder indicar quando o seu respectivo recurso já foi deletado.
    // Trata-se de um ponteiro que pode se tornar 'dangling' e tem a capacidade
    // de indicar esta informação.
    // Contexto usuais de aplicação são no uso de 'caches', padrão de projeto
    // 'Observer', evitar dependência cíclica entre ponteiros, dentre outros.
    {
        cout << endl;
        auto consult_wpf = [](const std::weak_ptr<Foo>& wpf) {
            cout << "const auto& a = wpf.lock(); ";
            if (const auto& a = wpf.lock()) {
                cout << "a->x: " << a->x << endl;
            } else {
                cout << "Expired 'wpf' pointer!" << endl;
            }
        };

        auto spf = std::make_shared<Foo>(42);
        cout << "auto spf = std::make_shared<Foo>(42);" << endl;
        // std::weak_ptr<Foo> wpf;
        // wpf = spf;
        std::weak_ptr<Foo> wpf{spf};
        cout << "std::weak_ptr<Foo> wpf{spw};" << endl;
        consult_wpf(wpf);
        spf.reset(new Foo{24});
        cout << "spf.reset(new Foo{24});" << endl;
        consult_wpf(wpf);
        wpf = spf;
        cout << "wpf = spf;" << endl;
        consult_wpf(wpf);
        spf.reset();
        cout << "spf.reset();" << endl;
        try {
            cout << "std::shared_ptr<Foo> spf2{wpf};" << endl;
            std::shared_ptr<Foo> spf2{wpf};
        } catch (const std::exception& e) {
            cout << "An exception of type {" << e.what() << "} has been thrown!"
                 << endl;
        } catch (...) {
            cout << "Another thing has been thrown! :x" << endl;
        }
    };
    {
        cout << endl;
        std::weak_ptr<int> gw;
        cout << "std::weak_ptr<int> gw;" << endl;

        auto observe = [&gw](string space = "") {
            cout << space << "gw.use_cout(): " << gw.use_count() << "; ";
            cout << "auto sptr = gw.lock(); ";
            if (auto sptr = gw.lock()) {
                cout << "*sptr: " << *sptr << endl;
            } else {
                cout << "Expired pointer!" << endl;
            }
        };
        {
            cout << "{" << endl;
            auto sp = std::make_shared<int>(24);
            cout << "    auto sp = std::make_shared<int>(24);" << endl;
            gw = sp;
            cout << "    gw = sp;" << endl;
            observe("    ");
            cout << "}" << endl;
        }
        observe();
    };
};
}  // namespace item_20
