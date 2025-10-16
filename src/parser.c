/** @file parser.c
 * @brief Implementation of command line parsing
 * @author Nom1
 * @author Nom2
 * @date 2025-26
 * @details Implémentation des fonctions d'analyse des lignes de commande.
 */

#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <fcntl.h>

#include "parser.h"
#include "processus.h"

/** @brief Fonction de suppression des espaces inutiles au début et à la fin d'une chaîne de caractères.
 * @param str Chaîne de caractères à traiter.
 * @return int 0 en cas de succès, -1 en cas d'erreur.
 */
int trim(char* str) {

}

/** @brief Fonction de nettoyage d'une chaîne de caractères en supprimant les doublons d'espaces.
 * @param str Chaîne de caractères à nettoyer.
 * @return int 0 en cas de succès, -1 en cas d'erreur.
 */
int clean(char* str) {

}

/** @brief Fonction d'ajout de caractères d'espacement autour de tous les caractères de la chaîne *s* présents dans *str*.
 * @param str Chaîne de caractères à traiter.
 * @param s Chaîne de caractères contenant les séparateurs.
 * @param max Taille maximale de la chaîne *str*.
 * @return int 0 en cas de succès, -1 en cas d'erreur (dépassement de taille).
 * @details Cette fonction ajoute un espace avant et après chaque occurrence d'un caractère de *s* dans *str*.
 *    Si l'ajout d'espaces dépasse la taille maximale *max*, la fonction retourne -1.
 */
