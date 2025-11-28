/** @file main.c
 * @brief Programme principal du shell
 * @author Sofiane FETTAH
 * @author Matthieu COMME
 * @date 2025-26
 * @details Ce fichier contient la fonction main de l'exécutable. Elle gère la boucle principale du shell,
 *   incluant l'affichage du prompt, la lecture de la ligne de commande, le parsing et l'exécution.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"
#include "processus.h"
#include "builtins.h"

/** @brief Affiche le prompt du shell.
 * @details Affiche le prompt "$ " et force l'affichage immédiat avec fflush.
 *   Cette fonction peut être modifiée pour afficher des informations supplémentaires (CWD, utilisateur, etc.).
 */
void prompt()
{
    printf("$ ");
    fflush(stdout);
}

/** @brief Fonction principale du shell.
 * @param argc Nombre d'arguments (non utilisé).
 * @param argv Tableau des arguments (non utilisé).
 * @return int Code de retour du programme. Ce code pourrait être le code de retour du dernier processus exécuté (optionnel).
 * @details Cette fonction gère la boucle principale du shell:
 * - Affiche le prompt
 * - Lit la ligne de commande
 * - Parse la ligne de commande
 * - Exécute les commandes
 * En cas d'erreur lors de l'exécution, un message est affiché sur stderr et la boucle continue.
 * Le shell se termine proprement en cas d'EOF (Ctrl+D) ou d'erreur fatale.
 */
int main(int argc, char *argv[])
{
    (void)argc; // Pour éviter les warnings inutilisés
    (void)argv; // Pour éviter les warnings inutilisés
    // Initialisation des structures nécessaires
    command_line_t cmdl;

    // Boucle principale du shell
    while (1)
    {
        // Initialisation de la structure de ligne de commande
        // On s'assure ici que tous les champs sont remis à zéro ou à leur valeur par défaut
        init_command_line(&cmdl);
        prompt();

        // Lecture de la ligne de commande
        if (fgets(cmdl.command_line, sizeof(cmdl.command_line), stdin) == NULL)
        {
            // EOF ou erreur de lecture (provoqué par exemple par Ctrl+D)
            processus_t exit_cmd;
            init_processus(&exit_cmd);
            exit_cmd.argv[0] = "exit";
            builtin_exit(&exit_cmd);
        }
        // Suppression du saut de ligne final conservé par fgets
        if (strlen(cmdl.command_line) > 0 && cmdl.command_line[strlen(cmdl.command_line) - 1] == '\n')
        {
            cmdl.command_line[strlen(cmdl.command_line) - 1] = '\0';
        }

        // La ligne de commande est vide, on passe à la suivante
        if (strlen(cmdl.command_line) == 0)
        {
            continue;
        }

        // Parsing de la ligne de commande
        if (parse_command_line(&cmdl, cmdl.command_line) != 0)
        {
            fprintf(stderr, "Erreur lors de l'analyse de la ligne de commandes.\n");
            continue;
        }

        // Traitement de la ligne de commande
        if (launch_command_line(&cmdl) != 0)
        {
            fprintf(stderr, "Erreur à l'exécution de la ligne de commandes.\n");
            continue;
        }
    }

    return 0;
}
