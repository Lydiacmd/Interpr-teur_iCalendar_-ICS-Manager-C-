#include "analyse_lexical.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

    // Initialise le lexer avec un fichier source
    void lexer_init(Lexer *lexer, const char *input)
    {
       // Créer un fichier temporaire en mémoire
       lexer->source = tmpfile();
       if (lexer->source == NULL)
       {
          fprintf(stderr, "Erreur: Impossible de créer un fichier temporaire\n");
          exit(EXIT_FAILURE);
       }

       // Écrire la chaîne d'entrée dans le fichier temporaire
       fputs(input, lexer->source);

       // Remettre le curseur au début du fichier
       rewind(lexer->source);

       // Initialiser les autres champs
       lexer->line = 1;
       lexer->hasError = 0;
       lexer->errorMsg[0] = '\0';
       lexer->column = 0;
       lexer_advance(lexer); // Lire le premier caractère
    }

// Avance d'un caractère dans le fichier source
void lexer_advance(Lexer *lexer)
{
   int c = fgetc(lexer->source);
   if (c == EOF)
   {
      lexer->current = '\0'; // fin fichier
   }
   else
   {
      lexer->current = c;
      lexer->column++;
      if (c == '\n')
      { // fin ligne
         lexer->line++;
         lexer->column = 0;
      }
   }
}

// Regarde le caractère suivant sans avancer
char lexer_peek(Lexer *lexer)
{
   int c = fgetc(lexer->source);
   if (c == EOF)
   {
      return '\0';
   }
   ungetc(c, lexer->source);
   return (char)c;
}

// Vérifie si le caractère courant est un chiffre
int lexer_is_digit(char c)
{
   return c >= '0' && c <= '9';
}

