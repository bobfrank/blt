reserved = {'extern': 'EXTERN', 'const': 'CONST'}
tokens = [
    'NAME','FLOAT', 'POINTS', 'BITAND',
    'PLUS','MINUS','TIMES','DIVIDE','EQUALS',
    'LPAREN','RPAREN', 'INT', 'SCOPE','OPEN',
    'CLOSE', 'DOT', 'COMMA', 'EXTERNCODE',
    'STRING', 'SEMICOLON'
    ] + list(reserved.values())

# Tokens

t_STRING  = r'".*?"'
t_POINTS  = r'=>'
t_OPEN    = r'{'
t_CLOSE   = r'}'
t_BITAND  = r'&'
t_PLUS    = r'\+'
t_MINUS   = r'-'
t_TIMES   = r'\*'
t_DIVIDE  = r'/'
t_EQUALS  = r'='
t_LPAREN  = r'\('
t_RPAREN  = r'\)'
t_SCOPE   = r'::'
def t_NAME(t):
    r'[a-zA-Z_][a-zA-Z0-9_]*'
    t.type = reserved.get(t.value, 'NAME')
    return t
t_FLOAT   = r'\d+\.\d+'
t_INT     = r'\d+'
t_DOT     = r'\.'
t_COMMA   = r','
t_EXTERNCODE= r'{{.*?}}'
t_SEMICOLON = r';'
# Ignored characters
t_ignore = " \t"

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
    print( "Syntax error at '%s'" % t.value)


# Parsing rules

precedence = ()

def p_code_one(t):
    'code : function'
    print (t[1])
def p_code_more(t):
    'code : code function'
    print (t[2])

def p_fnname_id(t):
    'fnname : NAME'
    t[0] = t[1]
def p_fnname_class(t):
    'fnname : NAME SCOPE NAME'
    t[0] = '%s::%s'%(t[1],t[3])


def p_type_plain(t):
    'type : NAME'
def p_rtype_plain(t):
    'rtype : type'
def p_ptype_ref(t):
    'rtype : type BITAND'
def p_ptype_plain(t):
    'ptype : rtype'
def p_ptype_ptr(t):
    'ptype : rtype TIMES'
def p_ctype_plain(t):
    'ctype : ptype'
def p_ctype_const(t):
    'ctype : CONST ptype'


def p_dname_name(t):
    'dname : NAME'
def p_dname_more(t):
    'dname : dname DOT NAME'

def p_data_int(t):
    'data : INT'
def p_data_float(t):
    'data : FLOAT'
def p_data_string(t):
    'data : STRING'
def p_data_name(t):
    'data : dname'
def p_data_ptr(t):
    'data : BITAND dname'
def p_data_deref(t):
    'data : TIMES dname'

def p_arg_required(t):
    'arg : ctype NAME'
    t[0] = '%s %s'%(t[1],t[2])

def p_arg_optional(t):
    'arg : ctype NAME EQUALS data'
    t[0] = '%s %s = %s'%(t[1],t[2], t[4])


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
    t[0] = ''
def p_argret_args(t):
    'argret : LPAREN args RPAREN'
    t[0] = 'args--'
def p_argret_full(t):
    'argret : LPAREN args RPAREN POINTS LPAREN args RPAREN'
    t[0] = 'full--'
def p_argret_return(t):
    'argret : POINTS LPAREN args RPAREN'
    t[0] = 'return--'


def p_function_args(t):
    'function : fnname argret OPEN block CLOSE'
    t[0] = 'void %s(%s) {%s}'%(t[1], t[2], t[4])
def p_function_extern(t):
    'function : fnname argret EQUALS EXTERN STRING argret EXTERNCODE SEMICOLON'
    t[0] = 'extern function == %s'%t[7]

import ply.yacc as yacc
import sys
yacc.yacc()
yacc.parse( open(sys.argv[1]).read() )
