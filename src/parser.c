/** @file parser.c
 * @brief Implementation of command line parsing
 * @author Nom1
 * @author Nom2
 * @date 2025-26
 * @details Implémentation des fonctions d'analyse des lignes de commande.
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>

#include "parser.h"
#include "processus.h"

/** @brief Fonction de suppression des espaces inutiles au début et à la fin d'une chaîne de caractères.
 * @param str Chaîne de caractères à traiter.
 * @return int 0 en cas de succès
 */
int trim(char *str)
{
    char *start = str;
    while (*start == ' ')
    {
        ++start;
    }

    memmove(str, start, strlen(start) + 1);

    char *end = str + strlen(str) - 1;
    while (*end == ' ')
    {
        *end = '\0';
        --end;
    }
    return 0;
}

/** @brief Fonction de nettoyage d'une chaîne de caractères en supprimant les doublons d'espaces.
 * @param str Chaîne de caractères à nettoyer.
 * @return int 0 en cas de succès, -1 en cas d'erreur.
 */
int clean(char *str)
{
    int len = strlen(str) + 1;
    while (replace(str, "  ", " ", len) == 0)
        ;
    return 0;
}

/** @brief Fonction d'ajout de caractères d'espacement autour de tous les caractères de la chaîne *s* présents dans *str*.
 * @param str Chaîne de caractères à traiter.
 * @param s Chaîne de caractères contenant les séparateurs.
 * @param max Taille maximale de la chaîne *str*.
 * @return int 0 en cas de succès, -1 en cas d'erreur (dépassement de taille).
 * @details Cette fonction ajoute un espace avant et après chaque occurrence d'un caractère de *s* dans *str*.
 *    Si l'ajout d'espaces dépasse la taille maximale *max*, la fonction retourne -1.
 */
int separate_s(char *str, char *s, size_t max)
{
    if (str == NULL || s == NULL)
        return -1;

    int len = strlen(str);
    int nb_s = 0;

    // compte le nombre de separate à effectuer
    for (int i = 0; i < len; i++)
    {
        if (strchr(s, str[i]) != NULL)
            nb_s++;
    }

    if (nb_s == 0) // rien à faire
        return 0;

    size_t new_len = len + (nb_s * 2);

    if (new_len + 1 > max) // dépassement de la taille maximale
        return -1;

    // remplissage de droite à gauche
    int read_i = len - 1;
    int write_i = new_len - 1;

    str[new_len] = '\0';

    while (read_i >= 0)
    {
        char current_char = str[read_i];

        // si le char courant est un séparateur
        if (strchr(s, current_char) != NULL)
        {
            str[write_i--] = ' ';
            str[write_i--] = current_char;
            str[write_i--] = ' ';
        }
        else
            str[write_i--] = current_char;

        read_i--;
    }

    return 0;
}

/** @brief Fonction de remplacement de toutes les occurrences de la sous-chaîne *s* par la sous-chaîne *t* dans la chaîne *str*.
 * @param str Chaîne de caractères à traiter.
 * @param s Sous-chaîne à remplacer.
 * @param t Sous-chaîne de remplacement.
 * @param max Taille maximale de la chaîne *str*.
 * @return int 0 en cas de succès, -1 en cas d'erreur (dépassement de taille).
 * @details Cette fonction remplace toutes les occurrences de la sous-chaîne *s* par la sous-chaîne *t* dans la chaîne *str*.
 *    Si le remplacement dépasse la taille maximale *max*, la fonction retourne -1.
 */
int replace(char *str, const char *s, const char *t, size_t max)
{

    char *pos = strstr(str, s);
    if (!pos)
    {
        // printf("%s n'est pas inclus dans %s\n", s, str);
        return -1;
    }
    char *src = pos + strlen(s);
    char *dst = pos + strlen(t);
    int nb = strlen(src) + 1;                              // taille de la partie à déplacer
    size_t new_size = strlen(str) - strlen(s) + strlen(t); // taille finale théorique
    if (new_size > max)
    {
        nb = nb - (new_size - max);
    }

    // printf("pos: '%s', src: '%s', dest: '%s'\n", pos, src, dst);
    int nb_cp = strlen(t);
    if (pos + nb_cp + 1 > str + max)
    {
        int diff = (pos + nb_cp + 1 - (str + max));
        if (diff < 0)
            diff = 0;
        nb_cp -= diff;
    }
    if (dst < str + max && src < str + max)
        memmove(dst, src, nb);
    // printf("Après memmove: '%s'\n", str);
    memcpy(pos, t, nb_cp);
    return 0;
}

