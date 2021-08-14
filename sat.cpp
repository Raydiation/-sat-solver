#include<bits/stdc++.h>
#include "parser.h"
using namespace std;

const int maxn = 2e5;
const int maxcla = 1e6;
const double decay_con = 1.2;
const double pick_decay = 1;
const int SD_size = 100;

struct Decide { int var, state, depth; };

vector<vector<int> >clauses;
vector<int> pos_watch[maxn];
vector<int> neg_watch[maxn];
vector<Decide> decide;
int maxIdx;
int Cnt;
int res[maxn];//-1 is undecided,assigned 0 or 1
int dep[maxn];//The depth of i be decided.
int watching[maxcla][2];//The watched index for each clauses
int antecedent[maxn];
int d_o[maxn];//decision order;
double vs[maxn * 2];//+-+-+-+-+-....

bool cmp(int a, int b) { return abs(a) < abs(b); }
bool cmp2(vector<int> a, vector<int> b) { return a.size() < b.size(); }

int Val(int lit)
{
    if (res[abs(lit)] == -1)return -1;
    if ((res[abs(lit)] == 0 && lit < 0) || (res[abs(lit)] == 1 && lit > 0))return 1;
    return 0;
}
void vs_decay()
{
    for (int i = 1; i <= maxIdx * 2; i++)vs[i] /= decay_con;
    return;
}
int vsids()
{
    double _max = -1;
    int var = 0;
    for (int i = 1; i <= maxIdx; i++)
    {
        if (res[i] != -1)continue;
        else
            for (int j = 0; j < 2; j++)
                if (vs[i * 2 - j] - _max > 1e-9)
                {
                    _max = vs[i * 2 - j];
                    j == 0 ? var = -i : var = i;
                }
    }
    var < 0 ? vs[abs(var) * 2] /= pick_decay : vs[abs(var) * 2 - 1] /= pick_decay;
    return var;
}
void init()
{
    decide.clear();
    clauses.clear();
    for (int i = 0; i < maxn; i++)
    {
        pos_watch[i].clear();
        neg_watch[i].clear();
    }

    memset(res, -1, sizeof(res));
    memset(dep, 0, sizeof(dep));
    memset(watching, -1, sizeof(watching));
    memset(antecedent, -1, sizeof(antecedent));
    memset(d_o, -1, sizeof(d_o));
    memset(vs, 0, sizeof(vs));
    Cnt = 0;
    return;
}

vector<int> presolve(vector<int> c_1)
{
    vector<int> c_p, mt;
    mt.clear();
    sort(c_1.begin(), c_1.end(), cmp);

    c_p.push_back(c_1[0]);
    for (int i : c_1)
    {
        if (i == c_p.back())continue;
        if (i == -c_p.back())return mt;
        c_p.push_back(i);
    }
    return c_p;
}

int prepare()
{
    vector<int> pre_assigned;
    for (int i = clauses.size() - 1; i >= 0; i--)
    {
        clauses[i] = presolve(clauses[i]);
        if (clauses[i].size() == 0) clauses.erase(clauses.begin() + i);
    }
    sort(clauses.begin(), clauses.end(), cmp2);
    while (clauses[0].size() == 1)
    {
        vector<int> tmp = clauses[0];
        if (Val(tmp[0]) == 0)return 0;
        else
        {
            if (Val(tmp[0]) == -1)
            {
                pre_assigned.push_back(tmp[0]);
                tmp[0] < 0 ? res[abs(tmp[0])] = 0 : res[abs(tmp[0])] = 1;
            }
            clauses.erase(clauses.begin() + 0);
        }
    }
    for (int pre : pre_assigned)
        pre < 0 ? decide.push_back({ abs(pre),0,1 }) : decide.push_back({ abs(pre),1,1 });

    for (int i = 0; i < clauses.size(); i++)
        for (int j = 0; j < 2; j++)
        {
            watching[i][j] = j;
            clauses[i][j] < 0 ? neg_watch[abs(clauses[i][j])].push_back(i) : pos_watch[abs(clauses[i][j])].push_back(i);
        }
    for (vector<int> C : clauses)
        for (int lit : C)
        {
            if (lit < 0)vs[abs(lit) * 2] += 1;
            else vs[abs(lit) * 2 - 1] += 1;
        }

    return 1;
}
int check_sol()
{
    int ans = 1;
    for (int i = 0; i < clauses.size() && ans == 1; i++)
        ans = ans & ((Val(clauses[i][watching[i][0]]) == 1) || (Val(clauses[i][watching[i][1]])) == 1);
    return ans;
}
vector<int> resolve(vector<int> c_1, vector<int> c_2)
{
    vector<int> c, c_p;
    for (int i : c_1)c.push_back(i);
    for (int i : c_2)c.push_back(i);
    sort(c.begin(), c.end(), cmp);

    int takeout = 0;
    c_p.push_back(c[0]);
    for (int i : c)
    {
        if (i == c_p.back())continue;
        if (i == -c_p.back())
        {
            takeout = 1;
            continue;
        }
        if (takeout)c_p.pop_back();
        takeout = 0;
        c_p.push_back(i);
    }
    if (takeout)c_p.pop_back();
    return c_p;
}

