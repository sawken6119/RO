#include "transport.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INF 1000000

// ==========================================================
// INITIALISATION ET LIBERATION
// ==========================================================

void initialize_problem_matrices(TransportProblem* p) {
    int n = p->n_suppliers;
    int m = p->m_clients;

    p->cost_matrix = (int**)calloc(n, sizeof(int*));
    p->transport_plan = (int**)calloc(n, sizeof(int*));
    p->marginal_costs = (int**)calloc(n, sizeof(int*));
    if (!p->cost_matrix || !p->transport_plan || !p->marginal_costs) {
        fprintf(stderr, "Erreur allocation pointeurs.\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < n; i++) {
        p->cost_matrix[i] = (int*)calloc(m, sizeof(int));
        p->transport_plan[i] = (int*)calloc(m, sizeof(int));
        p->marginal_costs[i] = (int*)calloc(m, sizeof(int));
        if (!p->cost_matrix[i] || !p->transport_plan[i] || !p->marginal_costs[i]) {
            fprintf(stderr, "Erreur allocation ligne i=%d.\n", i);
            exit(EXIT_FAILURE);
        }
    }

    p->provisions = (int*)calloc(n, sizeof(int));
    p->commands = (int*)calloc(m, sizeof(int));
    p->u_potentials = (int*)calloc(n, sizeof(int));
    p->v_potentials = (int*)calloc(m, sizeof(int));

    if (!p->provisions || !p->commands || !p->u_potentials || !p->v_potentials) {
        fprintf(stderr, "Erreur allocation tableaux 1D.\n");
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
    free(p->provisions); free(p->commands);
    free(p->u_potentials); free(p->v_potentials);
}

// ==========================================================
// LECTURE ET AFFICHAGE
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
                fprintf(stderr, "Erreur lecture cout.\n");
                fclose(file); return 0;
            }
        }
        if (fscanf(file, "%d", &p->provisions[i]) != 1) {
            fprintf(stderr, "Erreur lecture provision.\n");
            fclose(file); return 0;
        }
        total_supply += p->provisions[i];
    }

    for (int j = 0; j < p->m_clients; j++) {
        if (fscanf(file, "%d", &p->commands[j]) != 1) {
            fprintf(stderr, "Erreur lecture commande.\n");
            fclose(file); return 0;
        }
        total_demand += p->commands[j];
    }
    fclose(file);

    if (total_supply != total_demand) {
        fprintf(stderr, "ERREUR: Probleme non equilibre!\n");
        return 0;
    }
    return 1;
}

void display_table(const char* title, int rows, int cols, int** matrix) {
    printf("\n--- %s ---\n", title);
    int max_val = 0;
    for (int i = 0; i < rows; i++)
        for (int j = 0; j < cols; j++)
            if (abs(matrix[i][j]) > max_val) max_val = abs(matrix[i][j]);

    int cell_width = (max_val > 0) ? snprintf(NULL,0,"%d",max_val)+1 : 4;
    if (cell_width < 4) cell_width = 4;

    printf("%*s |", cell_width, "");
    for (int j = 0; j < cols; j++) printf(" C%-*d|", cell_width-1, j+1);
    printf("\n");

    for (int j = 0; j <= cols; j++)
        for(int k=0; k<cell_width+2; k++) printf("-");
    printf("\n");

    for (int i = 0; i < rows; i++) {
        printf("P%-*d |", cell_width-1, i+1);
        for (int j = 0; j < cols; j++) printf("%*d |", cell_width, matrix[i][j]);
        printf("\n");
    }
}

void display_problem_data(const TransportProblem* p) {
    printf("\n#################################################\n");
    printf("Probleme de Transport (%d fournisseurs x %d clients)\n",
           p->n_suppliers, p->m_clients);
    printf("#################################################\n");

    printf("\n--- Matrice des couts unitaires (A) ---\n");
    printf("%6s |", "");
    for (int j = 0; j < p->m_clients; j++) printf("  C%-3d |", j+1);
    printf("   Pi   |\n");

    for (int i = 0; i < p->n_suppliers; i++) {
        printf("P%-3d   |", i+1);
        for (int j = 0; j < p->m_clients; j++)
            printf("%6d |", p->cost_matrix[i][j]);
        printf(" %6d |\n", p->provisions[i]);
    }

    printf("%s     |", "Cj");
    for (int j = 0; j < p->m_clients; j++) printf("%6d |", p->commands[j]);
    printf("\n");
}