// Vérifie si le caractère courant est une lettre ou un underscore
int lexer_is_alpha(char c)
{
   return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

// Vérifie si le caractère courant est alphanumérique ou un underscore
int lexer_is_alphanumeric(char c)
{
   return lexer_is_alpha(c) || lexer_is_digit(c);
}

//  Vérifie si la chaîne correspond à un mot-clé et retourne le type de token correspondant
TokenType lexer_check_keyword(const char *word)
{
   if (strcmp(word, "ADD") == 0)
      return ADD;
   if (strcmp(word, "REMOVE") == 0)
      return REMOVE;
   if (strcmp(word, "MODIFY") == 0)
      return MODIFY;
   if (strcmp(word, "LIST") == 0)
      return LIST;
   if (strcmp(word, "SEARCH") == 0)
      return SEARCH;
   if (strcmp(word, "CLEAR") == 0)
      return CLEAR;

   if (strcmp(word, "AT") == 0)
      return AT;
   if (strcmp(word, "ON") == 0)
      return ON;
   if (strcmp(word, "TO") == 0)
      return TO;
   if (strcmp(word, "FROM") == 0)
      return FROM;
   if (strcmp(word, "EVENT") == 0)
      return EVENT;
   if (strcmp(word, "DESCRIPTION") == 0)
      return DESCRIPTION;
   if (strcmp(word, "LOCATION") == 0)
      return LOCATION;
   if (strcmp(word, "TITLE") == 0)
      return TITLE;
   if (strcmp(word, "TIME") == 0)
      return TIME;
   if (strcmp(word, "DATE") == 0)
      return DATE;
   if (strcmp(word, "DURATION") == 0)
      return DURATION;
   if (strcmp(word, "ALL") == 0)
      return ALL;

   // Si ce n'est pas un mot-clé connu
   return ERROR;
}

// Affiche un message d'erreur avec l'endroit de l'erreur
void lexer_error(Lexer *lexer, const char *message)
{
   lexer->hasError = 1;
   snprintf(lexer->errorMsg, sizeof(lexer->errorMsg),
            "Erreur lexicale ligne %d, colonne %d: %s",
            lexer->line, lexer->column, message);
}

// Traite une chaîne de caractères entre guillemets
Token lexer_string(Lexer *lexer)
{
   Token token;
   token.type = STRING;
   token.line = lexer->line;

   long unsigned int i = 0;
   lexer_advance(lexer); // lit (skip) guillement ouvrante

   while (lexer->current != '"' && lexer->current != '\0')
   {
      if (i < sizeof(token.lexeme) - 1)
      {
         token.lexeme[i] = lexer->current;
         i++;
      }
      lexer_advance(lexer);
   }

   if (lexer->current == '\0')
   {
      lexer_error(lexer, "Chaine non terminée");
      token.type = ERROR;
      strcpy(token.lexeme, "Chaine non terminée");
      return token;
   }

   lexer_advance(lexer); // lit (skip) guillement ouvrante
   token.lexeme[i] = '\0';

   return token;
}

// Traite une date au format jj/mm/aaaa
Token lexer_date(Lexer *lexer)
{
    Token token;
    token.type = DATE_VALUE;
    token.line = lexer->line;

    char dayStr[3] = "", monthStr[3] = "", yearStr[5] = "";
    int i = 0;

    // Les jours
    while (lexer_is_digit(lexer->current) && i < 2)
    {
        dayStr[i++] = lexer->current;
        lexer_advance(lexer);
    }
    dayStr[i] = '\0';

    if (lexer->current != '/')
    {
        lexer_error(lexer, "Format de date invalide, '/' attendu après le jour");
        token.type = ERROR;
        strcpy(token.lexeme, "Format de date invalide");
        return token;
    }
    lexer_advance(lexer);

    // Les mois
    i = 0;
    while (lexer_is_digit(lexer->current) && i < 2)
    {
        monthStr[i++] = lexer->current;
        lexer_advance(lexer);
    }
    monthStr[i] = '\0';

    if (lexer->current != '/')
    {
        lexer_error(lexer, "Format de date invalide, '/' attendu après le mois");
        token.type = ERROR;
        strcpy(token.lexeme, "Format de date invalide");
        return token;
    }
    lexer_advance(lexer);

    // Les années
    i = 0;
    while (lexer_is_digit(lexer->current) && i < 4)
    {
        yearStr[i++] = lexer->current;
        lexer_advance(lexer);
    }
    yearStr[i] = '\0';

    // Conversion
    int day = atoi(dayStr);
    int month = atoi(monthStr);
    int year = atoi(yearStr);

    // Validation
    int valid = 1;
    if (day < 1 || day > 31 || month < 1 || month > 12 || year <= 0)
    {
        valid = 0;
    }
    else
    {
        int daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
        // Bissextile
        if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0))
        {
            daysInMonth[1] = 29;
        }

        if (day > daysInMonth[month - 1])
        {
            valid = 0;
        }
    }

    if (!valid)
    {
        lexer_error(lexer, "Date invalide");
        token.type = ERROR;
        strcpy(token.lexeme, "Date invalide");
        return token;
    }

    // Reconstituer la date dans le champ lexeme
    snprintf(token.lexeme, sizeof(token.lexeme), "%02d/%02d/%04d", day, month, year);
    return token;
}
// Traite une heure au format hh/mm
Token lexer_time(Lexer *lexer)
{
   Token token;
   token.type = TIME_VALUE;
   token.line = lexer->line;

   int i = 0;

   // Les heures
   while (lexer_is_digit(lexer->current) && i < 2)
   {
      token.lexeme[i++] = lexer->current;
      lexer_advance(lexer);
   }

   // separateur
   if (lexer->current != ':')
   {
      lexer_error(lexer, "Format d'heure invalide, ':' attendu");
      token.type = ERROR;
      strcpy(token.lexeme, "Format d'heure invalide");
      return token;
   }
   token.lexeme[i++] = lexer->current;
   lexer_advance(lexer);

   // Les minutes
   while (lexer_is_digit(lexer->current) && i < 5)
   {
      token.lexeme[i++] = lexer->current;
      lexer_advance(lexer);
   }
   token.lexeme[i] = '\0';

   if (strlen(token.lexeme) != 5)
   {
      lexer_error(lexer, "Format d'heure invalide, HH:MM attendu");
      token.type = ERROR;
      strcpy(token.lexeme, "Format d'heure invalide");
   }

   return token;
}