int separate_s(char* str, char* s, size_t max) {

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
int replace(char* str, const char* s, const char* t, size_t max) {

}

/** @brief Fonction de substitution des variables d'environnement dans une chaîne de caractères.
 * @param str Chaîne de caractères à traiter.
 * @param max Taille maximale de la chaîne *str*.
 * @return int 0 en cas de succès, -1 en cas d'erreur (dépassement de taille).
 * @details Cette fonction remplace toutes les occurrences de variables d'environnement au format $VAR ou ${VAR} par leur valeur dans la chaîne *str*.
 *    Si une variable n'existe pas, elle est remplacée par une chaîne vide.
 *    Si le remplacement dépasse la taille maximale *max*, la fonction retourne -1.
 */
int substenv(char* str, size_t max) {

}

/** @brief Fonction de découpage d'une chaîne de caractères en tokens selon un séparateur.
 * @param str Chaîne de caractères à découper. Attention, cette chaîne est modifiée par la fonction.
 * @param sep Caractère séparateur.
 * @param tokens Tableau de chaînes de caractères pour stocker les tokens extraits. Le tableau est terminé par un pointeur NULL.
 * @param max Taille maximale du tableau *tokens*.
 * @return int Nombre de tokens extraits, -1 en cas d'erreur (dépassement de taille).
 * @details Cette fonction découpe la chaîne *str* en tokens en utilisant le caractère *sep* comme séparateur.
 *    Les tokens extraits sont stockés dans le tableau *tokens*.
 *    Si le nombre de tokens dépasse la taille maximale *max*, la fonction retourne -1.
 */
int strcut(char* str, char sep, char** tokens, size_t max) {

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
int parse_command_line(command_line_t* cmdl, const char* line) {
    // Copie de la ligne de commande dans la structure
    strncpy(cmdl->command_line, line, MAX_CMD_LINE - 1);
    cmdl->command_line[MAX_CMD_LINE - 1] = '\0';
    // Suppression des espaces inutiles au début et à la fin
    if (trim(cmdl->command_line) != 0) {
        return -1;
    }
    // Suppression des doublons d'espaces
    if (clean(cmdl->command_line) != 0) {
        return -1;
    }
    // Ajout d'espaces autour des caractères ;
    if (separate_s(cmdl->command_line, ";", MAX_CMD_LINE) != 0) {
        return -1;
    }
    // Traitement des variables d'environnement
    if (substenv(cmdl->command_line, MAX_CMD_LINE) != 0) {
        return -1;
    }
    // Découpage de la ligne en tokens
    int num_tokens = strcut(cmdl->command_line, ' ', cmdl->tokens , MAX_CMD_LINE / 2 + 1);
    if (num_tokens < 0) {
        return -1;
    }

    // Index des tokens
    int token_index = 0;
    // Index des arguments dans le processus courant
    int argv_index = 0;
    // Premier processus de la ligne de commande
    processus_t* current_proc = add_processus(cmdl, UNCONDITIONAL);

    while (cmdl->tokens[token_index] != NULL) {
        // TODO : vérifier que le nombre de processus ne dépasse pas MAX_CMDS
        //        Que le nombre d'arguments ne dépasse pas MAX_ARGS 
        //        ...
        char* token = cmdl->tokens[token_index];
        if (strcmp(token, ";") == 0) {
            // Fin d'une commande.
            // Si le ';' est le dernier token, on peut arrêter le parsing
            if (cmdl->tokens[token_index + 1] == NULL) {
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
        if (strcmp(token, "<") == 0) {
            // Redirection de l'entrée standard
            // Le token suivant doit être le fichier
            token_index++;
            if (cmdl->tokens[token_index] == NULL) {
                fprintf(stderr, "Erreur de syntaxe: fichier attendu après '<'\n");
                close_fds(cmdl);
                return -1;
            }
            // Ouvrir le fichier en lecture
            int fd = open(cmdl->tokens[token_index], O_RDONLY);
            if (fd < 0) {
                perror("open");
                close_fds(cmdl);
                return -1;
            }
            current_proc->stdin_fd = fd;
            // Ajouter le descripteur à la liste des descripteurs ouverts
            if (add_fd(cmdl, fd) != 0) {
                close(fd);
                close_fds(cmdl);
                return -1;
            }
            // On passe au token suivant
            token_index++;
            continue;
        }
        // Traitement des autres opérateurs du shell (>, >>, 2>, |, &&, ||, ...)
        if (strcmp(token, ">") == 0) {
            // Ouvrir le fichier en écriture (O_WRONLY | O_CREAT | O_TRUNC)
            // Ajouter le descripteur à la liste des descripteurs ouverts
            // Affecter le descripteur au stdout_fd du processus courant
        }

        if (strcmp(token, ">>") == 0) {
            // Ouvrir le fichier en écriture (O_WRONLY | O_CREAT | O_APPEND)
            // Ajouter le descripteur à la liste des descripteurs ouverts
            // Affecter le descripteur au stdout_fd du processus courant
        }

        if (strcmp(token, "2>") == 0) {
            // Reprendre le même traitement que pour ">", mais pour stderr_fd
        }

        if (strcmp(token, "2>>") == 0) {
            // Reprendre le même traitement que pour ">>", mais pour stderr_fd
        }

        if (strcmp(token, "|") == 0) {
            // Pour la gestion du pipe, vous pourrez utiliser next_processus(cmdl) pour initialiser les descripteurs
            // des IOs standards de la structure processus_t courante et de la suivante.
            // next_processus(cmdl) retourne un pointeur vers le processus qui sera renvoyé par add_processus(cmdl, mode)
            // lors du prochain appel.
        }

        if (strcmp(token, "&&") == 0) {
            // Même traitement que pour ";", mais avec le mode ON_SUCCESS
        }

        if (strcmp(token, "||") == 0) {
            // Même traitement que pour ";", mais avec le mode ON_FAILURE
        }

        if (strcmp(token, "&") == 0) {
           // Mettre le flag is_background du processus courant à 1
        }

        if (strcmp(token, "!") == 0) {
            // Mettre le flag invert du processus courant à 1
        }

        // Le token n'est pas un opérateur, c'est une commande ou un argument
        if (argv_index >= MAX_ARGS - 1) {
            fprintf(stderr, "Erreur: trop d'arguments pour une commande (max %d)\n", MAX_ARGS - 1);
            close_fds(cmdl);
            return -1;
        }
        // argv_index == 0 => C'est la commande
        if (argv_index == 0) {
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