void display_potentials(const TransportProblem* p) {
    printf("\n--- Potentiels ---\n");
    printf("u: ");
    for (int i = 0; i < p->n_suppliers; i++)
        printf("u%d=%d  ", i+1, p->u_potentials[i]);
    printf("\nv: ");
    for (int j = 0; j < p->m_clients; j++)
        printf("v%d=%d  ", j+1, p->v_potentials[j]);
    printf("\n");
}

void display_potential_costs(const TransportProblem* p) {
    printf("\n--- Table des couts potentiels (u_i + v_j) ---\n");
    int** pot_costs = (int**)malloc(p->n_suppliers * sizeof(int*));
    for (int i = 0; i < p->n_suppliers; i++) {
        pot_costs[i] = (int*)malloc(p->m_clients * sizeof(int));
        for (int j = 0; j < p->m_clients; j++) {
            pot_costs[i][j] = p->u_potentials[i] + p->v_potentials[j];
        }
    }
    display_table("Couts potentiels", p->n_suppliers, p->m_clients, pot_costs);
    for (int i = 0; i < p->n_suppliers; i++) free(pot_costs[i]);
    free(pot_costs);
}

void display_marginal_costs_table(const TransportProblem* p) {
    display_table("Couts marginaux", p->n_suppliers, p->m_clients, p->marginal_costs);
}

// ==========================================================
// ALGORITHMES INITIAUX
// ==========================================================

long long calculate_total_cost(const TransportProblem* p) {
    long long cost = 0;
    for (int i = 0; i < p->n_suppliers; i++)
        for (int j = 0; j < p->m_clients; j++)
            cost += (long long)p->cost_matrix[i][j] * p->transport_plan[i][j];
    return cost;
}

void north_west_corner(TransportProblem* p) {
    int i=0, j=0;
    int* remaining_P = (int*)malloc(p->n_suppliers*sizeof(int));
    int* remaining_C = (int*)malloc(p->m_clients*sizeof(int));

    for(int k=0; k<p->n_suppliers; k++) remaining_P[k] = p->provisions[k];
    for(int k=0; k<p->m_clients; k++) remaining_C[k] = p->commands[k];
    for(int r=0; r<p->n_suppliers; r++)
        for(int c=0; c<p->m_clients; c++) p->transport_plan[r][c] = 0;

    printf("\n========== ALGORITHME : Coin Nord-Ouest ==========\n");

    while(i < p->n_suppliers && j < p->m_clients) {
        int q = (remaining_P[i] < remaining_C[j]) ? remaining_P[i] : remaining_C[j];
        p->transport_plan[i][j] = q;
        remaining_P[i] -= q;
        remaining_C[j] -= q;

        printf("Affectation P%d -> C%d : %d (reste P%d=%d, C%d=%d)\n",
               i+1, j+1, q, i+1, remaining_P[i], j+1, remaining_C[j]);

        if(remaining_P[i] == 0) i++;
        if(remaining_C[j] == 0) j++;
    }

    free(remaining_P);
    free(remaining_C);

    display_table("Proposition initiale (Nord-Ouest)",
                  p->n_suppliers, p->m_clients, p->transport_plan);
    p->total_cost = calculate_total_cost(p);
    printf("\nCout total initial : %lld\n", p->total_cost);
}

