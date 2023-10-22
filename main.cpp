/*

Trabalho Barbeiro Dorminhoco

Lázaro José Rodrigues Júnior - 0027627
Julia Gabriella Corrêa Silva - 0048842

*/

#include<iostream>
#include<stdlib.h>
#include<unistd.h>
#include<thread>
#include<errno.h>
#include<semaphore.h>
#include<mutex>
#include<csignal>
#include <vector>

std::vector<std::thread> clientes;

std::mutex mtx;

int const BANCOS = 5;

sem_t sem_mutex,sem_cliente,sem_barbeiro;

int clientes_aguardando = 0;

class Cliente {
public:
    // método para a thread cliente
    void* chegar_cliente(int t) {
        while(1) {
            // esperar pelo semáforo mutex
            sem_wait(&sem_mutex);

            if(clientes_aguardando<BANCOS) {
                clientes_aguardando++;

                mtx.lock();
                std::cout << "Chegou novo Cliente." << std::endl;
                std::cout << "Existem " << BANCOS-clientes_aguardando << " de " <<
                BANCOS << " cadeiras livres para espera, Cliente aguardando sua vez." << std::endl;
                mtx.unlock();

                // liberando os semáforos mutex e cliente
                sem_post(&sem_mutex);
                sem_post(&sem_cliente);

                // esperando pelo semáforo do barbeiro
                sem_wait(&sem_barbeiro);
            } else {
                mtx.lock();
                std::cout << "Não existem cadeiras de espera disponíveis, Cliente foi embora." << std::endl;
                mtx.unlock();

                // liberando o semáforo mutex
                sem_post(&sem_mutex);
            }
            sleep(t);
        }
    }
};

class Barbeiro {
public:
    // método para a thread barbeiro
    void* trabalhar() {
        mtx.lock();
        std::cout << "Não há clientes para atender, o Barbeiro foi dormir. " << std::endl;
        mtx.unlock();
        while(1) {
            if (clientes_aguardando > 0) {
                // esperar pelo post do semáforo pelo cliente
                sem_wait(&sem_cliente);

                mtx.lock();
                std::cout << "Cliente acordou o barbeiro." << std::endl;
                mtx.unlock();

                // esperar pelo post do mutex
                sem_wait(&sem_mutex);

                mtx.lock();
                std::cout << "O Barbeiro está cortando o Cabelo do Cliente " << BANCOS-clientes_aguardando+1 << std::endl;
                mtx.unlock();

                clientes_aguardando--;

                // liberando os semáforos do mutex e do barbeiro
                sem_post(&sem_mutex);
                sem_post(&sem_barbeiro);

                mtx.lock();
                std::cout << "O Barbeiro está dormindo." << std::endl;
                mtx.unlock();

                sleep(5);
            } else {
                mtx.lock();
                std::cout << "Não há clientes em espera, o Barbeiro foi dormir. " << std::endl;
                mtx.unlock();
            }

        }
    }
};

// instanciando as funções barbeiro e cliente
void* barbeiro(Barbeiro b);
void* cliente(Cliente c, int t);

// tratador do sinal SIGINT
void tratarSinal(int signum) {
    std::cout << "\n Encerramos o expediente por hoje, volte amanhã!\n";
    exit(signum);
}

int main() {
    signal(SIGINT, tratarSinal); // registrando o sinal SIGINT e o seu tratador
    Barbeiro b; // instanciando a classe Barbeiro
    srand((unsigned)time(0)); // inicializando uma semente de aleatoriedade
    int tempo; // variável para receber o tempo aleatório

    // inicializando os semáforos
    sem_init(&sem_mutex,0,1);
	sem_init(&sem_cliente,0,0);
	sem_init(&sem_barbeiro,0,1);

	// inicializando a thread barbeiro
    std::thread thread_barbeiro (barbeiro, b);

    // inicializando as threads clientes com tempos aleatórios
    for (int i = 0; i < 10; i++) {
        Cliente c;
        tempo = rand() % 15 + 5;
        clientes.push_back(std::thread(cliente, c, tempo));
    }

    std::cout << "O expediente foi iniciado, bem vindo a todos!\n";

    // finalizando as threads cliente e barbeiro
    thread_barbeiro.join();
    for (auto &t : clientes)
        t.join();

    return 0;
}

// função para chamar o método da classe Barbeiro
void* barbeiro(Barbeiro b) {
    b.trabalhar();
}

// função para chamar o método da classe Cliente
void* cliente(Cliente c, int t) {
    c.chegar_cliente(t);
}