int compare(vector<int> C1, vector<int> C2)
{
    if (C1.size() != C2.size())return 0;
    sort(C1.begin(), C1.end());
    sort(C2.begin(), C2.end());
    for (int i = 0; i < C1.size(); i++)
        if (C1[i] != C2[i])
            return 0;
    return 1;
}

int Addcla(vector<int> C, int depth)
{
    C = presolve(C);
    if (C.size() == 0)return 0;
    if (C.size() == 1)
    {
        if (Val(C[0]) == 0 && dep[C[0]] == 1)return 0;
        if (Val(C[0]) == 1)return 1;
        C[0] < 0 ? decide.push_back({ abs(C[0]),0,1 }) : decide.push_back({ abs(C[0]),1,1 });
        return 1;
    }

    for (vector<int> C_p : clauses)
        if (compare(C_p, C))return depth;

    vs_decay();
    for (int lit : C)
        lit < 0 ? vs[abs(lit) * 2] += 1 : vs[abs(lit) * 2 - 1] += 1;

    if (C.size() > SD_size)return depth;

    clauses.push_back(C);
    int idx = clauses.size() - 1;
    int lit, w_cnt = 0;
    for (int i = 0; i < clauses[idx].size() && w_cnt < 2; i++)
    {
        lit = clauses[idx][i];
        if (Val(lit) == 0 && dep[abs(lit)] < depth)continue;//Val==0 before the depth we'll backtrack
        watching[idx][w_cnt] = i;
        lit < 0 ? neg_watch[abs(lit)].push_back(idx) : pos_watch[abs(lit)].push_back(idx);
        w_cnt++;
    }

    return depth;
}

int FirstUIP(vector<int> conf, int depth)
{
    int cnt, l_idx, l_lit;
    while (1)
    {
        cnt = 0;
        for (int lit : conf)
            if (res[abs(lit)] != -1 && dep[abs(lit)] == depth)
                cnt++;
        if (cnt <= 1)break;
        l_lit = -1; l_idx = -1;
        for (int lit : conf)
            if (d_o[abs(lit)] > l_idx)
            {
                l_idx = d_o[abs(lit)];
                l_lit = lit;
            }
        conf = resolve(conf, clauses[antecedent[abs(l_lit)]]);
    }
    if (conf.size() == 0)return 0;
    l_lit = -1; l_idx = -1;
    for (int lit : conf)
        if (res[abs(lit)] != -1 && d_o[abs(lit)] > l_idx && dep[abs(lit)] != depth)
        {
            l_idx = d_o[abs(lit)];
            l_lit = lit;
        }
    return Addcla(conf, dep[abs(l_lit)]);//back to dep[l_lit]
}

int four_case(int Idx, int c_idx, int d_idx, vector<Decide>& nxt_dedu)
{
    int lit = clauses[c_idx][d_idx];
    int d_val = res[abs(lit)];
    int ano_idx, w_idx;
    (d_idx == watching[c_idx][0]) ? w_idx = 0 : w_idx = 1;
    ano_idx = watching[c_idx][1 - w_idx];
    for (int k = 0; k < clauses[c_idx].size(); k++)
    {
        if (k == d_idx || k == ano_idx)continue;
        if (Val(clauses[c_idx][k]) == 0)continue;
        if (!d_val)
            pos_watch[abs(lit)].erase(pos_watch[abs(lit)].begin() + Idx);
        else
            neg_watch[abs(lit)].erase(neg_watch[abs(lit)].begin() + Idx);

        lit = clauses[c_idx][k];
        watching[c_idx][w_idx] = k;
        lit < 0 ? neg_watch[abs(lit)].push_back(c_idx) : pos_watch[abs(lit)].push_back(c_idx);
        return 1;//case 1
    }
    lit = clauses[c_idx][ano_idx];
    if (Val(lit) == -1)//case 2
    {
        lit < 0 ? nxt_dedu.push_back({ abs(lit),0,c_idx }) : nxt_dedu.push_back({ abs(lit),1,c_idx });
        return 1;
    }
    if (Val(lit) == 1) return 1;//case 3
    return 0;
}