void balas_hammer(TransportProblem* p) {
    printf("\n========== ALGORITHME : Balas-Hammer ==========\n");

    int* remaining_P = (int*)malloc(p->n_suppliers * sizeof(int));
    int* remaining_C = (int*)malloc(p->m_clients * sizeof(int));
    int* row_active = (int*)malloc(p->n_suppliers * sizeof(int));
    int* col_active = (int*)malloc(p->m_clients * sizeof(int));

    for (int i = 0; i < p->n_suppliers; i++) {
        remaining_P[i] = p->provisions[i];
        row_active[i] = 1;
    }
    for (int j = 0; j < p->m_clients; j++) {
        remaining_C[j] = p->commands[j];
        col_active[j] = 1;
    }
    for (int i = 0; i < p->n_suppliers; i++)
        for (int j = 0; j < p->m_clients; j++)
            p->transport_plan[i][j] = 0;

    int iterations = p->n_suppliers + p->m_clients;

    while (iterations > 0) {
        int best_penalty = -1;
        int best_i = -1, best_j = -1;
        int is_row = 0;

        // Pénalités lignes
        for (int i = 0; i < p->n_suppliers; i++) {
            if (!row_active[i]) continue;

            int min1 = INF, min2 = INF, min_j = -1;
            for (int j = 0; j < p->m_clients; j++) {
                if (!col_active[j]) continue;
                int c = p->cost_matrix[i][j];
                if (c < min1) {
                    min2 = min1;
                    min1 = c;
                    min_j = j;
                } else if (c < min2) {
                    min2 = c;
                }
            }

            int penalty = (min2 == INF) ? min1 : (min2 - min1);
            printf("Penalite ligne P%d : %d (min1=%d, min2=%d)\n",
                   i+1, penalty, min1, (min2==INF)?-1:min2);

            if (penalty > best_penalty) {
                best_penalty = penalty;
                best_i = i;
                best_j = min_j;
                is_row = 1;
            }
        }

        // Pénalités colonnes
        for (int j = 0; j < p->m_clients; j++) {
            if (!col_active[j]) continue;

            int min1 = INF, min2 = INF, min_i = -1;
            for (int i = 0; i < p->n_suppliers; i++) {
                if (!row_active[i]) continue;
                int c = p->cost_matrix[i][j];
                if (c < min1) {
                    min2 = min1;
                    min1 = c;
                    min_i = i;
                } else if (c < min2) {
                    min2 = c;
                }
            }

            int penalty = (min2 == INF) ? min1 : (min2 - min1);
            printf("Penalite colonne C%d : %d (min1=%d, min2=%d)\n",
                   j+1, penalty, min1, (min2==INF)?-1:min2);

            if (penalty > best_penalty) {
                best_penalty = penalty;
                best_i = min_i;
                best_j = j;
                is_row = 0;
            }
        }

        printf("--> Penalite maximale = %d ", best_penalty);
        if (is_row) printf("(ligne P%d)\n", best_i+1);
        else printf("(colonne C%d)\n", best_j+1);

        // Allocation
        int q = (remaining_P[best_i] < remaining_C[best_j]) ?
                 remaining_P[best_i] : remaining_C[best_j];
        p->transport_plan[best_i][best_j] = q;
        remaining_P[best_i] -= q;
        remaining_C[best_j] -= q;

        printf("Affectation P%d -> C%d : %d\n\n", best_i+1, best_j+1, q);

        if (remaining_P[best_i] == 0) {
            row_active[best_i] = 0;
            iterations--;
        }
        if (remaining_C[best_j] == 0) {
            col_active[best_j] = 0;
            iterations--;
        }
    }

    free(remaining_P); free(remaining_C);
    free(row_active); free(col_active);

    display_table("Proposition initiale (Balas-Hammer)",
                  p->n_suppliers, p->m_clients, p->transport_plan);
    p->total_cost = calculate_total_cost(p);
    printf("Cout total initial : %lld\n", p->total_cost);
}

// ==========================================================
// UTILITAIRES CYCLE
// ==========================================================

void init_cycle(Cycle* c) {
    c->edges = NULL;
    c->length = 0;
    c->capacity = 0;
}

void free_cycle(Cycle* c) {
    if (c->edges) free(c->edges);
    c->edges = NULL;
    c->length = 0;
    c->capacity = 0;
}

void add_edge_to_cycle(Cycle* c, int i, int j) {
    if (c->length >= c->capacity) {
        c->capacity = (c->capacity == 0) ? 10 : c->capacity * 2;
        c->edges = (Edge*)realloc(c->edges, c->capacity * sizeof(Edge));
    }
    c->edges[c->length].i = i;
    c->edges[c->length].j = j;
    c->length++;
}

void display_cycle(const Cycle* cycle) {
    if (cycle->length == 0) {
        printf("Aucun cycle detecte.\n");
        return;
    }
    printf("Cycle detecte : ");
    for (int k = 0; k < cycle->length; k++) {
        printf("P%d->C%d", cycle->edges[k].i+1, cycle->edges[k].j+1);
        if (k < cycle->length-1) printf(" -> ");
    }
    printf("\n");
}

