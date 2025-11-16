// analyse_syntaxique.c

#include "analyse_syntaxique.h"
#include "analyse_lexical.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Fonctions d'initialisation et utilitaires

void parser_init(Parser *parser, Lexer *lexer) {
    parser->lexer = lexer;
    parser->error = 0;
    parser->error_msg[0] = '\0';
    parser->current_token = lexer_get_next(lexer);
}

void parser_take(Parser *parser) {
    parser->current_token = lexer_get_next(parser->lexer);
}

int parser_check(Parser *parser, TokenType type) {
    return parser->current_token.type == type;
}

CmdType token_to_cmdtype(TokenType type) {
    switch (type) {
        case ADD: return CMD_Add;
        case REMOVE: return CMD_Remove;
        case MODIFY: return CMD_Modify;
        case LIST: return CMD_List;
        case SEARCH: return CMD_Search;
        case CLEAR: return CMD_Clear;
        default: return CMD_invalid;
    }
}

Parametre_type token_to_parameter_type(TokenType type) {
    switch (type) {
        case TITLE: return P_title;
        case DESCRIPTION: return P_description;
        case LOCATION: return P_location;
        case DATE: return P_Date;
        case TIME: return P_Time;
        case DURATION: return P_duration;
        case ON: return P_on;
        case FROM: return P_from;
        case TO: return P_to;
        case ALL: return P_all;
        default: return P_invalid;
    }
}

int parser_expect(Parser *parser, TokenType type, const char *error_msg) {
    if (parser_check(parser, type)) {
        parser_take(parser);
        return 1;
    } else {
        parser_error(parser, error_msg);
        return 0;
    }
}

int parser_error(Parser *parser, const char *error_msg) {
    parser->error = 1;
    snprintf(parser->error_msg, sizeof(parser->error_msg),
             "Erreur syntaxique ligne %d: %.100s, trouv\'e '%.100s' ",
             parser->current_token.line, error_msg, parser->current_token.lexeme);
    return 0;
}

int parameter_exists(Parameter *head, Parametre_type type) {
    while (head != NULL) {
        if (head->type == type) return 1;
        head = head->next;
    }
    return 0;
}

Parameter *create_parameter(Parametre_type type) {
    Parameter *param = malloc(sizeof(Parameter));
    if (!param) {
        fprintf(stderr, "Erreur d'allocation mémoire\n");
        exit(EXIT_FAILURE);
    }
    param->type = type;
    param->next = NULL;
    return param;
}

void add_parameter(Parameter **head, Parameter *param) {
    if (*head == NULL) {
        *head = param;
    } else {
        Parameter *current = *head;
        while (current->next) current = current->next;
        current->next = param;
    }
}

// Analyseur principal

ASTNode *parser_cmd(Parser *parser) {
    ASTNode *node = malloc(sizeof(ASTNode));
    if (!node) {
        fprintf(stderr, "Erreur d'allocation mémoire\n");
        exit(EXIT_FAILURE);
    }
    node->type = CMD;
    node->data.cmd.parameters = NULL;

    CmdType cmdtype = token_to_cmdtype(parser->current_token.type);

    if (cmdtype != CMD_invalid) {
        node->data.cmd.type = cmdtype;
        parser_take(parser);

        switch (cmdtype) {
            case CMD_Add:
            case CMD_Remove:
            case CMD_Modify:
                node->data.cmd.parameters = parse_event_parameters(parser);
                break;
            case CMD_List:
                node->data.cmd.parameters = parse_filters(parser);
                break;
            case CMD_Search:
                node->data.cmd.parameters = parse_search_query(parser);
                break;
            case CMD_Clear:
                break;
            default:
                break;
        }
    } else {
        node->type = NODE_ERROR;
        snprintf(node->data.error_msg, sizeof(node->data.error_msg),
                 "Commande non reconnue: %.100s", parser->current_token.lexeme);
        parser->error = 1;
        strcpy(parser->error_msg, node->data.error_msg);
    }

    if (!parser->error && !parser_check(parser, TOKEN_EOF)) {
        parser_error(parser, "Tokens supplémentaires après la fin de la commande");
    }

    return node;
}

