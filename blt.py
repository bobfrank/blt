reserved = {
    'extern':   'EXTERN',
    'const':    'CONST',
    'class':    'CLASS',
    'template': 'TEMPLATE'
    }
tokens = ['COMMENT',
    'NAME','FLOAT', 'POINTS', 'BITAND',
    'PLUS','MINUS','TIMES','DIVIDE','EQUALS',
    'LPAREN','RPAREN', 'INT', 'SCOPE','OPEN',
    'CLOSE', 'DOT', 'COMMA', 'EXTERNCODE',
    'STRING', 'COLON', 'SEMICOLON', 'DOLLAR'
    ] + list(reserved.values())

# Tokens

t_COMMENT = r'//[^\n\r]*'
t_STRING  = r'".*?"'
t_POINTS  = r'=>'
t_OPEN    = r'{'
t_CLOSE   = r'}'
t_BITAND  = r'&'
t_PLUS    = r'\+'
t_MINUS   = r'-'
t_DOLLAR  = r'\$'
t_TIMES   = r'\*'
t_DIVIDE  = r'/'
t_EQUALS  = r'='
t_LPAREN  = r'\('
t_RPAREN  = r'\)'
t_SCOPE   = r'::'
t_COLON   = r':'
t_FLOAT   = r'\d+\.\d+'
t_INT     = r'\d+'
t_DOT     = r'\.'
t_COMMA   = r','
t_EXTERNCODE= r'{{.*?}}'
t_SEMICOLON = r';'
# Ignored characters
t_ignore = " \t"

def t_NAME(t):
    r'[a-zA-Z_][a-zA-Z0-9_]*'
    t.type = reserved.get(t.value, 'NAME')
    return t

def t_newline(t):
    r'\n+'
    t.lexer.lineno += t.value.count("\n")
    
def t_error(t):
    print( "Illegal character '%s'" % t.value[0])
    t.lexer.skip(1)
    
# Build the lexer
import ply.lex as lex
lex.lex()
def p_error(t):
    print( "Syntax error at '%s' on line %d" % (t.value, t.lexer.lineno))


# Parsing rules
FUNDEF,FUNCTION,EXTFUNCTION,CLASS,TEMPLATED,ARG,FNAME,TYPE,EXTTYPE,REF,PTR,CONST,OBJ,SUBOBJ,DEREF,LOCAT = range(16)

precedence = ()

def p_code_one(t):
    'code : comsegment'
    t[0] = [t[1]]
def p_code_more(t):
    'code : code comsegment'
    tmp = t[1]
    tmp.append(t[2])
    t[0] = tmp

def p_comsegment_plain(t):
    'comsegment : segment'
    t[0] = t[1]
def p_comsegment_comment(t):
    'comsegment : COMMENT segment'
    t[0] = t[2]

def p_segment_fn(t):
    'segment : function'
    t[0] = t[1]
def p_segment_cls(t):
    'segment : class'
    t[0] = t[1]

def p_fnname_id(t):
    'fnname : NAME'
    t[0] = [FNAME, None, t[1]]
def p_fnname_class(t):
    'fnname : NAME SCOPE NAME'
    t[0] = [FNAME, t[1], t[3]]


def p_etype_plain(t):
    'etype : NAME'
    t[0] = [TYPE, t[1]]
def p_etype_external(t):
    'etype : DOLLAR NAME'
    t[0] = [EXTTYPE, t[1]]
def p_rtype_plain(t):
    'rtype : etype'
    t[0] = t[1]
def p_ptype_ref(t):
    'rtype : etype BITAND'
    t[0] = [REF, t[1]]
def p_ptype_plain(t):
    'ptype : rtype'
    t[0] = t[1]
def p_ptype_ptr(t):
    'ptype : rtype TIMES'
    t[0] = [PTR, t[1]]
def p_ctype_plain(t):
    'ctype : ptype'
    t[0] = t[1]
def p_ctype_const(t):
    'ctype : CONST ptype'
    t[0] = [CONST, t[2]] #TODO make this like c++ const


def p_dname_name(t):
    'dname : NAME'
    t[0] = [OBJ, t[1]]
def p_dname_more(t):
    'dname : dname DOT NAME'
    t[0] = [SUBOBJ, t[1], t[3]]

def p_data_int(t):
    'data : INT'
    t[0] = int(t[1])
def p_data_float(t):
    'data : FLOAT'
    t[0] = float(t[1])
def p_data_string(t):
    'data : STRING'
    t[0] = t[1][1:-1]
def p_data_name(t):
    'data : dname'
    t[0] = t[1]
def p_data_ptr(t):
    'data : BITAND dname'
    t[0] = [LOCAT, t[1]]
def p_data_deref(t):
    'data : TIMES dname'
    t[0] = [DEREF, t[1]]

def p_arg_required(t):
    'arg : ctype NAME'
    t[0] = [ARG, t[1], t[2], None]

def p_arg_optional(t):
    'arg : ctype NAME EQUALS data'
    t[0] = [ARG, t[1], t[2], t[4]]


def p_args_one(t):
    'args : arg'
    t[0] = [t[1]]
def p_args_more(t):
    'args : args COMMA arg'
    tmp = t[1]
    tmp.append(t[3])
    t[0] = tmp


def p_block(t):
    'block : '
    t[0] = ''


def p_argret_void(t):
    'argret :'
    t[0] = [None,None]
def p_argret_args(t):
    'argret : LPAREN args RPAREN'
    t[0] = [t[2],None]
def p_argret_full(t):
    'argret : LPAREN args RPAREN POINTS LPAREN args RPAREN'
    t[0] = [t[2], t[6]]
def p_argret_return(t):
    'argret : POINTS LPAREN args RPAREN'
    t[0] = [None, t[3]]


def p_function_args(t):
    'function : fnname argret OPEN block CLOSE'
    t[0] = [FUNCTION, t[1], t[2], t[4]]
def p_function_extern(t):
    'function : fnname argret EQUALS EXTERN STRING argret EXTERNCODE SEMICOLON'
    t[0] = [EXTFUNCTION, t[1], t[2], t[5][1:-1], t[6], t[7][2:-2]]

def p_fundef(t):
    'fundef : fnname argret SEMICOLON'
    t[0] = [FUNDEF, t[1], t[2]]

def p_fundefs_one(t):
    'fundefs : fundef'
    t[0] = [t[1]]
def p_fundefs_many(t):
    'fundefs : fundefs fundef'
    tmp = t[1]
    tmp.append(t[2])
    t[0] = tmp

def p_class(t):
    'class : CLASS OPEN fundefs CLOSE'
    t[0] = [CLASS, fundefs]

def p_tempclass_plain(t):
    'tempclass : class'
    t[0] = t[1]
def p_tempclass_template(t):
    'tempclass : TEMPLATE LPAREN args RPAREN class'
    t[0] = [TEMPLATED, t[3], t[5]]

import ply.yacc as yacc
import sys
yacc.yacc()
import pprint
pprint.pprint( yacc.parse( open(sys.argv[1]).read() ), indent=3 )