int count_basic_variables(const TransportProblem* p) {
    int count = 0;
    for (int i = 0; i < p->n_suppliers; i++)
        for (int j = 0; j < p->m_clients; j++)
            if (p->transport_plan[i][j] > 0) count++;
    return count;
}

// ==========================================================
// DETECTION DE CYCLE (BFS)
// ==========================================================

int is_acyclic(const TransportProblem* p, Cycle* cycle) {
    printf("\n--- Test d'acyclicite (detection de cycle) ---\n");

    (void)cycle; // plus utilisée ici
    int n = p->n_suppliers;
    int m = p->m_clients;
    int total = n + m;

    int** adj = calloc(total, sizeof(int*));
    int* adj_size = calloc(total, sizeof(int));
    int* adj_cap = calloc(total, sizeof(int));

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            if (p->transport_plan[i][j] > 0) {
                int u = i;
                int v = n + j;

                if (adj_size[u] >= adj_cap[u]) {
                    adj_cap[u] = adj_cap[u] ? adj_cap[u] * 2 : 4;
                    adj[u] = realloc(adj[u], adj_cap[u] * sizeof(int));
                }
                adj[u][adj_size[u]++] = v;

                if (adj_size[v] >= adj_cap[v]) {
                    adj_cap[v] = adj_cap[v] ? adj_cap[v] * 2 : 4;
                    adj[v] = realloc(adj[v], adj_cap[v] * sizeof(int));
                }
                adj[v][adj_size[v]++] = u;
            }
        }
    }

    int* visited = calloc(total, sizeof(int));
    int* parent = malloc(total * sizeof(int));
    for (int i = 0; i < total; i++) parent[i] = -1;

    int* queue = malloc(total * sizeof(int));

    for (int s = 0; s < total; s++) {
        if (visited[s] || adj_size[s] == 0) continue;

        int h = 0, t = 0;
        queue[t++] = s;
        visited[s] = 1;

        while (h < t) {
            int u = queue[h++];
            for (int k = 0; k < adj_size[u]; k++) {
                int v = adj[u][k];
                if (!visited[v]) {
                    visited[v] = 1;
                    parent[v] = u;
                    queue[t++] = v;
                } else if (parent[u] != v) {
                    // cycle détecté
                    printf("CYCLE DETECTE dans le graphe!\n");

                    for (int i = 0; i < total; i++) free(adj[i]);
                    free(adj);
                    free(adj_size);
                    free(adj_cap);
                    free(visited);
                    free(parent);
                    free(queue);
                    return 0; // PAS acyclique
                }
            }
        }
    }

    printf("Aucun cycle detecte - le graphe est ACYCLIQUE.\n");

    for (int i = 0; i < total; i++) free(adj[i]);
    free(adj);
    free(adj_size);
    free(adj_cap);
    free(visited);
    free(parent);
    free(queue);
    return 1; 
}

int find_cycle(const TransportProblem* p, int si, int sj, Cycle* cycle) {
    int n = p->n_suppliers;
    int m = p->m_clients;
    int total = n + m;

    int* visited = calloc(total, sizeof(int));
    int* parent = malloc(total * sizeof(int));
    for (int i = 0; i < total; i++) parent[i] = -1;

    int* queue = malloc(total * sizeof(int));

    int start = si;
    int target = n + sj;

    int h = 0, t = 0;
    queue[t++] = start;
    visited[start] = 1;

    while (h < t) {
        int u = queue[h++];
        for (int j = 0; j < m; j++) {
            if (u < n && p->transport_plan[u][j] > 0) {
                int v = n + j;
                if (!visited[v]) {
                    visited[v] = 1;
                    parent[v] = u;
                    queue[t++] = v;
                }
            }
        }
        for (int i = 0; i < n; i++) {
            if (u >= n && p->transport_plan[i][u - n] > 0) {
                int v = i;
                if (!visited[v]) {
                    visited[v] = 1;
                    parent[v] = u;
                    queue[t++] = v;
                }
            }
        }
    }

    if (!visited[target]) {
        free(visited); free(parent); free(queue);
        return 0;
    }

    init_cycle(cycle);
    add_edge_to_cycle(cycle, si, sj); // arête +

    int v = target;
    while (parent[v] != -1) {
        int u = parent[v];
        if (u < n && v >= n)
            add_edge_to_cycle(cycle, u, v - n);
        else if (u >= n && v < n)
            add_edge_to_cycle(cycle, v, u - n);
        v = u;
    }

    free(visited); free(parent); free(queue);
    return 1;
}

