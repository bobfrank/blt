reserved = {
    'extern':   'EXTERN',
    'const':    'CONST',
    'class':    'CLASS',
    'template': 'TEMPLATE',
    'operator': 'OPERATOR',
    'elif':     'ELIF',
    'else':     'ELSE',
    'if':       'IF',
    'char':     'CHAR',
    'char16':   'CHAR16',
    'float16':  'FLOAT32',
    'float32':  'FLOAT64',
    'int128':   'INT128',
    'int64':    'INT64',
    'int32':    'INT32',
    'int16':    'INT16',
    'int8':     'INT8',
    'uint128':  'UINT128',
    'uint64':   'UINT64',
    'uint32':   'UINT32',
    'uint16':   'UINT16',
    'uint8':    'UINT8',
    'word':     'WORD',
    'hword':    'HWORD',
    'dword':    'DWORD',
    'import' :  'IMPORT',
    'with':     'WITH',
    'as':       'AS'
    }
tokens = ['COMMENT',
    'ASSIGNOPER',
    'ID','POINTS', 'BITAND',
    'PLUS','MINUS','TIMES','DIVIDE','EQUALS',
    'LPAREN','RPAREN', 'SCOPE','OPEN',
    'CLOSE', 'DOT', 'COMMA', 'EXTERNCODE',
    'COLON', 'SEMICOLON', 'DOLLAR',
    'LBRACKET','RBRACKET', 'GT',
    'INTCONST', 'FLOATCONST', 'STRINGCONST'
    ] + list(reserved.values())

# Tokens

t_ASSIGNOPER    = r'(\+=|-=|\*=|/=|%=|^=|&=|>>=|<<=|\*\*=)'
t_COMMENT       = r'//[^\n\r]*'
t_STRINGCONST   = r'".*?"'
t_FLOATCONST    = r'\d+\.\d+'
t_INTCONST      = r'\d+'
t_POINTS        = r'=>'
t_GT            = r'>'
t_OPEN          = r'{'
t_CLOSE         = r'}'
t_BITAND        = r'&'
t_PLUS          = r'\+'
t_MINUS         = r'-'
t_DOLLAR        = r'\$'
t_TIMES         = r'\*'
t_DIVIDE        = r'/'
t_EQUALS        = r'='
t_LPAREN        = r'\('
t_RPAREN        = r'\)'
t_LBRACKET      = r'\['
t_RBRACKET      = r'\]'
t_SCOPE         = r'::'
t_COLON         = r':'
t_DOT           = r'\.'
t_COMMA         = r','
t_EXTERNCODE    = r'{{.*?}}'
t_SEMICOLON     = r';'
# Ignored characters
t_ignore = " \t"

def t_ID(t):
    r'[a-zA-Z_][a-zA-Z0-9_]*'
    t.type = reserved.get(t.value, 'ID')
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
    'segment : tclass'
    t[0] = t[1]
def p_segment_import(t):
    'segment : import'
    t[0] = t[1]

def p_import_extern(t):
    'import : IMPORT STRINGCONST WITH STRINGCONST AS ID SEMICOLON'
    t[0] = ['IMPORT_EXTERN',t[2][1:-1],t[4][1:-1],t[6]]
def p_import_blt(t):
    'import : IMPORT dname SEMICOLON'
    t[0] = ['IMPORT',t[2]]

def p_operator_bracket(t):
    'operator : OPERATOR LBRACKET RBRACKET'
    t[0] = ['OPERATOR', '[]']


def p_templatearg_type(t):
    'templatearg : ID EQUALS ID'
    t[0] = ['TEMPLATE_ARG', t[1], t[3]]
def p_templatearg_bdata(t):
    'templatearg : ID EQUALS constant'
    t[0] = ['TEMPLATE_ARG', t[1], t[3]]

def p_templateargs_one(t):
    'templateargs : templatearg'
    t[0] = [t[1]]
def p_templateargs_more(t):
    'templateargs : templateargs templatearg'
    tmp = t[1]
    tmp.append(t[2])
    t[0] = tmp


