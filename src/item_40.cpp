#include <atomic>
#include <boost/type_index.hpp>
#include <iostream>
#include <memory>
#include <ranges>
#include <thread>
#include <type_traits>
#include <vector>

namespace item_40 {
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

void main() {
    // No contexto de simultaneidade, é importante ter em mente a diferença
    // entre um objeto de tipo 'std::atomic<T>' e a qualificação de tipo
    // 'volatile'.
    //
    // Pode-se entender 'std::atomic<T>' como um objeto que garante a qualidade
    // de 'thread-safety'. Suas operações são atômicas, isto é, apenas uma
    // 'thread' poderá ler e escrever por vez e seu valor poderá ser acessado
    // igualmente por todas as 'threads'. É basicamente um objeto com um sistema
    // próprio de 'lock' e 'mutex' embutido, mas como faz uso de primitivas
    // especiais próprias, consegue ser mais eficiente e otimizado nestas
    // operações. Só é permitido utilizar tipos trivialmente copiáveis (e que
    // contenham como atributos variáveis de tipos também trivialmente
    // copiáveis) para instanciar o tipo 'T' que compõe 'std::atomic<T>'
    {
        cout << endl;

        cout << "std::atomic<int> a_count{0};" << endl;
        std::atomic<int> a_count{
            0};  // a variável 'atomic<T>' já propaga e compartilha seu estado
                 // para outras 'threads' que podem acessar e escrever nela.

        cout << "int count{0};" << endl;
        int count{0};
        cout << "volatile int v_count{0};" << endl;
        volatile int v_count{0};

        cout << "auto f = [&a_count, &count]() {\n"
                "    for (int n{10000}; n; --n) {\n"
                "        ++a_count;\n"
                "        ++count;\n"
                "        v_count += 1;\n"
                "    }\n"
                "};"
             << endl;
        auto f = [&a_count, &count, &v_count]() {
            for (int n{10000}; n; --n) {
                ++a_count;  // operação de incremento realizada de forma
                            // atômica. Cada thread poderá realizar esta
                            // operação apenas uma por vez. Esta operação em
                            // específico ('++') é uma operação do tipo
                            // 'read-modify-write' (RMW).
                ++count;    // Esta operação não está protegida, estando
                            // susceptível á condição de corrida.
                v_count += 1;
            }
        };

        cout << "{\n"
                "    std::vector<std::jthread> pool;\n"
                "    for (int n = 0; n < 10; ++n) pool.emplace_back(f);\n"
                "}"
             << endl;
        {
            std::vector<std::jthread> pool;
            for (int n = 0; n < 10; ++n) pool.emplace_back(f);
        }
        cout << "a_count: " << a_count
             << endl;  // nesta linha, estão pelo menos duas operações com o
                       // objeto 'std::atomic<T>'. Uma é a leitura de seu valor
                       // e outra é a representação deste valor no terminal por
                       // meio do operador '<<' no 'cout'. Apenas a leitura do
                       // valor do objeto é realizada de forma atômica. Nada
                       // impede que outra thread venha modificá-lo enquanto o
                       // valor deste objeto estiver sendo representado no
                       // terminal .
        cout << "count: " << count << endl;
        cout << "v_count: " << v_count << endl;
    };
    // Em contraste, objetos com a qualificação de 'volatile' são basicamente
    // objetos cuja sua memória não se comporta de forma "normal". Neste
    // sentido, um objeto cuja memória têm comportamento "normal" possui
    // certas características que permitem ao compilador realizar operações de
    // otimização e supressão de suas operações. Ao indicar que determinado
    // objeto possui a qualificação de 'volatile', estes procedimentos
    // implícitos de otimização realizados pelo compilador são automaticamente
    // inibidos. E desta forma, percebe-se que objetos 'volatile' não possuem
    // segurança nenhuma em contexto de simultaneidade, tal qual um objeto de
    // tipo convencional.
    {
        // Exemplo de otimização em objetos de memória de comportamento normal:
        int x{0};

        auto y = x;
        y = x;   // Esta operação é redundante com a operação anterior e,
                 // portanto, é suprimida pelo compilador.
                 //
        x = 10;  // Como imediatamente após possui outra operação de atribuição
                 // para 'x', então esta operação será suprimida pelo
                 // compilador.
        x = 20;

        // O código final ficaria nos moldes de:
        // auto y = x;
        // x = 20;

        // Para o caso de um objeto 'std::atomic<T>':
        std::atomic<int> xa{0};

        std::atomic<int> ya{xa.load()};  // Operações de cópia são deletadas
                                         // para objetos 'std::atomic<T>'
        ya.store(xa.load());  // Aqui são duas operações: uma de leitura da
                              // variável 'xa' e outra de escrita deste valor na
                              // variável 'ya'.
    };
    {
        // Variáveis 'volatile' são objetos de memória "especial". Neste sentido
        // têm-se como exemplo mais comum variáveis utilizadas para
        // 'memory-mapped I/O'. Locais na memória utilizados para realizar
        // comunicação com periféricos, sensores externos, displays,
        // impressoras, portas de rede, etc. ao invés de realizar escrita e
        // leitura em locais convencionais na memória RAM. Desta forma
        // leituras repetidas não são mais redundantes, já que podem significar
        // leituras de um periférico ou sensor. Igualmente, atribuições
        // repetidas podem signifcar envio de comandos ou sinal para determinado
        // periférico ou equipamento. Não faz sentido então o compilador
        // suprimir determinadas instruções "redundantes" pois estaria desta
        // forma omitindo comandos e sequẽncias fundamentais para o correto
        // funcionamento.
        // Breve exemplo:
        volatile int x{0};
        auto y = x;
        y = x;  // nova leitura de 'x'.

        x = 10;  // escrita em 'x'.
        x = 20;  // nova escrita em 'x'.
    }
    // Para variáveis de tipo 'std::atomic', como a ordem das operações
    // podem ser importantes dentro de um contexto de simultaneidade, os
    // processos de otimização do compilador são restringidos. Todas as
    // operações definidas no código fonte anteriores à uma escrita de uma
    // variável 'std::atomic<T>' não podem ser rearranjadas para uma posição
    // posterior.
    //
    // Já para o caso de variáveis 'volatile', não há a mesma restrição e o
    // compilador e hardware são livre para realizar o reordenamento que
    // melhor considerarem.
    //
    // Em resumo, variáveis:
    //
    // - 'std::atomic<T>' são úteis em contexto de simultaneidade, mas não para
    // realizar acesso em locais especias de memória.
    //
    // - 'volatile' são úteis para realizar acesso de variáveis de memória
    // especial, mas não em contexto de simultaneidade.
    //
    {
        // Por fim, é plenamente possível fazer uso de ambos, caso seja
        // necessário:
        volatile std::atomic<int> vai;
        // Ou seja, 'vai' pode ser útil caso seja necessário acessar um local de
        // 'memory-mapped I/O', de forma simultânea por meio de diversas
        // threads.
    }
};
}  // namespace item_40
