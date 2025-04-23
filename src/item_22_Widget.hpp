#pragma once

#include <memory>
#include <string>

using std::string;

class Widget {
    // declarações dos métodos (construtores, destruidores e métodos quaisquer)
    // apenas. As definições são feitas em arquivo fonte '.cpp' específico
    // (./item_22_Widget.cpp).
   public:
    Widget(string nome);
    ~Widget();

    // copy constructor
    Widget(const Widget& other);
    // copy assignment
    Widget& operator=(const Widget& other);

    // move constructor
    Widget(Widget&& other);
    // move assignment
    Widget& operator=(Widget&& other);

    string get_name();

   private:
    // declaração de 'incomplete type' para que se possa realizar a definição do
    // atributo privado 'std::unique_ptr<Impl>' pImpl:
    struct Impl;
    std::unique_ptr<Impl>
        pImpl;  // substitui o ponteiro 'raw' 'Impl* pImpl;' e garante o
                // gerenciamento automático do correspondente recurso alocado.
};