// Analyse des paramètres pour ADD / REMOVE / MODIFY

Parameter *parse_event_parameters(Parser *parser) {
    Parameter *head = NULL;
    Parameter *param = NULL;
    int date_required = 1;
    int title_found = 0;  // Nouvelle variable pour vérifier la présence du paramètre title

    while (!parser_check(parser, TOKEN_EOF) && !parser->error) {
        Parametre_type paramtype = token_to_parameter_type(parser->current_token.type);
        if (paramtype == P_invalid) break;

        if (parameter_exists(head, paramtype)) {
            parser_error(parser, "Paramètre dupliqué interdit");
            return head;
        }

        parser_take(parser);
        param = create_parameter(paramtype);

        switch (paramtype) {
            case P_title:
                title_found = 1;  // Si on trouve un titre, on le marque
                if (parser_check(parser, STRING)) {
                    param->value.type = STRING;
                    strcpy(param->value.data.strig, parser->current_token.lexeme);
                    parser_take(parser);
                    add_parameter(&head, param);
                } else {
                    parser_error(parser, "Attendu une chaîne pour le titre");
                    free(param);
                    return head;
                }
                break;
            case P_description:
            case P_location:
                if (parser_check(parser, STRING)) {
                    param->value.type = STRING;
                    strcpy(param->value.data.strig, parser->current_token.lexeme);
                    parser_take(parser);
                    add_parameter(&head, param);
                } else {
                    parser_error(parser, "Attendu une chaîne");
                    free(param);
                    return head;
                }
                break;
            case P_Date:
                date_required = 0;
                if (parser_check(parser, DATE_VALUE)) {
                    param->value.type = DATE_VALUE;
                    strcpy(param->value.data.date, parser->current_token.lexeme);
                    parser_take(parser);
                    add_parameter(&head, param);
                } else {
                    parser_error(parser, "Attendu une date");
                    free(param);
                    return head;
                }
                break;
            case P_Time:
                if (parser_check(parser, TIME_VALUE)) {
                    param->value.type = TIME_VALUE;
                    strcpy(param->value.data.time, parser->current_token.lexeme);
                    parser_take(parser);
                    add_parameter(&head, param);
                } else {
                    parser_error(parser, "Attendu une heure");
                    free(param);
                    return head;
                }
                break;
            case P_duration:
                if (parser_check(parser, NUMBER)) {
                    param->value.type = NUMBER;
                    param->value.data.number = atoi(parser->current_token.lexeme);
                    parser_take(parser);
                    add_parameter(&head, param);
                } else {
                    parser_error(parser, "Attendu un nombre");
                    free(param);
                    return head;
                }
                break;
            default:
                free(param);
                break;
        }
    }

    if (date_required) {
        parser_error(parser, "Une date est requise pour cette commande");
    }

    // Vérification si le titre est absent
    if (!title_found) {
        parser_error(parser, "Le paramètre '-title' est requis pour cette commande");
    }

    return head;
}


// Analyse des paramètres pour LIST

Parameter *parse_filters(Parser *parser) {
    Parameter *head = NULL;
    Parameter *param = NULL;

    while (!parser_check(parser, TOKEN_EOF) && !parser->error) {
        Parametre_type paramtype = token_to_parameter_type(parser->current_token.type);
        if (paramtype == P_invalid) break;

        if (parameter_exists(head, paramtype)) {
            parser_error(parser, "Paramètre dupliqué interdit");
            return head;
        }

        parser_take(parser);
        param = create_parameter(paramtype);

        switch (paramtype) {
            case P_all:
                add_parameter(&head, param);
                break;
            case P_on:
            case P_from:
            case P_to:
                if (parser_check(parser, DATE_VALUE)) {
                    param->value.type = DATE_VALUE;
                    strcpy(param->value.data.date, parser->current_token.lexeme);
                    parser_take(parser);
                    add_parameter(&head, param);
                } else {
                    parser_error(parser, "Attendu une date");
                    free(param);
                    return head;
                }
                break;
            default:
                free(param);
                break;
        }
    }

    if (head == NULL && !parser->error) {
        parser_error(parser, "La commande LIST nécessite au moins un filtre");
    }

    return head;
}