#TODO figure out what to use for string's... maybe force const char*?
def p_templargspec_type(t):
    'templargspec : ID ID EQUALS ID'
    t[0] = ['TEMPLATE_ARGSPEC', t[1], t[3]]
    if t[1] != 'typename':
        raise SyntaxError
def p_templargspec_bdata(t):
    'templargspec : ID ID EQUALS constant'
    t[0] = ['TEMPLATE_ARGSPEC', t[1], t[2], t[4]]
    if t[1] not in ['int','float','string']:
        raise SyntaxError
def p_templargspec_nodefault(t):
    'templargspec : ID ID'
    t[0] = ['TEMPLATE_ARGSPEC', t[1], t[2], None]
    if t[1] not in ['typename','int','float','string']:
        raise SyntaxError

def p_templargspecs_one(t):
    'templargspecs : templargspec'
    t[0] = [t[1]]
def p_templargspecs_more(t):
    'templargspecs : templargspecs templargspec'
    tmp = t[1]
    tmp.append(t[2])
    t[0] = tmp


def p_dname_name(t):
    'dname : ID'
    t[0] = t[1]
def p_dname_more(t):
    'dname : dname DOT ID'
    t[0] = ['SUBUTYPE', t[1], t[3]]

def p_basictype_char(t):
    'basictype : CHAR'
    t[0] = ['BASICTYPE', t[1]]
def p_basictype_char16(t):
    'basictype : CHAR16'
    t[0] = ['BASICTYPE', t[1]]
def p_basictype_int128(t):
    'basictype : INT128'
    t[0] = ['BASICTYPE', t[1]]
def p_basictype_int64(t):
    'basictype : INT64'
    t[0] = ['BASICTYPE', t[1]]
def p_basictype_int32(t):
    'basictype : INT32'
    t[0] = ['BASICTYPE', t[1]]
def p_basictype_int16(t):
    'basictype : INT16'
    t[0] = ['BASICTYPE', t[1]]
def p_basictype_int8(t):
    'basictype : INT8'
    t[0] = ['BASICTYPE', t[1]]
def p_basictype_uint128(t):
    'basictype : UINT128'
    t[0] = ['BASICTYPE', t[1]]
def p_basictype_uint64(t):
    'basictype : UINT64'
    t[0] = ['BASICTYPE', t[1]]
def p_basictype_uint32(t):
    'basictype : UINT32'
    t[0] = ['BASICTYPE', t[1]]
def p_basictype_uint16(t):
    'basictype : UINT16'
    t[0] = ['BASICTYPE', t[1]]
def p_basictype_uint8(t):
    'basictype : UINT8'
    t[0] = ['BASICTYPE', t[1]]
def p_basictype_word(t):
    'basictype : WORD'
    t[0] = ['BASICTYPE', t[1]]
def p_basictype_hword(t):
    'basictype : HWORD'
    t[0] = ['BASICTYPE', t[1]]
def p_basictype_dword(t):
    'basictype : DWORD'
    t[0] = ['BASICTYPE', t[1]]
def p_basictype_float32(t):
    'basictype : FLOAT32'
    t[0] = ['BASICTYPE', t[1]]
def p_basictype_float64(t):
    'basictype : FLOAT64'
    t[0] = ['BASICTYPE', t[1]]

def p_etype_plain(t):
    'etype : dname'
    t[0] = ['TYPE', t[1]]
def p_etype_basictype(t):
    'etype : basictype'
    t[0] = t[1]
def p_etype_templated(t):
    'etype : dname OPEN templateargs CLOSE'
    t[0] = ['TEMPLATED_TYPE', t[1], t[3]]
def p_etype_external(t):
    'etype : DOLLAR ID'
    t[0] = ['EXTTYPE', t[2]]
def p_rtype_plain(t):
    'rtype : etype'
    t[0] = t[1]
def p_ptype_ref(t):
    'rtype : etype BITAND'
    t[0] = ['REF', t[1]]
