# a bad sat solver
在BCP上做2-literal watching去檢查clause，  
遇到矛盾時做FirstUIP製造新clause並且non-chronological backtracking，  
利用VSIDS去挑下一個給值的變數。我覺得蠻有趣的一個實作，  
SAT solver只能解到1000個以內的變數，效能有待加強。