// Analyse de SEARCH

Parameter *parse_search_query(Parser *parser) {
    Parameter *head = NULL;
    Parameter *param = NULL;

    if (parser_check(parser, STRING)) {
        param = create_parameter(P_description);
        param->value.type = STRING;
        strcpy(param->value.data.strig, parser->current_token.lexeme);
        parser_take(parser);
        add_parameter(&head, param);
    } else {
        parser_error(parser, "Attendu une chaîne de recherche");
    }

    return head;
}

// Libération de l’arbre

void free_ast(ASTNode *node) {
    if (node == NULL) return;

    if (node->type == CMD && node->data.cmd.parameters != NULL) {
        Parameter *current = node->data.cmd.parameters;
        while (current != NULL) {
            Parameter *next = current->next;
            free(current);
            current = next;
        }
    }

    free(node);
}

// Affichage de l’AST

void print_ast(ASTNode *node, int depth) {
    if (node == NULL) return;

    const char *branch = "├── ";
    const char *last_branch = "└── ";
    const char *vertical = "│   ";

    for (int i = 0; i < depth; i++) printf("%s", vertical);

    if (node->type == CMD) {
        printf("%s\033[1;32mCommande: ", branch);
        switch (node->data.cmd.type) {
            case CMD_Add:    printf("ADD\033[0m\n"); break;
            case CMD_Remove: printf("REMOVE\033[0m\n"); break;
            case CMD_Modify: printf("MODIFY\033[0m\n"); break;
            case CMD_List:   printf("LIST\033[0m\n"); break;
            case CMD_Search: printf("SEARCH\033[0m\n"); break;
            case CMD_Clear:  printf("CLEAR\033[0m\n"); break;
            default:         printf("INVALIDE\033[0m\n"); break;
        }

        Parameter *current = node->data.cmd.parameters;
        int count = 0;
        for (Parameter *p = current; p; p = p->next) count++;

        int index = 0;
        while (current) {
            index++;
            for (int i = 0; i < depth + 1; i++) printf("%s", vertical);

            const char *curr_branch = (index == count) ? last_branch : branch;
            printf("%s\033[1;34mParamètre: ", curr_branch);

            switch (current->type) {
                case P_title:       printf("TITLE\033[0m = \"%s\"\n", current->value.data.strig); break;
                case P_description: printf("DESCRIPTION\033[0m = \"%s\"\n", current->value.data.strig); break;
                case P_location:    printf("LOCATION\033[0m = \"%s\"\n", current->value.data.strig); break;
                case P_Date:        printf("DATE\033[0m = %s\n", current->value.data.date); break;
                case P_Time:        printf("TIME\033[0m = %s\n", current->value.data.time); break;
                case P_duration:    printf("DURATION\033[0m = %d\n", current->value.data.number); break;
                case P_on:          printf("ON\033[0m = %s\n", current->value.data.date); break;
                case P_from:        printf("FROM\033[0m = %s\n", current->value.data.date); break;
                case P_to:          printf("TO\033[0m = %s\n", current->value.data.date); break;
                case P_all:         printf("ALL\033[0m\n"); break;
                default:            printf("INVALIDE\033[0m\n"); break;
            }

            current = current->next;
        }

        if (count == 0) {
            for (int i = 0; i < depth + 1; i++) printf("%s", vertical);
            printf("%s\033[3m[Pas de paramètres]\033[0m\n", last_branch);
        }
    } else if (node->type == NODE_ERROR) {
        printf("%s\033[1;31mERREUR: %s\033[0m\n", branch, node->data.error_msg);
    }
}

// Fonction principale

ASTNode *parse(Lexer *lexer) {
    Parser parser;
    parser_init(&parser, lexer);
    ASTNode *root = parser_cmd(&parser);

    if (parser.error) {
        if (root->type != NODE_ERROR) {
            free_ast(root);
            root = malloc(sizeof(ASTNode));
            root->type = NODE_ERROR;
            strcpy(root->data.error_msg, parser.error_msg);
        }
    }

    return root;
}
