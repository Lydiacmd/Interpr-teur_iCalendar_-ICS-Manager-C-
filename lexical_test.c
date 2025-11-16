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

   // Afficher le contenu à analyser
   printf("Analyse lexicale du fichier %s:\n", argv[1]);
   printf("=====================================\n");
   printf("Contenu à analyser: %s\n\n", buffer);

   // Initialiser le lexer
   Lexer lexer;
   lexer_init(&lexer, buffer);

   // Analyser et afficher tous les tokens
   printf("Tokens identifiés:\n");
   printf("------------------\n");

   Token token;
   int token_count = 0;

   do
   {
      token = lexer_get_next(&lexer);
      printf("Token #%d: ", ++token_count);
      token_print(token);
   } while (token.type != TOKEN_EOF && token.type != ERROR);

   // Afficher le résultat de l'analyse
   if (lexer.hasError)
   {
      printf("\nErreur d'analyse lexicale: %s\n", lexer.errorMsg);
   }
   else
   {
      printf("\nAnalyse lexicale réussie. %d tokens identifiés.\n", token_count);
   }

   // Libérer la mémoire
   free(buffer);

   return lexer.hasError ? 1 : 0;
}