#include <atomic>
#include <boost/type_index.hpp>
#include <chrono>
#include <condition_variable>
#include <future>
#include <iostream>
#include <memory>
#include <mutex>
#include <ranges>
#include <thread>
#include <type_traits>
#include <vector>

namespace item_39 {
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
    using namespace std::chrono_literals;
    // Num contexto de simultaneidade, em determinadas situações se faz
    // necessário que um número tarefas ocorram numa ordem específica. À seguir
    // são apresentadas algumas implementações, com diversos graus de
    // complexidade, para a adequada execução destas tarefas:
    {
        // uso de uma 'std::condition_variable' para realizar a comunicação
        // entre as threads de execução. Neste caso a thread principal irá
        // notificar para uma thread de execução quando que esta poderá acordar
        // e realizar a tarefa:
        cout << endl;
        std::condition_variable cv;
        std::mutex mx;
        bool flag{false};
        std::string message;

        cout << "(1) message: " << message << endl;

        // instanciação de uma thread para execução de uma tarefa. Neste caso a
        // thread fica inativa até que a condição necessária (flag == true) seja
        // definida.
        std::jthread t{[&message, &mx, &cv, &flag]() {
            std::unique_lock<std::mutex> lock{mx};
            // while (!flag) cv.wait(lock);
            cv.wait(lock, [&flag]() {
                return flag;
            });  // Necessário definir condicionais para liberar a execuçao da
                 // thread. Podem ocorrer destravamentos espúrios das threads
                 // que estejam esperando. Para o caso desta API, uma função
                 // predicado booleana controla se a thread continuará dormente
                 // ou não. O valor retornado 'false' mantém a thread dormente,
                 // enquanto o valor 'true' acorda a thread e libera a sua
                 // execução.
            message = "Hello World!";
        }};

        std::this_thread::sleep_for(100ms);
        cout << "(2) message: " << message << endl;
        // Mesmo após se passar um tempo considerável, a thread de execução
        // continua dormente e não irá executar sua tarefa até que as variáveis
        // 'flag' e 'cv' sejam corretamente definidas.

        flag = true;
        cv.notify_one();
        // Definição de 'flag' e liberação da 'std::condition_variable'.

        std::this_thread::sleep_for(
            5ms);  // Para que dê tempo da thread de execução realizar a sua
                   // tarefa e operação de 'join'.
        cout << "(3) message: " << message << endl;
    };
    {
        // Também é válido para o outro sentido. A thread principal fica
        // esperando a thread de execução terminar a realização de sua tarefa:
        cout << endl;
        std::condition_variable cv;
        std::mutex mx;
        bool flag{false};
        std::string message;

        cout << "(1) message: " << message << endl;

        std::jthread t{[&message, &mx, &cv, &flag]() {
            std::this_thread::sleep_for(10ms);
            {
                std::unique_lock<std::mutex> lock{mx};
                message = "Hello World!";
                flag = true;
            }
            cv.notify_one();
        }};

        // std::this_thread::sleep_for(50ms);
        cout << "(2) message: " << message
             << endl;  // Pode ser que a thread termine a sua execução antes
                       // deste ponto ou não. Não há garantias de que a variável
                       // 'message' já esteja com o seu valor.

        {
            std::unique_lock<std::mutex> lock{mx};
            // while (!flag) cv.wait(lock);
            cv.wait(lock, [&flag]() {
                return flag;
            });  // Aqui a thread principal espera a notificação
                 // ('cv.notify_one()') e confirmação pela 'flag' de que
                 // 'message' já possui o seu valor calculado.
            cout << "(3) message: " << message << endl;
        }
    };
    {
        // De forma mais sucinta e direta, pode-se também fazer uso
        // 'std::promise<T>'/'std::future<T>' com o tipo 'T' como 'void'. Para o
        // primeiro caso em que a thread de execução deve esperar até que a
        // thread principal indique quando pode começar a realizar a tarefa:
        cout << endl;
        std::string message;
        std::mutex mx;
        std::promise<void> pm;
        auto fu = pm.get_future();

        cout << "(1) message: " << message << endl;

        std::jthread t{[&message, &mx, &fu]() {
            fu.wait();  // Aqui a thread irá esperar até que a 'std::promise'
                        // invoque o método 'set_value()'.
            {
                std::unique_lock<std::mutex> lock{mx};
                message = "Hello World!";
            }
        }};

        std::this_thread::sleep_for(100ms);
        cout << "(2) message: " << message << endl;
        // Mesmo após se passar um tempo considerável, a thread de execução
        // continua dormente e não irá executar sua tarefa até que a
        // 'std::promise' seja concluída.

        pm.set_value();  // Cumprimento da 'std::promise' e por consequência
                         // liberação da thread de execução para a realização de
                         // sua tarefa.

        std::this_thread::sleep_for(5ms);
        cout << "(3) message: " << message << endl;
    };
    {
        // Para o segundo caso em que a thread principal deve esperar até que a
        // thread de execução termine de realizar a tarefa:
        cout << endl;
        std::string message;
        std::mutex mx;
        std::promise<void> pm;
        auto fu = pm.get_future();

        cout << "(1) message: " << message << endl;

        std::jthread t{[&message, &mx, &pm]() {
            std::this_thread::sleep_for(10ms);
            {
                std::unique_lock<std::mutex> lock{mx};
                message = "Hello World!";
            }
            pm.set_value();  // Cumprimento da 'std::promise' e liberação da
                             // thread principal.
        }};

        // std::this_thread::sleep_for(50ms);
        cout << "(2) message: " << message
             << endl;  // Pode ser que a thread termine a sua execução antes
                       // deste ponto ou não. Não há garantias de que a variável
                       // 'message' já esteja com o seu valor.

        fu.wait();  // A thread principal irá ser bloqueada até que a
                    // 'std::promise' seja cumprida com a invocação do método
                    // 'set_value()' à partir da thread de execução.

        {
            std::unique_lock<std::mutex> lock{mx};
            cout << "(3) message: " << message << endl;
        }
    }
};
}  // namespace item_39
