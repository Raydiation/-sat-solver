#include<bits/stdc++.h>
#include "parser.cpp"
using namespace std;

bool compare(int a,int b)
{
    if(abs(a)==abs(b))return a<b;
    return abs(a)<abs(b);
}

struct Assigned
{
    int value;//0 or 1
    int depth;//in which layer
    int order;
};

//literal have sign, variable does not.
class sat_solver
{
public:
    sat_solver(vector<vector<int>> Clauses,int Maxindex,double Decay_constant,int Max_clause_size)
    {
        clauses = Clauses;
        maxindex = Maxindex;
        find_clauses.clear();
        positive_watch = vector<vector<int>> (maxindex+5);
        negative_watch = vector<vector<int>> (maxindex+5);
        two_literal_watching = vector<vector<int>> (clauses.size());
        antecedent = vector<vector<int>> (maxindex+5);
        assignment = vector<Assigned> (maxindex+5,{-1,-1,-1});
        single_literal.clear();
        vsids_value = vector<double> (maxindex*2+5,0);
        assigned_number = 0;
        decay_constant = Decay_constant;
        max_clause_size=Max_clause_size;

    }
    int valued(int);
    void vsids_value_decay();
    int vsids_pick();
    vector<int> tidy_up_clause(vector<int>);
    vector<int> resolve_clauses(vector<int>,vector<int>,int);
    bool prepare();
    int FirstUIP(vector<int>,int);
    int add_new_clause(vector<int>);
    void backtrack(int);
    int four_case(int,int);
    int BCP(int,int);
    int DPLL(int,int);
    bool check_solve();

    map<vector<int>,int> find_clauses;
    vector<vector<int>> clauses;
    vector<vector<int>> positive_watch;
    vector<vector<int>> negative_watch;
    vector<vector<int>> two_literal_watching; //store the index of watching literal
    vector<vector<int>> antecedent;//what clause imply the variable
    vector<Assigned> assignment;
    vector<int> single_literal;
    vector<double> vsids_value;

    int maxindex;
    int assigned_number;

    double decay_constant;
    int max_clause_size;
};

int sat_solver::valued(int literal)//OK
{
    if(assignment[abs(literal)].value==-1)return -1;
    if(assignment[abs(literal)].value==0&&literal<0)return 1;
    if(assignment[abs(literal)].value==1&&literal>0)return 1;
    return 0;
}

void sat_solver::vsids_value_decay()//OK
{
    for(int i=1;i<=maxindex*2;i++)
        vsids_value[i]/=decay_constant;
}

int sat_solver::vsids_pick()//OK
{
    int candidate_variable=0;
    for(int i=1;i<=maxindex*2;i++)
    {
        if(assignment[(i+1)/2].value==-1) //unassigned
        {
            if(vsids_value[i]>vsids_value[candidate_variable])
                candidate_variable=i;
        }
    }
    if(candidate_variable%2) return (candidate_variable+1)/2;
    return -(candidate_variable+1)/2;
}

vector<int> sat_solver::tidy_up_clause(vector<int> clause)//OK
{
    vector<int> simply_clause;
    sort(clause.begin(),clause.end(),compare);
    simply_clause.push_back(clause[0]);
    for(int num:clause)
    {
        if(num==simply_clause.back())continue;// repeated
        if(num==-simply_clause.back()) return vector<int>();// always true
        simply_clause.push_back(num);
    }
    sort(simply_clause.begin(),simply_clause.end(),compare);
    return simply_clause;
}

vector<int> sat_solver::resolve_clauses(vector<int> clause_1,vector<int> clause_2,int resolve_variable)//OK
{
    vector<int> new_clause;
    for(int num:clause_1)
        if(abs(num)!=resolve_variable)
            new_clause.push_back(num);
    for(int num:clause_2)
        if(abs(num)!=resolve_variable)
            new_clause.push_back(num);

    return tidy_up_clause(new_clause);
}

bool sat_solver::prepare()//OK
{
    // tidy up each clause
    for(int i=clauses.size()-1;i>=0;i--)
    {
        clauses[i]=tidy_up_clause(clauses[i]);
        if(clauses[i].empty())
            clauses.erase(clauses.begin()+i);
        else if(clauses[i].size()==1)
        {
            if(valued(clauses[i][0])==0)//conflict
                return 0;
            else //fix the value of this variable
            {
                single_literal.push_back(clauses[i][0]);
            }
            clauses.erase(clauses.begin()+i);
        }
    }
    //set watching, each clause has at least two literal
    for(int i=0;i<clauses.size();i++)
    {
        vector<int> clause = clauses[i];
        find_clauses[clause] = 1;
        for(int j=0;j<2;j++)
        {
            two_literal_watching[i].push_back(j);
            if(clause[j]<0) negative_watch[abs(clause[j])].push_back(i);
            if(clause[j]>0) positive_watch[abs(clause[j])].push_back(i);
        }
    }
    //update VSIDS value
    for(vector<int> clause:clauses)
        for(int literal:clause)
            (literal>0)?vsids_value[abs(literal)*2-1]+=1:vsids_value[abs(literal)*2]+=1;
    return 1;
}

