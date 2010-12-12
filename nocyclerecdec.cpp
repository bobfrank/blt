#include <vector>
#include <unordered_map>
#include <stack>
#include <iostream>

// now converting it to an acyclic recursive descent parser
typedef enum {PLUS,TIMES,LPAREN,RPAREN,ID,p_exp,p_exp_a,p_times,p_times_a,p_fact} Token;
const char*token_names[] = {"PLUS","TIMES","LPAREN","RPAREN","ID","exp","exp'","times","times'","fact" };

struct token_hash {
    int operator()(Token token) const {
        return static_cast<int>(token);
    }
};
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
        if( !m_children.empty() ) {
            os << "[" << token_names[m_token] << std::endl;
            for( int i = 0; i < m_children.size(); ++i ) {
                print_indent(os,indent+1);
                m_children[i].print(os, indent+1);
            }
            print_indent(os,indent);
            os << "]" << std::endl;
        }
        else {
            os << token_names[m_token] << std::endl;
        }
    }
    Token m_token;
    std::vector<Tree> m_children;
};


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
    //exp     : times exp_a
    //exp_a   : PLUS exp exp_a
    //        |
    //times   : fact times_a
    //times_a : TIMES fact times_a
    //        |
    //fact    : LPAREN exp RPAREN
    //        | ID
    grammar g = {
        {p_exp,     {p_times, p_exp_a}},
        {p_exp_a,   {PLUS, p_exp, p_exp_a}},
        {p_exp_a,   {}},
        {p_times,   {p_fact, p_times_a}},
        {p_times_a, {TIMES,p_fact,p_times_a}},
        {p_times_a, {}},
        {p_fact,    {LPAREN,p_exp,RPAREN}},
        {p_fact,    {ID}}
    };
    token_vector input = {LPAREN,ID,TIMES,ID,PLUS,LPAREN,ID,TIMES,ID,RPAREN,RPAREN};
    Tree root(p_exp);
    int nf=0;
    bool parsed = parse(&root, &nf, g, input);
    if( !parsed && input.size() > nf ) {
        std::cout << "expected end of file, found "
                  << token_names[input[nf]] << std::endl;
        throw SyntaxError();
    }
    if( parsed ) {
        root.print(std::cout);
    }
}
