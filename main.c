#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <time.h>

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

#define MAX_PACIENTES 50
#define MAX_MEDICOS 20
#define MAX_CONSULTAS 100

typedef unsigned int uint;

typedef struct MEDICO {
    uint ID;
    char nome[100];
    char especialidade[100];
} Medico;

typedef struct  ENDERECO {
    char rua[100];
    char bairro[100];
    char cidade[100];
    char numero[10];
    char complemento[50];
} Endereco;

typedef struct TELEFONE {
    char ddd[4];
    char numero[16];
} Telefone;

typedef struct PACIENTE {
    uint ID;
    char nome[100];
    char identidade[20];
    Endereco endereco;
    Telefone telefone;
    char sexo;
} Paciente;

typedef struct HORARIO {
    uint horas;
    uint minutos;
} Horario;

typedef struct DURACAO {
    Horario tempo;
} Duracao;

typedef struct DATA {
    uint dia;
    uint mes;
    uint ano;
} Data;

typedef struct CONSULTA {
    uint numero;
    Medico *medico;
    Paciente *paciente;
    Horario horario;
    Duracao duracao;
    Data data;
} Consulta;

// ------------------------------------------------------------------

void lerDadosPacientes(Paciente **pacientes, int *tamanho) {
    FILE *file = fopen("pacientes.bin", "rb");
    if (file == NULL) {
        *tamanho = 0;
        *pacientes = NULL;
        return;
    }
    fseek(file, 0, SEEK_END);
    *tamanho = ftell(file) / sizeof(Paciente);
    rewind(file);
    *pacientes = (Paciente *)malloc((*tamanho) * sizeof(Paciente));
    if (*pacientes == NULL) {
        printf("Erro ao alocar memoria.\n");
        fclose(file);
        return;
    }
    fread(*pacientes, sizeof(Paciente), *tamanho, file);
    fclose(file);
}

void lerDadosConsultas(Consulta **consultas, int *tamanho) {
    FILE *file = fopen("consultas.bin", "rb");
    if (file == NULL) {
        *tamanho = 0;
        *consultas = NULL;
        return;
    }
    fseek(file, 0, SEEK_END);
    *tamanho = ftell(file) / sizeof(Consulta);
    rewind(file);
    *consultas = (Consulta *)malloc((*tamanho) * sizeof(Consulta));
    if (*consultas == NULL) {
        printf("Erro ao alocar memoria.\n");
        fclose(file);
        return;
    }
    fread(*consultas, sizeof(Consulta), *tamanho, file);
    fclose(file);
}

void lerDadosMedicos(Medico **medicos, int *tamanho) {
    FILE *file = fopen("medicos.bin", "rb");
    if (file == NULL) {
        *tamanho = 0;
        *medicos = NULL;
        return;
    }
    fseek(file, 0, SEEK_END);
    *tamanho = ftell(file) / sizeof(Medico);
    rewind(file);
    *medicos = (Medico *)malloc((*tamanho) * sizeof(Medico));
    if (*medicos == NULL) {
        printf("Erro ao alocar memoria.\n");
        fclose(file);
        return;
    }
    fread(*medicos, sizeof(Medico), *tamanho, file);
    fclose(file);
}

// ------------------------------------

void gravarDadosPacientes(Paciente *pacientes, int tamanho) {
    FILE *file = fopen("pacientes.bin", "wb");
    if (file == NULL) {
        printf("Erro ao abrir o arquivo.\n");
        return;
    }
    fwrite(pacientes, sizeof(Paciente), tamanho, file);
    fclose(file);
}

void gravarDadosConsultas(Consulta *consultas, int tamanho) {
    FILE *file = fopen("consultas.bin", "wb");
    if (file == NULL) {
        printf("Erro ao abrir o arquivo.\n");
        return;
    }
    fwrite(consultas, sizeof(Consulta), tamanho, file);
    fclose(file);
}

void gravarDadosMedicos(Medico *medicos, int tamanho) {
    FILE *file = fopen("medicos.bin", "wb");
    if (file == NULL) {
        printf("Erro ao abrir o arquivo.\n");
        return;
    }
    fwrite(medicos, sizeof(Medico), tamanho, file);
    fclose(file);
}

// ---------------------------------

int setIDMedico() {
    srand(time(NULL));
    Medico *medicos = NULL;
    int tam;

    lerDadosMedicos(&medicos, &tam);
    
    int id = 700 + rand() % 100;

    for (int i = 0; i < tam; i++) {
        if (id == medicos[i].ID) {
            id = 700 + rand() % 100;
            i = 0;
        }
    }

    free(medicos);

    return id;
}

int setIDPaciente() {
    srand(time(NULL));
    Paciente *pacientes = NULL;
    int tam;

    lerDadosPacientes(&pacientes, &tam);
    
    int id = 200 + rand() % 100;

    for (int i = 0; i < tam; i++) {
        if (id == pacientes[i].ID) {
            id = 200 + rand() % 100;
            i = 0;
        }
    }

    free(pacientes);

    return id;
}

