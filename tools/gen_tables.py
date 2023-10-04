import ply.yacc as yacc
import csv
import numpy as np
import json


def dump_csv(csv_path, data):
    with open(csv_path, mode="w", newline="") as csv_file:
        writer = csv.writer(
            csv_file, delimiter=",", quotechar='"', quoting=csv.QUOTE_MINIMAL
        )
        for row in data:
            if type(row) == str:
                writer.writerow([row])
            else:
                writer.writerow(row)


# Define the lexer
tokens = [
    "ARRAY",
    "BOOLEAN",
    "BREAK",
    "CHAR",
    "CONTINUE",
    "DO",
    "ELSE",
    "FALSE",
    "FUNCTION",
    "IF",
    "INTEGER",
    "OF",
    "RETURN",
    "STRING",
    "STRUCT",
    "TRUE",
    "TYPE",
    "VAR",
    "WHILE",
    "COLON",
    "SEMI_COLON",
    "COMMA",
    "EQUALS",
    "LEFT_SQUARE",
    "RIGHT_SQUARE",
    "LEFT_BRACES",
    "RIGHT_BRACES",
    "LEFT_PARENTHESIS",
    "RIGHT_PARENTHESIS",
    "AND",
    "OR",
    "LESS_THAN",
    "GREATER_THAN",
    "LESS_OR_EQUAL",
    "GREATER_OR_EQUAL",
    "NOT_EQUAL",
    "EQUAL_EQUAL",
    "PLUS",
    "PLUS_PLUS",
    "MINUS",
    "MINUS_MINUS",
    "TIMES",
    "DIVIDE",
    "DOT",
    "NOT",
    "CHARACTER",
    "NUMERAL",
    "STRINGVAL",
    "ID",
]


def p_p(p):
    """p : lde"""


def p_lde(p):
    """lde : lde de
    | de
    """


def p_de(p):
    """de : df
    | dt
    """


def p_t(p):
    """t : INTEGER
    | CHAR
    | BOOLEAN
    | STRING
    | idu
    """


def p_dt(p):
    """dt : TYPE idd EQUALS ARRAY LEFT_SQUARE num RIGHT_SQUARE OF t
    | TYPE idd EQUALS STRUCT nb LEFT_BRACES dc RIGHT_BRACES
    | TYPE idd EQUALS t
    """


def p_dc(p):
    """dc : dc SEMI_COLON li COLON t
    | li COLON t
    """


def p_df(p):
    """df : FUNCTION idd nf LEFT_PARENTHESIS lp RIGHT_PARENTHESIS COLON t mf b"""


def p_lp(p):
    """lp : lp COMMA idd COLON t
    | idd COLON t
    """


def p_b(p):
    """b : LEFT_BRACES ldv ls RIGHT_BRACES"""


def p_ldv(p):
    """ldv : ldv dv
    | dv
    """


def p_ls(p):
    """ls : ls s
    | s
    """


def p_dv(p):
    """dv : VAR li COLON t SEMI_COLON"""


def p_li(p):
    """li : li COMMA idd
    | idd
    """


def p_s(p):
    """s : IF LEFT_PARENTHESIS e RIGHT_PARENTHESIS mt s ELSE me s
    | IF LEFT_PARENTHESIS e RIGHT_PARENTHESIS mt s
    | WHILE mw LEFT_PARENTHESIS e RIGHT_PARENTHESIS mt s
    | DO mw s WHILE LEFT_PARENTHESIS e RIGHT_PARENTHESIS SEMI_COLON
    | nb b
    | lv EQUALS e SEMI_COLON
    | BREAK SEMI_COLON
    | CONTINUE SEMI_COLON
    | RETURN e SEMI_COLON
    """


