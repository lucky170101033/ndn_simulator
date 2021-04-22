import os
from collections import defaultdict

cur_dir = os.getcwd()
strategy = ("LCE", "LCD" , "CUSTOM" , "EDGE")
number_of_content = (10, 30, 50)
fre = (10,)

for st in strategy:
    for f in fre:
        for n in number_of_content:
            app_file_name = "app-delays-trace-" + str(st) + "-" + str(f) + "-" + str(n) + ".txt"
            app_dict = defaultdict(int)
            counter = defaultdict(int)
            file_location = os.path.join(cur_dir,str(st),app_file_name)
            with open(file_location) as fp:
                text = fp.read()
                line_text = text.split('\n')
                for l in line_text:
                    if len(l) < 4:
                        continue
                    else:
                        lt = l.split('\t')
                        if not lt[-1] == "HopCount":
                            app_dict[lt[1]] += int(lt[-1])
                            counter[lt[1]] += 1
            
            s = float(0)
            for k,v in app_dict.items():
                s += (v/counter[k])
            print(n,s/48)
