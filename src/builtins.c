/** @file builtins.c
 * @brief Implementation of built-in commands
 * @author Nom1
 * @author Nom2
 * @date 2025-26
 * @details Implémentation des fonctions des commandes intégrées.
 */

#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "builtins.h"
#include "processus.h"

/** @brief Fonction de vérification si une commande est une commande "built-in".
 * @param cmd Nom de la commande à vérifier.
 * @return int 1 si la commande est intégrée, 0 sinon.
 * @details Les commandes intégrées sont a minima: cd, exit, export, unset, pwd.
 */
int is_builtin(const processus_t* cmd) {

}

/** @brief Fonction d'exécution d'une commande intégrée.
 * @param cmd Pointeur vers la structure de commande à exécuter.
 * @return int 0 en cas de succès, -1 en cas d'erreur.
 */
int exec_builtin(processus_t* cmd) {

}

/** Fonctions spécifiques aux commandes intégrées. */
/** @brief Fonction d'exécution de la commande "cd".
 * @param cmd Pointeur vers la structure de commande à exécuter.
 * @return int 0 en cas de succès, -1 en cas d'erreur.
 * @details Déplace le CWD du processus vers le répertoire spécifié dans le premier argument de la commande.
 *  Si aucun argument n'est fourni, le CWD est déplacé vers le répertoire HOME de l'utilisateur.
 *  En cas d'erreur (répertoire inexistant, permission refusée, etc.), un message d'erreur est affiché sur *cmd->stderr* et la fonction retourne un code d'erreur.
 */
int builtin_cd(processus_t* cmd) {

}

/** @brief Fonction d'exécution de la commande "exit".
 * @param cmd Pointeur vers la structure de commande à exécuter.
 * @return int 0 en cas de succès, -1 en cas d'erreur.
 * @details Termine le shell avec le code de sortie spécifié dans le premier argument de la commande.
 *  Si aucun argument n'est fourni, le shell se termine avec le code de sortie 0.
 *  En cas d'erreur (argument non numérique, etc.), un message d'erreur est affiché sur *cmd->stderr* et la fonction retourne un code d'erreur
 */
int builtin_exit(processus_t* cmd) {

}

/** @brief Fonction d'exécution de la commande "export".
 * @param cmd Pointeur vers la structure de commande à exécuter.
 * @return int 0 en cas de succès, -1 en cas d'erreur.
 * @details Ajoute ou modifie une variable d'environnement dans l'environnement du shell. En cas d'erreur (format invalide, etc.), un message d'erreur est affiché sur *cmd->stderr* et la fonction retourne un code d'erreur.
 */
int builtin_export(processus_t* cmd) {

}

/** @brief Fonction d'exécution de la commande "unset".
 * @param cmd Pointeur vers la structure de commande à exécuter.
 * @return int 0 en cas de succès, -1 en cas d'erreur.
 * @details Supprime une variable d'environnement de l'environnement du shell. En cas d'erreur (variable inexistante, etc.), un message d'erreur est affiché sur *cmd->stderr* et la fonction retourne un code d'erreur.
 */
int builtin_unset(processus_t* cmd) {

}

/** @brief Fonction d'exécution de la commande "pwd".
 * @param cmd Pointeur vers la structure de commande à exécuter.
 * @return int 0 en cas de succès, -1 en cas d'erreur.
 * @details Affiche le répertoire de travail actuel (CWD) du processus sur la sortie standard *cmd->stdout*. En cas d'erreur, un message d'erreur est affiché sur *cmd->stderr* et la fonction retourne un code d'erreur.
 */
int builtin_pwd(processus_t* cmd) {

}