// ==========================================================
// MAXIMISATION SUR CYCLE
// ==========================================================

int maximize_on_cycle(TransportProblem* p, const Cycle* cycle) {
    if (cycle->length == 0) return 0;

    printf("\n--- Maximisation du transport sur le cycle ---\n");

    // Trouver delta (minimum sur arêtes à signe -)
    int delta = INF;
    int edge_to_remove = -1;

    for (int k = 0; k < cycle->length; k++) {
        if (k % 2 == 1) { // Arêtes à signe -
            int i = cycle->edges[k].i;
            int j = cycle->edges[k].j;
            printf("Arete P%d->C%d : b[%d][%d] = %d\n",
                   i+1, j+1, i+1, j+1, p->transport_plan[i][j]);
            if (p->transport_plan[i][j] < delta) {
                delta = p->transport_plan[i][j];
                edge_to_remove = k;
            }
        }
    }

    printf("Delta = %d\n", delta);

    if (delta == 0) {
        printf("Cycle degeneré (delta = 0)\n");
        int i = cycle->edges[edge_to_remove].i;
        int j = cycle->edges[edge_to_remove].j;
        p->transport_plan[i][j] = 0;
        printf("Arete P%d->C%d retiree\n", i+1, j+1);
        return 1;
    }


    // Appliquer la maximisation
    for (int k = 0; k < cycle->length; k++) {
        int i = cycle->edges[k].i;
        int j = cycle->edges[k].j;

        if (k % 2 == 0) {
            p->transport_plan[i][j] += delta;
        } else {
            p->transport_plan[i][j] -= delta;
        }
    }

    printf("Arete supprimee : P%d->C%d\n",
           cycle->edges[edge_to_remove].i+1,
           cycle->edges[edge_to_remove].j+1);

    return 1;
}

// ==========================================================
// TEST DE CONNEXITE (BFS)
// ==========================================================

int is_connected(const TransportProblem* p) {
    printf("\n--- Test de connexite (parcours BFS) ---\n");

    int n = p->n_suppliers;
    int m = p->m_clients;
    int total_nodes = n + m;

    // Liste d'adjacence
    int** adj = (int**)calloc(total_nodes, sizeof(int*));
    int* adj_size = (int*)calloc(total_nodes, sizeof(int));
    int* adj_capacity = (int*)calloc(total_nodes, sizeof(int));

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            if (p->transport_plan[i][j] > 0) {
                int u = i;
                int v = n + j;

                if (adj_size[u] >= adj_capacity[u]) {
                    adj_capacity[u] = (adj_capacity[u] == 0) ? 4 : adj_capacity[u] * 2;
                    adj[u] = (int*)realloc(adj[u], adj_capacity[u] * sizeof(int));
                }
                adj[u][adj_size[u]++] = v;

                if (adj_size[v] >= adj_capacity[v]) {
                    adj_capacity[v] = (adj_capacity[v] == 0) ? 4 : adj_capacity[v] * 2;
                    adj[v] = (int*)realloc(adj[v], adj_capacity[v] * sizeof(int));
                }
                adj[v][adj_size[v]++] = u;
            }
        }
    }

    // BFS à partir du premier nœud actif
    int start = -1;
    for (int i = 0; i < total_nodes; i++) {
        if (adj_size[i] > 0) {
            start = i;
            break;
        }
    }

    if (start == -1) {
        printf("Aucune arete dans le graphe!\n");
        for (int i = 0; i < total_nodes; i++) if (adj[i]) free(adj[i]);
        free(adj); free(adj_size); free(adj_capacity);
        return 0;
    }

    int* visited = (int*)calloc(total_nodes, sizeof(int));
    int* queue = (int*)malloc(total_nodes * sizeof(int));

    int head = 0, tail = 0;
    queue[tail++] = start;
    visited[start] = 1;
    int visited_count = 1;

    while (head < tail) {
        int u = queue[head++];
        for (int k = 0; k < adj_size[u]; k++) {
            int v = adj[u][k];
            if (!visited[v]) {
                visited[v] = 1;
                visited_count++;
                queue[tail++] = v;
            }
        }
    }

    // Compter nœuds actifs
    int active_nodes = 0;
    for (int i = 0; i < total_nodes; i++) {
        if (adj_size[i] > 0) active_nodes++;
    }

    int is_conn = (visited_count == active_nodes);

    if (!is_conn) {
        printf("Le graphe est NON CONNEXE!\n");
        printf("Noeuds actifs: %d, Noeuds visites: %d\n", active_nodes, visited_count);

        // Afficher composantes connexes
        printf("Composantes connexes:\n");
        int comp_num = 1;
        for (int i = 0; i < total_nodes; i++) {
            if (adj_size[i] > 0 && !visited[i]) {
                printf("  Composante %d: ", comp_num++);
                // Afficher quelques nœuds
                if (i < n) printf("P%d ", i+1);
                else printf("C%d ", i-n+1);
                printf("(et autres)\n");
            }
        }
    } else {
        printf("Le graphe est CONNEXE.\n");
    }

    for (int i = 0; i < total_nodes; i++) if (adj[i]) free(adj[i]);
    free(adj); free(adj_size); free(adj_capacity);
    free(visited); free(queue);

    return is_conn;
}

