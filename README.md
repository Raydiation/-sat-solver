# sat solver
Do BCP with using 2-literal watching to check each clauses.    
When occuring conflict,do FirstUIP to generate a new clause and non-chronological backtracking.    
Pick next variable and give it a value by using VSIDS.  
I think it's interesting, it can be reinforce step by step. But my SAT solver can only solve small problems(less than about 1000 variable), the efficient should be reinforce.   