def p_ptype_plain(t):
    'ptype : rtype'
    t[0] = t[1]
def p_ptype_ptr(t):
    'ptype : rtype TIMES'
    t[0] = ['PTR', t[1]]
def p_ctype_plain(t):
    'ctype : ptype'
    t[0] = t[1]
def p_ctype_const(t):
    'ctype : CONST ptype'
    t[0] = ['CONST', t[2]] #TODO make this like c++ const


def p_constructor_args(t):
    'constructor : dname LPAREN args RPAREN'
    t[0] = ['CONSTRUCT', t[1], t[3]]
def p_constructor_void(t):
    'constructor : dname LPAREN RPAREN'
    t[0] = ['CONSTRUCT', t[1], None]
def p_constructor_tvoid(t):
    'constructor : dname LPAREN templateargs RPAREN LPAREN RPAREN'
    mytype = [TEMPLATED_TYPE, t[1], t[3]]
    t[0] = ['CONSTRUCT', mytype, None]
def p_constructor_targs(t):
    'constructor : dname LPAREN templateargs RPAREN LPAREN args RPAREN'
    mytype = [TEMPLATED_TYPE, t[1], t[3]]
    t[0] = ['CONSTRUCT', mytype, t[6]]

def p_constant_int(t):
    'constant : INTCONST'
    t[0] = int(t[1])
def p_constant_float(t):
    'constant : FLOATCONST'
    t[0] = float(t[1])
def p_constant_string(t):
    'constant : STRINGCONST'
    t[0] = t[1][1:-1]

def p_data_constant(t):
    'data : constant'
    t[0] = t[1]
def p_data_name(t):
    'data : dname'
    t[0] = t[1]
def p_data_ptr(t):
    'data : BITAND dname'
    t[0] = ['LOCATE', t[1]]
def p_data_deref(t):
    'data : TIMES dname'
    t[0] = ['DEREF', t[1]]
def p_data_construct(t):
    'data : constructor'
    t[0] = t[1]

def p_arg_required(t):
    'arg : ctype ID'
    t[0] = ['ARG', t[1], t[2], None]
def p_arg_optional(t):
    'arg : ctype ID EQUALS data'
    t[0] = ['ARG', t[1], t[2], t[4]]

def p_args_one(t):
    'args : arg'
    t[0] = [t[1]]
def p_args_more(t):
    'args : args COMMA arg'
    tmp = t[1]
    tmp.append(t[3])
    t[0] = tmp


def p_callarg(t):
    'callarg : ID EQUALS data'
    t[0] = ['CALLARG', t[1], t[3]]

def p_callargsa_one(t):
    'callargsa : callarg'
    t[0] = [t[1]]
def p_callargsa_more(t):
    'callargsa : callargsa COMMA callarg'
    tmp = t[1]
    tmp.append(t[3])
    t[0] = tmp

def p_callargs_plain(t):
    'callargs : callargsa'
    t[0] = t[1]
def p_callargs_single(t):
    'callargs : data'
    t[0] = ['CALLARG_ONE',t[1]]


def p_compare(t):
    'compare : rtype GT data' #TODO actually write this up.. just want it to compile
    t[0] = ['GT', t[1], t[3]]

def p_bareif(t):
    'bareif : IF LPAREN compare RPAREN block'
    t[0] = ['IF', t[3], t[5]]

def p_elif(t):
    'elif : ELIF LPAREN compare RPAREN block'
    t[0] = ['ELIF', t[3], t[5]]

def p_else(t):
    'else : ELSE block'
    t[0] = ['ELSE', t[2]]

def p_elifs_one(t):
    'elifs : elif'
    t[0] = [t[1]]
def p_elifs_many(t):
    'elifs : elifs elif'
    tmp = t[1]
    tmp.append(t[2])
    t[0] = tmp

def p_elses_with(t):
    'elses : elifs else'
    t[1].append(t[2])
    t[0] = t[1]