int setNumeroConsulta() {
    srand(time(NULL));
    Consulta *consultas = NULL;
    int tam;

    lerDadosConsultas(&consultas, &tam);
    
    int num = 1 + rand() % 100;

    for (int i = 0; i < tam; i++) {
        if (num == consultas[i].numero) {
            num = 1 + rand() % 100;
            i = 0;
        }
    }

    free(consultas);

    return num;
}

// Funções de pesquisa --------------------------------------------

Medico *pesquisarMedico(uint id) {
    Medico *medicos;
    int tam;

    lerDadosMedicos(&medicos, &tam);

    for (int i = 0; i < tam; ++i) {
        if (id == medicos[i].ID) return &medicos[i];
    }

    free(medicos);

    return NULL;
}

Paciente *pesquisarPaciente(uint id) {
    Paciente *pacientes;
    int tam;

    lerDadosPacientes(&pacientes, &tam);

    for (int i = 0; i < tam; ++i) {
        if (id == pacientes[i].ID) return &pacientes[i];
    }

    free(pacientes);
    
    return NULL;
}

// Pesquisas de consultas 
// Lista de consultas agendadas para o paciente.
// Lista de consultas agendadas para o médico.
// Lista de pacientes por especialidade.

// Funções relacionadas aos horarios da consulta --------------------------

void ajeitarHorario(Horario *horario) {
    if (horario->minutos >= 60) {
        horario->horas += horario->minutos / 60;
        horario->minutos = horario->minutos % 60;
    }

    if (horario->horas >= 24) {
        horario->horas = horario->horas % 24;
    }
}

int horariosConflitam(Horario inicio1, Horario fim1, Horario inicio2, Horario fim2) {
    // Verifica se o intervalo 1 começa antes do fim do intervalo 2
    // e se o intervalo 2 começa antes do fim do intervalo 1
    return (inicio1.horas < fim2.horas || (inicio1.horas == fim2.horas && inicio1.minutos < fim2.minutos)) &&
           (inicio2.horas < fim1.horas || (inicio2.horas == fim1.horas && inicio2.minutos < fim1.minutos));
}

// Função para calcular o horário final com base no horário inicial e na duração
Horario calcularHorarioFinal(Horario inicio, Duracao duracao) {
    Horario fim;
    fim.horas = inicio.horas + duracao.tempo.horas;
    fim.minutos = inicio.minutos + duracao.tempo.minutos;

    // Ajustar minutos e horas se os minutos ultrapassarem 60
    ajeitarHorario(&fim);

    return fim;
}

// Função principal para verificar se há consulta livre
int temConsultaLivre(Data data, Horario horario, Duracao duracao) {
    Consulta *consultas = NULL;
    int tam;

    // Carrega as consultas
    lerDadosConsultas(&consultas, &tam);

    // Calcula o horário final da consulta que está sendo verificada
    Horario horario_fim = calcularHorarioFinal(horario, duracao);

    // Verifica conflitos com cada consulta existente
    for (int i = 0; i < tam; ++i) {
        Horario hcons_ini = consultas[i].horario;
        Horario hcons_fim = calcularHorarioFinal(hcons_ini, consultas[i].duracao);

        // Verifica se os intervalos se sobrepõem
        if (horariosConflitam(horario, horario_fim, hcons_ini, hcons_fim)) {
            printf("Conflito de horário encontrado.\n");
            free(consultas); // Libera a memória alocada
            return 0;
        }
    }

    free(consultas); // Libera a memória alocada
    return 1; // Não há conflitos
}

// Funções de adicionar-----------------------------------------------------------

void adicionarMedico() {
    Medico *medicos = NULL;
    int tam = 0;

    lerDadosMedicos(&medicos, &tam);

    tam++;

    Medico *aux = (Medico *)realloc(medicos, tam * sizeof(Medico));
    if (aux == NULL) {
        printf("Erro ao realocar memória.\n");
        free(medicos);
        exit(1);
    }

    medicos = aux;

    Medico medico;
    medico.ID = setIDMedico();
    printf("Digite o nome: ");
    scanf(" %99[^\n]", medico.nome);
    printf("\nDigite a especialidade: ");
    scanf(" %99[^\n]", medico.especialidade);

    medicos[tam - 1] = medico;

    gravarDadosMedicos(medicos, tam);

    free(medicos);
}

void adicionarPaciente() {
    Paciente *pacientes = NULL;
    int tam = 0;

    lerDadosPacientes(&pacientes, &tam);

    tam++;

    Paciente *aux = (Paciente *)realloc(pacientes, tam * sizeof(Paciente));
    if (aux == NULL) {
        printf("Erro ao realocar memória.\n");
        free(pacientes);
        exit(1);
    }

    pacientes = aux;

    Paciente paciente;
    paciente.ID = setIDPaciente();

    printf("Digite o nome: ");
    scanf(" %63[^\n]", paciente.nome);
    printf("\nDigite a identidade: ");
    scanf(" %21[^\n]", paciente.identidade);

    printf("\n\nDigite a rua: ");
    scanf(" %99[^\n]", paciente.endereco.rua);
    printf("\nDigite a bairro: ");
    scanf(" %99[^\n]", paciente.endereco.bairro);
    printf("\nDigite a cidade: ");
    scanf(" %99[^\n]", paciente.endereco.cidade);
    printf("\nDigite o numero: ");
    scanf(" %9[^\n]", paciente.endereco.numero);
    printf("\nDigite o complemento: ");
    scanf(" %49[^\n]", paciente.endereco.complemento);

    printf("\n\nDigite o DDD: ");
    scanf(" %3[^\n]", paciente.telefone.ddd);
    printf("\nDigite a numero: ");
    scanf(" %15[^\n]", paciente.telefone.numero);

    printf("\n\nDigite o sexo: ");
    scanf(" %c", paciente.sexo);

    pacientes[tam - 1] = paciente;

    gravarDadosPacientes(pacientes, tam);

    free(pacientes);
}

