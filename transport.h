#ifndef TRANSPORT_H
#define TRANSPORT_H

#include <stddef.h>

// Structure principale du problème de transport
typedef struct {
    int n_suppliers;      // Nombre de fournisseurs (n)
    int m_clients;        // Nombre de clients (m)

    int** cost_matrix;    // Matrice des coûts unitaires A[i][j]
    int** transport_plan; // Proposition de transport b[i][j]
    int** marginal_costs; // Coûts marginaux

    int* provisions;      // Provisions P[i]
    int* commands;        // Commandes C[j]

    int* u_potentials;    // Potentiels u[i]
    int* v_potentials;    // Potentiels v[j]

    long long total_cost; // Coût total de la proposition
} TransportProblem;

// Structure pour représenter une arête (i,j)
typedef struct {
    int i, j;
} Edge;

// Structure pour un cycle
typedef struct {
    Edge* edges;
    int length;
    int capacity;
} Cycle;

// ==========================================================
// INITIALISATION ET LIBERATION
// ==========================================================
void initialize_problem_matrices(TransportProblem* p);
void free_problem(TransportProblem* p);

// ==========================================================
// LECTURE ET AFFICHAGE
// ==========================================================
int read_data_from_file(const char* filename, TransportProblem* p);
void display_table(const char* title, int rows, int cols, int** matrix);
void display_problem_data(const TransportProblem* p);
void display_potentials(const TransportProblem* p);
void display_potential_costs(const TransportProblem* p);
void display_marginal_costs_table(const TransportProblem* p);

// ==========================================================
// ALGORITHMES INITIAUX
// ==========================================================
void north_west_corner(TransportProblem* p);
void balas_hammer(TransportProblem* p);
long long calculate_total_cost(const TransportProblem* p);

// ==========================================================
// METHODE DU MARCHE-PIED
// ==========================================================
void run_step_stone(TransportProblem* p);

// Detection de cycle (parcours BFS)
int is_acyclic(const TransportProblem* p, Cycle* cycle);
int find_cycle(const TransportProblem* p, int si, int sj, Cycle* cycle);
void display_cycle(const Cycle* cycle);

// Maximisation sur un cycle
int maximize_on_cycle(TransportProblem* p, const Cycle* cycle);

// Test de connexité (parcours BFS)
int is_connected(const TransportProblem* p);
void make_connected(TransportProblem* p);

// Calcul des potentiels
void calculate_potentials(TransportProblem* p);

// Calcul des coûts marginaux et détection arête améliorante
void calculate_marginal_costs(TransportProblem* p);
int find_best_improving_edge(const TransportProblem* p, int* best_i, int* best_j);

// Ajout d'arête améliorante
void add_improving_edge(TransportProblem* p, int i, int j);

// ==========================================================
// FONCTIONS UTILITAIRES
// ==========================================================
void init_cycle(Cycle* c);
void free_cycle(Cycle* c);
void add_edge_to_cycle(Cycle* c, int i, int j);
int count_basic_variables(const TransportProblem* p);

#endif // TRANSPORT_H
