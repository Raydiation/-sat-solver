# sat solver
literal : in clause, with sign.  
variable : without sign.  

* DPLL
  1. Pick a unassigned variable by VSIDS value, assigned it a value. Decay the VSIDS value.
  2. Do BCP.
  3. If BCP return -1, then UNSAT.
  4. Return SAT if all clauses are satisfy(only need to check the literal we watching). Otherwise repeat 1 with depth+1.
* BCP
  1. Pick a variable we'll decide/deduce.
  2. As this variable assigned a value, consider those clauses which contain this literal and got a value 0, discuss with four case.
  3. If case 2, then we deduce a new variable. If case 4, do FirstUIP with the conflict clause and return.
  4. While we have some deduced variable repeat 1.
* four_case
  * Since we use 2-literal watching, when a literal we watched be assigned and get value 0, then in this clause there are 4 case : 
    1. There is a literal which is not value 0 and not be watched, then we watch it. (case 1)
    2. All other literals are assigned and value 0 except another literal we're watching, then we deduce what value it'll get. (case 2)
    3. All other literals are assigned and value 0, and another literal we're watching has value 1, then this clause has value 1, nothing to do. (case 3)
    4. All other literals are assigned and value 0, and another literal we're watching has value 0, then all literals in this clause are value 0, conflict. (case 4)
* FirstUIP
  1. While the conflict clause contain one more variable which assigned value in current depth, resolve(conflict,antecedent[variable]).Then it's a new clause we'll learned.
  2. Add new clause into our clauses.
* add_new_clause
  1. If new clause has only one literal, the add it into our database(single literal), we need to fixed it's value.
  2. Otherwise we find the second deepest depth in all assigned literal in this new clause, and backtracking to the depth(non-chronological backtracking).
* resolve_clauses
  1. (x'+F)(x+G)==(resolve on x)==> (F+G)
* tidy_up_clause
  1. Sort literal by absolute value
  2. If it contain x and x' simultaneously, then this clause always true, delete it.
  3. Delete the repeat literal. 
* VSIDS
  * Count appearance frequency of each literal as it's vsids value.
* antecedent
  * Record what clause deduce the variable, so the variables we decide do not have antecedent.

Two mechanism :
  1. VSIDS value decay : Every 50 times conflicts we encounter, devide all VSIDS value by a decay constant.
  2. Restart : Every 200 times conflicts we encounter, restart.

The benchmarks are this code can pass, the code still has some bugs(?).  
Some idea:
* We can elimate those fixed variable, right? If we fixed x=1, then all clauses contain x can be deleted(always true), and all clauses contains x' can takeout this literal.  
* How to deal with large clauses?(containing k more literal) Ignore it? 
