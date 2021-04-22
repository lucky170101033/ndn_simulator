import os
from collections import defaultdict

cur_dir = os.getcwd()
strategy = ("LCE", "LCD" , "CUSTOM" , "EDGE")
number_of_content = (10, 30, 50, 100 ,150 ,200, 250)
fre = (20,)

edge_node = ("Node1000", "Node1001", "Node1010","Node1011",
      "Node2000", "Node2001", "Node2010","Node2011",
      "Node3000", "Node3001", "Node3010","Node3011",
      "Node4000", "Node4001", "Node4010","Node4011",  
 )

for st in strategy:
    for f in fre:
        for n in number_of_content:
            cache_file_name = "custom_" + str(st) + "_" + str(f) + "_" + str(n) + ".txt"
            edge_cache_dict = defaultdict(float)
            rem_cache_dict = defaultdict(float)
            file_location = os.path.join(cur_dir,str(st),cache_file_name)
            with open(file_location) as fp:
                text = fp.read()
                line_text = text.split('\n')
                for l in line_text:
                    temp = l.split()
                    if len(temp) > 3 or len(temp) == 0:
                        continue
                    else:
                        if temp[0][:4] == "Node" :
                            if temp[0] in edge_node:
                                edge_cache_dict[temp[0]] = float(temp[1]) / (float(temp[1])+float(temp[2]))
                            else:
                                rem_cache_dict[temp[0]] = float(temp[1]) / (float(temp[1])+float(temp[2]))
            
            s = float(0)
            # for k,v in edge_cache_dict.items():
            #     s += v
            # print("({},{})".format( n, round(s/16,6) ) )
            # s = 0.0
            for k,v in rem_cache_dict.items():
                s += v
            print("({},{})".format( n, round(s/16,6) ) )