void backtrack(int depth)
{
    depth = max(depth, 1);
    Decide tp;
    for (int i = decide.size() - 1; i >= 0; i--)
    {
        tp = decide[i];
        if (tp.depth > depth)
        {
            res[tp.var] = -1;
            dep[tp.var] = 0;
            d_o[tp.var] = -1;
            antecedent[tp.var] = -1;
            decide.erase(decide.begin() + i);
        }
    }
    return;
}

int DPLL()
{
    vector<Decide> imply, nxt_dedu;
    int bk;
    int idx;
    int kase;
    int depth = 0;
    int cnt = 0;
    while (1)
    {
        cnt++;
        //if(cnt%2000==0)cout<<cnt<<endl;
        int conf = 0;
        depth++;
        if (depth == 1)
        {
            Cnt = 0;//restart
            for (Decide i : decide)
            {
                imply.push_back(i);
                res[i.var] = i.state;
                d_o[abs(i.var)] = Cnt++;
                dep[abs(i.var)] = 1;
                antecedent[abs(i.var)] = -1;
            }
        }
        else
        {
            int p = vsids();
            if (p == 0)return 0;
            p < 0 ? imply.push_back({ abs(p),0,-1 }) : imply.push_back({ abs(p),1,-1 });
        }
        while (!imply.empty())
        {
            conf = 0;
            for (Decide i : imply)
            {
                int imp = i.var;
                res[imp] = i.state;
                if (!dep[imp])
                {
                    d_o[imp] = Cnt++;
                    dep[imp] = depth;
                    antecedent[imp] = i.depth;
                    decide.push_back({ i.var,i.state,depth });
                }
                if (!res[imp])
                {
                    for (int Idx = pos_watch[imp].size() - 1; Idx >= 0; Idx--)
                    {
                        idx = pos_watch[imp][Idx];
                        if (imp == abs(clauses[idx][watching[idx][0]]))
                            kase = four_case(Idx, idx, watching[idx][0], nxt_dedu);
                        else
                            kase = four_case(Idx, idx, watching[idx][1], nxt_dedu);
                        if (kase)continue;
                        else
                        {
                            if (depth == 1) return 0;
                            conf = 1;
                            bk = FirstUIP(clauses[idx], depth);
                            if (bk <= 0)return 0;
                            break;
                        }
                    }
                    if (conf)break;
                }
                else
                {
                    for (int Idx = neg_watch[imp].size() - 1; Idx >= 0; Idx--)
                    {
                        idx = neg_watch[imp][Idx];
                        if (imp == abs(clauses[idx][watching[idx][0]]))
                            kase = four_case(Idx, idx, watching[idx][0], nxt_dedu);
                        else
                            kase = four_case(Idx, idx, watching[idx][1], nxt_dedu);
                        if (kase)continue;
                        else
                        {
                            if (depth == 1) return 0;
                            conf = 1;
                            bk = FirstUIP(clauses[idx], depth);
                            if (bk <= 0)return 0;
                            break;
                        }
                    }
                    if (conf)break;
                }
            }
            if (conf)break;
            imply.clear();
            for (Decide i : nxt_dedu)
            {
                int rep = 0;
                for (Decide j : imply)
                {
                    if (i.var == j.var)
                    {
                        rep = 1;
                        break;
                    }
                }
                if (!rep)imply.push_back(i);
            }
            nxt_dedu.clear();
        }
        if (check_sol())return 1;
        if (conf)
        {
            if (depth - bk > 50)bk = 1;
            bk--;
            backtrack(bk);
            depth = bk;
            imply.clear();
            nxt_dedu.clear();
        }
    }
    return 1;
}

void check()
{
    int ans = 1;
    for (vector<int> C : clauses)
    {
        int ok = 0;
        for (int lit : C)
        {
            if (Val(lit) != 0)
            {
                ok = 1;
                break;
            }
        }
        ans = ans & ok;
    }
    if (ans)cout << "Check ok\n";
    else cout << "Fail\n";
    return;
}

int main(int argc, char* argv[])
{
    init();
    string filename = argv[1];
    parse_DIMACS_CNF(clauses, maxIdx, argv[1]);
    filename = filename.substr(0, filename.size() - 3);
    filename = filename + "sat";
    freopen(filename.c_str(), "w", stdout);
    cout << "s ";
    if (!prepare())cout << "UNSATISFIABLE\n";
    else
    {
        int ans = DPLL();
        if (ans)
        {
            cout << "SATISFIABLE\n";
            cout << "v ";
            for (int i = 1; i <= maxIdx; i++)
            {
                if (res[i] == 0)cout << "-";
                cout << i << " ";
            }
            cout << endl;
        }
        else cout << "UNSATISFIABLE\n";
    }
    fclose(stdout);

    return 0;
}
