#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <time.h>
#include <windows.h>

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

typedef enum TIPOENTIDADE {
    TIPO_MEDICO,
    TIPO_PACIENTE,
    TIPO_CONSULTA
} TipoEntidade;


// Funções relacionadas a arquivo ------------------------------------------------------------

void lerDadosBIN(TipoEntidade tipo, void **vetor, int *tam) {
    size_t tipo_bytes;
    char *filename;

    switch (tipo) {
        case TIPO_MEDICO:
            tipo_bytes = sizeof(Medico);
            filename = "medicos.bin";
            break;
        case TIPO_PACIENTE:
            tipo_bytes = sizeof(Paciente);
            filename = "pacientes.bin";
            break;
        case TIPO_CONSULTA:
            tipo_bytes = sizeof(Consulta);
            filename = "consultas.bin";
            break;
        default:
            *tam = 0;
            *vetor = NULL;
            return;
    }

    FILE *file = fopen(filename, "rb");
    if (!file) {
        *tam = 0;
        *(void **)vetor = NULL;
        return;
    }

    fseek(file, 0, SEEK_END);
    *tam = ftell(file) / tipo_bytes;
    rewind(file);

    *(void **)vetor = malloc((*tam) * tipo_bytes);
    if (*(void **)vetor == NULL) {
        perror("Erro ao alocar memória");
        fclose(file);
        return;
    }

    fread(*(void **)vetor, tipo_bytes, *tam, file);
    fclose(file);
}

int gravarDadosBIN(TipoEntidade tipo, void *vetor, int tam) {
    if (vetor == NULL || tam <= 0) {
        fprintf(stderr, "Erro: Vetor inválido ou tamanho inválido.\n");
        return 0;
    }

    const char *filename = NULL;
    size_t tipo_bytes = 0;

    switch (tipo) {
        case TIPO_MEDICO:
            tipo_bytes = sizeof(Medico);
            filename = "medicos.bin";
            break;

        case TIPO_PACIENTE:
            tipo_bytes = sizeof(Paciente);
            filename = "pacientes.bin";
            break;

        case TIPO_CONSULTA:
            tipo_bytes = sizeof(Consulta);
            filename = "consulta.bin";
            break;

        default:
            fprintf(stderr, "Erro: Tipo de entidade desconhecido.\n");
            return 0;
    }

    FILE *file = fopen(filename, "wb");
    if (file == NULL) {
        perror("Erro ao abrir o arquivo");
        return 0;
    }

    size_t escritos = fwrite(vetor, tipo_bytes, tam, file);
    if (escritos != (size_t)tam) {
        fprintf(stderr, "Erro ao gravar os dados no arquivo.\n");
        fclose(file);
        return 0;
    }

    fclose(file);
    return 1;
}


// Funções de verificação dos horários da consulta -------------------------------------------

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
    return (inicio1.horas < fim2.horas || (inicio1.horas == fim2.horas && inicio1.minutos < fim2.minutos)) &&
           (inicio2.horas < fim1.horas || (inicio2.horas == fim1.horas && inicio2.minutos < fim1.minutos));
}

Horario calcularHorarioFinal(Horario inicio, Duracao duracao) {
    Horario fim;
    fim.horas = inicio.horas + duracao.tempo.horas;
    fim.minutos = inicio.minutos + duracao.tempo.minutos;

    ajeitarHorario(&fim);

    return fim;
}

int temConsultaLivre(Data data, Horario horario, Duracao duracao) {
    void *vetor = NULL;
    int tam;

    lerDadosBIN(TIPO_CONSULTA, &vetor, &tam);

    if (vetor == NULL || tam <= 0) {
        perror("Erro ao ler os dados ou dados não encontrados.");
        return 0;
    }

    Consulta *consultas = (Consulta *)vetor;

    Horario horario_fim = calcularHorarioFinal(horario, duracao);

    for (int i = 0; i < tam; ++i) {
        Horario hcons_ini = consultas[i].horario;
        Horario hcons_fim = calcularHorarioFinal(hcons_ini, consultas[i].duracao);

        if (horariosConflitam(horario, horario_fim, hcons_ini, hcons_fim)) {
            printf("Conflito de horário encontrado.\n");
            free(consultas);
            return 0;
        }
    }

    free(consultas);
    return 1;
}


// Funções essenciais do programa (Pesquisar, Adicionar, Atualizar/Alterar, Remover) ---------

