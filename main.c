#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

#ifdef _WIN32
    #define LIMPAR_TELA() system("cls")
#else
    #define LIMPAR_TELA() system("clear")
#endif

#define LIMPAR_BUFFER()                                 \
    {                                                   \
        int c;                                          \
        while ((c = getchar()) != '\n' && c != EOF);    \
    }

#define AVANCAR()                                       \
    {                                                   \
        printf("Pressione ENTER para continuar...");    \
        LIMPAR_BUFFER();                                \
        getchar();                                      \
        LIMPAR_TELA();                                  \
    }

typedef unsigned int uint;
/**
 * Medico
 * - ID:            unsigned int
 * - Nome:          String
 * - Especialidade: String
 * 
 * Endereco
 * - Rua:           String
 * - Bairro:        String
 * - Cidade:        String
 * - Numero:        String
 * - Complemento:   String
 * 
 * Telefone
 * - DDD:           String
 * - Numero:        String   
 * 
 * Paciente
 * - ID:            unsigned int
 * - Nome:          String
 * - Identidade     unsigned int
 * -> Endereço      Endereco
 * -> Telefone      Telefone
 * - Sexo:          char
 * 
 * 
 * Horario
 * - Horas:         unsigned int
 * - Minutos:       unsigned int
 * 
 * Duração
 * - Tempo:         Horario
 * 
 * Data
 * - Dia            unsigned int
 * - Mes            unsigned int
 * - Ano            unsigned int
 * 
 * Consulta
 * - Numero:        unsigned int
 * -> Medico        Medico
 * -> Paciente      Paciente
 * -> Horario       Horario
 * -> Duração       Duracao
 * -> Data          DD/MM/YYYY
 */

typedef struct {
    uint ID;
    char *nome;
    char *especialidade;
} Medico;

typedef struct{
    char *rua;
    char *bairro;
    char *cidade;
    char *numero;
    char *complemento;
} Endereco;

typedef struct {
    char *ddd;
    char *numero;
} Telefone;

typedef struct {
    uint ID;
    char *nome;
    char *identidade;
    Endereco endereco;
    Telefone telefone;
    char sexo;
} Paciente;

typedef struct {
    uint horas;
    uint minutos;
} Horario;

typedef struct {
    Horario tempo;
} Duracao;

typedef struct {
    uint dia;
    uint mes;
    uint ano;
} Data;

typedef struct {
    uint numero;
    Medico medico;
    Paciente paciente;
    Horario horario;
    Duracao duracao;
    Data data;
} Consulta;

void menu(int *opcao) {
    printf("HOSPITAL CURAS INACREDITAVEIS\n");
    printf("1 - Consulta                 \n");
    printf("2 - Paciente                 \n");
    printf("3 - Medico                   \n");
    printf("4 - Relatorio                \n");
    printf("5 - Sair                     \n");
    printf("Escolha uma opcao: ");
    scanf("%d", opcao);
}

void avancar() {
    printf("Pressione ENTER para continuar...");
    LIMPAR_BUFFER();
    getchar();
    LIMPAR_TELA();
}

void opcao_sair() {
    LIMPAR_TELA();
    printf("Saindo do programa...\n\n");
    AVANCAR();
}

void opcao_avancar() {
    LIMPAR_TELA();
    printf("Insira um número válido.\n\n");
    AVANCAR();
}

void menu_consulta() {
    int opcao;
    do {
        printf("CONSULTA\n");
        printf("1 - Pesquisar consulta      \n");
        printf("2 - Nova consulta           \n");
        printf("3 - Alterar consulta        \n");
        printf("4 - Remover consulta        \n");
        printf("5 - Voltar ao menu          \n");
        printf("Escolha uma opcao: ");
        scanf("%d", opcao);

        switch (opcao)
        {
            case 1: // Pesquisar
                
                break;

            case 2: // Add
                
                break;

            case 3: // Edit
                
                break;

            case 4: // Delete
                
                break;

            case 5: // Go back
                return;
                break;
            
            default:
                opcao_avancar();
                break;
        }
    } while (opcao != 5);
}

void menu_paciente() {
    int opcao;
    do {
        printf("PACIENTE\n");
        printf("1 - Pesquisar paciente      \n");
        printf("2 - Nova paciente           \n");
        printf("3 - Alterar paciente        \n");
        printf("4 - Remover paciente        \n");
        printf("5 - Voltar ao menu          \n");
        printf("Escolha uma opcao: ");
        scanf("%d", opcao);
        
        switch (opcao)
        {
            case 1: // Pesquisar
                
                break;

            case 2: // Add
                
                break;

            case 3: // Edit
                
                break;

            case 4: // Delete
                
                break;

            case 5: // Go back
                return;
                break;
            
            default:
                opcao_avancar();
                break;
        }
    } while (opcao != 5);
}

void menu_medico() {
    int opcao;
    do {
        printf("MEDICO\n");
        printf("1 - Pesquisar medico      \n");
        printf("2 - Nova medico           \n");
        printf("3 - Alterar medico        \n");
        printf("4 - Remover medico        \n");
        printf("5 - Voltar ao menu        \n");
        printf("Escolha uma opcao: ");
        scanf("%d", opcao);
        
        switch (opcao)
        {
            case 1: // Pesquisar
                
                break;

            case 2: // Add
                
                break;

            case 3: // Edit
                
                break;

            case 4: // Delete
                
                break;

            case 5: // Go back
                return;
                break;
            
            default:
                opcao_avancar();
                break;
        }
    } while (opcao != 5);
}

int main(int argc, char const *argv[]) {
    setlocale(LC_ALL, "pt_BR.UTF-8");

    int opcao;
    do {
        menu(&opcao);
        switch (opcao)
        {
            case 1: // Consulta
                
                break;

            case 2: // Paciente
                
                break;

            case 3: // Médico
                
                break;

            case 4: // Relatórios
                
                break;

            case 5: // Sair
                opcao_sair();
                break;
            
            default:
                opcao_avancar();
                break;
        }
    } while (opcao != 5);

    return 0;
}


// typedef struct {
//     Medico atual;
//     Medico *prox;
// } MedicoLista;

// typedef struct {
//     Paciente atual;
//     Paciente *prox;
// } PacienteLista;

// typedef struct {
//     Consulta atual;
//     Consulta *prox;
// } ConsultaLista;