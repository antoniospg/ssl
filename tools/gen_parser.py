import csv


def open_csv(path):
    with open(path, "r") as csv_file:
        data = []
        csv_reader = csv.reader(csv_file)
        for row in csv_reader:
            data.append(row)

        return data


def gen_parser_tables(path):
    file = open(path, "w")
    action_table = open_csv("action_table.csv")
    aux_table = open_csv("aux_table.csv")
    all_toks = open_csv("markers.csv")

    action_r = len(action_table)
    action_c = len(action_table[0])
    aux_r = len(aux_table)
    aux_c = len(aux_table[0])

    file.write(
"""
#ifndef PARSER_TABLES_H
#define PARSER_TABLES_H

#include "Lex.h"

"""
    )

    # Enum of toks
    file.write("typedef enum {")
    for tok in all_toks:
        new_tok = str.upper(tok[0])
        if new_tok == '$END':
            new_tok = 'UNKNOWN'

        file.write("{}, ".format(new_tok))
    file.write("} markers;\n")

    file.write(
        """
static int action[{}][{}] = {{
""".format(
            action_r, action_c
        )
    )

    for action in action_table:
        file.write("\t{")
        for rule in action:
            file.write(str(rule) + ", ")
        file.write("},\n")

    file.write("};\n")

    file.write(
        """
static int aux[{}][{}] = {{
""".format(
            aux_r, aux_c - 1
        )
    )

    for rule in aux_table:
        file.write("\t{{ {}, {} }},\n".format(rule[1], str.upper(rule[2])))

    file.write(
        """
};
#endif
"""
    )


action_table = open_csv("action_table.csv")
aux_table = open_csv("aux_table.csv")
gen_parser_tables("ParserTables.h")