// Pesquisar {
void *pesquisarEntidade(TipoEntidade tipo, uint id) {
    void *entidades = NULL;
    int tam;

    lerDadosBIN(tipo, &entidades, &tam);

    if (entidades == NULL || tam == 0) {
        return NULL;
    }

    size_t tipo_bytes;
    switch (tipo) {
        case TIPO_MEDICO:
            tipo_bytes = sizeof(Medico);
            for (int i = 0; i < tam; ++i) {
                Medico *m = (Medico *)((char *)entidades + i * tipo_bytes);
                if (m->ID == id) {
                    Medico *resultado = malloc(sizeof(Medico));
                    *resultado = *m;
                    free(entidades);
                    return resultado;
                }
            }
            break;
        case TIPO_PACIENTE:
            tipo_bytes = sizeof(Paciente);
            for (int i = 0; i < tam; ++i) {
                Paciente *p = (Paciente *)((char *)entidades + i * tipo_bytes);
                if (p->ID == id) {
                    Paciente *resultado = malloc(sizeof(Paciente));
                    *resultado = *p;
                    free(entidades);
                    return resultado;
                }
            }
            break;
        case TIPO_CONSULTA:
            tipo_bytes = sizeof(Consulta);
            for (int i = 0; i < tam; ++i) {
                Consulta *c = (Consulta *)((char *)entidades + i * tipo_bytes);
                if (c->numero == id) {
                    Consulta *resultado = malloc(sizeof(Consulta));
                    *resultado = *c;
                    free(entidades);
                    return resultado;
                }
            }
            break;

        default:
            fprintf(stderr, "Tipo desconhecido!\n");
            break;
    }
    
    free(entidades);
    return NULL;
}

void imprimirEntidade(uint id, TipoEntidade tipo) {
    if (tipo == TIPO_MEDICO) {
        Medico *medico = (Medico *)pesquisarEntidade(TIPO_MEDICO, id);
        if (medico != NULL) {
            printf("\n----MÉDICO----\n");
            printf("ID: %u\n", medico->ID);
            printf("Nome: %s\n", medico->nome);
            printf("Especialidade: %s\n\n", medico->especialidade);
        } else {
            printf("Não existe um MÉDICO com o ID: %d\n", id);
        }
        free(medico);
    } 
    
    if (tipo == TIPO_PACIENTE) {
        Paciente *paciente = (Paciente *)pesquisarEntidade(TIPO_PACIENTE, id);
        if (paciente != NULL) {
            printf("\n----PACIENTE----\n");
            printf("Nome: %s\n", paciente->nome);
            printf("Identidade: %s\n\n", paciente->identidade);
            if (strlen(paciente->telefone.numero) >= 9) {
                printf("Telefone: (%s)%.5s-%.4s\n", 
                       paciente->telefone.ddd, 
                       paciente->telefone.numero, 
                       &paciente->telefone.numero[5]);
            } else {
                printf("Número de telefone inválido ou muito curto.\n");
            }
            printf("Sexo: %c\n", paciente->sexo);
        }else {
            printf("Não existe um PACIENTE com o ID: %d\n", id);
        }
        free(paciente);
    }

    if (tipo == TIPO_CONSULTA) {
        Consulta *consulta = (Consulta *)pesquisarEntidade(TIPO_CONSULTA, id);
        if (consulta != NULL) {
            printf("\n----CONSULTA----\n");
            printf("Número: %u\n", consulta->numero);
            printf("Médico: %s\n", consulta->medico->nome);
            printf("- Especialidade: %s\n", consulta->medico->especialidade);
            printf("Paciente:\n", consulta->paciente->nome);
            printf("- Identidade: %s\n", consulta->paciente->identidade);
            printf("Data: %u/%u/%u às %u:%u\n",
                    consulta->data.dia,
                    consulta->data.mes,
                    consulta->data.ano,
                    consulta->horario.horas,
                    consulta->horario.minutos);
            printf("Duração: %u:%u\n\n", 
                    consulta->duracao.tempo.horas, 
                    consulta->duracao.tempo.minutos);
        }else {
            printf("Não existe uma CONSULTA com o NÚMERO: %d\n", id);
        }
        free(consulta);
    }
}

