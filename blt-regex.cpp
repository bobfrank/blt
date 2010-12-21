/*
 -- ASSIGNOPER    = r'(\+=|-=|\*=|/=|%=|^=|&=|>>=|<<=|\*\*=)'
 -- STRINGCONST   = r'".*?"'
 -- FLOATCONST    = r'\d+\.\d+'
 -- INTCONST      = r'\d+'
OPEN          = r'{'
CLOSE         = r'}'
SUBSCRIPT     = r'\[\]'
EQ            = r'=='
POINTS        = r'=>'
MEMBERPTR     = r'->'
LTE           = r'<='
GTE           = r'>='
NE            = r'!='
GT            = r'>'
LT            = r'<'
-- LOGNOT        = r'!'
-- LOGOR         = r'\|\|'
-- LOGAND        = r'&&'
BITNOT        = r'~'
BITAND        = r'&'
BITOR         = r'\|'
BITXOR        = r'\^'
BITLEFT       = r'<<'
BITRIGHT      = r'>>'
MOD           = r'%'
INCREMENT     = r'\+\+'
DECREMENT     = r'--'
PLUS          = r'\+'
MINUS         = r'-'
TIMES         = r'\*'
DIVIDE        = r'/'
EQUALS        = r'='
LPAREN        = r'\('
RPAREN        = r'\)'
LBRACKET      = r'\['
RBRACKET      = r'\]'
SCOPE         = r'::'
QUESTION      = r'\?'
COLON         = r':'
-- DOT           = r'\.'
COMMA         = r','
EXTERNCODE    = r'{{.*?}}'
SEMICOLON     = r';'
// Ignored characters
t_ignore = " \t"

-- def t_ID(t):
--     r'[a-zA-Z_][a-zA-Z0-9_]*'
--     t.type = reserved.get(t.value, 'ID')
--     return t

-- def t_newline(t):
--    r'(//[^\n]*)?\n+'
--    t.lexer.lineno += t.value.count("\n")
--    t.lexer.current = t.lexer.lexpos
*/
#include <boost/regex.hpp>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>

typedef enum {
    EXTERN,CONST,CLASS,TEMPLATE,OPERATOR,
    ELIF,ELSE,IF,CHAR,CHAR16,FLOAT32,
    FLOAT64,FLOAT128,INT128,INT64,INT32,
    INT16,INT8, UINT128,UINT64,UINT32,
    UINT16, UINT8,WORD,HWORD, DWORD,
    IMPORT,WITH, AS,TYPEDEF, ASSIGNOPER,
    LTE,GTE,NE,LOGNOT,LOGOR,LOGAND,
    BITNOT,BITAND,BITOR,BITXOR,BITLEFT,
    BITRIGHT,MOD,INCREMENT,DECREMENT,
    QUESTION, EQ, SUBSCRIPT, MEMBERPTR,
    ID,POINTS,
    PLUS,MINUS,TIMES,DIVIDE,EQUALS,
    LPAREN,RPAREN, SCOPE,OPEN,
    CLOSE, DOT, COMMA, EXTERNCODE,
    COLON, SEMICOLON,
    LBRACKET,RBRACKET, GT, LT,
    INTCONST, FLOATCONST, STRINGCONST,
    p_arg, p_argret,p_args,p_assignment,p_bareif,
    p_basictype,p_block,p_blocks,p_callarg,p_callargs,
    p_callargsa,p_class,p_cmpop,p_code,
    p_compare,p_constant,p_construct,p_ctype,p_data,
    p_deftype,p_dname,p_elif,p_elifs,p_else,p_elses,
    p_etype,p_fnname,p_funcall,p_function,p_fundec,
    p_fundecs,p_ifstatement,p_import,p_oblock,p_operator,
    p_overloadable,p_ptype,p_rtype,p_segment,p_tclass,
    p_templargspec,p_templargspecs,p_templatearg,
    p_templateargs,p_S,
    FINISHED,//shown as $ in a lot of algorithms
    NEWLINE,
    TOKEN_COUNT
} Token;

const char* token_names[] = {
    "EXTERN","CONST","CLASS","TEMPLATE","OPERATOR",
    "ELIF","ELSE","IF","CHAR","CHAR16","FLOAT32",
    "FLOAT64","FLOAT128","INT128","INT64","INT32",
    "INT16","INT8", "UINT128","UINT64","UINT32",
    "UINT16", "UINT8","WORD","HWORD", "DWORD",
    "IMPORT","WITH", "AS","TYPEDEF", "ASSIGNOPER",
    "LTE","GTE","NE","LOGNOT","LOGOR","LOGAND",
    "BITNOT","BITAND","BITOR","BITXOR","BITLEFT",
    "BITRIGHT","MOD","INCREMENT","DECREMENT",
    "QUESTION", "EQ", "SUBSCRIPT", "MEMBERPTR",
    "ID","POINTS",
    "PLUS","MINUS","TIMES","DIVIDE","EQUALS",
    "LPAREN","RPAREN", "SCOPE","OPEN",
    "CLOSE", "DOT", "COMMA", "EXTERNCODE",
    "COLON", "SEMICOLON",
    "LBRACKET","RBRACKET", "GT", "LT",
    "INTCONST", "FLOATCONST", "STRINGCONST",
    "p_arg"," p_argret","p_args","p_assignment","p_bareif",
    "p_basictype","p_block","p_blocks","p_callarg","p_callargs",
    "p_callargsa","p_class","p_cmpop","p_code",
    "p_compare","p_constant","p_construct","p_ctype","p_data",
    "p_deftype","p_dname","p_elif","p_elifs","p_else","p_elses",
    "p_etype","p_fnname","p_funcall","p_function","p_fundec",
    "p_fundecs","p_ifstatement","p_import","p_oblock","p_operator",
    "p_overloadable","p_ptype","p_rtype","p_segment","p_tclass",
    "p_templargspec","p_templargspecs","p_templatearg",
    "p_templateargs", "p_S'", "$", "NL"
};

