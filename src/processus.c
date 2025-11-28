/** @file processus.c
 * @brief Implementation of process management functions
 * @author Nom1
 * @author Nom2
 * @date 2025-26
 * @details Implémentation des fonctions de gestion des processus.
 */
#ifndef MAX_OPENED
#define MAX_OPENED 64
#endif

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "processus.h"
#include "builtins.h"

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
 * - *start_time*: {0}
 * - *end_time*: {0}
 * - *cf*: NULL
 */
 int init_processus(processus_t* proc) {

    if (!proc) return -1;
    memset(proc, 0, sizeof(*proc));
    proc->stdin_fd = 0; proc->stdout_fd = 1; proc->stderr_fd = 2;
    return 0;
}

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
        int launch_processus(processus_t* proc) {
            if (!proc || !proc->argv[0]) {
                fprintf(stderr, "Erreur: commande invalide\n");
                return -1;
            }

        if (is_builtin(proc)) {
        int rc = exec_builtin(proc);
        proc->status = (rc == 0) ? 0 : 1;
        return 0;
    }

            /* ========= 2) COMMANDES EXTERNES ========= */
            pid_t pid = fork();
            if (pid < 0) {
                perror("fork failed");
                proc->status = 1;
                return -1;
            }

            if (pid == 0) {
                /* ----- Processus fils ----- */

                // Appliquer les redirections
                if (proc->stdin_fd != 0) {
                    dup2(proc->stdin_fd, STDIN_FILENO);
                }
                if (proc->stdout_fd != 1) {
                    dup2(proc->stdout_fd, STDOUT_FILENO);
                }
                if (proc->stderr_fd != 2) {
                    dup2(proc->stderr_fd, STDERR_FILENO);
                }

                // Fermer les descripteurs recensés dans opened_descriptors
                if (proc->cf && proc->cf->cmdl) {
                    for (int i = 0; i < MAX_OPENED; ++i) {
                        int fd = proc->cf->cmdl->opened_descriptors[i];
                        if (fd >= 3) {   // on ne ferme pas 0,1,2
                            close(fd);
                        }
                    }
                }

                // Choisir le chemin à exécuter : path si défini, sinon argv[0]
                const char* path = proc->path ? proc->path : proc->argv[0];
                execvp(path, proc->argv);
                perror("execvp failed");
                _exit(127);
            }

            /* ----- Processus père ----- */
            proc->pid = pid;

            // Très important pour les pipes et redirections :
            // le père n'a plus besoin de ses copies des FDs de redirection.
            if (proc->stdin_fd > 2)  close(proc->stdin_fd);
            if (proc->stdout_fd > 2) close(proc->stdout_fd);
            if (proc->stderr_fd > 2) close(proc->stderr_fd);

            // (optionnel mais propre) : revenir aux valeurs par défaut dans la struct
            proc->stdin_fd  = 0;
            proc->stdout_fd = 1;
            proc->stderr_fd = 2;

            // Si exécution en arrière-plan (&), on ne bloque pas
            if (proc->is_background) {
                proc->status = 0;   // on considère que le lancement est OK
                return 0;
            }

            // Avant-plan : on attend la fin du processus
            int status = 0;
            if (waitpid(pid, &status, 0) < 0) {
                perror("waitpid");
                proc->status = 1;
                return -1;
            }

            if (WIFEXITED(status)) {
                proc->status = WEXITSTATUS(status);
            } else if (WIFSIGNALED(status)) {
                proc->status = 128 + WTERMSIG(status);
            } else {
                proc->status = 1;
            }

            return 0;
        }



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
 int init_control_flow(control_flow_t* cf) {
    // Ici, un appel à bzero permettrais sûrement de faire le travail en une seule ligne
    if (!cf) return -1;
    memset(cf, 0, sizeof(*cf));
    return 0;
}

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

processus_t* add_processus(command_line_t* cmdl, control_flow_mode_t mode) {
    if (cmdl == NULL || cmdl->num_commands >= MAX_CMDS)
        return NULL;

    // Get the current process and control flow
    processus_t* proc = &cmdl->commands[cmdl->num_commands];
    control_flow_t* cf = &cmdl->flow[cmdl->num_commands];

    // Initialize the process and control flow
    init_processus(proc);
    init_control_flow(cf);

    cf->proc = proc;
    cf->cmdl = cmdl;

    // Link to the previous control flow if it exists
    if (cmdl->num_commands > 0) {
        control_flow_t* prev_cf = &cmdl->flow[cmdl->num_commands - 1];
        if (mode == UNCONDITIONAL)
            prev_cf->unconditionnal_next = cf;
        else if (mode == ON_SUCCESS)
            prev_cf->on_success_next = cf;
        else if (mode == ON_FAILURE)
            prev_cf->on_failure_next = cf;
    }
    cmdl->num_commands++;
    return proc;
}




/** @brief Fonction de récupération du prochain processus à exécuter selon le contrôle de flux.
 * @param cmdl Pointeur vers la structure de ligne de commande.
 * @return processus_t* Pointeur vers le prochain processus à exécuter, ou NULL si le nombre maximum est atteint.
 * @details Cette fonction retourne un pointeur vers la structure processus_t dans le tableau *commands* situé à l'indice *num_commands + 1*.
 *  Si le nombre maximum de commandes est atteint (MAX_CMDS), la fonction retourne NULL.
 *  Cela permet notamment d'initialiser les descripteurs des IOs standards qui dépendent du processus en court de traitement (dans le cas des pipes par exemple).
 */
 processus_t* next_processus(command_line_t* cmdl) {

    if (!cmdl) return NULL;
    if (cmdl->num_commands >= MAX_CMDS) return NULL;
    return &cmdl->commands[cmdl->num_commands];
}

