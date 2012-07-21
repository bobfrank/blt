
tokens = (
        'ID',
        'BEGIN_ASSEMBLY',
        'END_ASSEMBLY',
        'SET_INDENT',
        'DOLLAR',
        'CONTINUATION',
        'NEXT_STMT',
        'DOT_DOT_DOT',
        'AT',
        'OPERATOR',
        'STRING',
        'NUMBER',
        'AT_ID',
        'DD_ID',
        'DOLLAR_ID',
        'DOT_DOT_DOT_ID',
        'INDENT',
        'DEDENT',
        'WS'
    )
import re
t_ID            = r'[a-zA-Z_][a-zA-Z0-9_]*'
t_BEGIN_ASSEMBLY= r'\$\['
t_END_ASSEMBLY  = r'\]\$'
t_AT_ID         = r'@'+t_ID
t_DD_ID         = r'(\$\$'+t_ID+')|(\$\$[0-9])'
t_DOLLAR_ID     = r'\$'+t_ID
t_DOT_DOT_DOT_ID= r'\.\.\.'+t_ID
t_DOLLAR        = r'\$'
t_DOT_DOT_DOT   = r'\.\.\.'
t_AT            = r'@'
t_OPERATOR      = r'[%s]+|[\(\)\[\]\{\}]'%re.escape('/+-*\\?#%!~,.:`=')
t_STRING        = r'''"(?:(?:\\.)|(?:[^"]))*"'''
t_NUMBER        = r'[+-]?\d+(?:\.\d+)?(?:[eE][+-]\d+)?'
t_NEXT_STMT     = r';'
def t_CONTINUATION(t):
    r'\\[ \t]*\n+'
    t.lexer.lineno += len(t.value.split('\n'))-1
    return t
def t_SET_INDENT(t):
    r'\n+[ \t]*' # one tab is equal to one space in terms of indentation
    #print(t.lexer.lineno)
    t.lexer.lineno += len(t.value.split('\n'))-1
    #print(t.lexer.lineno)
    return t
def t_COMMENT(t):
    r'\/\/.*'
    pass
t_WS            = r'[ \t]+'

import ply.lex as lex
lexer = lex.lex()

class MyLexer:
    def __init__(self):
        self.__gen = self.generator()
        self.peeked = None

    def token(self):
        try:
            ret = next(self.__gen)
            #print(ret)
            return ret
        except StopIteration:
            return None

    def input(self, data):
        lexer.input(data)

    def otoken(self):
        if self.peeked:
            tok = self.peeked
            self.peeked = None
        else:
            tok = lexer.token()
        return tok

    def peek_otoken(self):
        if not self.peeked:
            self.peeked = lexer.token()
        return self.peeked

    def generator(self):
        indentation_stack = [0]
        last_lineno = 0
        last_token_type = None
        while True:
            tok = self.otoken()
            if not tok: break      # No more input
            new_tok = lex.LexToken()
            new_tok.value  = tok.value
            new_tok.type   = tok.type
            new_tok.lineno = tok.lineno
            new_tok.lexpos = tok.lexpos
            if tok.type == 'SET_INDENT':
                #print( last_lineno, last_token_type, tok.lineno, tok.type, tok.value)
                if lexer.lineno != last_lineno and last_token_type != 'CONTINUATION':
                    new_tok.type = 'NEXT_STMT'
                    yield new_tok
                    set_indent = len(tok.value.replace('\n',''))
                    while indentation_stack[-1] < set_indent:
                        indentation_stack.append(set_indent)
                        new_tok.type = 'INDENT'
                        yield new_tok
                    while indentation_stack[-1] > set_indent:
                        indentation_stack = indentation_stack[:-1]
                        new_tok.type = 'DEDENT'
                        yield new_tok
            elif tok.type != 'CONTINUATION' and tok.type != 'WS':
                if tok.type == 'OPERATOR' and tok.value == ']':
                    future_tok = self.peek_otoken()
                    if future_tok.type == 'DOLLAR':
                        self.otoken()
                        new_tok.type  = 'END_ASSEMBLY'
                        new_tok.value = ']$'
                        yield new_tok
                    else:
                        yield new_tok
                else:
                    yield new_tok
            last_lineno = lexer.lineno
            last_token_type = tok.type
        new_tok = lex.LexToken()
        new_tok.value  = ''
        new_tok.type   = 'DEDENT'
        new_tok.lineno = last_lineno+1
        new_tok.lexpos = 0
        for i in range(len(indentation_stack)-1):
            yield new_tok

def p_code_1(t):
    'code : codetype code'
    out = [t[1]]
    out.extend(t[2])
    t[0] = out
def p_code_2(t):
    'code : codetype'
    t[0] = [t[1]]
def p_codetype(t):
    '''
    codetype : macrodef
             | macrocall
             | assembly
             | NEXT_STMT'''
    t[0] = t[1]
def p_macrodef(t):
    'macrodef : DOLLAR macrocall'
    t[0] = {'macro_def': t[2]['macro_call'], 'block': t[2].get('block')}
def p_macrocall_1(t):
    'macrocall : macroprefix_stmt block'
    t[0] = {'macro_call': t[1], 'block': t[2]}
def p_macrocall_2(t):
    'macrocall : macroprefix_stmt'
    t[0] = {'macro_call': t[1]}
def p_block(t):
    'block : INDENT code DEDENT'
    t[0] = {'block': t[2]}
def p_macroprefix_stmt(t):
    'macroprefix_stmt : macroprefix NEXT_STMT'
    t[0] = t[1]
def p_macroprefix_1(t):
    'macroprefix : macroprefix macroelem'
    out = t[1]
    out.append(t[2])
    t[0] = out
def p_macroprefix_2(t):
    'macroprefix : macroelem_nd'
    t[0] = [t[1]]
def p_macroelem(t):
    '''
     macroelem : macroelem_nd
               | DOLLAR'''
    t[0] = t[1]
def p_macroelem_nd(t):
    '''
     macroelem_nd : ID
                  | OPERATOR
                  | DOLLAR_ID
                  | DD_ID
                  | AT_ID
                  | AT
                  | NUMBER
                  | STRING
                  | DOT_DOT_DOT
                  | DOT_DOT_DOT_ID'''
    t[0] = t[1]
def p_assembly_1(t):
    'assembly : BEGIN_ASSEMBLY assembly_code END_ASSEMBLY'
    t[0] = {'assembly': t[2]}
def p_assembly_2(t):
    'assembly : BEGIN_ASSEMBLY DOLLAR macroprefix END_ASSEMBLY'
    t[0] = {'assembly': {'macro_call': t[3]}}
def p_asm_1(t):
    'assembly_code : assembly_code assembly_codetype'
    out = t[1]
    out.append(t[2])
    t[0] = out
def p_asm_2(t):
    'assembly_code : assembly_codetype'
    t[0] = [t[1]]
def p_assembly_codetype_1(t):
    ' assembly_codetype : NUMBER'
    t[0] = ('number',float(t[1]))
def p_assembly_codetype_2(t):
    ' assembly_codetype : STRING'
    t[0] = ('const_string',t[1])
def p_assembly_codetype_3(t):
    ' assembly_codetype : DD_ID'
    t[0] = ('id_as_string',t[1])


data = open('static_typing.blt3').read()
import ply.yacc as yacc
parser = yacc.yacc()
tree = parser.parse(data, lexer=MyLexer())
import pprint
pprint.pprint(tree)