void make_connected(TransportProblem* p) {
    printf("\n--- Modification pour rendre le graphe connexe ---\n");

    int n = p->n_suppliers;
    int m = p->m_clients;
    int expected = n + m - 1;
    int current = count_basic_variables(p);

    printf("Variables de base actuelles: %d, attendues: %d\n", current, expected);

    // Créer liste d'arêtes disponibles triées par coût
    typedef struct { int i, j, cost; } EdgeCost;
    EdgeCost* edges = (EdgeCost*)malloc(n * m * sizeof(EdgeCost));
    int edge_count = 0;

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            if (p->transport_plan[i][j] == 0) {
                edges[edge_count].i = i;
                edges[edge_count].j = j;
                edges[edge_count].cost = p->cost_matrix[i][j];
                edge_count++;
            }
        }
    }

    // Tri par coût croissant
    for (int i = 0; i < edge_count-1; i++) {
        for (int j = i+1; j < edge_count; j++) {
            if (edges[j].cost < edges[i].cost) {
                EdgeCost temp = edges[i];
                edges[i] = edges[j];
                edges[j] = temp;
            }
        }
    }

    // Ajouter arêtes jusqu'à n+m-1
    int added = 0;
    for (int k = 0; k < edge_count && current < expected; k++) {
        p->transport_plan[edges[k].i][edges[k].j] = 0; // Epsilon symbolique
        printf("Ajout arete P%d->C%d (cout=%d)\n",
               edges[k].i+1, edges[k].j+1, edges[k].cost);
        current++;
        added++;

        // Revérifier connexité
        if (is_connected(p)) break;
    }

    free(edges);
    printf("Total aretes ajoutees: %d\n", added);
}

// ==========================================================
// CALCUL DES POTENTIELS
// ==========================================================

void calculate_potentials(TransportProblem* p) {
    printf("\n--- Calcul des potentiels u_i et v_j ---\n");

    int n = p->n_suppliers;
    int m = p->m_clients;

    // Initialiser à une valeur invalide
    for (int i = 0; i < n; i++) p->u_potentials[i] = INF;
    for (int j = 0; j < m; j++) p->v_potentials[j] = INF;

    // Fixer u_0 = 0
    p->u_potentials[0] = 0;

    // Itérer jusqu'à convergence
    int changed = 1;
    int iterations = 0;

    while (changed && iterations < 100) {
        changed = 0;
        iterations++;

        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                if (p->transport_plan[i][j] > 0) {
                    // u_i + v_j = a_ij
                    if (p->u_potentials[i] != INF && p->v_potentials[j] == INF) {
                        p->v_potentials[j] = p->cost_matrix[i][j] - p->u_potentials[i];
                        changed = 1;
                    }
                    else if (p->v_potentials[j] != INF && p->u_potentials[i] == INF) {
                        p->u_potentials[i] = p->cost_matrix[i][j] - p->v_potentials[j];
                        changed = 1;
                    }
                }
            }
        }
    }

    display_potentials(p);
}