Token lexer_number(Lexer *lexer)
{
   Token token;
   token.type = NUMBER;
   token.line = lexer->line;

   long unsigned int i = 0;
   while (lexer_is_digit(lexer->current))
   {
      if (i < sizeof(token.lexeme) - 1)
      {
         token.lexeme[i++] = lexer->current;
      }
      lexer_advance(lexer);
   }
   token.lexeme[i] = '\0';

   return token;
}

Token lexer_identifier(Lexer *lexer)
{
   Token token;
   token.line = lexer->line;

   long unsigned int i = 0;
   while (lexer_is_alphanumeric(lexer->current))
   {
      if (i < sizeof(token.lexeme) - 1)
      {
         token.lexeme[i++] = lexer->current;
      }
      lexer_advance(lexer);
   }
   token.lexeme[i] = '\0';

   // Vérifier si c'est un mot-clé
   TokenType type = lexer_check_keyword(token.lexeme);
   if (type == ERROR)
   {
      // Ce n'est pas un mot-clé connu, donc c'est une erreur
      lexer_error(lexer, "Identifiant non reconnu");
      token.type = ERROR;
   }
   else
   {
      token.type = type;
   }

   return token;
}

Token lexer_get_next(Lexer *lexer)
{
   Token token;

   // ignorer les espaces
   while (isspace(lexer->current))
   {
      lexer_advance(lexer);
   }

   // Vérifier la fin du fichier
   if (lexer->current == '\0')
   {
      token.type = TOKEN_EOF;
      token.lexeme[0] = '\0';
      token.line = lexer->line;
      return token;
   }

   if (lexer->current == '"')
   {
      return lexer_string(lexer);
   }

   // Si c'est un chiffre
   if (lexer_is_digit(lexer->current))
   {

      // Sauvegarde pour reconstruction potentielle
      int startDigit = lexer->current;
      long startPos = ftell(lexer->source);
      int startCol = lexer->column;

      // Lire le premier chiffre
      lexer_advance(lexer);

      // Si le deuxième caractère est un chiffre
      if (lexer_is_digit(lexer->current))
      {
         lexer_advance(lexer);

         // Vérifier si c'est une heure (HH:MM)
         if (lexer->current == ':')
         {
            // Revenir au début et utiliser lexer_time
            fseek(lexer->source, startPos, SEEK_SET);
            lexer->current = startDigit;
            lexer->column = startCol;
            return lexer_time(lexer);
         }
         // Vérifier si c'est une date (JJ/MM/AAAA)
         else if (lexer->current == '/')
         {
            // Revenir au début et utiliser lexer_date
            fseek(lexer->source, startPos, SEEK_SET);
            lexer->current = startDigit;
            lexer->column = startCol;
            return lexer_date(lexer);
         }
      }

      // Si on arrive ici, ce n'est ni une heure ni une date
      // Revenir au début et traiter comme un nombre
      fseek(lexer->source, startPos, SEEK_SET);
      lexer->current = startDigit;
      lexer->column = startCol;
      return lexer_number(lexer);
   }

   if (lexer_is_alpha(lexer->current))
   {
      return lexer_identifier(lexer);
   }

   token.type = ERROR;
   token.line = lexer->line;
   sprintf(token.lexeme, "Caractère non reconnu: %c", lexer->current);
   lexer_error(lexer, token.lexeme);
   lexer_advance(lexer);

   return token;
}

void token_print(Token token)
{
   const char *typeNames[] = {
       "ADD", "REMOVE", "MODIFY", "LIST", "SEARCH", "CLEAR",
       "AT", "ON", "TO", "FROM", "EVENT", "DESCRIPTION", "LOCATION", "TITLE", "TIME", "DATE", "DURATION", "ALL",
       "STRING", "DATE_VALUE", "TIME_VALUE", "NUMBER",
       "EOF", "ERROR"};

   printf("Token { type: %s, lexeme: '%s', line: %d }\n",
          typeNames[token.type], token.lexeme, token.line);
}
