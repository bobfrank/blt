#include <unordered_map>
#include <set>
#include <vector>
#include <iostream>

typedef int token;
typedef std::vector<token> token_vector;
typedef std::unordered_multimap<token,token_vector> grammar;
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
    "p_templateargs", "p_S'"
};

class Atom {
public:
    Atom(token t, const token_vector& tokens, int point_location=0);
    Atom( const Atom& a );
    int point_location() const { return m_point_location; }
    const token tok() const { return m_t; }
    const token_vector& tokens() const { return m_tokens; }
    void print( std::ostream& os ) const;
    bool compare( const Atom& rhs ) const;
private:
    const token_vector& m_tokens;
    token               m_t;
    int                 m_point_location;
};
typedef std::set<Atom> atom_set;

Atom::Atom( token t, const token_vector& tokens, int point_location )
    : m_t(t)
    , m_tokens(tokens)
    , m_point_location(point_location)
{}
Atom::Atom( const Atom& a )
    : m_t(a.m_t)
    , m_tokens(a.m_tokens)
    , m_point_location(a.m_point_location)
{
}
bool
Atom::compare(const Atom& rhs) const
{
    if( m_t < rhs.m_t ) return true;
    if( m_t > rhs.m_t ) return false;
    if( m_tokens < rhs.m_tokens ) return true;
    if( m_tokens > rhs.m_tokens ) return false;
    if( m_point_location < rhs.m_point_location ) return true;
    if( m_point_location > rhs.m_point_location ) return false;
    return false;
}

void
Atom::print( std::ostream& os ) const
{
    os << "[" << token_names[m_t] << "] --> ";
    for( int i = 0; i < m_tokens.size(); ++i )
    {
        if( i == m_point_location ) {
            os << " . ";
        }
        os << token_names[m_tokens[i]] << " ";
    }
    if( m_point_location >= m_tokens.size() ) {
        os << " . ";
    }
}

std::ostream&
operator<<(std::ostream& os, const Atom& a)
{
    a.print(os);
    return os;
}

bool
operator<(const Atom& lhs, const Atom& rhs)
{
    return lhs.compare(rhs);
}

class labelled_atom_set {
public:
    labelled_atom_set(int label, const atom_set& atoms) : m_atoms(atoms), m_label(label) {}
    int label() const { return m_label; }
    const atom_set& atoms() const { return m_atoms; }
private:
    int m_label;
    atom_set m_atoms;
};
bool
operator<(const labelled_atom_set& lhs,
          const labelled_atom_set& rhs)
{
    return lhs.atoms() < rhs.atoms();
}
typedef std::set<labelled_atom_set> dfa;


// a lot of this is from https://www.l2f.inesc-id.pt/~david/wiki/pt/index.php/Bottom-Up_Parsing
void get_closure(
        atom_set&       atoms,
        const grammar&  g )
{
    
    //Consider I a set of LR(0) items. The following conditions apply to rules in ε-closure(I):
    //
    //1. Initially, all elements of I are in ε-closure(I)
    //2. If A->α•Bβ is in ε-closure(I) and B->γ is a production, the add B->•γ to ε-closure(I) 
    //
    //(repeat until the ε-closure(I) is unchanged)
    bool changed = true;
    while( changed ) {
        changed = false;
        for( atom_set::const_iterator a_it = atoms.begin();
                a_it != atoms.end(); ++a_it )
        {
            if( a_it->point_location() < a_it->tokens().size() )
            {
                token B = a_it->tokens()[a_it->point_location()];
                for( grammar::const_iterator g_it = g.find(B);
                    g_it != g.end() && g_it->first == B; ++g_it )
                {
                    std::pair<atom_set::iterator,bool> out = atoms.insert( Atom(g_it->first, g_it->second, 0) );
                    if( out.second ) {
                        changed = true;
                        a_it = out.first;
                        break;
                    }
                }
            }
        }
    }
}

void get_goto( atom_set&        out,
               const atom_set&  I,
               const grammar&   g,
               token            X )
{
    //Goto
    //The goto function is formally defined as goto(I,X)
    //      (I is a set of LR(0) items, and X is a grammar symbol).
    // goto(I,X) = ε-closure({[A->αX•β]: ∀[A->α•Xβ]∈I})

    for( atom_set::const_iterator a_it = I.begin();
         a_it != I.end(); ++a_it )
    {
        int point_location = a_it->point_location();
        if( point_location < a_it->tokens().size() &&
                a_it->tokens()[point_location] == X )
        {
            Atom atom(a_it->tok(), a_it->tokens(), point_location+1);
            out.insert( atom );
        }
    }
    get_closure(out, g);
}

