
/**
 * lexer.h
 * Analyseur lexical pour l'interpréteur de commandes vers iCalendar
 */

#ifndef ANALYSE_LEXICAL_H
#define ANALYSE_LEXICAL_H

#include <stdio.h>

    /**
     * Types reconnus par l'analyseur lexical
     */

typedef enum {
        // Mot clés de commande
        ADD,
        REMOVE,
        MODIFY,
        LIST,
        SEARCH,
        CLEAR,

        // Mot clés de parametres
        AT,
        ON,
        TO,
        FROM,
        EVENT,
        DESCRIPTION,
        LOCATION,
        TITLE,
        TIME,
        DATE,
        DURATION,
        ALL,

        // Mot clés de valeurs
        STRING,     // chaine entre "..."
        DATE_VALUE, // format jj/mm/aaaa
        TIME_VALUE, // format hh/mm
        NUMBER,     // entier

        TOKEN_EOF, // fin fichier
        ERROR      // erreur lexical
    } TokenType;

typedef struct
{
    TokenType type;
    char lexeme[256];
    int line;
} Token;

typedef struct
{
    FILE *source;       // Fichier source
    char current;       // Caractère courant
    int line;           // Ligne courante
    int column;         // Colonne courante
    int hasError;       // Indicateur d'erreur
    char errorMsg[256]; // Message d'erreur
} Lexer;

/**
 * Initialise le lexer avec un fichier source
 */
void lexer_init(Lexer *lexer, const char *input);

/**
 * Avance d'un caractère dans le fichier source
 */
void lexer_advance(Lexer *lexer);

/**
 * Regarde le caractère suivant sans avancer
 */
char lexer_peek(Lexer *lexer);

/**
 * Vérifie si le caractère courant est un chiffre
 */
int lexer_is_digit(char c);

/**
 * Vérifie si le caractère courant est une lettre ou un underscore
 */
int lexer_is_alpha(char c);

/**
 * Vérifie si le caractère courant est alphanumérique ou un underscore
 */
int lexer_is_alphanumeric(char c);

/**
 * Vérifie si la chaîne correspond à un mot-clé et retourne le type de token correspondant
 */
TokenType lexer_check_keyword(const char *word);

/**
 * Récupère le prochain token du fichier source
 */
Token lexer_get_next(Lexer *lexer);

/**
 * Traite une chaîne de caractères entre guillemets
 */
Token lexer_string(Lexer *lexer);

/**
 * Traite un identifiant ou un mot-clé
 */
Token lexer_identifier(Lexer *lexer);

/**
 * Traite un nombre
 */
Token lexer_number(Lexer *lexer);

/**
 * Traite une date au format JJ/MM/AAAA
 */
Token lexer_date(Lexer *lexer);

/**
 * Traite une heure au format HH:MM
 */
Token lexer_time(Lexer *lexer);

/**
 * Définit une erreur dans le lexer
 */
void lexer_error(Lexer *lexer, const char *message);

/**
 * Affiche les informations d'un token (pour le débogage)
 */
void token_print(Token token);

#endif /* LEXER_H */
