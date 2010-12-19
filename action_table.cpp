#include <unordered_map>
#include <regex>
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
    FINISHED,//shown as $ in a lot of algorithms
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
    "p_templateargs", "p_S'", "$"
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
    const token_vector  m_tokens;
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
    int      m_label;
    atom_set m_atoms;
};
bool
operator<(const labelled_atom_set& lhs,
          const labelled_atom_set& rhs)
{
    return lhs.atoms() < rhs.atoms();
}
typedef std::set<labelled_atom_set>         dfa;
typedef std::pair<int,std::pair<int,int> >  alink;
typedef std::set<alink>                     links;

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
    while( changed )
    {
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
                        a_it    = atoms.end();
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

void build_dfa( dfa&   my_dfa,
                links& dfa_links,
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
    labelled_atom_set first_label(0,atoms);
    my_dfa.insert( first_label );
    int count = 0;

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
                        dfa_it  = my_dfa.end();
                        changed = true;
                        ++count;
                        break;
                    }
                    dfa_links.insert( std::make_pair(dfa_it->label(),std::make_pair(out.first->label(),i)) );
                }
            }
        }
    }
}

void
print_dfa( std::ostream& os, const dfa& my_dfa, const links& dfa_links )
{
    os << "digraph finite_state_machine {" << std::endl
       << "    rankdir=LR;" << std::endl
       //<< "    size=\"67,40\";" << std::endl
       << "    graph [fontsize=6];" << std::endl
       << "    edge  [fontsize=6];" << std::endl
       << "    node  [fontsize=6];" << std::endl
       << "    node [shape = rect];" << std::endl;
    for( dfa::const_iterator dfa_it = my_dfa.begin(); dfa_it != my_dfa.end(); ++dfa_it )
    {
        os << "    LR_" << dfa_it->label() << " [label=\"";
        for( atom_set::iterator atom_it = dfa_it->atoms().begin(); atom_it != dfa_it->atoms().end(); ++atom_it ) { os << *atom_it << "\\l"; }
        os << "\"];" << std::endl;
    }
    for( links::const_iterator links_it = dfa_links.begin(); links_it != dfa_links.end(); ++links_it )
    {
        const int from     = links_it->first;
        const int to       = links_it->second.first;
        const int token_id = links_it->second.second;
        os << "    LR_" << from << " -> LR_" << to
           << " [ label=\"" << token_names[token_id] << "\" ];" << std::endl;
    }
    os << "}" << std::endl;
}

typedef enum {SHIFT, REDUCE} sor;
struct action {
    sor act;
    int tok;
};

typedef std::unordered_map< int, std::unordered_map<int, action> > action_table;
typedef action_table goto_table;
void
build_parse_table( goto_table&      gotos,
                   action_table&    actions,
                   const dfa&       my_dfa,
                   const links&     my_links )
{
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
    // If step 1 above produces cells with multiple values, then the grammar is said not to be SLR(1) (in general, a grammar is
    //      said to be SLR(1) if an SLR(1) parser can be built).

    typedef std::unordered_map<int, std::vector<int> > terminal_atomsets;
    terminal_atomsets terminals;

    for(dfa::const_iterator dfa_it = my_dfa.begin(); dfa_it != my_dfa.end(); ++dfa_it)
    {
        for(atom_set::const_iterator atom_it = dfa_it->atoms().begin(); atom_it != dfa_it->atoms().end(); ++atom_it)
        {
            if(atom_it->point_location() == atom_it->tokens().size()) {
                terminals[dfa_it->label()].push_back(  )
            }
        }
    }

    for(links::const_iterator links_it = my_links.begin(); links_it != my_links.end(); ++links_it)
    {
    }
}