# def p_s(p):
#    """s : u
#    | m
#    """
# def p_m(p):
#    """m : IF LEFT_PARENTHESIS e RIGHT_PARENTHESIS mt m ELSE me u
#    | WHILE mw LEFT_PARENTHESIS e RIGHT_PARENTHESIS mt m
#    | DO mw m WHILE LEFT_PARENTHESIS e RIGHT_PARENTHESIS SEMI_COLON
#    | nb b
#    | lv EQUALS e SEMI_COLON
#    | BREAK SEMI_COLON
#    | CONTINUE SEMI_COLON
#    """
# def p_u(p):
#    """u : IF LEFT_PARENTHESIS e RIGHT_PARENTHESIS mt m ELSE me s
#    | IF LEFT_PARENTHESIS e RIGHT_PARENTHESIS mt s
#    """


def p_e(p):
    """e : e AND l
    | e OR l
    | l
    """


def p_l(p):
    """l : l LESS_THAN r
    | l GREATER_THAN r
    | l LESS_OR_EQUAL r
    | l GREATER_OR_EQUAL r
    | l EQUAL_EQUAL r
    | l NOT_EQUAL r
    | r
    """


def p_r(p):
    """r : r PLUS y
    | r MINUS y
    | y
    """


def p_y(p):
    """y : y TIMES f
    | y DIVIDE f
    | f
    """


def p_f(p):
    """f :  lv
    | PLUS_PLUS lv
    | MINUS_MINUS lv
    | lv PLUS_PLUS
    | lv MINUS_MINUS
    | LEFT_PARENTHESIS e RIGHT_PARENTHESIS
    | idu mc LEFT_PARENTHESIS le RIGHT_PARENTHESIS
    | MINUS f
    | NOT f
    | true
    | false
    | c
    | str
    | num
    """


def p_le(p):
    """le : le COMMA e
    | e
    """


def p_lv(p):
    """lv : lv DOT idu
    | lv LEFT_SQUARE e RIGHT_SQUARE
    | idu
    """


def p_true(p):
    """true : TRUE"""


def p_false(p):
    """false : FALSE"""


def p_c(p):
    """c : CHARACTER"""


def p_str(p):
    """str : STRINGVAL"""


def p_num(p):
    """num : NUMERAL"""


def p_idd(p):
    """idd : ID"""


def p_idu(p):
    """idu : ID"""


def p_nb(p):
    """nb :"""


def p_mf(p):
    """mf :"""


def p_mc(p):
    """mc :"""


def p_mt(p):
    """mt :"""


def p_me(p):
    """me :"""


def p_mw(p):
    """mw :"""


def p_nf(p):
    """nf :"""


def p_error(p):
    print(f"Parser Error: Unexpected token '{p.value}'")


parser = yacc.yacc(debug=True)

# Generate action and goto tables
action_table = dict((sorted(parser.action.items())))
goto_table = dict(sorted(parser.goto.items()))
productions = parser.productions

non_term_tokens = list(set([key for val in goto_table.values() for key in val.keys()]))
all_tokens = sorted(non_term_tokens) + tokens + ["$end"]
action_idxs = sorted(action_table.keys())
at_f = np.zeros((len(action_idxs), len(all_tokens)), dtype=np.int32)

for action, state in action_table.items():
    for tok, val in state.items():
        idx = all_tokens.index(tok)
        at_f[action][idx] = val

    for tok, val in goto_table.get(action, {}).items():
        idx = all_tokens.index(tok)
        at_f[action][idx] = val

aux = []
for idx, rule in enumerate(productions):
    # Don't care for the first rule
    if idx == 0:
        continue
    aux += [[idx, rule.len, rule.str.split(" ")[0]]]

# print(at_f[6-1][49])
# print(aux[72])

# for r, line in enumerate(at_f):
#    for tok, el in enumerate(line):
#        if el != 0:
#            print("[{}, {}] = ".format(r, all_tokens[tok]), el)

dump_csv("action_table.csv", at_f)
dump_csv("aux_table.csv", aux)
dump_csv("markers.csv", sorted(non_term_tokens))
