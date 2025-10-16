/**
 * @file processus.h
 * @brief Header file for process management
 * @author Nom1
 * @author Nom2
 * @date 2025-26
 * @details Définitions des structures, des constantes et des fonctions de gestion des processus.
 **/

#ifndef PROCESSUS_H
#define PROCESSUS_H

#include <unistd.h>

#include <stdint.h>
#include <time.h>

/// Nombre maximum d'arguments
#define MAX_ARGS 128
/// Nombre maximum de variables d'environnement
#define MAX_ENV 128
/// Nombre maximum de commandes sur une ligne
#define MAX_CMDS 128
/// Taille maximale d'une ligne de commande
#define MAX_CMD_LINE 4096

/** @brief Modes de contrôle de flux pour les processus.
 * @enum control_flow_mode_t
 * @details Cette énumération définit les différents modes de contrôle de flux pour l'exécution des processus.
 */
typedef enum {
    UNCONDITIONAL, ///< Exécution inconditionnelle
    ON_SUCCESS,    ///< Exécution en cas de succès
    ON_FAILURE     ///< Exécution en cas d'échec
} control_flow_mode_t;

struct control_flow; // Déclaration anticipée pour l'utilisation dans processus_t
struct command_line; // Déclaration anticipée pour l'utilisation dans control_flow_t

/**
 * @brief Structure représentant un processus.
 * @struct processus_t
 * @details Cette structure contient des informations sur un processus: PID, arguments, variables d'environnement, descripteurs des IO standards, le statut de sortie, un "flag" d'exécution en arrière-plan, et les temps de démarrage/arrêt.
 */
typedef struct {
    pid_t pid;                  ///< Process ID
    char* argv[MAX_ARGS];       ///< Liste des arguments
    char* envp[MAX_ENV];        ///< Variables d'environnement
    char* path;                 ///< Chemin de l'exécutable

    int stdin_fd;               ///< Descripteur d'entrée standard
    int stdout_fd;              ///< Descripteur de sortie standard
    int stderr_fd;              ///< Descripteur d'erreur standard
    int status;                 ///< Statut de sortie
    uint8_t is_background;      ///< Background flag
    uint8_t invert;             ///< Inversion du code de retour pour le contrôle de flux
    struct timespec start_time; ///< Start time
    struct timespec end_time;   ///< End time
    struct control_flow* cf;    ///< Pointeur vers la structure de contrôle de flux associée
} processus_t;

/** @brief Structure de contrôle de flux. 
 * @struct control_flow_t
 * @details Cette structure permet de gérer le flux d'exécution des processus.
*/
typedef struct control_flow {
    processus_t* proc;                     ///< Pointeur vers la structure du processus courant
    struct control_flow* unconditionnal_next; ///< Pointeur vers la prochaine structure de processus en cas d'exécution inconditionnelle
    struct control_flow* on_success_next;     ///< Pointeur vers la prochaine structure de processus en cas d'exécution réussie
    struct control_flow* on_failure_next;     ///< Pointeur vers la prochaine structure de processus en cas d'échec de l'exécution
    struct command_line* cmdl;                     ///< Pointeur vers la structure de ligne de commande associée
} control_flow_t;

/**
 * @brief Structure représentant une ligne de commande.
 * @struct command_line_t
 * @details Cette structure contient la ligne de commande complète, un tableau de structures de processus, une structure de contrôle de flux, et un tableau des descripteurs de fichiers ouverts.
 * La structure permet à la fois d'allouer l'espace mémoire nécessaire pour les processus et de gérer le flux d'exécution entre eux.
 * Le schéma suivant illustre la relation entre les structures:
 * \image html schema_struct.png
 */
typedef struct command_line {
    char command_line[MAX_CMD_LINE];  ///< Ligne de commande complète
    char* tokens[MAX_CMD_LINE / 2 + 1]; ///< Tableau des tokens extraits de la ligne de commande
    processus_t commands[MAX_CMDS];   ///< Tableau des structures de processus
    control_flow_t flow[MAX_CMDS];    ///< Structure de contrôle de flux
    unsigned int num_commands;        ///< Nombre de commandes
    int opened_descriptors[MAX_CMDS * 3 + 1]; ///< Tableau des descripteurs de fichiers ouverts
} command_line_t;

/**
 * @brief Fonction d'initialisation d'une structure de processus.
 * @param proc Pointeur vers la structure de processus à initialiser.
 * @return int 0 en cas de succès, -1 en cas d'erreur.
 * @details Cette fonction initialise les champs de la structure avec les valeurs suivantes:
 * - *pid*: 0
 * - *argv*: {NULL}
 * - *envp*: {NULL}
 * - *path*: NULL
 * - *stdin_fd*: 0
 * - *stdout_fd*: 1
 * - *stderr_fd*: 2
 * - *status*: 0
 * - *is_background*: 0
 * - *invert*: 0
 * - *start_time*: {0}
 * - *end_time*: {0}
 * - *cf*: NULL
 */
int init_processus(processus_t* proc);

/** @brief Fonction de lancement d'un processus à partir d'une structure de processus.
 * @param proc Pointeur vers la structure de processus à lancer.
 * @return int 0 en cas de succès, un code d'erreur sinon.
 * @details Cette fonction utilise *fork()* et *execve()* pour lancer le processus décrit par la structure.
 *    Elle gère également les redirections des IOs standards (via *dup2()*).
 *    En cas de succès, le champ *pid* de la structure est mis à jour avec le PID du processus fils.
 *    Le flag *is_background* détermine si on attend la fin du processus ou non.
 *    La valeur de *status* est mise à jour à l'issue de l'exécution avec le code de retour du processus fils lorsque le flag *is_background* est désactivé.
 *    Les temps de démarrage et d'arrêt sont enregistrés dans *start_time* et *end_time* respectivement. *end_time* est mis à jour uniquement si *is_background* est désactivé.
 *    Les descripteurs de fichiers ouverts sont gérés dans *cf->cmdl->opened_descriptors* : le processus "fils" ferme tous les descripteurs listés dans ce tableau avant d'exécuter la commande.
 */
