#include "transport.h"
#include <stdio.h>
#include <stdlib.h>


// ==========================================================
// INITIALISATION ET LIBERATION DE MEMOIRE
// ==========================================================

void initialize_problem_matrices(TransportProblem* p) {
    int n = p->n_suppliers;
    int m = p->m_clients;

    // Allouer les pointeurs de lignes pour les matrices 2D
    p->cost_matrix = (int**)calloc(n, sizeof(int*));
    p->transport_plan = (int**)calloc(n, sizeof(int*));
    p->marginal_costs = (int**)calloc(n, sizeof(int*));
    if (!p->cost_matrix || !p->transport_plan || !p->marginal_costs) {
        fprintf(stderr, "Erreur allocation pointeurs de lignes.\n");
        exit(EXIT_FAILURE);
    }

    // Allouer les colonnes
    for (int i = 0; i < n; i++) {
        p->cost_matrix[i] = (int*)calloc(m, sizeof(int));
        p->transport_plan[i] = (int*)calloc(m, sizeof(int));
        p->marginal_costs[i] = (int*)calloc(m, sizeof(int));
        if (!p->cost_matrix[i] || !p->transport_plan[i] || !p->marginal_costs[i]) {
            fprintf(stderr, "Erreur allocation ligne matrice i=%d.\n", i);
            for (int k = 0; k <= i; k++) {
                free(p->cost_matrix[k]);
                free(p->transport_plan[k]);
                free(p->marginal_costs[k]);
            }
            free(p->cost_matrix); free(p->transport_plan); free(p->marginal_costs);
            exit(EXIT_FAILURE);
        }
    }

    // Allouer tableaux 1D
    p->provisions = (int*)calloc(n, sizeof(int));
    p->commands = (int*)calloc(m, sizeof(int));
    p->u_potentials = (int*)calloc(n, sizeof(int));
    p->v_potentials = (int*)calloc(m, sizeof(int));

    if (!p->provisions || !p->commands || !p->u_potentials || !p->v_potentials) {
        fprintf(stderr, "Erreur allocation tableaux 1D.\n");
        free_problem(p);
        exit(EXIT_FAILURE);
    }
}

void free_problem(TransportProblem* p) {
    if (!p) return;

    if (p->cost_matrix) {
        for (int i = 0; i < p->n_suppliers; i++) free(p->cost_matrix[i]);
        free(p->cost_matrix); p->cost_matrix = NULL;
    }
    if (p->transport_plan) {
        for (int i = 0; i < p->n_suppliers; i++) free(p->transport_plan[i]);
        free(p->transport_plan); p->transport_plan = NULL;
    }
    if (p->marginal_costs) {
        for (int i = 0; i < p->n_suppliers; i++) free(p->marginal_costs[i]);
        free(p->marginal_costs); p->marginal_costs = NULL;
    }

    free(p->provisions); p->provisions = NULL;
    free(p->commands); p->commands = NULL;
    free(p->u_potentials); p->u_potentials = NULL;
    free(p->v_potentials); p->v_potentials = NULL;
}

// ==========================================================
// LECTURE DES DONNEES ET AFFICHAGE
// ==========================================================

int read_data_from_file(const char* filename, TransportProblem* p) {
    FILE* file = fopen(filename, "r");
    if (!file) { perror("Erreur ouverture fichier"); return 0; }

    if (fscanf(file, "%d %d", &p->n_suppliers, &p->m_clients) != 2) {
        fprintf(stderr, "Erreur lecture n ou m.\n");
        fclose(file); return 0;
    }

    initialize_problem_matrices(p);

    long long total_supply = 0, total_demand = 0;

    for (int i = 0; i < p->n_suppliers; i++) {
        for (int j = 0; j < p->m_clients; j++) {
            if (fscanf(file, "%d", &p->cost_matrix[i][j]) != 1) {
                fprintf(stderr, "Erreur lecture cout a_%d,%d.\n", i+1, j+1);
                fclose(file); return 0;
            }
        }
        if (fscanf(file, "%d", &p->provisions[i]) != 1) {
            fprintf(stderr, "Erreur lecture P_%d.\n", i+1);
            fclose(file); return 0;
        }
        total_supply += p->provisions[i];
    }

    for (int j = 0; j < p->m_clients; j++) {
        if (fscanf(file, "%d", &p->commands[j]) != 1) {
            fprintf(stderr, "Erreur lecture C_%d.\n", j+1);
            fclose(file); return 0;
        }
        total_demand += p->commands[j];
    }

    fclose(file);

    if (total_supply != total_demand) {
        fprintf(stderr, "ATTENTION: probleme non equilibre. Somme(Pi)=%lld, Somme(Cj)=%lld\n",
                total_supply, total_demand);
        return 0;
    }

    return 1;
}