/** @brief Fonction de substitution des variables d'environnement dans une chaîne de caractères.
 * @param str Chaîne de caractères à traiter.
 * @param max Taille maximale de la chaîne *str*.
 * @return int 0 en cas de succès, -1 en cas d'erreur (dépassement de taille).
 * @details Cette fonction remplace toutes les occurrences de variables d'environnement au format $VAR ou ${VAR} par leur valeur dans la chaîne *str*.
 *    Si une variable n'existe pas, elle est remplacée par une chaîne vide.
 *    Si le remplacement dépasse la taille maximale *max*, la fonction retourne -1.
 */
int substenv(char *str, size_t max)
{
    if (str == NULL || max == 0)
        return -1;

    // buffer temporaire pour éviter les écrasements pendant la construction
    char *res = (char *)malloc(max);
    if (res == NULL)
        return -1;

    unsigned int r = 0; // index de lecture dans str
    unsigned int w = 0; // index d'écriture dans res

    while (str[r] != '\0')
    {
        if (str[r] == '$')
        {
            int var_start = r + 1;
            int var_end = var_start;
            int is_bracket = 0;

            // si ${VAR}
            if (str[var_start] == '{')
            {
                is_bracket = 1;
                var_start++;
                var_end = var_start;

                // cherche la fin de l'accolade
                while (str[var_end] != '\0' && str[var_end] != '}')
                    var_end++;

                if (str[var_end] != '}') // on annule si pas d'accolade fermante
                {
                    if (w + 1 >= max)
                    {
                        free(res);
                        return -1;
                    }
                    res[w++] = str[r++];
                    continue;
                }
            }
            else // si $VAR
            {    // on garde alphanumérique et underscore
                while (isalnum(str[var_end]) || str[var_end] == '_')
                    var_end++;
            }

            int var_len = var_end - var_start;

            // $ suivi d'un char non valide
            if (var_len == 0)
            {
                if (w + 1 >= max)
                {
                    free(res);
                    return -1;
                }
                res[w++] = str[r++];
                // si ${} vide
                if (is_bracket && str[var_end] == '}')
                    r = var_end + 1;

                continue;
            }

            // récupère le nom de la variable
            char var_name[256];
            memcpy(var_name, str + var_start, var_len);
            var_name[var_len] = '\0';

            char *env_val = getenv(var_name);

            // si la variable existe
            if (env_val != NULL)
            {
                int val_len = strlen(env_val);
                if (w + val_len >= max)
                {
                    free(res);
                    return -1;
                }
                strcpy(res + w, env_val);
                w += val_len;
            }
            // si la var existe pas, on fait rien

            r = (is_bracket) ? var_end + 1 : var_end;
        }
        else // char normal
        {
            if (w + 1 >= max) // dépassement
            {
                free(res);
                return -1;
            }
            res[w++] = str[r++];
        }
    }

    res[w] = '\0';

    strcpy(str, res);
    free(res);

    return 0;
}

/** @brief Fonction de découpage d'une chaîne de caractères en tokens selon un séparateur.
 * @param str Chaîne de caractères à découper. Attention, cette chaîne est modifiée par la fonction.
 * @param sep Caractère séparateur.
 * @param tokens Tableau de chaînes de caractères pour stocker les tokens extraits. Le tableau est terminé par un pointeur NULL.
 * @param max Taille maximale du tableau *tokens*, NULL compris.
 * @return int Nombre de tokens extraits, -1 en cas d'erreur (dépassement de taille).
 * @details Cette fonction découpe la chaîne *str* en tokens en utilisant le caractère *sep* comme séparateur.
 *    Les tokens extraits sont stockés dans le tableau *tokens*.
 *    Si le nombre de tokens dépasse la taille maximale *max*, la fonction retourne -1.
 */
int strcut(char *str, char sep, char **tokens, size_t max)
{
    if (max == 0)
        return -1;

    const char separator[] = {sep, '\0'};
    unsigned int nb_tokens = 0;

    char *token = strtok(str, separator);
    while (token != NULL)
    {
        if (nb_tokens >= max - 1) // dépassement de taille
        {
            tokens[max - 1] = NULL;
            return -1;
        }
        tokens[nb_tokens] = token;
        nb_tokens++;
        token = strtok(NULL, separator);
    }

    tokens[nb_tokens] = NULL;
    return nb_tokens;
}