// ==========================================================
// COUTS MARGINAUX
// ==========================================================

void calculate_marginal_costs(TransportProblem* p) {
    for (int i = 0; i < p->n_suppliers; i++) {
        for (int j = 0; j < p->m_clients; j++) {
            p->marginal_costs[i][j] = p->cost_matrix[i][j] -
                                      (p->u_potentials[i] + p->v_potentials[j]);
        }
    }
}

int find_best_improving_edge(const TransportProblem* p, int* best_i, int* best_j) {
    int min_marginal = 0;
    *best_i = -1;
    *best_j = -1;

    for (int i = 0; i < p->n_suppliers; i++) {
        for (int j = 0; j < p->m_clients; j++) {
            if (p->transport_plan[i][j] == 0) {
                if (p->marginal_costs[i][j] < min_marginal) {
                    min_marginal = p->marginal_costs[i][j];
                    *best_i = i;
                    *best_j = j;
                }
            }
        }
    }

    if (*best_i != -1) {
        printf("\n==> Arete ameliorante trouvee: P%d->C%d (cout marginal = %d)\n",
               *best_i+1, *best_j+1, min_marginal);
        return 1;
    }

    printf("\n==> Aucune arete ameliorante (SOLUTION OPTIMALE)\n");
    return 0;
}

void add_improving_edge(TransportProblem* p, int i, int j) {
    printf("Ajout de l'arete ameliorante P%d->C%d\n", i+1, j+1);
    p->transport_plan[i][j] = 1; // Valeur symbolique qui sera ajustée
}

// ==========================================================
// METHODE DU MARCHE-PIED
// ==========================================================

void run_step_stone(TransportProblem* p) {
    printf("\n\n");
    printf("##########################################################\n");
    printf("## METHODE DU MARCHE-PIED AVEC POTENTIELS ##\n");
    printf("##########################################################\n");

    int iteration = 0;
    int max_iterations = 50;

    while (iteration < max_iterations) {
        iteration++;
        printf("\n");
        printf("==========================================================\n");
        printf(" ITERATION %d\n", iteration);
        printf("==========================================================\n");

        // 1. Afficher proposition actuelle
        display_table("Proposition de transport actuelle",
                     p->n_suppliers, p->m_clients, p->transport_plan);
        p->total_cost = calculate_total_cost(p);
        printf("Cout total: %lld\n", p->total_cost);

        // 2. Vérifier si dégénérée
        int nb_basic = count_basic_variables(p);
        int expected = p->n_suppliers + p->m_clients - 1;
        printf("\nVariables de base: %d (attendu: %d)\n", nb_basic, expected);

        if (nb_basic < expected) {
            printf("La proposition est DEGENEREE!\n");
        }

        // 3. Test acyclicité EN PREMIER (avant connexité)
        Cycle cycle;
        init_cycle(&cycle);
        int acyclic = is_acyclic(p, &cycle);

        while (!acyclic) {
            printf("\n>>> Elimination du cycle detecte...\n");
            // Maximiser sur le cycle
            int changed = maximize_on_cycle(p, &cycle);
            if (!changed) {
                printf("Impossible de maximiser (delta=0)\n");
                break;
            }

            display_table("Proposition apres maximisation",
                         p->n_suppliers, p->m_clients, p->transport_plan);

            // Re-tester acyclicité
            free_cycle(&cycle);
            init_cycle(&cycle);
            acyclic = is_acyclic(p, &cycle);
        }
        free_cycle(&cycle);

        // 4. Test connexité (APRÈS avoir éliminé les cycles)
        int connected = is_connected(p);
        if (!connected) {
            make_connected(p);
            display_table("Proposition apres ajout aretes",
                         p->n_suppliers, p->m_clients, p->transport_plan);
        }

        // 5. Calcul des potentiels
        calculate_potentials(p);

        // 6. Tables coûts potentiels et marginaux
        display_potential_costs(p);
        calculate_marginal_costs(p);
        display_marginal_costs_table(p);

        // 7. Recherche arête améliorante
        int best_i, best_j;
        int has_improving = find_best_improving_edge(p, &best_i, &best_j);

        if (!has_improving) {
            printf("\n*** SOLUTION OPTIMALE ATTEINTE ***\n");
            break;
        }

        // 8. Ajouter l'arête améliorante
        add_improving_edge(p, best_i, best_j);

        init_cycle(&cycle);
        if (!find_cycle(p, best_i, best_j, &cycle)) {
            printf("ERREUR: cycle non trouve apres ajout\n");
            free_cycle(&cycle);
            break;
        }

        display_cycle(&cycle);
        maximize_on_cycle(p, &cycle);
        free_cycle(&cycle);

        printf("\n--- Fin iteration %d ---\n", iteration);
    }

    if (iteration >= max_iterations) {
        printf("\nATTENTION: Nombre maximal d'iterations atteint!\n");
    }
}