/** @brief Fonction d'ajout d'un descripteur de fichier à la structure de contrôle de flux.
 * @param cmdl Pointeur vers la structure de ligne de commande.
 * @param fd Descripteur de fichier à ajouter.
 * @return int 0 en cas de succès, -1 en cas d'erreur (tableau plein ou fd invalide).
 * @details Cette fonction ajoute le descripteur de fichier *fd* au tableau *opened_descriptors* de la structure *cf*.
 *    Si le tableau est plein ou si *fd* est invalide (négatif), la fonction retourne -1
 */
 int add_fd(command_line_t* cmdl, int fd) {

    if (!cmdl || fd < 0) return -1;
    for (int i = 0; i < MAX_OPENED; ++i) {
        if (cmdl->opened_descriptors[i] == -1) { cmdl->opened_descriptors[i] = fd; return 0; }
    }
    return -1;
}

/** @brief Fonction de fermeture des descripteurs de fichiers listés dans la structure de contrôle de flux.
 * @param cmdl Pointeur vers la structure de ligne de commande.
 * @return int 0 en cas de succès, -1 en cas d'erreur.
 * @details Cette fonction ferme tous les descripteurs de fichiers listés dans le tableau *opened_descriptors* de la structure *cf*.
 *    Après fermeture, les entrées du tableau sont remises à -1.
 */
 int close_fds(command_line_t* cmdl) {

    if (!cmdl) return -1;
    for (int i = 0; i < MAX_OPENED; ++i) {
        if (cmdl->opened_descriptors[i] >= 0) { close(cmdl->opened_descriptors[i]); cmdl->opened_descriptors[i] = -1; }
    }
    return 0;
}

/** @brief Fonction d'initialisation d'une structure de ligne de commande.
 * @param cmdl Pointeur vers la structure de ligne de commande à initialiser.
 * @return int 0 en cas de succès, -1 en cas d'erreur.
 * @details Cette fonction initialise les champs de la structure avec les valeurs suivantes:
 * - *command_line*: "\0"
 * - *tokens*: {NULL}
 * - *commands*: tableau de processus initialisés via *init_processus()*
 * - *flow*: tableau de contrôle de flux initialisé via *init_control_flow()*
 * - *num_commands*: 0
 * - *opened_descriptors*: {-1}
 */
 int init_command_line(command_line_t* cmdl) {
    if (!cmdl) return -1;
    memset(cmdl->command_line, 0, sizeof cmdl->command_line);
    for (size_t i = 0; i < (MAX_CMD_LINE/2 + 1); ++i) cmdl->tokens[i] = NULL;
    for (int i = 0; i < MAX_CMDS; ++i) { init_processus(&cmdl->commands[i]); init_control_flow(&cmdl->flow[i]); }
    for (int i = 0; i < MAX_OPENED; ++i) cmdl->opened_descriptors[i] = -1;
    cmdl->num_commands = 0;
    return 0;
}
/** @brief Fonction de lancement d'une ligne de commande.
 * @param cmdl Pointeur vers la structure de ligne de commande à lancer.
 * @return int 0 en cas de succès, un code d'erreur sinon.
 * @details Cette fonction lance les processus selon le flux défini dans la structure *cmdl*. Les lancements sont effectués via *launch_processus()* en
 *    respectant les conditions de contrôle de flux (inconditionnel, en cas de succès, en cas d'échec).
 *    Le tableau *opened_descriptors* est utilisé pour fermer les descripteurs ouverts au moment de l'initialisation des structures processus_t.
 *    La fonction retourne 0 si tous les processus à lancer en fonction du contrôle de flux ont pu être lancés sans erreur.
 */


int launch_command_line(command_line_t* cmdl) {
    if (!cmdl || cmdl->num_commands == 0) return -1;

    control_flow_t* cur = &cmdl->flow[0];  // Début du flux de commandes

    while (cur) {
        processus_t* p = cur->proc;

        // Lancer le processus courant
        launch_processus(p);   // Lance le processus
        int status = p->status; // Récupère le statut du processus

        // Inversion éventuelle (si le processus a échoué, inverser le statut)
        if (p->invert) {
            status = (status == 0) ? 1 : 0;  // Inverse le statut (0 -> 1, 1 -> 0)
        }

        // Choisir le prochain maillon en fonction du statut
        if (cur->unconditionnal_next) {
            cur = cur->unconditionnal_next;  // Passage au prochain processus (inconditionnel)
        } else if (status == 0 && cur->on_success_next) {
            // Si succès (status == 0), passer à la commande suivante en cas de succès
            cur = cur->on_success_next;
        } else if (status != 0 && cur->on_failure_next) {
            // Si échec (status != 0), passer à la commande suivante en cas d'échec
            cur = cur->on_failure_next;
        } else {
            // Si aucun maillon suivant, arrêter la boucle
            cur = NULL;
        }
    }

    // Nettoyage
    close_fds(cmdl);
    return 0;
}