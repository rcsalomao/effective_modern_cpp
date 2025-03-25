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

namespace item_9 {
using boost::typeindex::type_id_with_cvr;
using std::cout;
using std::endl;
using std::string;
using std::vector;
namespace rg = std::ranges;
namespace vw = std::views;

template <std::ranges::forward_range Rng>
auto stringify(Rng&& seq) {
    using std::to_string;

    auto b = seq | vw::transform([](const auto& a) { return to_string(a); }) |
             vw::join_with(',') | vw::common;
    return "{" + std::string(std::begin(b), std::end(b)) + "}";
}

template <class T>
struct MyAlloc {
    using value_type = T;

    MyAlloc() = default;

    template <class U>
    constexpr MyAlloc(const MyAlloc<U>&) noexcept {}

    [[nodiscard]] T* allocate(std::size_t n) {
        if (n > std::numeric_limits<std::size_t>::max() / sizeof(T))
            throw std::bad_array_new_length();

        if (auto p = static_cast<T*>(std::malloc(n * sizeof(T)))) {
            report(p, n);
            return p;
        }

        throw std::bad_alloc();
    }

    void deallocate(T* p, std::size_t n) noexcept {
        report(p, n, 0);
        std::free(p);
    }

   private:
    void report(T* p, std::size_t n, bool alloc = true) const {
        std::cout << (alloc ? "Alloc: " : "Dealloc: ") << sizeof(T) * n
                  << " bytes at " << std::hex << std::showbase
                  << reinterpret_cast<void*>(p) << std::dec << '\n';
    }
};
template <class T, class U>
bool operator==(const MyAlloc<T>&, const MyAlloc<U>&) {
    return true;
}
template <class T, class U>
bool operator!=(const MyAlloc<T>&, const MyAlloc<U>&) {
    return false;
}

// 'alias template':
template <typename T>
using MyAllocList1 = std::list<T, MyAlloc<T>>;
// gambiarra para 'typedef' com uma struct:
template <typename T>
struct MyAllocList2 {
    typedef std::list<T, MyAlloc<T>>
        type;  // MyAllocList2<T>::type é sinonimo de std::list<T, MyAlloc<T>>.
};

template <typename T>
class Widget1 {
   private:
    MyAllocList1<T>
        list;  // Já que 'MyAllocList1<T>' foi definido com um 'alias template',
               // este É um tipo não dependente (o compilador sabe que é o nome
               // de um tipo) e portanto, não necessita e nem é permitido, o uso
               // da sintaxe 'typename'.
};

template <typename T>
class Widget2 {
   private:
    typename MyAllocList2<T>::type
        list;  // 'MyAllocList2<T>::type' é um tipo dependente do tipo 'T' (o
               // compilador não tem a certeza de que se trata do nome de um
               // tipo) e portanto, exige a sintaxe 'typename' para o seu uso. O
               // que não é o caso quando se utiliza 'alias'.
};
// como exemplo de um membro de uma classe com nome 'type' que não se trata de
// um tipo:
class Wine {};
template <>
class MyAllocList2<Wine> {  // especialização de 'MyAllocList2' para quando 'T'
                            // for do tipo 'Wine'.
   private:
    enum class WineType { White, Red, Rose };
    WineType type;  // Agora 'type' passa a ser um 'data member'.
    // ...
};

void main() {
    cout << endl;
    // declarações 'alias' são excelentes alternativas ergonômicas para
    // situações com tipagens inconvenientes. Ainda, deve-se dar preferência
    // para 'using' ao invés de 'typedef':
    typedef std::unique_ptr<std::unordered_map<std::string, std::string>>
        UPtrMapSS1;  // existe alternativa melhor ('using');
    using UPtrMapSS2 =
        std::unique_ptr<std::unordered_map<std::string, std::string>>;
    // comparação entre 'typedef' e 'type alias' para ponteiro de funções:
    typedef void (*FP1)(int, const std::string&);
    using FP2 = void (*)(int, const std::string&);

    // Ao contrário de 'typedef', declarações 'aliais' podem ser templatizadas
    // ('alias templates'). ver 'MyAllocList'.
    MyAllocList1<Widget1<int>> lw1;
    MyAllocList2<Widget2<int>>::type lw2;

    // ainda neste tópico, pode-se citar o mecanismo de 'type traits' (da
    // biblioteca <type_traits>) de C++, que nos permite a manipulação e
    // armazenamento de tipos para diversos fins, por exemplo:
    // const int a1{3}; // tipo de 'a1': const int
    std::remove_const<const int>::type a1{3};  // tipo de 'a1': int
    cout << "Tipo de 'a1': " << type_id_with_cvr<decltype(a1)>().pretty_name()
         << endl;
    // const int& a2{3};  // tipo de 'a2': const int&
    std::remove_reference<const int&>::type a2{3};  // tipo de 'a2': const int
    cout << "Tipo de 'a2': " << type_id_with_cvr<decltype(a2)>().pretty_name()
         << endl;
    std::remove_const_t<std::remove_reference<const int&>::type> a3{
        3};  // tipo de 'a3': int
    cout << "Tipo de 'a3': " << type_id_with_cvr<decltype(a3)>().pretty_name()
         << endl;
    std::add_lvalue_reference_t<const int> a4{3};  // tipo de 'a4': const int&
    cout << "Tipo de 'a4': " << type_id_with_cvr<decltype(a4)>().pretty_name()
         << endl;
    std::remove_reference_t<int&&> a5{3};  // tipo de 'a5': int
    cout << "Tipo de 'a5': " << type_id_with_cvr<decltype(a5)>().pretty_name()
         << endl;
    // como se percebe, existem duas sintaxes para 'type traits'. uma
    // sintaxe exige a invocação de '::type' ao fim da aplicação da
    // transformação: 'std::transformation<T>::type'. Esta foi definida na
    // standard C++11 e faz uso do mecanismo de 'typedef' com a gambiarra dos
    // 'template structs'. Já a nova sintaxe, com o posfixo '_t' ao fim do nome
    // da transformação: 'std::transformation_t<T>', faz uso do mecanismo de
    // 'alias', definido pela standard C++14.
};
}  // namespace item_9
