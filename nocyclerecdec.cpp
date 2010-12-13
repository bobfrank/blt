#include <vector>
#include <unordered_map>
#include <set>
#include <iostream>

// now converting it to an acyclic recursive descent parser
typedef enum {PLUS,TIMES,LPAREN,RPAREN,ID,p_exp,p_exp_a,p_times,p_times_a,p_fact,TOKEN_COUNT} Token;
const char*token_names[] = {"PLUS","TIMES","LPAREN","RPAREN","ID","exp","exp'","times","times'","fact" };

struct token_hash { int operator()(Token token) const { return static_cast<int>(token); } };
typedef std::vector<Token> token_vector;
typedef std::unordered_multimap<Token,token_vector, token_hash> grammar;

struct SyntaxError {};

// AST
struct Tree {
    Tree(Token token) : m_token(token) {}
    Tree* add_child(Token token) { m_children.push_back(Tree(token)); return &m_children[m_children.size()-1];} //first return is invalid if called again
    Token id() const { return m_token; }
    void clear() { m_children.clear(); }
    void print_indent(std::ostream& os, int indent) const {
        for( int k = 0; k < indent; ++k ) {
            os << "    ";
        }
    }
    void print(std::ostream& os, int indent=0) const {
        int token = m_token;
        std::string name;
        if( token >= TOKEN_COUNT ) {
            token -= TOKEN_COUNT;
            name = "\\";
        }
        name += token_names[token];
        if( !m_children.empty() ) {
            os << "[" << name << std::endl;
            for( int i = 0; i < m_children.size(); ++i ) {
                print_indent(os,indent+1);
                m_children[i].print(os, indent+1);
            }
            print_indent(os,indent);
            os << "]" << std::endl;
        }
        else {
            os << name << std::endl;
        }
    }
    Token m_token;
    std::vector<Tree> m_children;
};

void remove_left_recursion( grammar& new_g,
                            const grammar& g )
{
    //TODO clean this up... just want a first scetch working for now

    typedef std::set<Token> token_set;
    token_set left_recursive_tokens;

    // first pass -- find left recursive items
    for( grammar::const_iterator it = g.begin(); it != g.end(); ++it )
    {
        if( !it->second.empty() && it->first == it->second[0] ) {
            left_recursive_tokens.insert( it->first );
        }
    }

    // now add the left recursive ones back in... fixed
    for( token_set::const_iterator lrf_it = left_recursive_tokens.begin();
           lrf_it != left_recursive_tokens.end(); ++lrf_it )
    {
        token_vector tail_1;
        new_g.insert( std::make_pair((Token)(*lrf_it+TOKEN_COUNT), tail_1) );
        for( grammar::const_iterator g_it = g.find(*lrf_it);
              g_it != g.end() && g_it->first == *lrf_it; ++g_it)
        {
            if( !g_it->second.empty() && g_it->second[0] == *lrf_it ) {
                token_vector tail;
                for( int i = 1; i < g_it->second.size(); ++i ) {
                    tail.push_back( g_it->second[i] );
                }
                tail.push_back( (Token)(*lrf_it+TOKEN_COUNT) );
                new_g.insert( std::make_pair((Token)(*lrf_it+TOKEN_COUNT), tail) );
            }
            else {
                token_vector head = g_it->second;
                head.push_back( (Token)(*lrf_it+TOKEN_COUNT) );
                new_g.insert( std::make_pair(*lrf_it, head) );
            }
        }
    }

    // and add anything else back in
    for( grammar::const_iterator it = g.begin(); it != g.end(); ++it )
    {
        if( left_recursive_tokens.count(it->first)==0 ) {
            new_g.insert( *it );
        }
    }
}

// actual parsing
bool parse( Tree*               root,
            int*                new_offset,
            const grammar&      g,
            const token_vector& input,
            int                 offset = 0 )
{
    grammar::const_iterator it = g.find(root->id());
    if( it == g.end() ) {
        *new_offset = offset + 1;
        return input[offset] == root->id();
    }
    bool empty_able = false;
    for( ; it != g.end() && it->first == root->id(); ++it )
    {
        if( it->second.empty() ) {
            empty_able = true;
        }
        else {
            bool good = true;
            for( int i = 0; i < it->second.size(); ++i ) {
                Tree* tree = root->add_child(it->second[i]);
                int nf = offset;
                bool parsed = parse(tree, &nf, g, input, offset);
                if( !parsed ) {
                    good = false;
                    root->clear();
                    break;
                } else {
                    offset = nf;
                }
            }
            if( good ) {
                *new_offset = offset;
                return true;
            }
        }
    }
    if( empty_able ) {
        return true;
    }
    return false;
}

int main()
{
    //exp     : exp PLUS times | times
    //times   : times TIMES fact | fact
    //fact    : LPAREN exp RPAREN | ID
    grammar g = {
        {p_exp,     {p_exp,PLUS,p_times}},
        {p_exp,     {p_times}},
        {p_times,   {p_times,TIMES,p_fact}},
        {p_times,   {p_fact}},
        {p_fact,    {LPAREN,p_exp,RPAREN}},
        {p_fact,    {ID}}
    };
    grammar nlr_g;
    remove_left_recursion( nlr_g, g );
    token_vector input = {LPAREN,ID,TIMES,ID,PLUS,LPAREN,ID,TIMES,ID,RPAREN,RPAREN};
    Tree root(p_exp);
    int nf=0;
    bool parsed = parse(&root, &nf, nlr_g, input);
    if( parsed ) {
        root.print(std::cout);
    }
}