void adicionarConsulta() {
    Consulta *consultas = NULL;
    int tam = 0;

    lerDadosConsultas(&consultas, &tam);

    tam++;

    Consulta *aux = (Consulta *)realloc(consultas, tam * sizeof(Consulta));
    if (aux == NULL) {
        printf("Erro ao realocar memória.\n");
        free(consultas);
        exit(1);
    }

    consultas = aux;

    Consulta consulta;
    consulta.numero = setNumeroConsulta();

    int id_medico, id_paciente;

    Medico *medico;
    do {
        printf("Digite o ID do MÉDICO: ");
        scanf("%3u", &id_medico);
        medico = pesquisarMedico(id_medico);
        if (medico == NULL) {
            printf("Não existe um MÉDICO com o ID informado!\n");
            AVANCAR();
        }
    } while(medico == NULL);
    consulta.medico = medico;

    Paciente *paciente;
    do {
        printf("Digite o ID do PACIENTE: ");
        scanf("%3u", &id_paciente);
        paciente = pesquisarPaciente(id_paciente);
        if (paciente == NULL) {
            printf("Não existe um PACIENTE com o ID informado!\n");
            AVANCAR();
        }
    } while(paciente == NULL);
    consulta.paciente = paciente;

    Data data;
    printf("\nDigite a data da consulta: (dd/mm/aaaa)");
    scanf("%2u/%2u/%4u", &data.dia, &data.mes, &data.ano);

    Horario horario;
    Duracao duracao;
    do {
        printf("\n\nDigite o horário da consulta: (HH:MM)");
        scanf("%2u:%2u", &horario.horas, &horario.minutos);

        printf("\nDigite o tempo de duração da consulta: (HH:MM)");
        scanf("%2u:%2u", &duracao.tempo.horas, &duracao.tempo.minutos);

        if (!temConsultaLivre(data, horario, duracao)) AVANCAR();
    } while (!temConsultaLivre(data, horario, duracao));

    consulta.data = data;
    consulta.horario = horario;
    consulta.duracao = duracao;

    consultas[tam - 1] = consulta;

    gravarDadosConsultas(consultas, tam);

    free(consultas);
}

// Funções do Menu Principal---------------------------------------------

void menu(int *opcao) {
    printf("HOSPITAL CURAS INACREDITAVEIS\n");
    printf("1 - Consulta\n");
    printf("2 - Paciente\n");
    printf("3 - Médico\n");
    printf("4 - Relatório\n");
    printf("5 - Sair\n");
    printf("Escolha uma opcao: ");
    scanf("%d", opcao);
}

void opcao_sair() {
    LIMPAR_TELA();
    printf("Saindo do programa...\n\n");
    AVANCAR();
}

void opcao_invalida() {
    LIMPAR_TELA();
    printf("Insira um número válido.\n\n");
    AVANCAR();
}

void menu_secundario(char *string) {
    int opcao;
    do {
        LIMPAR_TELA();
        LIMPAR_BUFFER();
        printf("%sS\n", string);
        printf("1 - Pesquisar\n");
        printf("2 - Adicionar\n");
        printf("3 - Alterar\n");
        printf("4 - Remover\n");
        printf("5 - Voltar ao menu\n");
        printf("Escolha uma opcao: ");
        scanf("%d", &opcao);

        LIMPAR_BUFFER();

        switch (opcao) {
            case 1:
                // Exemplo de pesquisa (por ID)
                break;
            case 2:
                if (!strcmp(string, "MÉDICO"))
                    adicionarMedico();
                if (!strcmp(string, "PACIENTE"))
                    adicionarPaciente();
                if (!strcmp(string, "CONSULTA"))
                    adicionarConsulta();
                break;
            case 3:
                // Função de alteração (não implementada)
                break;
            case 4:
                // Função de remoção (não implementada)
                break;
            case 5:
                return;
            default:
                printf("Opção inválida!\n");
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
                menu_secundario("CONSULTA");
                break;
            case 2: // Paciente
                menu_secundario("PACIENTE");
                break;
            case 3: // Médico
                menu_secundario("MÉDICO");
                break;

            case 4: // Relatórios
                
                break;

            case 5: // Sair
                opcao_sair();
                break;
            
            default:
                opcao_invalida();
                break;
        }
    } while (opcao != 5);

    return 0;
}