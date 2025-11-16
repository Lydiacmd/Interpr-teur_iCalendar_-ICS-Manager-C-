#ifndef ANALYSE_SYNTAXIQUE_H
#define ANALYSE_SYNTAXIQUE_H

#include "analyse_lexical.h"

// Type de noeuds de l'arbre syntaxique
typedef enum
{
   CMD,
   PARAMETR,
   VALUE,
   NODE_ERROR
} NodeType;

// Type cmd
typedef enum
{
   CMD_Add,
   CMD_Remove,
   CMD_Modify,
   CMD_List,
   CMD_Search,
   CMD_Clear,
   CMD_invalid
} CmdType;

typedef enum
{
   P_title,
   P_description,
   P_location,
   P_Date,
   P_Time,
   P_duration,
   P_on,
   P_from,
   P_to,
   P_all,
   P_invalid
} Parametre_type;

// Struct val
typedef struct
{
   union
   {
      char strig[256];
      char date[11];
      char time[6];
      int number;
   } data;
   TokenType type; // {string, date_value, time_value, Number}
} Value;

// Struct parametre
typedef struct parameter
{
   Parametre_type type;
   Value value;
   struct parameter *next; // liste chainée parametres
} Parameter;

// Struct cmd
typedef struct
{
   CmdType type;
   Parameter *parameters;
} Cmd;

// noeud AST
typedef struct
{
   NodeType type;
   union
   {
      Cmd cmd;
      Parameter parametr;
      Value value;
      char error_msg[256];
   } data;
} ASTNode;

typedef struct
{
   Lexer *lexer;
   Token current_token;
   int error;
   char error_msg[256];
} Parser;

// Initialise le parser avec un lexer
void parser_init(Parser *parser, Lexer *lexer);

// Prends le token courant et avance au suivant
void parser_take(Parser *parser);

// Verifie si le token est de type attendu
int parser_check(Parser *parser, TokenType type);

// Vérifie et consomme le token si son type correspond, sinon génère une erreur
int parser_expect(Parser *parser, TokenType type, const char *error_msg);

// Génère une erreur syntaxique
int parser_error(Parser *parser, const char *error_msg);

// Conversion de TokenType vers CmdType
CmdType token_to_cmdtype(TokenType type);

// Conversion de TokenType vers Parametre_type
Parametre_type token_to_parameter_type(TokenType type);

// Création d'un nouveau paramètre
Parameter *create_parameter(Parametre_type type);

// Ajoute un paramètre à la liste chaînée
void add_parameter(Parameter **head, Parameter *param);

// Analyse une commande complète
ASTNode *parser_cmd(Parser *parser);

// Analyse les paramètres d'un événement (pour ADD, REMOVE, MODIFY)
Parameter *parse_event_parameters(Parser *parser);

// Analyse les filtres (pour LIST)
Parameter *parse_filters(Parser *parser);

// Analyse la requête de recherche (pour SEARCH)
Parameter *parse_search_query(Parser *parser);

// Fonction principale d'analyse
ASTNode *parse(Lexer *lexer);

// Affiche l'arbre syntaxique sous forme d'arborescence
void print_ast(ASTNode *node, int depth);

// Libère la mémoire occupée par l'arbre syntaxique
void free_ast(ASTNode *node);

#endif /* ANALYSE_SYNTAXIQUE_H */