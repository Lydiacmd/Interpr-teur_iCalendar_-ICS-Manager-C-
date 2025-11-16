#include "analyse_syntaxique.h"
#include "analyse_lexical.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
   // Vérifier les arguments
   if (argc != 2)
   {
      printf("Usage: %s <fichier_commandes>\n", argv[0]);
      return 1;
   }

   // Ouvrir le fichier d'entrée
   FILE *fichier_entre = fopen(argv[1], "r");
   if (fichier_entre == NULL)
   {
      printf("Impossible d'ouvrir le fichier %s\n", argv[1]);
      return 1;
   }

   // Lire le contenu du fichier
   fseek(fichier_entre, 0, SEEK_END);
   long taille = ftell(fichier_entre);
   rewind(fichier_entre);

   char *buffer = malloc(taille + 1);
   if (buffer == NULL)
   {
      printf("Erreur d'allocation mémoire\n");
      fclose(fichier_entre);
      return 1;
   }

   size_t lu = fread(buffer, 1, taille, fichier_entre);
   buffer[lu] = '\0';
   fclose(fichier_entre);

   // Afficher l'analyse syntaxique
   printf("Analyse syntaxique du fichier %s:\n", argv[1]);
   printf("=====================================\n");
   printf("Contenu à analyser: %s\n\n", buffer);

   // Lire le fichier ligne par ligne
   char *ligne = strtok(buffer, "\n");
   int numero_ligne = 0;

   while (ligne != NULL)
   {
      numero_ligne++;

      // Ignorer les lignes vides ou commençant par # (commentaires)
      if (ligne[0] == '\0' || ligne[0] == '#')
      {
         ligne = strtok(NULL, "\n");
         continue;
      }

      printf("Analyse de la ligne %d: %s\n", numero_ligne, ligne);

      // Initialiser le lexer et parser pour cette ligne
      Lexer lexer;
      lexer_init(&lexer, ligne);

      // Analyse syntaxique
      ASTNode *root = parse(&lexer);

      // Afficher l'arbre syntaxique
      if (root != NULL)
      {
         printf("\nArbre syntaxique:\n");
         print_ast(root, 0);

         if (root->type == NODE_ERROR)
         {
            printf("\nErreur d'analyse syntaxique: %s\n", root->data.error_msg);
         }
         else
         {
            printf("\nAnalyse syntaxique réussie.\n");
         }

         // Libérer la mémoire de l'arbre
         free_ast(root);
      }
      else
      {
         printf("Erreur d'analyse\n");
      }

      printf("\n--------------------------------------------------\n");

      // Passer à la ligne suivante
      ligne = strtok(NULL, "\n");
   }

   // Libérer la mémoire
   free(buffer);

   return 0;
}