def p_elses_just(t):
    'elses : else'
    t[0] = [t[1]]

def p_ifstatement_if(t):
    'ifstatement : bareif'
    t[0] = t[1]
def p_ifstatement_elses(t):
    'ifstatement : bareif elses'
    t[0] = ['IFELSE',t[1], t[2]]


def p_assignment_op(t):
    'assignment : dname ASSIGNOPER data'
    t[0] = ['ASSIGN',t[2],t[1],t[3]]
def p_assignment_eq(t):
    'assignment : dname EQUALS data'
    t[0] = ['ASSIGN','=',t[1],t[3]]

def p_construct_equals(t):
    'construct : ctype ID EQUALS data'
    t[0] = ['CONSTRUCT',t[1],t[2],t[4]]
def p_construct_plain(t):
    'construct : ctype ID LPAREN callargs RPAREN'
    t[0] = ['CONSTRUCT',t[1],t[2],t[4]]


def p_fnname_id(t):
    'fnname : ID'
    t[0] = ['FNAME', None, t[1]]
def p_fnname_operator(t):
    'fnname : operator'
    t[0] = ['FNAME', None, t[1]]
def p_fnname_class(t):
    'fnname : ID SCOPE ID'
    t[0] = ['FNAME', t[1], t[3]]

def p_funcall_args(t):
    'funcall : fnname LPAREN callargs RPAREN'
    t[0] = ['FUNCALL', t[1], t[3]]
def p_funcall_void(t):
    'funcall : fnname LPAREN RPAREN'
    t[0] = ['FUNCALL', t[1], None]


def p_oblock_if(t):
    'oblock : ifstatement'
    t[0] = t[1]
def p_oblock_fncall(t):
    'oblock : funcall SEMICOLON'
    t[0] = t[1]
def p_oblock_assignment(t):
    'oblock : assignment SEMICOLON'
    t[0] = t[1]
def p_oblock_construct(t):
    'oblock : construct SEMICOLON'
    t[0] = t[1]
def p_oblock_block(t):
    'oblock : block'
    t[0] = t[1]

def p_blocks_one(t):
    'blocks : oblock'
    t[0] = t[1]
def p_blocks_more(t):
    'blocks : blocks oblock'
    tmp = t[1]
    tmp.append(t[2])
    t[0] = tmp

def p_block_empty(t):
    'block : OPEN CLOSE'
    t[0] = []
def p_block(t):
    'block : OPEN blocks CLOSE'
    t[0] = t[2]


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
    'function : fnname argret block'
    t[0] = ['FUNCTION', t[1], t[2], t[3]]
def p_function_init(t):
    'function : fnname argret COLON callargs block'
    t[0] = ['INITFUNCTION', t[1], t[2], t[4], t[5]]
def p_function_extern(t):
    'function : fnname argret EQUALS EXTERN STRINGCONST argret EXTERNCODE SEMICOLON'
    t[0] = ['EXTFUNCTION', t[1], t[2], t[5][1:-1], t[6], t[7][2:-2]]


def p_fundef(t):
    'fundef : fnname argret SEMICOLON'
    t[0] = ['FUNDEF', t[1], t[2]]

def p_fundefs_one(t):
    'fundefs : fundef'
    t[0] = [t[1]]
def p_fundefs_many(t):
    'fundefs : fundefs fundef'
    tmp = t[1]
    tmp.append(t[2])
    t[0] = tmp

def p_class(t):
    'class : CLASS ID OPEN fundefs CLOSE'
    t[0] = ['CLASS', t[2], t[4]]

def p_tclass_template(t):
    'tclass : TEMPLATE OPEN templargspecs CLOSE class'
    t[0] = ['TEMPLATE_CLASS', t[3], t[5]]
def p_tclass_plain(t):
    'tclass : class'
    t[0] = t[1]

import ply.yacc as yacc
import sys
yacc.yacc()
import pprint
pprint.pprint( yacc.parse( open(sys.argv[1]).read() ), indent=3 )