void imprimirEntidadesGeral(TipoEntidade tipo) {
    void *vetor;
    int tam;

    lerDadosBIN(tipo, &vetor, &tam);
    if (vetor == NULL || tam <= 0) {
        perror("Erro ao ler os dados ou dados não encontrados.");
        return;
    }

    if (tipo == TIPO_MEDICO) {
        Medico *medicos = (Medico *)vetor;
        for (int i = 0; i < tam; ++i) {
            printf("\n----- MÉDICO %d ----\n", i + 1);
            printf("ID: %u\n", medicos[i].ID);
            printf("Nome: %s\n", medicos[i].nome);
            printf("Especialidade: %s\n\n", medicos[i].especialidade);
        }
         
    } 
    
    if (tipo == TIPO_PACIENTE) {
        Paciente *pacientes = (Paciente *)vetor;
        for (int i = 0; i < tam; ++i) {
            printf("\n----PACIENTE----\n");
            printf("ID: %u\n", pacientes[i].ID);
            printf("Nome: %s\n", pacientes[i].nome);
            printf("Identidade: %s\n\n", pacientes[i].identidade);
            if (strlen(pacientes[i].telefone.numero) >= 9) {
                printf("Telefone: (%s)%.5s-%.4s\n", 
                       pacientes[i].telefone.ddd, 
                       pacientes[i].telefone.numero, 
                       &pacientes[i].telefone.numero[5]);
            } else {
                printf("Número de telefone inválido ou muito curto.\n");
            }
            printf("Sexo: %c\n", pacientes[i].sexo);
        }
    }

    if (tipo == TIPO_CONSULTA) {
        Consulta *consultas = (Consulta *)vetor;
        for (int i = 0; i < tam; ++i) {
            printf("\n----CONSULTA----\n");
            printf("Número: %u\n", consultas[i].numero);
            printf("Médico: %s (%s)\n", 
                    consultas[i].medico->nome, 
                    consultas[i].medico->especialidade);
            printf("Paciente: %s (%s)\n", 
                    consultas[i].paciente->nome,
                    consultas[i].paciente->identidade);
            printf("Data: %u/%u/%u às %u:%u\n",
                    consultas[i].data.dia,
                    consultas[i].data.mes,
                    consultas[i].data.ano,
                    consultas[i].horario.horas,
                    consultas[i].horario.minutos);
            printf("Duração: %u:%u\n\n", 
                    consultas[i].duracao.tempo.horas, 
                    consultas[i].duracao.tempo.minutos);
        }
    }
    AVANCAR();
}
// }

// Adicionar {
void adicionarMedico() {
    void *vetor = NULL;
    int tam = 0;

    lerDadosBIN(TIPO_MEDICO, &vetor, &tam);

    if (vetor == NULL || tam <= 0) {
        perror("Erro ao ler os dados ou dados não encontrados.");
        return;
    }
    
    Medico *medicos = (Medico *)vetor;

    if (tam == MAX_MEDICOS) {
        printf("\n\nO tamanho máximo de médicos foi alcançado.\n\n");
        free(medicos);
        return;
    }

    tam++;

    Medico *aux = (Medico *)realloc(medicos, tam * sizeof(Medico));
    if (aux == NULL) {
        printf("Erro ao realocar memória.\n");
        free(medicos);
        exit(1);
    }

    medicos = aux;

    Medico medico;
    printf("\nDigite o ID: ");
    scanf("%u", &medico.ID);
    printf("Digite o nome: ");
    scanf(" %99[^\n]", medico.nome);
    printf("\nDigite a especialidade: ");
    scanf(" %99[^\n]", medico.especialidade);

    medicos[tam - 1] = medico;

    gravarDadosBIN(TIPO_MEDICO, medicos, tam);

    free(medicos);

    printf("Médico adicionado com sucesso!\n\n");
    AVANCAR();
}

void adicionarPaciente() {
    void *vetor = NULL;
    int tam = 0;

    lerDadosBIN(TIPO_PACIENTE, &vetor, &tam);

    if (vetor == NULL || tam <= 0) {
        perror("Erro ao ler os dados ou dados não encontrados.");
        return;
    }
    
    Paciente *pacientes = (Paciente *)vetor;

    if (tam == MAX_PACIENTES) {
        printf("\n\nO tamanho máximo de pacientes foi alcançado.\n\n");
        free(pacientes);
        return;
    }

    tam++;

    Paciente *aux = (Paciente *)realloc(pacientes, tam * sizeof(Paciente));
    if (aux == NULL) {
        printf("Erro ao realocar memória.\n");
        free(pacientes);
        exit(1);
    }

    pacientes = aux;

    Paciente paciente;

    printf("\nDigite o ID: ");
    scanf("%u", &paciente.ID);
    printf("\nDigite o nome: ");
    scanf(" %63[^\n]", paciente.nome);
    printf("\nDigite a identidade: ");
    scanf(" %21[^\n]", paciente.identidade);

    printf("\n\nDigite a rua: ");
    scanf(" %99[^\n]", paciente.endereco.rua);
    printf("\nDigite a bairro: ");
    scanf(" %99[^\n]", paciente.endereco.bairro);
    printf("\nDigite a cidade: ");
    scanf(" %99[^\n]", paciente.endereco.cidade);
    printf("\nDigite o número da residência: ");
    scanf(" %9[^\n]", paciente.endereco.numero);
    printf("\nDigite o complemento: ");
    scanf(" %49[^\n]", paciente.endereco.complemento);

    printf("\n\nDigite o DDD: ");
    scanf(" %3[^\n]", paciente.telefone.ddd);
    printf("\nDigite a número do contato: ");
    scanf(" %15[^\n]", paciente.telefone.numero);

    printf("\n\nDigite o sexo: ");
    scanf(" %c", &paciente.sexo);

    pacientes[tam - 1] = paciente;

    gravarDadosBIN(TIPO_PACIENTE, pacientes, tam);

    free(pacientes);

    printf("Paciente adicionado com sucesso!\n\n");
    AVANCAR();
}

