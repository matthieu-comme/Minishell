/** @file builtins.c
 * @brief Implementation of built-in commands
 * @author Nom1
 * @author Nom2
 * @date 2025-26
 * @details Implémentation des fonctions des commandes intégrées.
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "builtins.h"
#include "processus.h"

/** @brief Fonction de vérification si une commande est une commande "built-in".
 * @param cmd Nom de la commande à vérifier.
 * @return int 1 si la commande est intégrée, 0 sinon.
 * @details Les commandes intégrées sont a minima: cd, exit, export, unset, pwd.
 */
int is_builtin(const processus_t *cmd)
{
    if (!cmd || !cmd->argv[0])
        return 0; // On enlève la vérification de cmd->argv
    const char *c = cmd->argv[0];
    return (strcmp(c, "cd") == 0) ||
           (strcmp(c, "exit") == 0) ||
           (strcmp(c, "export") == 0) ||
           (strcmp(c, "unset") == 0) ||
           (strcmp(c, "pwd") == 0);
}

/** @brief Fonction d'exécution d'une commande intégrée.
 * @param cmd Pointeur vers la structure de commande à exécuter.
 * @return int 0 en cas de succès, -1 en cas d'erreur.
 */
int exec_builtin(processus_t *cmd)
{

    if (!cmd || !cmd->argv[0])
        return -1;
    if (strcmp(cmd->argv[0], "cd") == 0)
        return builtin_cd(cmd);
    if (strcmp(cmd->argv[0], "exit") == 0)
        return builtin_exit(cmd);
    if (strcmp(cmd->argv[0], "export") == 0)
        return builtin_export(cmd);
    if (strcmp(cmd->argv[0], "unset") == 0)
        return builtin_unset(cmd);
    if (strcmp(cmd->argv[0], "pwd") == 0)
        return builtin_pwd(cmd);
    return -1;
}

/** Fonctions spécifiques aux commandes intégrées. */
/** @brief Fonction d'exécution de la commande "cd".
 * @param cmd Pointeur vers la structure de commande à exécuter.
 * @return int 0 en cas de succès, -1 en cas d'erreur.
 * @details Déplace le CWD du processus vers le répertoire spécifié dans le premier argument de la commande.
 *  Si aucun argument n'est fourni, le CWD est déplacé vers le répertoire HOME de l'utilisateur.
 *  En cas d'erreur (répertoire inexistant, permission refusée, etc.), un message d'erreur est affiché sur *cmd->stderr* et la fonction retourne un code d'erreur.
 */
int builtin_cd(processus_t *cmd)
{

    const char *path = cmd->argv[1] ? cmd->argv[1] : getenv("HOME");
    if (!path)
    {
        dprintf(cmd->stderr_fd, "cd: HOME non défini\n");
        return -1;
    }
    if (chdir(path) != 0)
    {
        dprintf(cmd->stderr_fd, "cd: impossible d'accéder à %s\n", path);
        return -1;
    }
    return 0;
}

/** @brief Fonction d'exécution de la commande "exit".
 * @param cmd Pointeur vers la structure de commande à exécuter.
 * @return int 0 en cas de succès, -1 en cas d'erreur.
 * @details Termine le shell avec le code de sortie spécifié dans le premier argument de la commande.
 *  Si aucun argument n'est fourni, le shell se termine avec le code de sortie 0.
 *  En cas d'erreur (argument non numérique, etc.), un message d'erreur est affiché sur *cmd->stderr* et la fonction retourne un code d'erreur
 */
int builtin_exit(processus_t *cmd)
{

    int code = 0;
    if (cmd->argv[1])
    {
        char *end = NULL;
        long v = strtol(cmd->argv[1], &end, 10);
        if (!end || *end != '\0')
        {
            dprintf(cmd->stderr_fd, "exit: argument non numérique\n");
            return -1;
        }
        code = (int)(v & 0xFF);
    }
    exit(code);
}

/** @brief Fonction d'exécution de la commande "export".
 * @param cmd Pointeur vers la structure de commande à exécuter.
 * @return int 0 en cas de succès, -1 en cas d'erreur.
 * @details Ajoute ou modifie une variable d'environnement dans l'environnement du shell. En cas d'erreur (format invalide, etc.), un message d'erreur est affiché sur *cmd->stderr* et la fonction retourne un code d'erreur.
 */
int builtin_export(processus_t *cmd)
{
    // Format attendu: export VAR=val
    if (!cmd->argv[1])
        return 0;

    for (int i = 1; cmd->argv[i]; ++i)
    {
        char *arg = cmd->argv[i];
        char *eq = strchr(arg, '=');

        // pas de = ou = au début
        if (!eq || eq == arg)
        {
            dprintf(cmd->stderr_fd, "export: format VAR=val requis\n");
            return -1;
        }

        int var_len = eq - arg;

        // extrait le nom de la var et sa valeur
        char *var_name = malloc(var_len + 1);
        if (!var_name)
        {
            perror("malloc");
            return -1;
        }

        strncpy(var_name, arg, var_len);
        var_name[var_len] = '\0';

        char *val = eq + 1;

        if (setenv(var_name, val, 1) != 0)
        {
            dprintf(cmd->stderr_fd, "export: échec pour %s\n", var_name);
            free(var_name);
            return -1;
        }

        free(var_name);
    }
    return 0;
}

/** @brief Fonction d'exécution de la commande "unset".
 * @param cmd Pointeur vers la structure de commande à exécuter.
 * @return int 0 en cas de succès, -1 en cas d'erreur.
 * @details Supprime une variable d'environnement de l'environnement du shell. En cas d'erreur (variable inexistante, etc.), un message d'erreur est affiché sur *cmd->stderr* et la fonction retourne un code d'erreur.
 */
int builtin_unset(processus_t *cmd)
{
    if (!cmd->argv[1])
        return 0;

    int ret = 0;

    for (int i = 1; cmd->argv[i]; ++i)
    {
        if (unsetenv(cmd->argv[i]) != 0)
        {
            dprintf(cmd->stderr_fd, "unset: identifiant invalide '%s'\n", cmd->argv[i]);
            ret = -1;
        }
    }

    return ret;
}

/** @brief Fonction d'exécution de la commande "pwd".
 * @param cmd Pointeur vers la structure de commande à exécuter.
 * @return int 0 en cas de succès, -1 en cas d'erreur.
 * @details Affiche le répertoire de travail actuel (CWD) du processus sur la sortie standard *cmd->stdout*. En cas d'erreur, un message d'erreur est affiché sur *cmd->stderr* et la fonction retourne un code d'erreur.
 */
int builtin_pwd(processus_t *cmd)
{

    char buf[4096];
    if (!getcwd(buf, sizeof buf))
    {
        dprintf(cmd->stderr_fd, "pwd: erreur getcwd\n");
        return -1;
    }
    dprintf(cmd->stdout_fd, "%s\n", buf);
    return 0;
}