int sat_solver::FirstUIP(vector<int> conflict_clause,int current_depth)
{
    /*cout<<"FirstUIP   :";
    for(int num:conflict_clause)cout<<num<<" ";
    cout<<"\n\n";*/
    while(1)//page 70
    {
        int current_depth_assigned = 0;
        for(int literal:conflict_clause)
            if(assignment[abs(literal)].value!=-1&&assignment[abs(literal)].depth==current_depth)
                current_depth_assigned++;

        if(current_depth_assigned<=1)
            break;

        int last_assigned=abs(conflict_clause[0]);
        for(int i=0;i<conflict_clause.size();i++)
        {
            if(assignment[abs(conflict_clause[i])].order>assignment[last_assigned].order)
                last_assigned=abs(conflict_clause[i]);
        }
        conflict_clause = resolve_clauses(conflict_clause,antecedent[last_assigned],last_assigned);
    }
    if(conflict_clause.empty())return -1;//conflict (?)

    return add_new_clause(conflict_clause);
}

int sat_solver::add_new_clause(vector<int> new_clause)
{
    if(new_clause.size()==1)//imply a fixed variable
    {
        backtrack(0);
        single_literal.push_back(new_clause[0]);
        return 0;//backtrack to initial
    }
    /*if(new_clause.size()>max_clause_size)
    {
        //this clause is too large.
    }*/
    //find second deepest and backtrack
    int deepest=0;
    int second_deepest=0;
    for(int num:new_clause)
        deepest = max(deepest,assignment[abs(num)].depth);
    for(int num:new_clause)
        if(assignment[abs(num)].depth!=deepest&&assignment[abs(num)].depth>second_deepest)
            second_deepest=assignment[abs(num)].depth;
    backtrack(second_deepest);
    //add new clause
    sort(new_clause.begin(),new_clause.end(),compare);
    if(!find_clauses[new_clause])
    {
        find_clauses[new_clause]=1;
        clauses.push_back(new_clause);
        two_literal_watching.push_back(vector<int>());
        for(int i=0,watching_count=0;i<new_clause.size(),watching_count<2;i++)
        {
            if(valued(new_clause[i])!=0)
            {
                two_literal_watching[clauses.size()-1].push_back(i);
                if(new_clause[i]<0)
                    negative_watch[abs(new_clause[i])].push_back(clauses.size()-1);
                else
                    positive_watch[abs(new_clause[i])].push_back(clauses.size()-1);
                watching_count++;
            }
        }
    }
    return second_deepest-1;
}

void sat_solver::backtrack(int goal_layer)//OK?
{
    //cout<<"backtrack to "<<goal_layer<<"\n\n";
    for(int num=1;num<=maxindex;num++)
    {
        if(assignment[num].depth>=goal_layer)// >?>=?
        {
            assigned_number--;
            assignment[num]={-1,-1,-1};
            antecedent[num]=vector<int>();
        }
    }
}

int sat_solver::four_case(int clause_index,int assigned_literal)
{
    vector<int> current_clause = clauses[clause_index];
    int watching_index;// 0 or 1 in 2-literal watching
    for(int i=0;i<2;i++)
    {
        if(current_clause[two_literal_watching[clause_index][i]]==assigned_literal)
            watching_index=i;
    }
    int another_watching_index = 1-watching_index;
    //int done_variable = current_clause[two_literal_watching[current_clause][watching_index]];
    for(int i=0;i<current_clause.size();i++)
    {
        if(i==two_literal_watching[clause_index][0]||i==two_literal_watching[clause_index][1])continue;
        if(valued(current_clause[i])==0)continue;
        //find case 1 literal
        two_literal_watching[clause_index][watching_index] = i;//update watching
        if(current_clause[i] < 0)
            negative_watch[abs(current_clause[i])].push_back(clause_index);
        else
            positive_watch[abs(current_clause[i])].push_back(clause_index);
        return 1;// case 1 return
    }
    if(valued(current_clause[two_literal_watching[clause_index][another_watching_index]])==-1) //case 2
    {
        // imply;
        return 2;
    }
    if(valued(current_clause[two_literal_watching[clause_index][another_watching_index]])==1) //case 3
    {
        //nothing to do
        return 3;
    }
    return 4;//conflict
}