void adicionarConsulta() {
    void *vetor = NULL;
    int tam = 0;

    lerDadosBIN(TIPO_CONSULTA, &vetor, &tam);

    if (vetor == NULL || tam <= 0) {
        perror("Erro ao ler os dados ou dados não encontrados.");
        return;
    }
    
    Consulta *consultas = (Consulta *)vetor;

    if (tam == MAX_CONSULTAS) {
        printf("\n\nO tamanho máximo de consultados foi alcançado.\n\n");
        AVANCAR();
        free(consultas);
        return;
    }
    
    tam++;

    Consulta *aux = (Consulta *)realloc(consultas, tam * sizeof(Consulta));
    if (aux == NULL) {
        perror("Erro ao realocar memória.");
        free(consultas);
        exit(1);
    }

    consultas = aux;

    Consulta consulta;

    printf("\nDigite o nº da consulta: ");
    scanf("%u", &consulta.numero);

    int id_medico, id_paciente;

    Medico *medico;
    do {
        printf("Digite o ID do MÉDICO: ");
        scanf("%3u", &id_medico);
        medico = (Medico *)pesquisarEntidade(TIPO_MEDICO, id_medico);
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
        paciente = (Paciente *)pesquisarEntidade(TIPO_PACIENTE, id_paciente);
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

    gravarDadosBIN(TIPO_CONSULTA, consultas, tam);

    free(consultas);

    printf("Consulta adicionada com sucesso!\n\n");
    AVANCAR();
}
//}

// Remover {
void removerMedico() {
    uint id_medico;
    printf("\n----- REMOVENDO: ------\n");
    printf("Digite o ID do médico: ");
    scanf("%u", &id_medico);

    void *vet1 = NULL;
    void *vet2 = NULL;

    int tam_med;
    int tam_cons;

    lerDadosBIN(TIPO_MEDICO, &vet1, &tam_med);
    lerDadosBIN(TIPO_CONSULTA, &vet2, &tam_cons);

    if (vet1 == NULL || vet2 == NULL) {
        perror("Erro ao ler os dados.\n");
        exit(1);
        return;
    }

    Medico *medicos = (Medico *)vet1;
    Consulta *consultas = (Consulta *)vet2;

    if (medicos == NULL || tam_med == 0) {
        printf("Não há médicos cadastrados no sistema.\n");
        AVANCAR();
        return;
    }

    int id_encontrado = 0;
    for (int i = 0; i < tam_med; ++i) {
        if (id_medico == medicos[i].ID) {
            id_encontrado = 1;
            printf("Você está removendo o seguinte usuário:\n\n");
            imprimirEntidade(medicos[i].ID, TIPO_MEDICO);
            AVANCAR();
            for (int j = i; j < tam_med - 1; ++j) {
                medicos[j] = medicos[j + 1];
            }
            --tam_med;
            break;
        }
    }

    for (int i = 0; i < tam_cons; ++i) {
        if (id_medico == consultas[i].medico->ID) {
            for (int j = i; j < tam_cons - 1; ++j) {
                consultas[j] = consultas[j + 1];
            }
            --tam_cons;
            break;
        }
    }

    if (id_encontrado) {
        Medico *temp = realloc(medicos, tam_med * sizeof(Medico));
        if (temp == NULL && tam_med > 0) {
            perror("Erro ao realocar memória");
            free(medicos);
            free(consultas);
            return;
        }
        medicos = temp;
        gravarDadosBIN(TIPO_MEDICO, medicos, tam_med);

        Consulta *temp2 = realloc(consultas, tam_cons * sizeof(Consulta));
        if (temp2 == NULL && tam_cons > 0) {
            perror("Erro ao realocar memória");
            free(medicos);
            free(consultas);
            return;
        }
        consultas = temp2;
        gravarDadosBIN(TIPO_CONSULTA, consultas, tam_cons);

        printf("\nMédico removido com sucesso.\n\n");
        
    } else {
        printf("\nMédico com ID %u não encontrado.\n\n", id_medico);
    }

    AVANCAR();
    free(medicos);
    free(consultas);
}

void removerPaciente() {
    uint id_paciente;
    printf("\n----- REMOVENDO: ------\n");
    printf("Digite o ID do paciente: ");
    scanf("%u", &id_paciente);

    void *vet1 = NULL;
    void *vet2 = NULL;

    int tam_pac = 0;
    int tam_cons = 0;

    lerDadosBIN(TIPO_PACIENTE, &vet1, &tam_pac);
    lerDadosBIN(TIPO_CONSULTA, &vet2, &tam_cons);

    if (vet1 == NULL || vet2 == NULL) {
        perror("Erro ao ler os dados.\n");
        exit(1);
        return;
    }
    
    Paciente *pacientes = (Paciente *)vet1;
    Consulta *consultas = (Consulta *)vet2;

    if (pacientes == NULL || tam_pac == 0) {
        printf("Não há pacientes cadastrados no sistema.\n");
        AVANCAR();
        return;
    }

    int id_encontrado = 0;
    for (int i = 0; i < tam_pac; ++i) {
        if (id_paciente == pacientes[i].ID) {
            id_encontrado = 1;
            printf("Você está removendo o seguinte usuário:\n\n");
            imprimirEntidade(pacientes[i].ID, TIPO_PACIENTE);
            AVANCAR();
            for (int j = i; j < tam_pac - 1; ++j) {
                pacientes[j] = pacientes[j + 1];
            }
            --tam_pac;
            break;
        }
    }

    for (int i = 0; i < tam_cons; ++i) {
        if (id_paciente == consultas[i].paciente->ID) {
            for (int j = i; j < tam_cons - 1; ++j) {
                consultas[j] = consultas[j + 1];
            }
            --tam_cons;
            break;
        }
    }

    if (id_encontrado) {
        Paciente *temp = realloc(pacientes, tam_pac * sizeof(Paciente));
        if (temp == NULL && tam_pac > 0) {
            perror("Erro ao realocar memória");
            free(pacientes);
            free(consultas);
            return;
        }
        pacientes = temp;
        gravarDadosBIN(TIPO_PACIENTE, pacientes, tam_pac);

        Consulta *temp2 = realloc(consultas, tam_cons * sizeof(Consulta));
        if (temp2 == NULL && tam_cons > 0) {
            perror("Erro ao realocar memória");
            free(pacientes);
            free(consultas);
            return;
        }
        consultas = temp2;
        gravarDadosBIN(TIPO_CONSULTA, consultas, tam_cons);

        printf("\nPaciente removido com sucesso.\n\n");
    } else {
        printf("\nPaciente com ID %u não encontrado.\n\n", id_paciente);
    }
    free(pacientes);
    free(consultas);
    AVANCAR();
}

void removerConsulta() {
    uint num_consulta;
    printf("\n----- REMOVENDO: ------\n");
    printf("Digite o nº da consulta: ");
    scanf("%u", &num_consulta);

    void *vetor = NULL;
    int tam = 0;

    lerDadosBIN(TIPO_CONSULTA, &vetor, &tam);

    if (vetor == NULL || tam <= 0) {
        perror("Erro ao ler os dados ou dados não encontrados.");
        return;
    }
    
    Consulta *consultas = (Consulta *)vetor;

    int id_encontrado = 0;
    for (int i = 0; i < tam; ++i) {
        if (num_consulta == consultas[i].numero) {
            id_encontrado = 1;
            printf("Você está removendo o seguinte usuário:\n\n");
            imprimirEntidade(consultas[i].numero, TIPO_PACIENTE);
            AVANCAR();
            for (int j = i; j < tam - 1; ++j) {
                consultas[j] = consultas[j + 1];
            }
            --tam;
            break;
        }
    }

    if (id_encontrado) {
        Consulta *temp = realloc(consultas, tam * sizeof(Consulta));
        if (temp == NULL && tam > 0) {
            perror("Erro ao realocar memória");
            free(consultas);
            return;
        }
        consultas = temp;
        gravarDadosBIN(TIPO_CONSULTA, consultas, tam);
        printf("\nConsulta removido com sucesso.\n\n");
    } else {
        printf("\nConsulta com nº %u não encontrado.\n\n", num_consulta);
    }
    AVANCAR();
    free(consultas);
}
// }

// Atualizar {
int atualizarMedicosBIN(Medico *medico) {
    void *vetor = NULL;
    int tam = 0;

    lerDadosBIN(TIPO_MEDICO, &vetor, &tam);

    if (vetor == NULL || tam <= 0) {
        perror("Erro ao ler os dados ou dados não encontrados.");
        return 0;
    }
    
    Medico *medicos = (Medico *)vetor;

    int encontrado = 0;
    for (int i = 0; i < tam; ++i) {
        if (medicos[i].ID == medico->ID) {
            medicos[i] = *medico;
            encontrado = 1;
            break;
        }
    }

    if (!encontrado) {
        printf("Médico com ID %u não encontrado.\n", medico->ID);
        free(medicos);
        return 0;
    }

    gravarDadosBIN(TIPO_MEDICO, medicos, tam);

    free(medicos);
    printf("Médico atualizado com sucesso!\n");
    return 1;
}

int atualizarPacienteBIN(Paciente *paciente) {
    void *vetor = NULL;
    int tam = 0;

    lerDadosBIN(TIPO_PACIENTE, &vetor, &tam);

    if (vetor == NULL || tam <= 0) {
        perror("Erro ao ler os dados ou dados não encontrados.");
        return 0;
    }
    
    Paciente *pacientes = (Paciente *)vetor;

    int encontrado = 0;
    for (int i = 0; i < tam; ++i) {
        if (pacientes[i].ID == paciente->ID) {
            pacientes[i] = *paciente;
            encontrado = 1;
            break;
        }
    }

    if (!encontrado) {
        printf("Paciente com ID %u não encontrado.\n", paciente->ID);
        free(pacientes);
        return 0;
    }

    gravarDadosBIN(TIPO_PACIENTE, pacientes, tam);

    free(pacientes);
    printf("Médico atualizado com sucesso!\n");
    return 1;
}

int atualizarConsultaBIN(Consulta *consulta) {
    void *vetor = NULL;
    int tam = 0;

    lerDadosBIN(TIPO_CONSULTA, &vetor, &tam);

    if (vetor == NULL || tam <= 0) {
        perror("Erro ao ler os dados ou dados não encontrados.");
        return 0;
    }
    
    Consulta *consultas = (Consulta *)vetor;

    int encontrado = 0;
    for (int i = 0; i < tam; ++i) {
        if (consultas[i].numero == consulta->numero) {
            consultas[i] = *consulta;
            encontrado = 1;
            break;
        }
    }

    if (!encontrado) {
        printf("Consulta com nº %u não encontrado.\n", consulta->numero);
        free(consultas);
        return 0;
    }

    gravarDadosBIN(TIPO_CONSULTA, consultas, tam);

    free(consultas);
    printf("Consulta atualizado com sucesso!\n");

    return 1;
}

void alterarMedico() {   
    uint id_medico;
    printf("\n----- Editando: ------\n");
    printf("Digite o ID do médico: ");
    scanf("%u", &id_medico);

    Medico *medico = (Medico *)pesquisarEntidade(TIPO_MEDICO, id_medico);

    if (medico == NULL) {
        printf("Não foi encontrado um médico com o ID %d.\n", id_medico);
        AVANCAR();
        return;
    }
    
    printf("\nVocê está alterando o seguinte médico:\n");
    imprimirEntidade(id_medico, TIPO_MEDICO);
    AVANCAR();

    printf("\n--- Insira os novos dados abaixo: ---\n");
    printf("Nome: ");
    scanf(" %[^\n]", medico->nome);
    printf("\nEspecialidade: ");
    scanf(" %99[^\n]", medico->especialidade);

    int atualizado = atualizarMedicosBIN(medico);
    if (atualizado) {
        printf("\nMédico atualizado com sucesso!\n");
    } else {
        printf("\nErro ao atualizar o médico.\n");
    }

    AVANCAR();
}

void alterarPaciente() {   
    uint id_paciente;
    printf("\n----- Editando: ------\n");
    printf("Digite o ID do paciente: ");
    scanf("%u", &id_paciente);

    Paciente *paciente = (Paciente *)pesquisarEntidade(TIPO_PACIENTE, id_paciente);

    if (paciente == NULL) {
        printf("Não foi encontrado um paciente com o ID %d.\n", id_paciente);
        AVANCAR();
        return;
    }
    
    printf("\nVocê está alterando o seguinte paciente:\n");
    imprimirEntidade(id_paciente, TIPO_PACIENTE);
    AVANCAR();

    printf("\n--- Insira os novos dados abaixo: ---\n");
    printf("\nNome: ");
    scanf(" %99[^\n]", paciente->nome);
    printf("\nIdentidade: ");
    scanf(" %19[^\n]", paciente->identidade);

    printf("\n\nDigite a rua: ");
    scanf(" %99[^\n]", paciente->endereco.rua);
    printf("\nDigite a bairro: ");
    scanf(" %99[^\n]", paciente->endereco.bairro);
    printf("\nDigite a cidade: ");
    scanf(" %99[^\n]", paciente->endereco.cidade);
    printf("\nDigite o número da residência: ");
    scanf(" %9[^\n]", paciente->endereco.numero);
    printf("\nDigite o complemento: ");
    scanf(" %49[^\n]", paciente->endereco.complemento);

    printf("\n\nDigite o DDD: ");
    scanf(" %3[^\n]", paciente->telefone.ddd);
    printf("\nDigite a número do contato: ");
    scanf(" %15[^\n]", paciente->telefone.numero);

    printf("\n\nDigite o sexo: ");
    scanf(" %c", &paciente->sexo);

    int atualizado = atualizarPacienteBIN(paciente);
    if (atualizado) {
        printf("\nPaciente atualizado com sucesso!\n");
    } else {
        printf("\nErro ao atualizar o paciente.\n");
    }

    AVANCAR();
}

void alterarConsulta() {   
    uint num_consulta;
    printf("\n----- Editando: ------\n");
    printf("Digite o nº da consulta: ");
    scanf("%u", &num_consulta);

    Consulta *consulta = (Consulta *)pesquisarEntidade(TIPO_CONSULTA, num_consulta);

    if (consulta == NULL) {
        printf("Não foi encontrado uma consulta com o nº %d.\n", num_consulta);
        AVANCAR();
        return;
    }
    
    printf("\nVocê está alterando a seguinte consulta:\n");
    imprimirEntidade(num_consulta, TIPO_PACIENTE);
    AVANCAR();

    printf("\n--- Insira os novos dados abaixo: ---\n");

    printf("\nDigite o nº da consulta: ");
    scanf("%u", &consulta->numero);

    int id_medico, id_paciente;

    Medico *medico;
    do {
        printf("Digite o ID do MÉDICO: ");
        scanf("%3u", &id_medico);
        medico = (Medico *)pesquisarEntidade(TIPO_MEDICO, id_medico);
        if (medico == NULL) {
            printf("Não existe um MÉDICO com o ID informado!\n");
            AVANCAR();
        }
    } while(medico == NULL);
    consulta->medico = medico;

    Paciente *paciente;
    do {
        printf("Digite o ID do PACIENTE: ");
        scanf("%3u", &id_paciente);
        paciente = (Paciente *)pesquisarEntidade(TIPO_PACIENTE, id_paciente);
        if (paciente == NULL) {
            printf("Não existe um PACIENTE com o ID informado!\n");
            AVANCAR();
        }
    } while(paciente == NULL);
    consulta->paciente = paciente;

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

    consulta->data = data;
    consulta->horario = horario;
    consulta->duracao = duracao;

    int atualizado = atualizarConsultaBIN(consulta);
    if (atualizado) {
        printf("\nPaciente atualizado com sucesso!\n");
    } else {
        printf("\nErro ao atualizar o paciente.\n");
    }

    AVANCAR();
}
// }


// Funções do Menu de Relatórios -------------------------------------------------------------

void listarConsultasPorPaciente(uint idPaciente) {
    void *vetor = NULL;
    int tam = 0;

    lerDadosBIN(TIPO_CONSULTA, &vetor, &tam);

    if (vetor == NULL || tam <= 0) {
        perror("Erro ao ler os dados ou dados não encontrados.");
        return;
    }
    
    Consulta *consultas = (Consulta *)vetor;

    printf("\n---- CONSULTAS AGENDADAS PARA O PACIENTE ID %u ----\n", idPaciente);
    int encontrou = 0;
    for (int i = 0; i < tam; ++i) {
        if (consultas[i].paciente->ID == idPaciente) {
            
            printf("Número da Consulta: %u\n", consultas[i].numero);
            printf("Médico: %s (%s)\n", consultas[i].medico->nome, consultas[i].medico->especialidade);
            printf("Data: %u/%u/%u às %u:%u\n",
                   consultas[i].data.dia,
                   consultas[i].data.mes,
                   consultas[i].data.ano,
                   consultas[i].horario.horas,
                   consultas[i].horario.minutos);
            printf("Duração: %u:%u\n\n",
                   consultas[i].duracao.tempo.horas,
                   consultas[i].duracao.tempo.minutos);
            encontrou = 1;
        }
    }

    if (!encontrou) {
        printf("Nenhuma consulta agendada para o paciente com ID %u.\n", idPaciente);
    }

    free(consultas);
}

void listarPacientesPorEspecialidade(const char *especialidade) {
    void *vetor = NULL;
    int tam = 0;

    lerDadosBIN(TIPO_CONSULTA, &vetor, &tam);

    if (vetor == NULL || tam <= 0) {
        perror("Erro ao ler os dados ou dados não encontrados.");
        return;
    }
    
    Consulta *consultas = (Consulta *)vetor;

    printf("\n---- PACIENTES COM CONSULTAS NA ESPECIALIDADE %s ----\n", especialidade);
    int encontrou = 0;
    for (int i = 0; i < tam; ++i) {
        if (strcmp(consultas[i].medico->especialidade, especialidade) == 0) {
            printf("Paciente: %s (ID: %u)\n", consultas[i].paciente->nome, consultas[i].paciente->ID);
            encontrou = 1;
        }
    }

    if (!encontrou) {
        printf("Nenhum paciente encontrado nas consultas com a especialidade \"%s\".\n", especialidade);
    }

    free(consultas);
}

void listarConsultasPorMedico(uint idMedico) {
    void *vetor = NULL;
    int tam = 0;

    lerDadosBIN(TIPO_CONSULTA, &vetor, &tam);

    if (vetor == NULL || tam <= 0) {
        perror("Erro ao ler os dados ou dados não encontrados.");
        return;
    }
    
    Consulta *consultas = (Consulta *)vetor;

    printf("\n---- CONSULTAS AGENDADAS PARA O MÉDICO ID %u ----\n", idMedico);
    int encontrou = 0;
    for (int i = 0; i < tam; ++i) {
        if (consultas[i].medico->ID == idMedico) {
            printf("Número da Consulta: %u\n", consultas[i].numero);
            printf("Paciente: %s (ID: %u)\n", consultas[i].paciente->nome, consultas[i].paciente->ID);
            printf("Data: %u/%u/%u às %u:%u\n",
                   consultas[i].data.dia,
                   consultas[i].data.mes,
                   consultas[i].data.ano,
                   consultas[i].horario.horas,
                   consultas[i].horario.minutos);
            printf("Duração: %u:%u\n\n",
                   consultas[i].duracao.tempo.horas,
                   consultas[i].duracao.tempo.minutos);
            encontrou = 1;
        }
    }

    if (!encontrou) {
        printf("Nenhuma consulta agendada para o médico com ID %u.\n", idMedico);
    }

    free(consultas);
}


// Funções do Menu Principal------------------------------------------------------------------
char *string_menu(TipoEntidade tipo) {
    switch (tipo) {
        case TIPO_MEDICO:
            return "MÉDICO";
        case TIPO_PACIENTE:
            return "PACIENTE";
        case TIPO_CONSULTA:
            return "CONSULTA";
        default:
            return "INDEFINIDO";
    }
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

void opcao_pesquisar(TipoEntidade tipo) {
    int opcao;
    do {
        printf("1 - Pesquisar um usuário específico\n");
        printf("2 - Mostrar lista de %sS          \n", string_menu(tipo));
        printf("3 - Voltar ao menu                 \n");
        printf("Escolha uma opção: ");
        scanf("%d", &opcao);

        switch(opcao) {
            case 1:
                int id;
                printf("Digite o Nº ou ID: ");
                scanf("%d", &id);

                imprimirEntidade(id, tipo);

                AVANCAR();
                break;

            case 2:
                imprimirEntidadesGeral(tipo);
                AVANCAR();
                break;

            case 3:
                LIMPAR_TELA();
                return;
            
            default:
                opcao_invalida();
                break;
        }
    } while(opcao != 3);
}


// Menus -------------------------------------------------------------------------------------
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

void menu_secundario(TipoEntidade tipo) {
    int opcao;
    do {
        LIMPAR_TELA();
    
        printf("MENU %s\n", string_menu(tipo));    
        printf("1 - Pesquisar\n");
        printf("2 - Adicionar\n");
        printf("3 - Alterar\n");
        printf("4 - Remover\n");
        printf("5 - Voltar ao menu\n");
        printf("Escolha uma opção: ");
        scanf("%d", &opcao);

        LIMPAR_BUFFER();
        LIMPAR_TELA();

        switch (opcao) {
            case 1:
                opcao_pesquisar(tipo);
                break;
            case 2:
                if (tipo == TIPO_MEDICO)
                    adicionarMedico();
                else if (tipo == TIPO_PACIENTE)
                    adicionarPaciente();
                else if (tipo == TIPO_CONSULTA)
                    adicionarConsulta();
                break;
            case 3:
                if (tipo == TIPO_MEDICO)
                    alterarMedico();
                else if (tipo == TIPO_PACIENTE)
                    alterarPaciente();
                else if (tipo == TIPO_CONSULTA)
                    alterarConsulta();
                break;
            case 4:
                if (tipo == TIPO_MEDICO)
                    removerMedico();
                else if (tipo == TIPO_PACIENTE)
                    removerPaciente();
                else if (tipo == TIPO_CONSULTA)
                    removerConsulta();
                break;
            case 5:
                return;
            default:
                opcao_invalida();
                break;
        }
    } while (opcao != 5);
}

void relatorio_hospital(){
    int opcao;
    do
    {
        LIMPAR_TELA();
        printf("Relatorios\n");
        printf("1 - Listar consultas por paciente\n");
        printf("2 - Listar consultas por médico\n");
        printf("3 - Listar pacientes por especialidade\n");
        printf("4 - Voltar ao menu principal\n");
        printf("Escolha uma opcao: ");
        scanf("%d", &opcao);
        switch (opcao)
        {
            case 1:
                uint id_paciente;
                printf("Digite o ID do paciente: ");
                scanf("%u", &id_paciente);
                listarConsultasPorPaciente(id_paciente);
                AVANCAR();
                break;
            
            case 2:
                uint id_medico;
                printf("Digite o ID do medico: ");
                scanf("%u", &id_medico);
                listarConsultasPorMedico(id_medico);
                AVANCAR();
                break;
            
            case 3:
                char especialidade[100];
                printf("Digite a especialidade: ");
                scanf(" %99[^\n]", especialidade);
                listarPacientesPorEspecialidade(especialidade);
                AVANCAR();
                break;
                
            case 4:
                return;

            default:
                printf("Opção inválida!\n");
                AVANCAR();
                break;
        }
    } while (opcao!=4);
    
}


int main(int argc, char const *argv[]) {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    setlocale(LC_ALL, "pt_BR.UTF-8");

    int opcao;
    do {
        menu(&opcao);
        switch (opcao)
        {
            case 1:
                menu_secundario(TIPO_CONSULTA);
                break;
            case 2:
                menu_secundario(TIPO_PACIENTE);
                break;
            case 3:
                menu_secundario(TIPO_MEDICO);
                break;

            case 4: 
                relatorio_hospital();
                break;

            case 5:
                opcao_sair();
                break;
            
            default:
                opcao_invalida();
                break;
        }
    } while (opcao != 5);

    return 0;
}