/** @brief Fonction d'analyse d'une ligne de commande.
 * @param cmdl Pointeur vers la structure de ligne de commande à remplir.
 * @param line Chaîne de caractères contenant la ligne de commande à analyser.
 * @return int 0 en cas de succès, -1 en cas d'erreur (ligne trop longue, trop de commandes, etc.).
 * @details Cette fonction analyse la ligne de commande *line* et remplit la structure *cmdl* avec les informations extraites.
 *    La ligne de commande est copiée dans *cmdl->command_line* dans la limite de MAX_CMD_LINE caractères.
 *    La ligne est ensuite nettoyée (trim, clean, separate_s, replace, substenv), puis découpée en tokens.
 *    Les tokens sont ensuite utilisés pour remplir les structures processus_t et control_flow_t dans *cmdl*.
 *    Si la ligne dépasse la taille maximale ou si le nombre de commandes dépasse MAX_CMDS, la fonction retourne -1.
 *    Si une erreur est détectée, les descripteurs de fichiers ouverts sont fermés via close_fds(cmdl) avant de retourner -1.
 */
int parse_command_line(command_line_t *cmdl, const char *line)
{
    // Copie de la ligne de commande dans la structure
    // ces 2 lignes peuvent causer un bug selon le prof ?
    strncpy(cmdl->command_line, line, MAX_CMD_LINE - 1);
    cmdl->command_line[MAX_CMD_LINE - 1] = '\0';

    // Suppression des espaces inutiles au début et à la fin
    if (trim(cmdl->command_line) != 0)
    {
        return -1;
    }
    // Suppression des doublons d'espaces
    if (clean(cmdl->command_line) != 0)
    {
        return -1;
    }
    // Ajout d'espaces autour des caractères ;
    if (separate_s(cmdl->command_line, ";", MAX_CMD_LINE) != 0)
    {
        return -1;
    }
    // Traitement des variables d'environnement
    if (substenv(cmdl->command_line, MAX_CMD_LINE) != 0)
    {
        return -1;
    }
    // Découpage de la ligne en tokens
    int num_tokens = strcut(cmdl->command_line, ' ', cmdl->tokens, MAX_CMD_LINE / 2 + 1);
    if (num_tokens < 0)
    {
        return -1;
    }

    // Index des tokens
    int token_index = 0;
    // Index des arguments dans le processus courant
    int argv_index = 0;
    // Premier processus de la ligne de commande
    processus_t *current_proc = add_processus(cmdl, UNCONDITIONAL);

    while (cmdl->tokens[token_index] != NULL)
    {
        // TODO : vérifier que le nombre de processus ne dépasse pas MAX_CMDS
        //        Que le nombre d'arguments ne dépasse pas MAX_ARGS
        //        ...
        char *token = cmdl->tokens[token_index];
        if (strcmp(token, ";") == 0)
        {
            // Fin d'une commande.
            // Si le ';' est le dernier token, on peut arrêter le parsing
            if (cmdl->tokens[token_index + 1] == NULL)
            {
                break;
            }
            // Sinon, on passe au processus suivant
            current_proc = add_processus(cmdl, UNCONDITIONAL);
            // On réinitialise l'index des arguments
            argv_index = 0;
            // On passe au token suivant
            token_index++;
            continue;
        }
        if (strcmp(token, "<") == 0)
        {
            // Redirection de l'entrée standard
            // Le token suivant doit être le fichier
            token_index++;
            if (cmdl->tokens[token_index] == NULL)
            {
                fprintf(stderr, "Erreur de syntaxe: fichier attendu après '<'\n");
                close_fds(cmdl);
                return -1;
            }
            // Ouvrir le fichier en lecture
            int fd = open(cmdl->tokens[token_index], O_RDONLY);
            if (fd < 0)
            {
                perror("open");
                close_fds(cmdl);
                return -1;
            }
            current_proc->stdin_fd = fd;
            // Ajouter le descripteur à la liste des descripteurs ouverts
            if (add_fd(cmdl, fd) != 0)
            {
                close(fd);
                close_fds(cmdl);
                return -1;
            }
            // On passe au token suivant
            token_index++;
            continue;
        }
        // Traitement des autres opérateurs du shell (>, >>, 2>, |, &&, ||, ...)
        if (strcmp(token, ">") == 0)
        {
            // Ouvrir le fichier en écriture (O_WRONLY | O_CREAT | O_TRUNC)
            // Ajouter le descripteur à la liste des descripteurs ouverts
            // Affecter le descripteur au stdout_fd du processus courant
            token_index++;
            if (!cmdl->tokens[token_index])
            {
                fprintf(stderr, "Erreur de syntaxe: fichier attendu après '>'\n");
                close_fds(cmdl);
                return -1;
            }
            int fd = open(cmdl->tokens[token_index], O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0)
            {
                perror("open");
                close_fds(cmdl);
                return -1;
            }
            current_proc->stdout_fd = fd;
            if (add_fd(cmdl, fd) != 0)
            {
                close(fd);
                close_fds(cmdl);
                return -1;
            }
            token_index++;
            continue;
        }

        if (strcmp(token, ">>") == 0)
        {
            // Ouvrir le fichier en écriture (O_WRONLY | O_CREAT | O_APPEND)
            // Ajouter le descripteur à la liste des descripteurs ouverts
            // Affecter le descripteur au stdout_fd du processus courant
            token_index++;
            if (!cmdl->tokens[token_index])
            {
                fprintf(stderr, "Erreur de syntaxe: fichier attendu après '>>'\n");
                close_fds(cmdl);
                return -1;
            }
            int fd = open(cmdl->tokens[token_index], O_WRONLY | O_CREAT | O_APPEND, 0644);
            if (fd < 0)
            {
                perror("open");
                close_fds(cmdl);
                return -1;
            }
            current_proc->stdout_fd = fd;
            if (add_fd(cmdl, fd) != 0)
            {
                close(fd);
                close_fds(cmdl);
                return -1;
            }
            token_index++;
            continue;
        }

        if (strcmp(token, "2>") == 0)
        {
            // Reprendre le même traitement que pour ">", mais pour stderr_fd
            token_index++;
            if (!cmdl->tokens[token_index])
            {
                fprintf(stderr, "Erreur de syntaxe: fichier attendu après '2>'\n");
                close_fds(cmdl);
                return -1;
            }
            int fd = open(cmdl->tokens[token_index], O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0)
            {
                perror("open");
                close_fds(cmdl);
                return -1;
            }
            current_proc->stderr_fd = fd;
            if (add_fd(cmdl, fd) != 0)
            {
                close(fd);
                close_fds(cmdl);
                return -1;
            }
            token_index++;
            continue;
        }

        if (strcmp(token, "2>>") == 0)
        {
            // Reprendre le même traitement que pour ">>", mais pour stderr_fd
            token_index++;
            if (!cmdl->tokens[token_index])
            {
                fprintf(stderr, "Erreur de syntaxe: fichier attendu après '2>>'\n");
                close_fds(cmdl);
                return -1;
            }
            int fd = open(cmdl->tokens[token_index], O_WRONLY | O_CREAT | O_APPEND, 0644);
            if (fd < 0)
            {
                perror("open");
                close_fds(cmdl);
                return -1;
            }
            current_proc->stderr_fd = fd;
            if (add_fd(cmdl, fd) != 0)
            {
                close(fd);
                close_fds(cmdl);
                return -1;
            }
            token_index++;
            continue;
        }

        if (strcmp(token, "|") == 0)
        {
            // Pour la gestion du pipe, vous pourrez utiliser next_processus(cmdl) pour initialiser les descripteurs
            // des IOs standards de la structure processus_t courante et de la suivante.
            // next_processus(cmdl) retourne un pointeur vers le processus qui sera renvoyé par add_processus(cmdl, mode)
            // lors du prochain appel.
        }

        if (strcmp(token, "&&") == 0)
        {
            // Même traitement que pour ";", mais avec le mode ON_SUCCESS
        }

        if (strcmp(token, "||") == 0)
        {
            // Même traitement que pour ";", mais avec le mode ON_FAILURE
        }

        if (strcmp(token, "&") == 0)
        {
            // Mettre le flag is_background du processus courant à 1

            current_proc->is_background = 1;
            token_index++;
            continue;
        }

        if (strcmp(token, "!") == 0)
        {
            // Mettre le flag invert du processus courant à 1
            current_proc->invert = 1;
            token_index++;
            continue;
        }

        // Le token n'est pas un opérateur, c'est une commande ou un argument
        if (argv_index >= MAX_ARGS - 1)
        {
            fprintf(stderr, "Erreur: trop d'arguments pour une commande (max %d)\n", MAX_ARGS - 1);
            close_fds(cmdl);
            return -1;
        }
        // argv_index == 0 => C'est la commande
        if (argv_index == 0)
        {
            current_proc->path = token;
        }
        current_proc->argv[argv_index++] = token;
        // On passe au token suivant
        token_index++;
    }
    // On a traité tous les tokens.
    // À ce moment, la structure cmdl contient toutes les informations nécessaires
    // pour exécuter la ligne de commande avec le controle de flux associé.
    return 0;
}