// In order to fill the parsing table, we have to establish what grammar rule the parser should choose if it sees
//  a nonterminal A on the top of its stack and a symbol a on its input stream. It is easy to see that such a rule
//  should be of the form A → w and that the language corresponding to w should have at least one string starting
//  with a. For this purpose we define the First-set of w, written here as Fi(w), as the set of terminals that can
//  be found at the start of any string in w, plus ε if the empty string also belongs to w. Given a grammar with
//  the rules A1 → w1, ..., An → wn, we can compute the Fi(wi) and Fi(Ai) for every rule as follows:
// 
//    1. initialize every Fi(wi) and Fi(Ai) with the empty set
//    2. add Fi(wi) to Fi(wi) for every rule Ai → wi, where Fi is defined as follows:
//           * Fi(a w' ) = { a } for every terminal a
//           * Fi(A w' ) = Fi(A) for every nonterminal A with ε not in Fi(A)
//           * Fi(A w' ) = Fi(A) \ { ε } ∪ Fi(w' ) for every nonterminal A with ε in Fi(A)
//           * Fi(ε) = { ε } 
//    3. add Fi(wi) to Fi(Ai) for every rule Ai → wi
//    4. repeat the steps 2 and 3 until all Fi sets stay the same. 
// 
// Unfortunately, the First-sets are not sufficient to compute the parsing table. This is because a right-hand side
//  w of a rule might ultimately be rewritten to the empty string. So the parser should also use the a rule A → w
//  if ε is in Fi(w) and it sees on the input stream a symbol that could follow A. Therefore we also need the
//  Follow-set of A, written as Fo(A) here, which is defined as the set of terminals a such that there is a string
//  of symbols αAaβ that can be derived from the start symbol. Computing the Follow-sets for the nonterminals in a
//  grammar can be done as follows:
// 
//    1. initialize every Fo(Ai) with the empty set
//    2. if there is a rule of the form Aj → wAiw' , then
//           * if the terminal a is in Fi(w' ), then add a to Fo(Ai)
//           * if ε is in Fi(w' ), then add Fo(Aj) to Fo(Ai) 
//    3. repeat step 2 until all Fo sets stay the same. 
// 
// Now we can define exactly which rules will be contained where in the parsing table. If T[A, a] denotes the entry
//  in the table for nonterminal A and terminal a, then
// 
//     T[A,a] contains the rule A → w iff
// 
//         a is in Fi(w) or 
//         ε is in Fi(w) and a is in Fo(A). 
// 
// If the table contains at most one rule in every one of its cells, then the parser will always know which rule it
//  has to use and can therefore parse strings without backtracking. It is in precisely this case that the grammar
//  is called an LL(1) grammar. 





// Constructing LR(1) parsing tables
// 
// An LR(1) item is a production with a marker together with a terminal, e.g., [S → a A • B e, c]. Intuitively, such an item indicates how much of a certain production we have seen already (a A), what we could expect next (B e), and a lookahead that agrees with what should follow in the input if we ever reduce by the production S → a A B e. By incorporating such lookahead information into the item concept, we can make wiser reduce decisions. The lookahead of an LR(1) item is used directly only when considering reduce actions (i.e., when the • marker is at the right end).
// 
// The core of an LR(1) item [S → a A • B e, c] is the LR(0) item S → a A • B e. Different LR(1) items may share the same core. For example, if we have two LR(1) items of the form
// 
//     * [A → α •, a] and
//     * [B → α •, b],
// 
// we take advantage of the lookahead to decide which reduction to use. (The same setting would perhaps produce a reduce/reduce conflict in the SLR approach.)
// [edit] Validity
// 
// The notion of validity changes. An item [A → β1 • β2, a] is valid for a viable prefix α β1 if there is a rightmost derivation that yields α A a w which in one step yields α β1β2 a w
// [edit] Initial item
// 
// To get the parsing started, we begin with the initial item of
// 
//     [S’ → • S, $].
// 
// Here $ is a special character denoting the end of the string.
// [edit] Closure
// 
// Closure is more refined. If [A → α • B β, a] belongs to the set of items, and B → γ is a production of the grammar, then we add the item [B → • γ, b] for all b in FIRST(β a).
// [edit] Goto
// 
// Goto is the same. A state containing [A → α • X β, a] will move to a state containing [A → α X • β, a] with label X.
// 
// Every state has transitions according to Goto. for all
// [edit] Shift actions
// 
// The shift actions are the same. If [A → α • b β, a] is in state Ik and Ik moves to state Im with label b, then we add the action
// 
//     action[k, b] = "shift m"
// 
// [edit] Reduce actions
// 
// The reduce actions are more refined than SLR . If [A→α •, a] is in state Ik, then we add the action: "Reduce A → α" to action[Ik, a]. Observe that we don’t use information from FOLLOW(A) anymore. The goto part of the table is as before.
// [edit] 

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
    links my_links; //actually part of the dfa... probably will collapse to the same structure in the future
    build_dfa( my_dfa, my_links, g, p_code );
    print_dfa( std::cout, my_dfa, my_links );
}
