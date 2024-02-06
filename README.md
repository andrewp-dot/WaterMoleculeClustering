## Project for clustering water molecules

The project is aimed for process synchronization using semaphors.

### Usage: 
<code>./proj2 [OXYGEN AMOUNT] [HYDROGEN AMOUNT] [ATOM MAX WAIT TIME TO JOIN QUEUE (0-1000 ms)] [CREATE MOLECULE MAX WAIT TIME (0-1000 ms)]<code>

### Output example:
<code>
1: H 1: started
2: H 2: started
3: H 3: started
4: H 4: started
5: H 5: started
6: H 6: started
7: H 7: started
8: O 1: started
9: O 2: started
10: O 3: started
11: O 4: started
12: O 5: started
13: H 1: going to queue
14: H 6: going to queue
15: H 3: going to queue
16: H 2: going to queue
17: H 5: going to queue
18: H 4: going to queue
19: H 7: going to queue
20: O 2: going to queue 
21: O 4: going to queue 
22: H 1: creating molecule 1
23: O 2: creating molecule 1 
24: H 6: creating molecule 1
25: O 1: going to queue 
26: O 3: going to queue 
27: O 5: going to queue 
28: O 2: molecule 1 created
29: H 1: molecule 1 created
30: H 6: molecule 1 created
31: O 4: creating molecule 2 
32: H 3: creating molecule 2
33: H 2: creating molecule 2
34: O 4: molecule 2 created
35: H 3: molecule 2 created
36: H 2: molecule 2 created
37: O 1: creating molecule 3 
38: H 5: creating molecule 3
39: H 4: creating molecule 3
40: O 1: molecule 3 created
41: H 5: molecule 3 created
42: H 4: molecule 3 created
43: O 3: Not enough H
44: O 5: Not enough H
45: H 7: Not enough O or H
</code>