void display_table(const char* title, int rows, int cols, int** matrix) {
    printf("\n--- %s ---\n", title);
    int max_val = 0;
    for (int i = 0; i < rows; i++)
        for (int j = 0; j < cols; j++)
            if (matrix[i][j] > max_val) max_val = matrix[i][j];

    int cell_width = (max_val > 0) ? snprintf(NULL,0,"%d",max_val)+1 : 4;

    printf("%*s |", cell_width, "");
    for (int j = 0; j < cols; j++) printf(" C%-*d|", cell_width-2, j+1);
    printf("\n");

    for (int j = 0; j < cols+1; j++) printf("-%.*s", cell_width+2, "------------------------------------");
    printf("\n");

    for (int i = 0; i < rows; i++) {
        printf("P%-*d |", cell_width-1, i+1);
        for (int j = 0; j < cols; j++) printf("%*d |", cell_width, matrix[i][j]);
        printf("\n");
    }
}

void display_problem_data(const TransportProblem* p) {
    printf("\n#################################################\n");
    printf("Probleme de Transport (%d fournisseurs x %d clients)\n", p->n_suppliers, p->m_clients);
    printf("#################################################\n");

    printf("\n--- Matrice des couts unitaires (A) ---\n");
    printf("%8s |", "");
    for (int j = 0; j < p->m_clients; j++) printf("  C%-3d |", j+1);
    printf("   Pi  |\n");

    for (int i = 0; i < p->n_suppliers; i++) {
        printf("P%-3d   |", i+1);
        for (int j = 0; j < p->m_clients; j++)
            printf("%6d |", p->cost_matrix[i][j]);
        printf(" %6d |\n", p->provisions[i]);
    }

    printf("%8s |", "Cj");
    for (int j = 0; j < p->m_clients; j++) printf("%6d |", p->commands[j]);
    printf("\n");
}

// ==========================================================
// ALGORITHMES
// ==========================================================

long long calculate_total_cost(const TransportProblem* p) {
    long long cost = 0;
    for (int i = 0; i < p->n_suppliers; i++)
        for (int j = 0; j < p->m_clients; j++)
            cost += (long long)p->cost_matrix[i][j] * p->transport_plan[i][j];
    return cost;
}

void north_west_corner(TransportProblem* p) {
    int i=0,j=0;
    int* remaining_P = (int*)malloc(p->n_suppliers*sizeof(int));
    int* remaining_C = (int*)malloc(p->m_clients*sizeof(int));
    for(int k=0;k<p->n_suppliers;k++) remaining_P[k]=p->provisions[k];
    for(int k=0;k<p->m_clients;k++) remaining_C[k]=p->commands[k];

    for(int r=0;r<p->n_suppliers;r++)
        for(int c=0;c<p->m_clients;c++) p->transport_plan[r][c]=0;

    printf("\nALGORITHME : Coin Nord-Ouest\n");
    while(i<p->n_suppliers && j<p->m_clients){
        int q = (remaining_P[i]<remaining_C[j])? remaining_P[i] : remaining_C[j];
        p->transport_plan[i][j]=q;
        remaining_P[i]-=q; remaining_C[j]-=q;
        printf("Affectation P%d -> C%d : %d (reste P%d=%d, C%d=%d)\n",
               i+1,j+1,q,i+1,remaining_P[i],j+1,remaining_C[j]);
        if(remaining_P[i]==0) i++;
        if(remaining_C[j]==0) j++;
    }

    free(remaining_P); free(remaining_C);
    display_table("Proposition initiale (Nord-Ouest)", p->n_suppliers, p->m_clients, p->transport_plan);
    p->total_cost = calculate_total_cost(p);
    printf("\nCout total initial : %lld\n", p->total_cost);
}

