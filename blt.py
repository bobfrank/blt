tokens = (
    'NAME','FLOAT', 'POINTS',
    'PLUS','MINUS','TIMES','DIVIDE','EQUALS',
    'LPAREN','RPAREN', 'INT', 'SCOPE','OPEN',
    'CLOSE', 'DOT', 'COMMA'
    )

# Tokens

t_POINTS  = r'=>'
t_OPEN    = r'{'
t_CLOSE   = r'}'
t_PLUS    = r'\+'
t_MINUS   = r'-'
t_TIMES   = r'\*'
t_DIVIDE  = r'/'
t_EQUALS  = r'='
t_LPAREN  = r'\('
t_RPAREN  = r'\)'
t_SCOPE   = r'::'
t_NAME    = r'[a-zA-Z_][a-zA-Z0-9_]*'
t_FLOAT   = r'\d+\.\d+'
t_INT     = r'\d+'
t_DOT     = r'\.'
t_COMMA   = r','
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

def p_arg_required(t):
    'arg : NAME NAME'
    t[0] = '%s %s'%(t[1],t[2])

def p_arg_optional(t):
    'arg : NAME NAME EQUALS NAME'
    t[0] = '%s %s = %s'%(t[1],t[2], t[4])

def p_arg_optional_int(t):
    'arg : NAME NAME EQUALS INT'
    t[0] = '%s %s = %s'%(t[1],t[2], t[4])

def p_arg_optional_float(t):
    'arg : NAME NAME EQUALS FLOAT'
    t[0] = '%s %s = %s'%(t[1],t[2], t[4])

def p_args_one(t):
    'args : arg'
    t[0] = [t[1]]

def p_args_more(t):
    'args : args COMMA arg'
    tmp = t[1]
    tmp.append(t[3])
    t[0] = tmp

def p_function_void(t):
    'function : fnname OPEN CLOSE'
    t[0] = 'void %s() {}'%t[1]

def p_function_args(t):
    'function : fnname LPAREN args RPAREN OPEN CLOSE'
    t[0] = 'void %s(%s) {}'%(t[1], ','.join(t[3]))

def p_function_full(t):
    'function : fnname LPAREN args RPAREN POINTS LPAREN args RPAREN OPEN CLOSE'
    t[0] = 'struct %s_ret {%s};\n%s_ret %s(%s) {}'%(t[1], ';'.join(t[7]), t[1], t[1], ','.join(t[3]))

def p_function_return(t):
    'function : fnname POINTS LPAREN args RPAREN OPEN CLOSE'
    t[0] = 'struct %s_ret {%s};\n%s_ret %s() {}'%(t[1], ';'.join(t[4]), t[1], t[1])


import ply.yacc as yacc
import sys
yacc.yacc()
yacc.parse( open(sys.argv[1]).read() )
