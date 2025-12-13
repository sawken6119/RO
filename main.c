#include "transport.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// ==========================================================
// DEFINITION DES FICHIERS ET FONCTIONS UTILES
// ==========================================================

// Liste des fichiers pour les problemes de transport
const char *nomsFichiers[] = {
    "problem_1.txt",  "problem_2.txt",  "problem_3.txt",
    "problem_4.txt",  "problem_5.txt",  "problem_6.txt",
    "problem_7.txt",  "problem_8.txt",  "problem_9.txt",
    "problem_10.txt", "problem_11.txt", "problem_12.txt",
};
#define NB_GRAPHES (sizeof(nomsFichiers) / sizeof(nomsFichiers[0]))

// Fonction pour vider le buffer apres un scanf
void clear_input_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

// ==========================================================
// PROGRAMME PRINCIPAL
// ==========================================================

int main() {
    int problem_index;
    char algorithm_choice[10];
    char continuer = 'O';

    printf("========================================================\n");
    printf("   PROJET DE RECHERCHE OPERATIONNELLE - EFREI\n");
    printf("   Resolution du Probleme de Transport\n");
    printf("   Methode du Marche-Pied avec Potentiels\n");
    printf("========================================================\n");

    // Boucle principale pour tester plusieurs fichiers
    while (continuer == 'O' || continuer == 'o') {
        TransportProblem problem;

        // Initialiser la structure (important!)
        problem.cost_matrix = NULL;
        problem.transport_plan = NULL;
        problem.marginal_costs = NULL;
        problem.provisions = NULL;
        problem.commands = NULL;
        problem.u_potentials = NULL;
        problem.v_potentials = NULL;

        // Choix de l'indice du probleme
        printf("\n========================================================\n");
        printf("Entrez le numero du probleme a traiter (1 a %d): ", NB_GRAPHES);

        if (scanf("%d", &problem_index) != 1) {
            fprintf(stderr, "ERREUR: Entree invalide pour l'indice.\n");
            clear_input_buffer();
            continue;
        }
        clear_input_buffer();

        if (problem_index < 1 || problem_index > NB_GRAPHES) {
            fprintf(stderr, "ERREUR: Indice invalide (doit etre entre 1 et %d).\n", NB_GRAPHES);
            continue;
        }

        const char *filename = nomsFichiers[problem_index - 1];
        printf("Chargement du fichier: %s\n", filename);
        printf("========================================================\n");

        // Lire les donnees du fichier
        if (!read_data_from_file(filename, &problem)) {
            fprintf(stderr, "\nERREUR: Impossible de lire le fichier '%s'.\n", filename);
            fprintf(stderr, "Verifiez que le fichier existe dans le repertoire courant.\n");
            free_problem(&problem);
            continue;
        }

        // Afficher les matrices pour verification
        display_problem_data(&problem);

        // Choix de l'algorithme pour la proposition initiale
        printf("\n========================================================\n");
        printf("Choisissez l'algorithme pour la proposition initiale:\n");
        printf("  - NO : Nord-Ouest\n");
        printf("  - BH : Balas-Hammer\n");
        printf("Votre choix: ");

        if (scanf("%9s", algorithm_choice) != 1) {
            fprintf(stderr, "ERREUR: Entree invalide pour l'algorithme.\n");
            clear_input_buffer();
            free_problem(&problem);
            continue;
        }
        clear_input_buffer();

        // Execution de l'algorithme choisi
        if (strcasecmp(algorithm_choice, "NO") == 0) {
            north_west_corner(&problem);
        } else if (strcasecmp(algorithm_choice, "BH") == 0) {
            balas_hammer(&problem);
        } else {
            fprintf(stderr, "\nERREUR: Choix invalide '%s'.\n", algorithm_choice);
            fprintf(stderr, "Veuillez choisir NO ou BH.\n");
            free_problem(&problem);
            continue;
        }

        // Methode du marche-pied avec potentiels
        run_step_stone(&problem);

        // Affichage du resultat final optimal
        printf("\n\n");
        printf("##########################################################\n");
        printf("##              RESULTAT FINAL OPTIMAL                  ##\n");
        printf("##########################################################\n");

        display_table("PROPOSITION DE TRANSPORT OPTIMALE",
                      problem.n_suppliers,
                      problem.m_clients,
                      problem.transport_plan);

        long long final_cost = calculate_total_cost(&problem);
        printf("\n>>> COUT MINIMAL TOTAL: %lld <<<\n", final_cost);
        printf("##########################################################\n");

        // Liberer la memoire avant de continuer
        free_problem(&problem);

        // Demander si on veut tester un autre probleme
        printf("\n========================================================\n");
        printf("Voulez-vous tester un autre probleme de transport? (O/N): ");

        if (scanf(" %c", &continuer) != 1) {
            continuer = 'N';
        }
        clear_input_buffer();
    }

    printf("\n========================================================\n");
    printf("   Fin du programme. Merci d'avoir utilise l'outil!\n");
    printf("========================================================\n");

    return 0;
}