void balas_hammer(TransportProblem* p) {
    printf("\nALGORITHME : Balas-Hammer simplifie\n");

    int* remaining_P = (int*)malloc(p->n_suppliers * sizeof(int));
    int* remaining_C = (int*)malloc(p->m_clients * sizeof(int));
    if (!remaining_P || !remaining_C) { fprintf(stderr, "Erreur allocation\n"); exit(EXIT_FAILURE); }

    for (int i = 0; i < p->n_suppliers; i++) remaining_P[i] = p->provisions[i];
    for (int j = 0; j < p->m_clients; j++) remaining_C[j] = p->commands[j];

    // Réinitialiser le plan
    for (int i = 0; i < p->n_suppliers; i++)
        for (int j = 0; j < p->m_clients; j++)
            p->transport_plan[i][j] = 0;

    int remaining = p->n_suppliers + p->m_clients; // nombre d’allocations à faire
    while (remaining > 0) {
        // Calculer pénalités lignes et colonnes
        int best_penalty = -1, best_i = -1, best_j = -1;
        int allocate_i = -1, allocate_j = -1;

        // Pénalités lignes
        for (int i = 0; i < p->n_suppliers; i++) {
            if (remaining_P[i] == 0) continue;
            int min1 = 1e9, min2 = 1e9;
            int min_j = -1;
            for (int j = 0; j < p->m_clients; j++) {
                if (remaining_C[j] == 0) continue;
                int c = p->cost_matrix[i][j];
                if (c < min1) { min2 = min1; min1 = c; min_j = j; }
                else if (c < min2) min2 = c;
            }
            int penalty = (min2 == 1e9) ? min1 : min2 - min1;
            if (penalty > best_penalty) { best_penalty = penalty; best_i = i; best_j = min_j; }
        }

        // Pénalités colonnes
        for (int j = 0; j < p->m_clients; j++) {
            if (remaining_C[j] == 0) continue;
            int min1 = 1e9, min2 = 1e9;
            int min_i = -1;
            for (int i = 0; i < p->n_suppliers; i++) {
                if (remaining_P[i] == 0) continue;
                int c = p->cost_matrix[i][j];
                if (c < min1) { min2 = min1; min1 = c; min_i = i; }
                else if (c < min2) min2 = c;
            }
            int penalty = (min2 == 1e9) ? min1 : min2 - min1;
            if (penalty > best_penalty) { best_penalty = penalty; best_i = min_i; best_j = j; }
        }

        // Allouer le max possible
        int q = (remaining_P[best_i] < remaining_C[best_j]) ? remaining_P[best_i] : remaining_C[best_j];
        p->transport_plan[best_i][best_j] = q;
        remaining_P[best_i] -= q;
        remaining_C[best_j] -= q;

        printf("Affectation P%d -> C%d : %d (reste P%d=%d, C%d=%d)\n",
               best_i+1, best_j+1, q, best_i+1, remaining_P[best_i], best_j+1, remaining_C[best_j]);

        if (remaining_P[best_i] == 0) remaining--;
        if (remaining_C[best_j] == 0) remaining--;
    }

    free(remaining_P); free(remaining_C);

    display_table("Proposition initiale (Balas-Hammer)", p->n_suppliers, p->m_clients, p->transport_plan);
    p->total_cost = calculate_total_cost(p);
    printf("Cout total initial : %lld\n", p->total_cost);
}

void run_step_stone(TransportProblem* p) {
    printf("\nALGORITHME : Methode du marche-pied (Step-Stone) - placeholder\n");
    p->total_cost = calculate_total_cost(p);
    display_table("Plan actuel", p->n_suppliers, p->m_clients, p->transport_plan);
    printf("Cout courant (non optimise) : %lld\n", p->total_cost);
}

// ==========================================================
// SOUS-FONCTIONS PLACEHOLDER POUR STEP-STONE
// ==========================================================
// Verifie si la solution est acyclique
int is_acyclic(const TransportProblem* p) {
    // Placeholder : on suppose que la solution est toujours acyclique
    printf("Verification acyclicite (placeholder) ... OK\n");
    return 1;
}

// Recherche et amelioration d'un cycle
int find_and_maximize_cycle(TransportProblem* p) {
    // Placeholder : on ne fait pas de vraie optimisation
    printf("Recherche et amelioration cycle (placeholder) ... rien fait\n");
    return 0; // pas de changement
}

// Verifie si la solution est connexe
int is_connected(const TransportProblem* p) {
    // Placeholder : on suppose que la solution est toujours connexe
    printf("Verification connexion (placeholder) ... OK\n");
    return 1;
}

// Calcule les potentiels u_i et v_j
void calculate_potentials(TransportProblem* p) {
    printf("Calcul des potentiels u et v (placeholder)\n");
    for (int i = 0; i < p->n_suppliers; i++) p->u_potentials[i] = 0;
    for (int j = 0; j < p->m_clients; j++) p->v_potentials[j] = 0;
}

// Calcule les couts marginaux a_ij - (u_i + v_j)
void calculate_marginal_costs(TransportProblem* p) {
    printf("Calcul des couts marginaux (placeholder)\n");
    for (int i = 0; i < p->n_suppliers; i++) {
        for (int j = 0; j < p->m_clients; j++) {
            p->marginal_costs[i][j] = p->cost_matrix[i][j] - (p->u_potentials[i] + p->v_potentials[j]);
        }
    }
}
