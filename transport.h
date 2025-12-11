#ifndef TRANSPORT_H
#define TRANSPORT_H


// Structure principale du problème de transport
typedef struct {
    int n_suppliers;        // Nombre de fournisseurs
    int m_clients;          // Nombre de clients

    // Données de base
    int** cost_matrix;      // Coût pour chaque fournisseur vers chaque client
    int* provisions;        // Quantité disponible par fournisseur
    int* commands;          // Quantité demandée par client

    // Solution et données intermédiaires
    int** transport_plan;   // Quantité envoyée par chaque fournisseur
    long long total_cost;   // Coût total du transport

    // Pour la méthode Step-Stone
    int* u_potentials;      // Valeurs fournisseurs
    int* v_potentials;      // Valeurs clients
    int** marginal_costs;   // Coûts supplémentaires possibles
} TransportProblem;

// ==========================================================
// Prototypes des fonctions
// ==========================================================

// Gestion mémoire
void initialize_problem_matrices(TransportProblem* p); // Créer tableaux
void free_problem(TransportProblem* p);                // Libérer mémoire

// Lecture et affichage
int read_data_from_file(const char* filename, TransportProblem* p); // Lire fichier
void display_table(const char* title, int rows, int cols, int** matrix); // Afficher tableau
void display_problem_data(const TransportProblem* p); // Afficher données

// Méthodes principales
void north_west_corner(TransportProblem* p);        // Méthode coin Nord-Ouest
void balas_hammer(TransportProblem* p);             // Méthode Balas-Hammer
long long calculate_total_cost(const TransportProblem* p); // Calcul coût total
void run_step_stone(TransportProblem* p);           // Optimisation Step-Stone

// Fonctions internes Step-Stone
int is_acyclic(const TransportProblem* p);          // Vérifier absence de cycle
int find_and_maximize_cycle(TransportProblem* p);   // Trouver et améliorer cycle
int is_connected(const TransportProblem* p);        // Vérifier connexion complète
void calculate_potentials(TransportProblem* p);     // Calculer u et v
void calculate_marginal_costs(TransportProblem* p); // Calculer coûts marginaux

#endif // TRANSPORT_H
