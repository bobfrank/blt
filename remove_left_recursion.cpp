#include <vector>
#include <unordered_map>
#include <stack>
#include <iostream>

typedef enum {PLUS,TIMES,LPAREN,RPAREN,ID,p_exp,p_exp_a,p_times,p_times_a,p_fact} Token;

typedef std::vector<Token> token_vector;
typedef std::unordered_multimap<Token,token_vector, token_hash> grammar;

void remove_left_recursion( grammar& new_grammar,
                            const grammar& grammar )
{
}

int main
{
}
