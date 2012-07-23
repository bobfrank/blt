import ply.yacc as yacc
import re
import sys
import ply.lex as lex
import pprint

tokens = (
        'ID',
        'BEGIN_ASSEMBLY',
        'END_ASSEMBLY',
        'SET_INDENT',
        'DOLLAR',
        'CONTINUATION',
        'NEXT_STMT',
        'DOT_DOT_DOT',
        'DOT_DOT_DOT_ID',
        'AT',
        'OPERATOR',
        'STRING',
        'NUMBER',
        'AT_ID',
        'DD_ID',
        'DOLLAR_ID',
        'INDENT',
        'DEDENT',
        'WS'
    )
EID             = r'[a-zA-Z0-9_]'
t_ID            = r'[a-zA-Z_]%s*'%EID
t_BEGIN_ASSEMBLY= r'\$\['
t_END_ASSEMBLY  = r'\]\$'
t_AT_ID         = r'@%s+'%EID
t_DD_ID         = r'\$\$%s+'%EID
t_DOLLAR_ID     = r'\$%s+'%EID
t_DOT_DOT_DOT_ID= r'\.\.\.%s+'%EID
t_DOLLAR        = r'\$'
t_DOT_DOT_DOT   = r'\.\.\.'
t_AT            = r'@'
t_OPERATOR      = r'[%s]+|[\(\)\[\]\{\}]'%re.escape('/+-*\\?#%!~,.:`=<>&|^')
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

class MyLexer:
    def __init__(self):
        self.__gen = self.generator()
        self.peeked = None
        self.lexer = lex.lex()

    def token(self):
        try:
            ret = next(self.__gen)
            #print(ret)
            return ret
        except StopIteration:
            return None

    def input(self, data):
        self.lexer.input(data)

    def otoken(self):
        if self.peeked:
            tok = self.peeked
            self.peeked = None
        else:
            tok = self.lexer.token()
        return tok

    def peek_otoken(self):
        if not self.peeked:
            self.peeked = self.lexer.token()
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
                #print( last_lineno, last_token_type, self.lexer.lineno, tok.type, tok.value)
                if last_token_type != 'CONTINUATION':
                    new_tok.type = 'NEXT_STMT'
                    yield new_tok
                    set_indent = len(tok.value.replace('\n',''))
                    #print('to_indent=',set_indent,', from_indent=',indentation_stack[-1])
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
                elif tok.type == 'OPERATOR' and tok.value == '...':
                    future_tok = self.peek_otoken()
                    #print('tok',tok,',ftok',future_tok)
                    if future_tok.type == 'ID':
                        self.otoken()
                        new_tok.type = 'DOT_DOT_DOT_ID'
                        new_tok.value = '...%s'%future_tok.value
                        yield new_tok
                    else:
                        yield new_tok
                elif tok.type == 'OPERATOR' and tok.value == '...':
                    new_tok.type = 'DOT_DOT_DOT'
                    yield new_tok
                else:
                    yield new_tok
            last_lineno = self.lexer.lineno
            last_token_type = tok.type
        new_tok = lex.LexToken()
        new_tok.value  = ''
        new_tok.type   = 'DEDENT'
        new_tok.lineno = last_lineno+1
        new_tok.lexpos = 0
        for i in range(len(indentation_stack)-1):
            yield new_tok

def p_code_1(t):
    'code : code codetype'
    out = t[1]
    if not isinstance(t[2],str) or t[2][0] != '\n':
        out.append(t[2])
    t[0] = out
def p_code_2(t):
    'code : codetype'
    if not isinstance(t[1],str) or t[1][0] != '\n':
        t[0] = [t[1]]
    else:
        t[0] = []
def p_codetype(t):
    '''
    codetype : macrodef
             | macrocall
             | assembly
             | NEXT_STMT'''
    t[0] = t[1]
def p_macrodef(t):
    'macrodef : DOLLAR macrocall'
    t[0] = {'_macro_def': t[2]['_macro_call'], 'block': t[2].get('block')}
def p_macrocall_1(t):
    'macrocall : macroprefix_stmt block'
    t[0] = {'_macro_call': t[1], 'block': t[2]}
def p_macrocall_2(t):
    'macrocall : macroprefix_stmt'
    t[0] = {'_macro_call': t[1]}
def p_block(t):
    'block : INDENT code DEDENT'
    t[0] = t[2]
def p_macroprefix_stmt(t):
    'macroprefix_stmt : macroprefix NEXT_STMT'
    t[0] = t[1]
def p_macroprefix_1(t):
    '''
    macroprefix : macroseq blk macroprefix'''
    t[0] = t[1]
    t[0].append(t[2])
    t[0].extend(t[3])
def p_macroprefix_2(t):
    '''
    macroprefix : blk macroprefix'''
    t[0] = [t[1]]
    t[0].extend(t[2])
def p_macroprefix_3(t):
    '''
    macroprefix : blk'''
    t[0] = [t[1]]
def p_macroprefix_4(t):
    '''
    macroprefix : macroseq blk'''
    t[0] = t[1]
    t[0].append(t[2])
def p_macroprefix_5(t):
    '''
    macroprefix : macroseq'''
    t[0] = t[1]
def p_blk(t):
    '''
    blk : AT_ID
        | AT'''
    t[0] = t[1]
def p_macroseq_1(t):
    '''
    macroseq : macroseq_bddd ddd macroseq_addd'''
    t[0] = t[1]
    t[0].append(t[2])
    t[0].extend(t[3])
def p_macroseq_2(t):
    '''
    macroseq : macroseq_bddd'''
    t[0] = t[1]
def p_macroseq_3(t):
    '''
    macroseq : macroseq_bddd ddd'''
    t[0] = t[1]
    t[0].append(t[2])
def p_macroseq_4(t):
    '''
    macroseq : ddd'''
    t[0] = [t[1]]
def p_ddd(t):
    '''ddd : DOT_DOT_DOT
           | DOT_DOT_DOT_ID'''
    t[0] = t[1]
def p_macroseq_addd_1(t):
    '''
    macroseq_addd : macroseq_addd macroelem'''
    out = t[1]
    out.append(t[2])
    t[0] = out
def p_macroseq_addd_2(t):
    '''
    macroseq_addd : macroelem'''
    t[0] = [t[1]]
def p_macroseq_bddd_1(t):
    '''
    macroseq_bddd : macroseq_bddd macroelem'''
    out = t[1]
    out.append(t[2])
    t[0] = out
def p_macroseq_bddd_2(t):
    '''
    macroseq_bddd : macroelem_nd'''
    t[0] = [t[1]]
def p_macroelem(t):
    '''
    macroelem : macroelem_nd
              | DOLLAR'''
    t[0] = t[1]
def p_macroelem_nd(t):
    '''macroelem_nd : ID
                  | OPERATOR
                  | DOLLAR_ID
                  | DD_ID
                  | NUMBER
                  | STRING'''
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

def parse(data):
    parser = yacc.yacc()
    return parser.parse(data, lexer=MyLexer())

if __name__ == '__main__':
    if len(sys.argv) >= 2:
        fname = sys.argv[1]
        data = open(fname).read()
        tree = parse(data)
        pprint.pprint(tree)
    else:
        print('Usage: %s fname'%sys.argv[0])
