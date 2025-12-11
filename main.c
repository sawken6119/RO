#include "transport.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


// ==========================================================
// DEFINITION DES FICHIERS ET FONCTIONS UTILES
// ==========================================================

// Liste des fichiers pour les problemes de transport
const char *nomsFichiers[] = {
    "problem_1.txt","problem_2.txt","problem_3.txt",
    "problem_4.txt","problem_5.txt","problem_6.txt",
    "problem_7.txt","problem_8.txt","problem_9.txt",
    "problem_10.txt","problem_11.txt","problem_12.txt",
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
    TransportProblem problem;
    int problem_index;
    char algorithm_choice[10];
    const char *filename; // pointeur vers le nom de fichier

    printf("PROJET DE RECHERCHE OPERATIONNELLE - EFREI\n");
    printf("Resolution du Probleme de Transport (Methode du Marche-Pied)\n");

    char continuer = 'O';

    // Boucle principale pour tester plusieurs fichiers
    while (continuer == 'O' || continuer == 'o') {

        // Choix de l'indice du probleme
        printf("\nEntrez l'indice du probleme de transport a traiter (1 a %d): ", NB_GRAPHES);

        if (scanf("%d", &problem_index) != 1) {
            fprintf(stderr, "Erreur: entree invalide pour l'indice.\n");
            clear_input_buffer();
            break;
        }
        clear_input_buffer();

        if (problem_index < 1 || problem_index > NB_GRAPHES) {
            fprintf(stderr, "Erreur: indice invalide (doit etre entre 1 et %d).\n", NB_GRAPHES);
            continue;
        }

        filename = nomsFichiers[problem_index - 1];
        printf("Chargement du fichier: %s\n", filename);

        // Lire les donnees du fichier
        if (!read_data_from_file(filename, &problem)) {
            fprintf(stderr, "Impossible de lire le fichier '%s'. Verifiez qu'il existe.\n", filename);
            free_problem(&problem);
            continue;
        }

        // Afficher les matrices pour verifier
        display_problem_data(&problem);

        // Choix de l'algorithme pour la proposition initiale
        printf("\nChoisissez l'algorithme initial (NO pour Nord-Ouest, BH pour Balas-Hammer): ");
        if (scanf("%9s", algorithm_choice) != 1) {
            fprintf(stderr, "Erreur: entree invalide pour l'algorithme.\n");
            clear_input_buffer();
            free_problem(&problem);
            break;
        }
        clear_input_buffer();

        // Execution de l'algorithme choisi
        if (strcasecmp(algorithm_choice, "NO") == 0) {
            north_west_corner(&problem);
        } else if (strcasecmp(algorithm_choice, "BH") == 0) {
            balas_hammer(&problem);
        } else {
            fprintf(stderr, "Choix invalide. Choisissez NO ou BH.\n");
            free_problem(&problem);
            continue;
        }

        // Methode du marche-pied
        run_step_stone(&problem);

        // Affichage du resultat final
        printf("\n\n-------------------------------------------------\n");
        display_table("Proposition de Transport OPTIMALE", problem.n_suppliers, problem.m_clients, problem.transport_plan);
        printf("COUT MINIMAL TOTAL: %lld\n", calculate_total_cost(&problem));
        printf("-------------------------------------------------\n");

        // Demander si on veut tester un autre probleme
        printf("\nVoulez-vous tester un autre probleme de transport (O/N)? ");
        if (scanf(" %c", &continuer) != 1) {
            continuer = 'N';
        }
        clear_input_buffer();

        free_problem(&problem);
    }

    printf("\nFin du programme.\n");
    return 0;
}