int launch_processus(processus_t* proc);

/** @brief Fonction d'initialisation d'une structure de contrôle de flux.
 * @param cf Pointeur vers la structure de contrôle de flux à initialiser.
 * @return int 0 en cas de succès, -1 en cas d'erreur.
 * @details Cette fonction initialise les champs de la structure avec les valeurs suivantes:
 * - *proc*: NULL
 * - *unconditionnal_next*: NULL
 * - *on_success_next*: NULL
 * - *on_failure_next*: NULL
 * - *cmdl*: NULL
 */
int init_control_flow(control_flow_t* cf);

/** @brief Fonction d'ajout d'un processus à la structure de contrôle de flux.
 * @param cmdl Pointeur vers la structure de ligne de commande dans laquelle le processus doit être ajouté.
 * @param mode Mode d'ajout (UNCONDITIONAL, ON_SUCCESS, ON_FAILURE).
 * @return processus_t* Pointeur vers le processus ajouté, ou NULL en cas d'erreur (tableau plein par exemple).
 * @details Cette fonction ajoute le processus *proc* à la structure de contrôle de flux *cf* selon le mode spécifié:
 * Le dernier élément du tableau *commands* est retourné après avoir été initialisé dans le dernier élément du tableau *flow*.
 * Cette structure control_flow_t est mise à jour pour que le champ *proc* pointe vers le processus ajouté et la liste est mise à jour de la manière suivante :
 * - Si *mode* est UNCONDITIONAL, *proc* est ajouté à la liste des processus à exécuter inconditionnellement après le processus courant.
 * - Si *mode* est ON_SUCCESS, *proc* est ajouté à la liste des processus à exécuter uniquement si le processus courant s'est terminé avec succès (code de retour 0).
 * - Si *mode* est ON_FAILURE, *proc* est ajouté à la liste des processus à exécuter uniquement si le processus courant s'est terminé avec un échec (code de retour non nul).
 */
processus_t* add_processus(command_line_t* cmdl, control_flow_mode_t mode);

/** @brief Fonction de récupération du prochain processus à exécuter selon le contrôle de flux.
 * @param cmdl Pointeur vers la structure de ligne de commande.
 * @return processus_t* Pointeur vers le prochain processus à exécuter, ou NULL si le nombre maximum est atteint.
 * @details Cette fonction retourne un pointeur vers la structure processus_t dans le tableau *commands* situé à l'indice *num_commands + 1*.
 *  Si le nombre maximum de commandes est atteint (MAX_CMDS), la fonction retourne NULL.
 *  Cela permet notamment d'initialiser les descripteurs des IOs standards qui dépendent du processus en court de traitement (dans le cas des pipes par exemple).
 */
processus_t* next_processus(command_line_t* cmdl);

/** @brief Fonction d'ajout d'un descripteur de fichier à la structure de contrôle de flux.
 * @param cmdl Pointeur vers la structure de ligne de commande.
 * @param fd Descripteur de fichier à ajouter.
 * @return int 0 en cas de succès, -1 en cas d'erreur (tableau plein ou fd invalide).
 * @details Cette fonction ajoute le descripteur de fichier *fd* au tableau *opened_descriptors* de la structure *cf*.
 *    Si le tableau est plein ou si *fd* est invalide (négatif), la fonction retourne -1
 */
int add_fd(command_line_t* cmdl, int fd);

/** @brief Fonction de fermeture des descripteurs de fichiers listés dans la structure de contrôle de flux.
 * @param cmdl Pointeur vers la structure de ligne de commande.
 * @return int 0 en cas de succès, -1 en cas d'erreur.
 * @details Cette fonction ferme tous les descripteurs de fichiers listés dans le tableau *opened_descriptors* de la structure *cf*.
 *    Après fermeture, les entrées du tableau sont remises à -1.
 */
int close_fds(command_line_t* cmdl);

/** @brief Fonction d'initialisation d'une structure de ligne de commande.
 * @param cmdl Pointeur vers la structure de ligne de commande à initialiser.
 * @return int 0 en cas de succès, -1 en cas d'erreur.
 * @details Cette fonction initialise les champs de la structure avec les valeurs suivantes:
 * - *command_line*: "\0"
 * - *tokens*: {NULL}
 * - *commands*: tableau de processus initialisé via *init_processus()*
 * - *flow*: tableau de contrôle de flux initialisé via *init_control_flow()*
 * - *num_commands*: 0
 * - *opened_descriptors*: {-1}
 */
int init_command_line(command_line_t* cmdl);

/** @brief Fonction de lancement d'une ligne de commande.
 * @param cmdl Pointeur vers la structure de ligne de commande à lancer.
 * @return int 0 en cas de succès, un code d'erreur sinon.
 * @details Cette fonction lance les processus selon le flux défini dans la structure *cmdl*. Les lancements sont effectués via *launch_processus()* en
 *    respectant les conditions de contrôle de flux (inconditionnel, en cas de succès, en cas d'échec).
 *    Le tableau *opened_descriptors* est utilisé pour fermer les descripteurs ouverts au moment de l'initialisation des structures processus_t.
 *    La fonction retourne 0 si tous les processus à lancer en fonction du contrôle de flux ont pu être lancés sans erreur.
 */
int launch_command_line(command_line_t* cmdl);
#endif