void build_dfa( dfa&  my_dfa,
                const grammar& g,
                token first_token )
{
    // Building the DFA
    // The DFA (a set of sets) starts by containing a single element, consisting of the set
    //     obtained by computing the ε-closure of the singular set {[S'->•S$]} (S is the start symbol):
    //  DFA = { ε-closure({[S'->•S$]}) }
    // Then, repeat until it is not possible to add any further elements: for each element
    //     (set) I in the DFA and for each grammar symbol X, add goto(I,X) (if not empty) to the DFA.

    token_vector first = {first_token};
    atom_set atoms = {Atom(p_S, first, 0)};
    get_closure(atoms, g);
    //std::cout << "printing first closure:" << std::endl;
    //for( atom_set::iterator it = atoms.begin(); it != atoms.end(); ++it ) { std::cout << *it << std::endl; }
    //std::cout << std::endl;
    labelled_atom_set first_label(0,atoms);
    my_dfa.insert( first_label );
    int count = 0;
    std::cout << "digraph finite_state_machine {" << std::endl
              << "    rankdir=LR;" << std::endl
              //<< "    size=\"67,40\";" << std::endl
              << "    graph [fontsize=4];" << std::endl
              << "    edge  [fontsize=4];" << std::endl
              << "    node  [fontsize=4];" << std::endl
              << "    node [shape = rect];" << std::endl;
    typedef std::pair<int,std::pair<int,int> > link;
    typedef std::set<link> links;
    links mylinks;
    bool changed = true;
    while( changed )
    {
        changed=false;
        for( dfa::iterator dfa_it = my_dfa.begin(); dfa_it != my_dfa.end(); ++dfa_it )
        {
            for( int i = 0; i < TOKEN_COUNT; ++i ) {
                atom_set after_token_i;
                get_goto( after_token_i, dfa_it->atoms(), g, i );
                if( !after_token_i.empty() )
                {
                    labelled_atom_set here(count+1,after_token_i);
                    std::pair<dfa::iterator,bool> out = my_dfa.insert( here );
                    if( out.second ) {
                        std::cout << "    LR_" << here.label() << " [label=\"";
                        for( atom_set::iterator it = here.atoms().begin(); it != here.atoms().end(); ++it ) { std::cout << *it << "\\l"; }
                        std::cout << "\"];" << std::endl;
                        //dfa_it = my_dfa.begin();
                        dfa_it = out.first;
                        changed = true;
                        ++count;
                        break;
                    }
                    std::pair<links::iterator,bool> sout = mylinks.insert( std::make_pair(dfa_it->label(),std::make_pair(out.first->label(),i)) );
                    if( sout.second)
                    std::cout << "    LR_" << dfa_it->label() << " -> LR_" << out.first->label()
                            << " [ label=\"" << token_names[i] << "\" ];" << std::endl;
                    //std::cout << "go from " << dfa_it->label() << " to " << out.first->label() << " on " << token_names[i] << std::endl;
                }
            }
        }
    }
    std::cout << "}" << std::endl;
}
// The Parse Table
// 
// The SLR(1) parse table is built, as mentioned above, based on the transitions of the DFA and on the FOLLOWs associated with reduced non-terminals (in each state where reductions occur -- the dot is at the end of the LR(0) item, meaning that all of its components have been either seen or reduced and they are available on the parser's stack).
// 
// The parse table has two zones, that associate states and symbols: action (shifts, reduces, accept), defined over states and terminal symbols (as well as with the end of phrase); and goto (defined over states and non-terminal symbols; associated with transitions after reducing non-terminals).
// 
// Assuming that the DFA has been built and, thus, the I0 ... In states are known, the following steps produce the parse table:
// 
//    1. State i is corresponds to Ii; the corresponding actions are:
//           * If [A->α•aβ] (a is a terminal) is in Ii and goto(Ii,a) = Ij, then define action[i,a] = shift j
//           * If [A->α•] is in Ii, then define action[i,a] = reduce A->α for all α ∈ FOLLOW(A) (A≠S', S' being the start symbol)
//           * If [S'->S•$] (S' being the start symbol) is in Ii, then define action[i,$] = ACCEPT 
//    2. For state i and A a non-terminal symbol, define goto(i,A) = j if goto(Ii, A) = Ij 
// 
// All cells left empty correspond to syntax errors.
// 
// If step 1 above produces cells with multiple values, then the grammar is said not to be SLR(1) (in general, a grammar is said to be SLR(1) if an SLR(1) parser can be built).
// 
// [TO BE CONTINUED...]
int main()
{
    grammar g = {
        {p_code,            {p_segment}},
        {p_code,            {p_code,p_segment}},
        {p_segment,         {p_function}},
        {p_segment,         {p_tclass}},
        {p_segment,         {p_import,SEMICOLON}},
        {p_segment,         {p_deftype,SEMICOLON}},
        {p_import,          {IMPORT,STRINGCONST,WITH,STRINGCONST,AS,ID}},
        {p_import,          {p_dname}},
        {p_overloadable,    {EQUALS}},
        {p_overloadable,    {PLUS}},
        {p_overloadable,    {MINUS}},
        {p_overloadable,    {TIMES}},
        {p_overloadable,    {DIVIDE}},
        {p_overloadable,    {MOD}},
        {p_overloadable,    {INCREMENT}},
        {p_overloadable,    {DECREMENT}},
        {p_overloadable,    {EQ}},
        {p_overloadable,    {NE}},
        {p_overloadable,    {GT}},
        {p_overloadable,    {LT}},
        {p_overloadable,    {GTE}},
        {p_overloadable,    {LTE}},
        {p_overloadable,    {LOGNOT}},
        {p_overloadable,    {LOGAND}},
        {p_overloadable,    {LOGOR}},
        {p_overloadable,    {BITNOT}},
        {p_overloadable,    {BITAND}},
        {p_overloadable,    {BITOR}},
        {p_overloadable,    {BITXOR}},
        {p_overloadable,    {BITLEFT}},
        {p_overloadable,    {BITRIGHT}},
        {p_overloadable,    {ASSIGNOPER}},
        {p_overloadable,    {SUBSCRIPT}},
        {p_overloadable,    {MEMBERPTR}},
        {p_overloadable,    {LPAREN,RPAREN}},
        {p_operator,        {OPERATOR,p_overloadable}},
        {p_templatearg,     {ID, EQUALS, ID}},
        {p_templatearg,     {ID, EQUALS, p_constant}},
        {p_templateargs,    {p_templatearg}},
        {p_templateargs,    {p_templateargs, p_templatearg}},
        {p_templargspec,    {ID, ID, EQUALS, ID}}, // first id must be typename
        {p_templargspec,    {ID, ID, EQUALS, p_constant}}, // first id must be in [int,float,string]
        {p_templargspec,    {ID, ID}}, // first id must be in [int,float,string,typename]
        {p_templargspecs,   {p_templargspec}},
        {p_templargspecs,   {p_templargspecs,p_templargspec}},
        {p_dname,           {ID}},
        {p_dname,           {p_dname,DOT,ID}},
        {p_basictype,       {CHAR}},
        {p_basictype,       {CHAR16}},
        {p_basictype,       {INT128}},
        {p_basictype,       {INT64}},
        {p_basictype,       {INT32}},
        {p_basictype,       {INT16}},
        {p_basictype,       {INT8}},
        {p_basictype,       {UINT128}},
        {p_basictype,       {UINT64}},
        {p_basictype,       {UINT32}},
        {p_basictype,       {UINT16}},
        {p_basictype,       {UINT8}},
        {p_basictype,       {WORD}},
        {p_basictype,       {HWORD}},
        {p_basictype,       {DWORD}},
        {p_basictype,       {FLOAT32}},
        {p_basictype,       {FLOAT64}},
        {p_basictype,       {FLOAT128}},
        {p_etype,           {p_dname}},
        {p_etype,           {p_basictype}},
        {p_rtype,           {p_etype}},
        {p_rtype,           {p_etype,BITAND}},
        {p_ptype,           {p_rtype}},
        {p_ptype,           {p_rtype,TIMES}},
        {p_ctype,           {p_ptype}},
        {p_ctype,           {CONST,p_ptype}}, //TODO make this like c++ const
        {p_deftype,         {TYPEDEF, p_ctype, ID}},
        {p_deftype,         {TYPEDEF, p_dname, LT, p_templateargs, GT, ID}},
        {p_constant,        {INTCONST}},
        {p_constant,        {FLOATCONST}},
        {p_constant,        {STRINGCONST}},
        {p_data,            {p_constant}},
        {p_data,            {CONST, LBRACKET, ID, RBRACKET}},
        {p_data,            {p_dname}},
        {p_data,            {BITAND,p_dname}},
        {p_data,            {TIMES,p_dname}},
        {p_data,            {p_funcall}},
        {p_arg,             {p_ctype, ID}},
        {p_arg,             {p_ctype, ID, EQUALS, p_data}},
        {p_args,            {p_arg}},
        {p_args,            {p_args,COMMA,p_arg}},
        {p_callarg,         {ID,EQUALS,p_data}},
        {p_callargsa,       {p_callarg}},
        {p_callargsa,       {p_callargsa,COMMA,p_callarg}},
        {p_callargs,        {p_callargsa}},
        {p_callargs,        {p_data}},
        {p_cmpop,           {EQ}},
        {p_cmpop,           {LTE}},
        {p_cmpop,           {GTE}},
        {p_cmpop,           {NE}},
        {p_cmpop,           {GT}},
        {p_cmpop,           {LT}},
        {p_compare,         {p_data,p_cmpop,p_data}},
        {p_bareif,          {IF,LPAREN,p_compare,RPAREN,p_block}},
        {p_elif,            {ELIF,LPAREN,p_compare,RPAREN,p_block}},
        {p_else,            {ELSE,p_block}},
        {p_elifs,           {p_elif}},
        {p_elifs,           {p_elifs,p_elif}},
        {p_elses,           {p_elifs,p_else}},
        {p_elses,           {p_else}},
        {p_ifstatement,     {p_bareif}},
        {p_ifstatement,     {p_bareif,p_elses}},
        {p_assignment,      {p_dname,ASSIGNOPER,p_data}},
        {p_assignment,      {p_dname,EQUALS,p_data}},
        {p_construct,       {p_ctype,ID,EQUALS,p_data}},
        {p_construct,       {p_ctype,ID,LPAREN,p_callargs,RPAREN}},
        {p_funcall,         {p_dname,LPAREN,p_callargs,RPAREN}},
        {p_funcall,         {p_dname,LPAREN,RPAREN}},
        {p_oblock,          {p_ifstatement}},
        {p_oblock,          {p_funcall,SEMICOLON}},
        {p_oblock,          {p_assignment,SEMICOLON}},
        {p_oblock,          {p_construct,SEMICOLON}},
        {p_oblock,          {p_block}},
        {p_blocks,          {p_oblock}},
        {p_blocks,          {p_blocks,p_oblock}},
        {p_block,           {OPEN,CLOSE}},  // TODO do I really want this to be valid?
        {p_block,           {OPEN,p_blocks,CLOSE}},
        {p_argret,          {}},
        {p_argret,          {LPAREN,p_args,RPAREN}},
        {p_argret,          {LPAREN,p_args,RPAREN,POINTS,LPAREN,p_args,RPAREN}},
        {p_argret,          {POINTS,LPAREN,p_args,RPAREN}},
        {p_fnname,          {ID}},
        {p_fnname,          {p_operator}},
        {p_fnname,          {ID,SCOPE,ID}},
        {p_function,        {p_fnname,p_argret,p_block}},
        {p_function,        {SCOPE,p_fnname,p_argret,p_block}},
        {p_function,        {p_fnname,p_argret,COLON,p_callargs,p_block}},
        {p_function,        {p_fnname,p_argret,EQUALS,EXTERN,STRINGCONST,p_argret,EXTERNCODE,SEMICOLON}},
        {p_fundec,          {p_fnname,p_argret}},
        {p_fundec,          {p_deftype}},
        {p_fundecs,         {p_fundec,SEMICOLON}},
        {p_fundecs,         {p_fundecs,p_fundec,SEMICOLON}},
        {p_class,           {CLASS,ID,OPEN,p_fundecs,CLOSE}},
        {p_tclass,          {TEMPLATE,LT,p_templargspecs,GT,p_class}},
        {p_tclass,          {p_class}}
    };
    dfa my_dfa;
    build_dfa( my_dfa, g, p_code );
//     for( dfa::iterator dfa_it = my_dfa.begin(); dfa_it != my_dfa.end(); ++dfa_it )
//     {
//         std::cout << "state:" << std::endl;
//         for( atom_set::iterator it = dfa_it->atoms().begin(); it != dfa_it->atoms().end(); ++it ) { std::cout << *it << std::endl; }
//         std::cout << std::endl;
//     }
}
