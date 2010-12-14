#include <unordered_map>
#include <set>
#include <vector>
#include <iostream>

typedef int token;
typedef std::vector<token> token_vector;
typedef std::unordered_multimap<token,token_vector> grammar;
typedef enum {PLUS,TIMES,LPAREN,RPAREN,ID,DOT,p_exp,p_times,p_fact,p_S,TOKEN_COUNT} Token;
const char*token_names[] = {"PLUS","TIMES","LPAREN","RPAREN","ID","DOT","p_exp","p_times","p_fact","p_S"};

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
                    a_it = atoms.begin();
                    --a_it;
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
    labelled_atom_set first_label(0,atoms);
    my_dfa.insert( first_label );
    int count = 0;
    std::cout << "digraph finite_state_machine {" << std::endl
              << "    rankdir=LR;" << std::endl
              << "    size=\"8,5\";" << std::endl
              << "    node [shape = circle];" << std::endl;
    typedef std::pair<int,std::pair<int,int> > link;
    typedef std::set<link> links;
    links mylinks;
    for( dfa::iterator dfa_it = my_dfa.begin(); dfa_it != my_dfa.end(); ++dfa_it )
    {
        for( int i = 0; i < 9; ++i ) {
            atom_set after_token_i;
            get_goto( after_token_i, dfa_it->atoms(), g, i );
            if( !after_token_i.empty() )
            {
                labelled_atom_set here(count+1,after_token_i);
                std::pair<dfa::iterator,bool> out = my_dfa.insert( here );
                if( out.second ) {
                    std::cout << "    LR_" << here.label() << " [label=\"";
                    for( atom_set::iterator it = here.atoms().begin(); it != here.atoms().end(); ++it ) { std::cout << *it << "\\n"; }
                    std::cout << "\"];" << std::endl;
                    dfa_it = my_dfa.begin();
                    ++count;
                }
                std::pair<links::iterator,bool> sout = mylinks.insert( std::make_pair(dfa_it->label(),std::make_pair(out.first->label(),i)) );
                if( sout.second)
                std::cout << "    LR_" << dfa_it->label() << " -> LR_" << out.first->label()
                          << " [ label=\"" << token_names[i] << "\" ];" << std::endl;
                //std::cout << "go from " << dfa_it->label() << " to " << out.first->label() << " on " << token_names[i] << std::endl;
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
        {p_exp,     {p_exp,PLUS,p_times}},
        {p_exp,     {p_times}},
        {p_times,   {p_times,TIMES,p_fact}},
        {p_times,   {p_fact}},
        {p_fact,    {LPAREN,p_exp,RPAREN}},
        {p_fact,    {ID}},
    };
    dfa my_dfa;
    build_dfa( my_dfa, g, p_exp );
//     for( dfa::iterator dfa_it = my_dfa.begin(); dfa_it != my_dfa.end(); ++dfa_it )
//     {
//         std::cout << "state:" << std::endl;
//         for( atom_set::iterator it = dfa_it->atoms().begin(); it != dfa_it->atoms().end(); ++it ) { std::cout << *it << std::endl; }
//         std::cout << std::endl;
//     }
}