int main()
{
    typedef std::vector< std::pair<std::string,int> > lexer_tokens;
    lexer_tokens tokens = {
            {"//[^\\n]*\\n+",                               NEWLINE},
            {"\\n+",                                        NEWLINE},
            {"extern",                                      EXTERN},
            {"const",                                       CONST},
            {"class",                                       CLASS},
            {"template",                                    TEMPLATE},
            {"operator",                                    OPERATOR},
            {"elif",                                        ELIF},
            {"else",                                        ELSE},
            {"if",                                          IF},
            {"char",                                        CHAR},
            {"char16",                                      CHAR16},
//            {"float16",                                     FLOAT16},
            {"float32",                                     FLOAT32},
            {"float64",                                     FLOAT64},
            {"float128",                                    FLOAT128},
            {"int8",                                        INT8},
            {"int16",                                       INT16},
            {"int32",                                       INT32},
            {"int64",                                       INT64},
            {"int128",                                      INT128},
            {"uint8",                                       UINT8},
            {"uint16",                                      UINT16},
            {"uint32",                                      UINT32},
            {"uint64",                                      UINT64},
            {"uint128",                                     UINT128},
            {"word",                                        WORD},
            {"hword",                                       HWORD},
            {"dword",                                       DWORD},
            {"import",                                      IMPORT},
            {"with",                                        WITH},
            {"as",                                          AS},
            {"typedef",                                     TYPEDEF},
            {"\".*?\"",                                     STRINGCONST},
            {"\\d+\\.\\d+",                                 FLOATCONST},
            {"\\d+",                                        INTCONST},
            {"[a-zA-Z_][a-zA-Z0-9_]*",                      ID},
            {"\\.",                                         DOT},
            {"\\+=|-=|\\*=|/=|%=|^=|&=|>>=|<<=|\\*\\*=",    ASSIGNOPER},
            {"\\{",                                         OPEN},
            {"\\}",                                         CLOSE},
            {"\\[\\]",                                      SUBSCRIPT},
            {"==",                                          EQ},
            {"=>",                                          POINTS},
            {"->",                                          MEMBERPTR},
            {"<=",                                          LTE},
            {">=",                                          GTE},
            {"!=",                                          NE},
            {"<",                                           LT},
            {">",                                           GT},
            {"!",                                           LOGNOT},
            {"\\|\\|",                                      LOGOR},
            {"&&",                                          LOGAND},
            {"~",                                           BITNOT},
            {"&",                                           BITAND},
            {"\\^",                                         BITXOR},
            {"\\|",                                         BITOR},
            {"<<",                                          BITLEFT},
            {">>",                                          BITRIGHT},
            {"%",                                           MOD},
            {"\\+\\+",                                      INCREMENT},
            {"--",                                          DECREMENT},
            {"\\+",                                         PLUS},
            {"-",                                           MINUS},
            {"\\*",                                         TIMES},
            {"/",                                           DIVIDE},
            {"=",                                           EQUALS},
            {"\\(",                                         LPAREN},
            {"\\)",                                         RPAREN},
            {"\\[",                                         LBRACKET},
            {"\\]",                                         RBRACKET},
            {"::",                                          SCOPE},
            {"\\?",                                         QUESTION},
            {":",                                           COLON},
            {",",                                           COMMA},
            {"\\{\\{.*?\\}\\}",                             EXTERNCODE},
            {";",                                           SEMICOLON}
    };
    std::string re;
    for( int i = 0; i < tokens.size(); ++i )
    {
        if( re.size() > 0 ) {
            re += "|";
        }
        re += "(";
        re += tokens[i].first;
        re += ")";
    }
    std::cout << "regex = " << re << std::endl;
    std::ifstream fp("functions.blt");
    char text_in[10000];
    fp.read(text_in, sizeof(text_in));
    boost::regex expression(re.c_str());
    std::string text = text_in; //"with blah.token 3.5 += 2";
    boost::sregex_iterator m1(text.begin(), text.end(), expression);
    boost::sregex_iterator m2;
    for( ; m1 != m2;  ++m1 )
    {
        for( int i = 0; i < tokens.size(); ++i ) {
            const std::string& curr = (*m1)[i+1].str();
            if( !curr.empty() ) {
                std::cout << "token --> " << token_names[tokens[i].second] << "("<<curr<<")" << std::endl;
                break;
            }
        }
    }
}