int generate_random_problem(TransportProblem *problem,
                            int n_suppliers,
                            int m_clients,
                            int min_cost,
                            int max_cost,
                            int min_capacity,
                            int max_capacity) {

    problem->n_suppliers = n_suppliers;
    problem->m_clients = m_clients;

    // Allocation de la matrice des couts
    problem->cost_matrix = (long long **)malloc(n_suppliers * sizeof(long long *));
    if (!problem->cost_matrix) {
        fprintf(stderr, "ERREUR: Allocation memoire pour cost_matrix\n");
        return 0;
    }
    for (int i = 0; i < n_suppliers; i++) {
        problem->cost_matrix[i] = (long long *)malloc(m_clients * sizeof(long long));
        if (!problem->cost_matrix[i]) {
            fprintf(stderr, "ERREUR: Allocation memoire pour cost_matrix[%d]\n", i);
            return 0;
        }
    }

    // Allocation des provisions et commandes
    problem->provisions = (long long *)malloc(n_suppliers * sizeof(long long));
    problem->commands = (long long *)malloc(m_clients * sizeof(long long));

    if (!problem->provisions || !problem->commands) {
        fprintf(stderr, "ERREUR: Allocation memoire pour provisions/commands\n");
        return 0;
    }

    // Generation aleatoire de la matrice des couts
    printf("\nGeneration de la matrice des couts...\n");
    for (int i = 0; i < n_suppliers; i++) {
        for (int j = 0; j < m_clients; j++) {
            problem->cost_matrix[i][j] = min_cost + rand() % (max_cost - min_cost + 1);
        }
    }

    // Generation aleatoire des provisions
    printf("Generation des provisions...\n");
    long long total_provisions = 0;
    for (int i = 0; i < n_suppliers; i++) {
        problem->provisions[i] = min_capacity + rand() % (max_capacity - min_capacity + 1);
        total_provisions += problem->provisions[i];
    }

    // Generation aleatoire des commandes (avec equilibre)
    printf("Generation des commandes...\n");
    long long total_commands = 0;

    // Generer m_clients-1 commandes aleatoires
    for (int j = 0; j < m_clients - 1; j++) {
        problem->commands[j] = min_capacity + rand() % (max_capacity - min_capacity + 1);
        total_commands += problem->commands[j];
    }

    // La derniere commande ajuste pour equilibrer
    if (total_commands < total_provisions) {
        problem->commands[m_clients - 1] = total_provisions - total_commands;
    } else {
        // Si total_commands >= total_provisions, on recalcule tout
        total_commands = 0;
        long long remaining = total_provisions;

        for (int j = 0; j < m_clients - 1; j++) {
            long long max_possible = remaining - (m_clients - 1 - j) * min_capacity;
            if (max_possible < min_capacity) max_possible = min_capacity;
            if (max_possible > max_capacity) max_possible = max_capacity;

            problem->commands[j] = min_capacity + rand() % (max_possible - min_capacity + 1);
            total_commands += problem->commands[j];
            remaining -= problem->commands[j];
        }
        problem->commands[m_clients - 1] = remaining;
    }

    printf("\n>>> Probleme aleatoire genere avec succes <<<\n");
    printf("Total provisions: %lld\n", total_provisions);
    printf("Total commandes:  %lld\n", total_provisions);
    printf("Probleme equilibre: OUI\n");

    return 1;
}