int sat_solver::BCP(int depth,int decide_literal)
{
    //cout<<"BCP, current depth and literal : "<<depth<<"   "<<decide_literal<<"\n\n";
    queue<pair<int,vector<int>>> imply;
    if(depth>1)
    {
        imply.push(pair<int,vector<int>>(decide_literal,vector<int>()));
    }
    else if(depth==1)
    {
        for(int num:single_literal)
        {
            imply.push(pair<int,vector<int>>(num,vector<int>()));
        }
    }
    while(!imply.empty())
    {
        pair<int,vector<int>> next_imply = imply.front();imply.pop();
        int literal = next_imply.first;
        int variable = abs(literal);
        if(valued(literal)==1)continue;// repeat the same imply
        if(valued(literal)==0)//conflict
        {
            if(depth==1)return -1;
            antecedent[variable]=next_imply.second;
            return FirstUIP(antecedent[variable],depth);
        }
        assignment[variable].value = (literal>0)?1:0;
        assignment[variable].depth = depth;
        assignment[variable].order = assigned_number;
        assigned_number++;
        antecedent[variable]=next_imply.second;//this clause imply this variable

        if(assignment[variable].value==1)
        {
            for(int i=negative_watch[variable].size()-1;i>=0;i--)
            {
                int kase = four_case(negative_watch[variable][i],-variable);
                if(kase == 1)
                {
                    negative_watch[variable].erase(negative_watch[variable].begin()+i);//remove this watching clause.
                    continue;
                }
                if(kase == 2)
                {
                    int clause_index = negative_watch[variable][i];
                    int imply_literal;
                    for(int j=0;j<2;j++)
                    {
                        if(abs(clauses[clause_index][two_literal_watching[clause_index][j]])!=variable)
                           imply_literal = clauses[clause_index][two_literal_watching[clause_index][j]];
                    }
                    imply.push({imply_literal,clauses[clause_index]});
                }
                if(kase == 3)//this clause is resolved.
                {
                    continue;
                }
                if(kase == 4)//conflict
                {
                    if(depth==1)return -1;
                    return FirstUIP(clauses[negative_watch[variable][i]],depth);
                }
            }
        }
        else
        {
            for(int i=positive_watch[variable].size()-1;i>=0;i--)
            {
                int kase = four_case(positive_watch[variable][i],variable);
                if(kase == 1)
                {
                    positive_watch[variable].erase(positive_watch[variable].begin()+i);//remove this watching clause.
                    continue;
                }
                if(kase == 2)
                {
                    int clause_index = positive_watch[variable][i];
                    int imply_literal;
                    for(int j=0;j<2;j++)
                    {
                        if(abs(clauses[clause_index][two_literal_watching[clause_index][j]])!=variable)
                           imply_literal = clauses[clause_index][two_literal_watching[clause_index][j]];
                    }
                    imply.push({imply_literal,clauses[clause_index]});
                }
                if(kase == 3)
                {
                    continue;
                }
                if(kase == 4)//conflict
                {
                    if(depth==1)return -1;
                    return FirstUIP(clauses[positive_watch[variable][i]],depth);
                }
            }
        }
    }
    return depth;
}

int sat_solver::DPLL(int current_depth,int decide_literal)
{
    current_depth = BCP(current_depth,decide_literal);
    /*for(int i=1;i<=maxindex;i++)
    {
        cout<<"\n----------\n";
        cout<<assignment[i].value<<"  |  "<<assignment[i].order<<"  |  "<<assignment[i].depth;
    }*/
    if(current_depth<0)return 0;
    if(check_solve())return 1;

    int next_decide = vsids_pick();
    vsids_value_decay();
    return DPLL(current_depth+1,next_decide);
}

bool sat_solver::check_solve()//OK
{
    bool ans = 1;
    for(int i=0;i<clauses.size();i++)
        ans=ans&((valued(clauses[i][two_literal_watching[i][0]])==1)||(valued(clauses[i][two_literal_watching[i][1]])==1));

    return ans;
}

int main()
{

    vector<vector<int>> clauses;
    int maxindex;
    parse_DIMACS_CNF(clauses,maxindex,"par32-1.cnf");

    sat_solver solver(clauses,maxindex,1.2,20);
    if(!solver.prepare())cout<<"UNSAT\n";
    else
    {
        if(solver.DPLL(1,0))
        {
            cout<<"SAT\n";
            for(int i=1;i<=maxindex;i++)
            {
                if(solver.assignment[i].value==0)cout<<"-";
                cout<<i<<" ";
            }
        }
        else cout<<"UNSAT\n";
    }

	return 0;
}
